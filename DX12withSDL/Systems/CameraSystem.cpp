#include "pch.h"

#include "Universe.h"
#include "CameraSystem.h"
#include "LogUtils.h"

#include "../Components/TransformComponent.h"
#include "../Components/CameraComponent.h"
#include "../Components/RendererIdComponent.h"

namespace GAL
{
    static const XMVECTOR s_smallVector = XMVectorSet(0.000001f, 0.000001f, 0.000001f, 0.000001f);

    CameraSystem::~CameraSystem()
    {
    }

    void CameraSystem::Initialize()
    {
        m_totalPitchDegrees = 0.0f;
        m_mouseMiddleButton = false;
        m_mouseRightButton = false;
        m_cameraInput = XMFLOAT4A(0, 0, 0, 0);

        m_activeCameraId = NullEntity;
        RenderEntityId activeRendCam = NullEntity;

        //Loop through all entities with cameras and add the cameras to the renderer.
        //We expect few entities so we will use standard view.
        Registry& registry = m_universe->GetRegistry();
        auto entitiesWithCam = registry.view<TransformComponent, CameraComponent>();
        for (auto camEntity : entitiesWithCam)
        {
            TransformComponent& transform = entitiesWithCam.get<TransformComponent>(camEntity);
            CameraComponent& camera = entitiesWithCam.get<CameraComponent>(camEntity);

            RenderEntityId rendererCameraEntity = m_renderer->AddCamera(camera.m_fieldOfViewDegrees,
                camera.m_nearClipDistance, camera.m_farClipDistance,
                XMLoadFloat4A(transform.GetPosition()), XMLoadFloat4A(transform.GetForward()), XMLoadFloat4A(transform.GetUp()));

            //Save the returned id.
            auto& ridComponent = registry.assign<RendererIdComponent>(camEntity);
            ridComponent.m_rendererEntityId = rendererCameraEntity;

            if (camera.m_isActive)
            {
                m_activeCameraId = camEntity;
                activeRendCam = rendererCameraEntity;
            }
        }

        if (m_activeCameraId != NullEntity)
        {
            assert(activeRendCam != NullEntity);
            m_renderer->SetActiveCamera(activeRendCam);
        }

        //Let's register for input events.
        m_universe->GetEventDispatcher().sink<InputComponent::InputEvent>().connect(this);
    }

    void CameraSystem::TickUpdate(Registry& registry, float deltaTimeMillis)
    {
        if (m_activeCameraId == NullEntity)
            return;

        TransformComponent& tc = registry.get<TransformComponent>(m_activeCameraId);
        RendererIdComponent& ric = registry.get<RendererIdComponent>(m_activeCameraId);
        CameraComponent& cic = registry.get<CameraComponent>(m_activeCameraId);

        // build view matrix
        XMVECTOR position = XMLoadFloat4A(tc.GetPosition());
        // Calculate how much to move in the current camera direction.
        XMVECTOR camForward = XMLoadFloat4A(tc.GetForward());
        XMVECTOR forwardDisplacement = XMVectorScale(camForward, m_cameraInput.z * cic.m_speed * (deltaTimeMillis * 0.001f));

        //XMVECTOR camUp = XMLoadFloat4A(tc.GetUp());//XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMVECTOR camRight = XMLoadFloat4A(tc.GetRight());//XMVector3Cross(camUp, camForward);

        //camRight = XMVector3Normalize(camRight);
        XMVECTOR rightDisplacement = XMVectorScale(camRight, m_cameraInput.x * cic.m_speed * (deltaTimeMillis * 0.001f));

        //camUp = XMVector3Cross(camForward, camRight);
        //camUp = XMVector3Normalize(camUp);
        XMVECTOR camUp = XMLoadFloat4A(tc.GetUp());
        XMVECTOR upDisplacement = XMVectorScale(camUp, m_cameraInput.y * cic.m_speed * (deltaTimeMillis * 0.001f));

        XMVECTOR deltaTotal = XMVectorAdd(forwardDisplacement, rightDisplacement);
        deltaTotal  = XMVectorAdd(deltaTotal, upDisplacement);
        if ( XMVector3NearEqual(XMVector3LengthSq(deltaTotal), XMVectorZero(), s_smallVector) )
        {
            return;
        }

        //Move the camera.
        position += deltaTotal;

        //Store the new position
        XMStoreFloat4A(tc.GetPosition(), position);

        //Notify the renderer
        m_renderer->UpdateCamera(ric.m_rendererEntityId, position, camForward, camUp);
    }

