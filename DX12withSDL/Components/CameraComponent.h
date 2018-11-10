#pragma once

namespace GAL
{
    class CameraComponent
    {
    public:
        CameraComponent();
        ~CameraComponent();

        float m_fieldOfViewDegrees;
        float m_nearClipDistance;
        float m_farClipDistance;

        //When the camera is listening to input. The speed is used
        //to determine how fast to move the camera.
        float m_speed; //World units per second

        //If you add multiple cameras the last active one is the active camera.
        bool m_isActive;
    };
};//namespace GAL

