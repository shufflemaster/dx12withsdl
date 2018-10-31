#pragma once

#include "HashedString.h"

namespace GAL
{
    //REMARK: See README for Notes on std::map vs std::unordered_map performace
    class InputComponent
    {
    public:
        enum class eInputName
        {
            UNKNOWN,

            KEYBOARD_A, // = SDLK_a
            KEYBOARD_D, // = SDLK_d
            KEYBOARD_E, // = SDLK_e
            KEYBOARD_Q, // = SDLK_q
            KEYBOARD_S, // = SDLK_s
            KEYBOARD_W, // = SDLK_w

            KEYBOARD_RIGHT,// = SDLK_RIGHT,
            KEYBOARD_LEFT, // = SDLK_LEFT,
            KEYBOARD_DOWN, // = SDLK_DOWN,
            KEYBOARD_UP, // = SDLK_UP,

            MOUSE_BUTTON_LEFT,
            MOUSE_BUTTON_MIDDLE,
            MOUSE_BUTTON_RIGHT,
            MOUSE_DELTA_X,
            MOUSE_DELTA_Y,
            MOUSE_DELTA_Z, //Scroll movement.
        };

        //When the event is triggered, it will convey the name as a string and the hashed version of the name
        //so the receiver/listener can switch using either.
        struct Event
        {
            float m_multiplier; //Multiply the RAW value given by a device by this number. usually 1.0f (or -1.0f)
            float m_deadZone; //The absolute value deadzone that the value should be larger than before reporting the event.
            HashedString m_hashedName;
        };

    public:
        InputComponent();
        ~InputComponent();


        bool AddInputEventHelper(const char * eventName, eInputName inputName, float multiplier = 1.0f, float deadZone = 0.1f)
        {
            if (m_generators.find(inputName) != m_generators.end()) {
                return false;
            }

            Event newEvt;
            newEvt.m_hashedName.SetString(eventName);
            newEvt.m_multiplier = multiplier;
            newEvt.m_deadZone = deadZone;
            m_generators[inputName] = newEvt;
            return true;
        }

        //The HashedString represents a regular event name String but it's been hashed so you
        //can use the hash for a fast switch case.
        std::map<eInputName, Event> m_generators;
    };

}; //namespace GAL