    void CameraSystem::receive(const InputComponent::InputEvent & evt)
    {
        //ODINFO("Got input event with hash=%x, name=%s, value=%f", evt.m_name.GetHash(), evt.m_name.GetString().c_str(), evt.m_value );
        StringHash evtHash = evt.m_name.GetHash();

        if (evtHash == entt::hashed_string("MoveForward"))
        {
            m_cameraInput.z = evt.m_value;
            return;
        }

        if (evtHash == entt::hashed_string("MoveRight"))
        {
            m_cameraInput.x = evt.m_value;
            return;
        }

        if (evtHash == entt::hashed_string("MoveUp"))
        {
            m_cameraInput.y = evt.m_value;
            return;
        }

        if (evtHash == entt::hashed_string("MouseMiddleButton"))
        {
            m_mouseMiddleButton = evt.m_value > 0.0f;
            return;
        }

        if (evtHash == entt::hashed_string("MouseRightButton"))
        {
            m_mouseRightButton = evt.m_value > 0.0f;
            return;
        }

        if (evtHash == entt::hashed_string("MouseDeltaX"))
        {
            //MouseDeltaX + MouseMiddleButton = MoveRight
            if (m_mouseMiddleButton)
            {

            }
            //MouseDeltaX + MouseRightButton = MoveYaw
            if (m_mouseRightButton)
            {
                UpdateCameraYaw(evt.m_value);
            }
            return;
        }

        if (evtHash == entt::hashed_string("MouseDeltaY"))
        {
            //MouseDeltaY + MouseMiddleButton = MoveUp
            if (m_mouseMiddleButton)
            {

            }
            //MouseDeltaY + MouseRightButton = MovePitch
            if (m_mouseRightButton)
            {
                UpdateCameraPitch(evt.m_value);
            }
            return;
        }

    }

    void CameraSystem::UpdateCameraYaw(float yawRads)
    {
        if (m_activeCameraId == NullEntity)
            return;

        //ODINFO("yaw = %f", yawRads);

        Registry& registry = m_universe->GetRegistry();
        TransformComponent& tc = registry.get<TransformComponent>(m_activeCameraId);
        RendererIdComponent& ric = registry.get<RendererIdComponent>(m_activeCameraId);
        CameraComponent& cic = registry.get<CameraComponent>(m_activeCameraId);


        XMVECTOR camForward = XMLoadFloat4A(tc.GetForward());

        XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMMATRIX matYaw = XMMatrixRotationAxis(camUp, yawRads);

        //transform forward
        camForward = XMVector3Transform(camForward,
            matYaw);
        camForward = XMVector3Normalize(camForward);

        //Recalculate Right
        XMVECTOR camRight = XMVector3Cross(camUp, camForward);
        camRight = XMVector3Normalize(camRight);

        //Save the new orientation vectors.
        XMStoreFloat4A(tc.GetRight(), camRight);
        XMStoreFloat4A(tc.GetForward(), camForward);

        XMVECTOR position = XMLoadFloat4A(tc.GetPosition());
        m_renderer->UpdateCamera(ric.m_rendererEntityId, position, camForward, camUp);
    }

    void CameraSystem::UpdateCameraPitch(float pitchRads)
    {
        if (m_activeCameraId == NullEntity)
            return;

        float totalPitchDegrees = m_totalPitchDegrees + pitchRads * (180.0f / 3.14159f);
        //ODINFO("delta pitch rads= %f, total pitch deg=%f", pitchRads, totalPitchDegrees);
        if (fabsf(totalPitchDegrees) > 70.0f)
        {
            return;
        }
        m_totalPitchDegrees = totalPitchDegrees;
          
        Registry& registry = m_universe->GetRegistry();
        TransformComponent& tc = registry.get<TransformComponent>(m_activeCameraId);
        RendererIdComponent& ric = registry.get<RendererIdComponent>(m_activeCameraId);
        CameraComponent& cic = registry.get<CameraComponent>(m_activeCameraId);


        XMVECTOR camForward = XMLoadFloat4A(tc.GetForward());
        XMVECTOR camRight = XMLoadFloat4A(tc.GetRight());
        XMMATRIX matPitch = XMMatrixRotationAxis(camRight, pitchRads);

        //transform forward vector
        camForward = XMVector3Transform(camForward,
            matPitch);
        camForward = XMVector3Normalize(camForward);

        //Recalculate Up
        XMVECTOR camUp = XMVector3Cross(camForward, camRight);
        camUp = XMVector3Normalize(camUp);

        //Save the new orientation vectors.
        XMStoreFloat4A(tc.GetUp(), camUp);
        XMStoreFloat4A(tc.GetForward(), camForward);

        //DirectX Math API requires always the up to be a perfect axis.
        camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMVECTOR position = XMLoadFloat4A(tc.GetPosition());
        m_renderer->UpdateCamera(ric.m_rendererEntityId, position, camForward, camUp);
    }

}; //namespace GAL
