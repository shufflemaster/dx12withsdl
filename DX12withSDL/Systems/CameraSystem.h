#pragma once

#include "System.h"
#include "../Components/InputComponent.h"

using namespace DirectX;

namespace GAL
{
    class D3D12Renderer;

    class CameraSystem : public System
    {
    public:
        ~CameraSystem();

        CameraSystem(Universe* universe, D3D12Renderer* renderer) : System(universe), m_renderer(renderer), m_activeCameraId(NullEntity)
        {
        }

        CameraSystem() = delete;
        CameraSystem(const CameraSystem&) = delete;
        CameraSystem(CameraSystem&&) = delete;

        void Initialize() override;
        void TickUpdate(Registry& /*registry*/, float /*deltaTimeMillis*/) override;

        //The camera system is listener of input system events
        void receive(const InputComponent::InputEvent &);

    private:
        D3D12Renderer* m_renderer;
        EntityId m_activeCameraId;
        float m_totalPitchDegrees;
        XMFLOAT4A m_cameraTranslationInput;
        XMFLOAT4A m_cameraRotationInput;

        bool m_mouseMiddleButton;
        bool m_mouseRightButton;

        void UpdateCameraYaw(float yawRads);
        void UpdateCameraPitch(float pitchRads);
    };

}; //namespace GAL

