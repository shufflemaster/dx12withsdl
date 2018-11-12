#include "pch.h"

#include "Universe.h"
#include "CameraSystem.h"
#include "LogUtils.h"

#include "../Components/TransformComponent.h"
#include "../Components/CameraComponent.h"
#include "../Components/RendererIdComponent.h"

namespace GAL
{
    static float kMaxPitchDegrees = 70.0f;

    CameraSystem::~CameraSystem()
    {
    }

    void CameraSystem::Initialize()
    {
        m_totalPitchDegrees = 0.0f;
        m_mouseMiddleButton = false;
        m_mouseRightButton = false;
        m_cameraTranslationInput = XMFLOAT4A(0, 0, 0, 0);
        m_cameraRotationInput = XMFLOAT4A(0, 0, 0, 0);

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

        bool doUpdate = false;
        float deltaTimeSecs = deltaTimeMillis * 0.001f;

        TransformComponent& tc = registry.get<TransformComponent>(m_activeCameraId);
        RendererIdComponent& ric = registry.get<RendererIdComponent>(m_activeCameraId);
        CameraComponent& cic = registry.get<CameraComponent>(m_activeCameraId);

        XMVECTOR camRight = XMLoadFloat4A(tc.GetRight());
        XMVECTOR camUp = XMLoadFloat4A(tc.GetUp());
        XMVECTOR camForward = XMLoadFloat4A(tc.GetForward());
        XMVECTOR position = XMLoadFloat4A(tc.GetPosition());

        //First let's update the orientation if needed.
        if ((m_cameraRotationInput.x != 0.0f) || (m_cameraRotationInput.y != 0.0f))
        {
            doUpdate = true;

            float yawRads = cic.m_rotationSpeed * deltaTimeSecs * m_cameraRotationInput.y;
            XMMATRIX matYaw = XMMatrixRotationAxis(camUp, yawRads);

            float pitchRads = cic.m_rotationSpeed * deltaTimeSecs * m_cameraRotationInput.x;

            float totalPitchDegrees = m_totalPitchDegrees + pitchRads * (180.0f / 3.14159f);
            XMMATRIX matPitch;
            if (totalPitchDegrees <= kMaxPitchDegrees)
            {
                matPitch = XMMatrixRotationAxis(camRight, pitchRads);
                m_totalPitchDegrees = totalPitchDegrees;
            }
            else
            {
                matPitch = XMMatrixRotationAxis(camRight, 0.0f);
            }

            XMMATRIX finalMat = XMMatrixMultiply(matYaw, matPitch);

            //transform forward
            camForward = XMVector3Transform(camForward,
                finalMat);
            camForward = XMVector3Normalize(camForward);

            //Recalculate Right
            camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            XMVECTOR camRight = XMVector3Cross(camUp, camForward);
            camRight = XMVector3Normalize(camRight);

            //Recalculate Up
            camUp = XMVector3Cross(camForward, camRight);
            camUp = XMVector3Normalize(camUp);

            //Save the new orientation vectors.
            XMStoreFloat4A(tc.GetUp(), camUp);
            XMStoreFloat4A(tc.GetRight(), camRight);
            XMStoreFloat4A(tc.GetForward(), camForward);
        }

        if ((m_cameraTranslationInput.x != 0.0f) || (m_cameraTranslationInput.y != 0.0f) || (m_cameraTranslationInput.z != 0.0f))
        {
            doUpdate = true;

            // Calculate how much to move in the current camera direction.
            XMVECTOR forwardDisplacement = XMVectorScale(camForward, m_cameraTranslationInput.z * cic.m_translationSpeed * deltaTimeSecs);
            XMVECTOR rightDisplacement = XMVectorScale(camRight, m_cameraTranslationInput.x * cic.m_translationSpeed * deltaTimeSecs);
            XMVECTOR upDisplacement = XMVectorScale(camUp, m_cameraTranslationInput.y * cic.m_translationSpeed * deltaTimeSecs);
            XMVECTOR deltaTotal = XMVectorAdd(forwardDisplacement, rightDisplacement);
            deltaTotal = XMVectorAdd(deltaTotal, upDisplacement);
            //Move the camera.
            position = XMVectorAdd(position, deltaTotal);
            //Store the new position
            XMStoreFloat4A(tc.GetPosition(), position);
        }

        //Notify the renderer
        if (!doUpdate)
        {
            return;
        }

        camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        m_renderer->UpdateCamera(ric.m_rendererEntityId, position, camForward, camUp);
    }

    void CameraSystem::receive(const InputComponent::InputEvent & evt)
    {
        //ODINFO("Got input event with hash=%x, name=%s, value=%f", evt.m_name.GetHash(), evt.m_name.GetString().c_str(), evt.m_value );
        StringHash evtHash = evt.m_name.GetHash();

        if (evtHash == entt::hashed_string("MoveForward"))
        {
            m_cameraTranslationInput.z = evt.m_value;
            return;
        }

        if (evtHash == entt::hashed_string("MoveRight"))
        {
            m_cameraTranslationInput.x = evt.m_value;
            return;
        }

        if (evtHash == entt::hashed_string("MoveUp"))
        {
            m_cameraTranslationInput.y = evt.m_value;
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

        if (evtHash == entt::hashed_string("RotatePitch"))
        {
            m_cameraRotationInput.x = evt.m_value;
            return;
        }
        if (evtHash == entt::hashed_string("RotateYaw"))
        {
            m_cameraRotationInput.y = evt.m_value;
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

        XMVECTOR camUp = XMLoadFloat4A(tc.GetUp());
        XMMATRIX matYaw = XMMatrixRotationAxis(camUp, yawRads);

        //transform forward
        camForward = XMVector3Transform(camForward, matYaw);
        camForward = XMVector3Normalize(camForward);

        //Recalculate Right
        XMVECTOR camRight = XMVector3Cross(camUp, camForward);
        camRight = XMVector3Normalize(camRight);

        //Save the new orientation vectors.
        XMStoreFloat4A(tc.GetRight(), camRight);
        XMStoreFloat4A(tc.GetForward(), camForward);

        XMVECTOR position = XMLoadFloat4A(tc.GetPosition());
        camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        m_renderer->UpdateCamera(ric.m_rendererEntityId, position, camForward, camUp);
    }

    void CameraSystem::UpdateCameraPitch(float pitchRads)
    {
        if (m_activeCameraId == NullEntity)
            return;

        float totalPitchDegrees = m_totalPitchDegrees + pitchRads * (180.0f / 3.14159f);
        //ODINFO("delta pitch rads= %f, total pitch deg=%f", pitchRads, totalPitchDegrees);
        if (fabsf(totalPitchDegrees) > kMaxPitchDegrees)
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
