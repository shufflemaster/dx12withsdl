#include "pch.h"

#include "InputSystem.h"
#include "InputComponent.h"

namespace GAL
{
    InputSystem::InputSystem(Universe* universe, HWND hWnd) : System(universe)
    {
        //get the client window area size.
        RECT clientSize;;
        GetClientRect(hWnd, &clientSize);
        m_windowWidth = clientSize.right;
        m_windowHeight = clientSize.bottom;


        m_wantsUpdate = false;
    }


    InputSystem::~InputSystem()
    {
    }

    void InputSystem::ProcessEvent(const SDL_Event& windowEvent)
    {
        switch (windowEvent.type)
        {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            ProcessKeyboardEvent(windowEvent.key);
            break;
        case SDL_MOUSEMOTION:
            ProcessMouseMotionEvent(windowEvent.motion);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            ProcessMouseButtonEvent(windowEvent.button);
            break;
        case SDL_MOUSEWHEEL:
            ProcessMouseWheelEvent(windowEvent.wheel);
            break;
        }
    }

    void InputSystem::ProcessKeyboardEvent(const SDL_KeyboardEvent& kbdEvent)
    {
        float value;
        if (kbdEvent.type == SDL_KEYDOWN)
        {
            value = 1.0f;
        }
        else
        {
            value = 0.0f;
        }

        InputComponent::eInputName inputName = InputComponent::eInputName::UNKNOWN;

        switch (kbdEvent.keysym.sym)
        {
        case SDLK_w:
        case SDLK_UP:
            for (auto listener : m_listeners) listener->OnMoveForward(value);
            break;
        case SDLK_s:
        case SDLK_DOWN:
            for (auto listener : m_listeners) listener->OnMoveForward(-value);
            break;
        case SDLK_a:
        case SDLK_LEFT:
            for (auto listener : m_listeners) listener->OnMoveRight(-value);
            break;
        case SDLK_d:
        case SDLK_RIGHT:
            for (auto listener : m_listeners) listener->OnMoveRight(value);
            break;
        case SDLK_q:
            for (auto listener : m_listeners) listener->OnMoveUp(-value);
            break;
        case SDLK_e:
            for (auto listener : m_listeners) listener->OnMoveUp(value);
            break;
        }
    }

    void InputSystem::ProcessMouseMotionEvent(const SDL_MouseMotionEvent& motionEvent)
    {
        bool isMiddleButtonPressed = motionEvent.state & SDL_BUTTON(SDL_BUTTON_MIDDLE);
        if (isMiddleButtonPressed)
        {
            float mouseDeltaX = (float)motionEvent.xrel / (float)m_windowWidth;
            for (auto listener : m_listeners) listener->OnMoveRight(mouseDeltaX * 5.0f, true);

            float mouseDeltaY = (float)motionEvent.yrel / (float)m_windowHeight;
            for (auto listener : m_listeners) listener->OnMoveUp(-mouseDeltaY * 5.0f, true);

            return;
        }
 
        bool isRightButtonPressed = motionEvent.state & SDL_BUTTON(SDL_BUTTON_RIGHT);

        if (!isRightButtonPressed)
            return;
        float mouseDeltaX = (float)motionEvent.xrel / (float)m_windowWidth;
        float mouseDeltaY = (float)motionEvent.yrel / (float)m_windowHeight;
        for (auto listener : m_listeners) listener->OnMoveYawPitch(mouseDeltaX, mouseDeltaY);
    }

    void InputSystem::ProcessMouseButtonEvent(const SDL_MouseButtonEvent& buttonEvent)
    {

    }

    void InputSystem::ProcessMouseWheelEvent(const SDL_MouseWheelEvent& wheelEvent)
    {

    }

} //namespace GAL