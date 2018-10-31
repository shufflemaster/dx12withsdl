#pragma once

union SDL_Event;
struct SDL_KeyboardEvent;
struct SDL_MouseMotionEvent;
struct SDL_MouseButtonEvent;
struct SDL_MouseWheelEvent;

#include "System.h"

namespace GAL
{
    struct IInputListener
    {
#if 0
        //value is between -1.0f and 1.0f;
        //if isDiscrete move by delta instead of speed.
        virtual void OnMoveForward(float value, bool isDiscrete = false) {};
        virtual void OnMoveRight(float value, bool isDiscrete = false) {};
        virtual void OnMoveUp(float value, bool isDiscrete = false) {};

        //yaw: value between 0.0f and 1.0f. Because this is usually
        //     produced by a mouse, a value of 1.0f means the user
        //     moved the mouse all over the X Screen dimension.
        //     yaw should be interpreted as a rotation across the 3D UP axis.
        //pitch: value between 0.0f and 1.0f. Because this is usually
        //     produced by a mouse, a value of 1.0f means the user
        //     moved the mouse all over the Y Screen dimension.
        //     pitch should be interpreted as a rotation across the 3D RIGHT axis.
        virtual void OnMoveYawPitch(float yaw, float pitch) {};
#endif
        virtual void OnInputEvent()
    };

    class InputSystem : public System
    {
    public:
        InputSystem(Universe* universe, HWND hWnd);

        InputSystem() = delete;
        InputSystem(const InputSystem&) = delete;
        InputSystem(InputSystem&&) = delete;

        void Update(Registry& /*registry*/, float /*deltaTime*/) override {};
        void ProcessEvent(const SDL_Event& windowEvent);

        virtual ~InputSystem();

    private:
        void ProcessKeyboardEvent(const SDL_KeyboardEvent& kbdEvent);

        //Mouse
        void ProcessMouseMotionEvent(const SDL_MouseMotionEvent& motionEvent);
        void ProcessMouseButtonEvent(const SDL_MouseButtonEvent& buttonEvent);
        void ProcessMouseWheelEvent(const SDL_MouseWheelEvent& wheelEvent);

        UINT m_windowWidth, m_windowHeight;
    }; //class InputManager

};//namespace GAL

