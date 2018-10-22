#include "pch.h"
#include "InputManager.h"

#include <SDL.h>

namespace GAL
{
    InputManager InputManager::s_inputManager;

    InputManager::InputManager() : m_windowWidth(0), m_windowHeight(0)
    {
    }


    InputManager::~InputManager()
    {
    }

    void InputManager::Init(HWND hWnd)
    {
        //get the client window area size.
        RECT clientSize;;
        GetClientRect(hWnd, &clientSize);
        m_windowWidth = clientSize.right;
        m_windowHeight = clientSize.bottom;
    }

    void InputManager::ProcessEvent(const SDL_Event& windowEvent)
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

    void InputManager::ProcessKeyboardEvent(const SDL_KeyboardEvent& kbdEvent)
    {
        float value = (kbdEvent.type == SDL_KEYDOWN) ? 1.0f : 0.0f;
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

    void InputManager::ProcessMouseMotionEvent(const SDL_MouseMotionEvent& motionEvent)
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

    void InputManager::ProcessMouseButtonEvent(const SDL_MouseButtonEvent& buttonEvent)
    {

    }

    void InputManager::ProcessMouseWheelEvent(const SDL_MouseWheelEvent& wheelEvent)
    {

    }


    void InputManager::AddListener(IInputListener* listener)
    {
        m_listeners.push_back(listener);
    }
} //namespace GAL