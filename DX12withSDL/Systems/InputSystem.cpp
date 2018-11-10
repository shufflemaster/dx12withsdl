#include "pch.h"

#include "Universe.h"
#include "InputSystem.h"

#include "LogUtils.h"

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

        TriggerIfRegistered(inputName, value);
    }

    void InputSystem::ProcessMouseMotionEvent(const SDL_MouseMotionEvent& motionEvent)
    {
        float mouseDeltaX = (float)motionEvent.xrel / (float)m_windowWidth;
        TriggerIfRegistered(InputComponent::eInputName::MOUSE_DELTA_X, mouseDeltaX);

        float mouseDeltaY = (float)motionEvent.yrel / (float)m_windowHeight;
        TriggerIfRegistered(InputComponent::eInputName::MOUSE_DELTA_Y, mouseDeltaY);

        //ODINFO("raw deltaX = %f, raw deltaY=%f", mouseDeltaX, mouseDeltaY);
    }

    void InputSystem::ProcessMouseButtonEvent(const SDL_MouseButtonEvent& buttonEvent)
    {
        float value;
        if (buttonEvent.type == SDL_MOUSEBUTTONDOWN)
        {
            value = 1.0f;
        }
        else
        {
            value = 0.0f;
        }

        InputComponent::eInputName inputName = InputComponent::eInputName::UNKNOWN;

        switch (buttonEvent.button)
        {
        case SDL_BUTTON_LEFT:
            inputName = InputComponent::eInputName::MOUSE_BUTTON_LEFT;
            break;
        case SDL_BUTTON_MIDDLE:
            inputName = InputComponent::eInputName::MOUSE_BUTTON_MIDDLE;
            break;
        case SDL_BUTTON_RIGHT:
            inputName = InputComponent::eInputName::MOUSE_BUTTON_RIGHT;
            break;
        default:
            return; //Not supported key event.
        }

        TriggerIfRegistered(inputName, value);
    }

    void InputSystem::ProcessMouseWheelEvent(const SDL_MouseWheelEvent& wheelEvent)
    {

    }

    void InputSystem::TriggerIfRegistered(InputComponent::eInputName inputName, float rawValue)
    {
        //Fetch the input components from the registry.
        Registry& registry = m_universe->GetRegistry();

        //Let's get a raw_view
        auto raw = registry.raw_view<InputComponent>();
        for (auto& inputComponent : raw)
        {
            for (HashedString& hashedEventName : inputComponent.m_eventNames)
            {
                //Get the vector of input genrators
                const auto& inputVec = inputComponent.m_eventInfoMap[hashedEventName.GetHash()];
                for (const InputComponent::InputItem& item : inputVec)
                {
                    if (item.m_inputName != inputName)
                    {
                        continue;
                    }
                    //There's a match let's dispatch the event.
                    if (fabsf(rawValue) < item.m_deadZone)
                    {
                        //The event data is not large enough.
                        continue;
                    }
                    m_universe->GetEventDispatcher().trigger<InputComponent::InputEvent>(hashedEventName, rawValue * item.m_multiplier);
                    break;
                }
            }
        }
    }

} //namespace GAL