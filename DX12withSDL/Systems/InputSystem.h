#pragma once

union SDL_Event;
struct SDL_KeyboardEvent;
struct SDL_MouseMotionEvent;
struct SDL_MouseButtonEvent;
struct SDL_MouseWheelEvent;

#include "System.h"
#include "..\Components\InputComponent.h"

namespace GAL
{
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

        void TriggerIfRegistered(InputComponent::eInputName inputName, float rawValue);

        UINT m_windowWidth, m_windowHeight;
    }; //class InputManager

};//namespace GAL

