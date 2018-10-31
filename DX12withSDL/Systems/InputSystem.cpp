#include "pch.h"

#include "Universe.h"
#include "InputSystem.h"
#include "..\Components\InputComponent.h"

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
            inputName = InputComponent::eInputName::KEYBOARD_W;
            break;
        case SDLK_UP:
            inputName = InputComponent::eInputName::KEYBOARD_UP;
            break;
        case SDLK_s:
            inputName = InputComponent::eInputName::KEYBOARD_S;
            break;
        case SDLK_DOWN:
            inputName = InputComponent::eInputName::KEYBOARD_DOWN;
            break;
        case SDLK_a:
            inputName = InputComponent::eInputName::KEYBOARD_A;
            break;
        case SDLK_LEFT:
            inputName = InputComponent::eInputName::KEYBOARD_LEFT;
            break;
        case SDLK_d:
            inputName = InputComponent::eInputName::KEYBOARD_D;
            break;
        case SDLK_RIGHT:
            inputName = InputComponent::eInputName::KEYBOARD_RIGHT;
            break;
        case SDLK_q:
            inputName = InputComponent::eInputName::KEYBOARD_Q;
            break;
        case SDLK_e:
            inputName = InputComponent::eInputName::KEYBOARD_E;
            break;
        default:
            return; //Not supported key event.
        }

        //Fetch the input components from the registry.
        Registry& registry = m_universe->GetRegistry();

        //Let's get a raw_view
        auto raw = registry.raw_view<InputComponent>();
        for (auto& inputComponent : raw)
        {
            auto itor = inputComponent.m_generators.find(inputName);
            if (itor == inputComponent.m_generators.end())
            {
                continue;
            }
            InputComponent::Event* evt = &itor->second;
            if (evt->m_deadZone > fabs(value))
            {
                //Skip the event because it is smaller than the deadzone.
                continue;
            }
            float evtValue = value * evt->m_multiplier;
            //Let's dispatch the event.

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