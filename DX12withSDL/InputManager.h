#pragma once

union SDL_Event;
struct SDL_KeyboardEvent;
struct SDL_MouseMotionEvent;
struct SDL_MouseButtonEvent;
struct SDL_MouseWheelEvent;

namespace GAL
{
    struct IInputListener
    {
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
    };

    class InputManager
    {
    public:
        static InputManager& Instance() { return s_inputManager; }

        void Init(HWND hWnd);
        void ProcessEvent(const SDL_Event& windowEvent);
        void AddListener(IInputListener* listener);

        virtual ~InputManager();

    private:
        InputManager();

        void ProcessKeyboardEvent(const SDL_KeyboardEvent& kbdEvent);

        //Mouse
        void ProcessMouseMotionEvent(const SDL_MouseMotionEvent& motionEvent);
        void ProcessMouseButtonEvent(const SDL_MouseButtonEvent& buttonEvent);
        void ProcessMouseWheelEvent(const SDL_MouseWheelEvent& wheelEvent);

        static InputManager s_inputManager;

        UINT m_windowWidth, m_windowHeight;
        std::vector<IInputListener*> m_listeners;
    }; //class InputManager

};//namespace GAL

