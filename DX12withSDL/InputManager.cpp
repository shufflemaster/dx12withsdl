#include "pch.h"
#include "InputManager.h"

#include <SDL.h>

namespace GAL
{
    InputManager InputManager::s_inputManager;

    InputManager::InputManager()
    {
    }


    InputManager::~InputManager()
    {
    }

    void InputManager::ProcessEvent(const SDL_Event& windowEvent)
    {
        if ((windowEvent.type == SDL_KEYDOWN) || (windowEvent.type == SDL_KEYUP))
        {
            ProcessKeyboardEvent(windowEvent.key);
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
            for (auto listener : m_listeners) listener->OnMoveBack(value);
            break;
        case SDLK_a:
        case SDLK_LEFT:
            for (auto listener : m_listeners) listener->OnMoveLeft(value);
            break;
        case SDLK_d:
        case SDLK_RIGHT:
            for (auto listener : m_listeners) listener->OnMoveRight(value);
            break;
        }
    }

    void InputManager::AddListener(IInputListener* listener)
    {
        m_listeners.push_back(listener);
    }
} //namespace GAL