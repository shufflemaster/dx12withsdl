#pragma once

#include <vector>

union SDL_Event;
struct SDL_KeyboardEvent;

namespace GAL
{
    struct IInputListener
    {
        //value is between 0.0f and 1.0f;
        virtual void OnMoveForward(float value) {};
        virtual void OnMoveBack(float value) {};
        virtual void OnMoveLeft(float value) {};
        virtual void OnMoveRight(float value) {};
    };

    class InputManager
    {
    public:
        static InputManager& Instance() { return s_inputManager;  }
        void ProcessEvent(const SDL_Event& windowEvent);
        void AddListener(IInputListener* listener);

        virtual ~InputManager();

    private:
        InputManager();

        void ProcessKeyboardEvent(const SDL_KeyboardEvent& kbdEvent);

        static InputManager s_inputManager;
        std::vector<IInputListener*> m_listeners;
    }; //class InputManager

};//namespace GAL

