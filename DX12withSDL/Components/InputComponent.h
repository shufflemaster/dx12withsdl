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
        struct InputItem
        {
            eInputName m_inputName;
            float m_multiplier; //Multiply the RAW value given by a device by this number. usually 1.0f (or -1.0f)
            float m_deadZone; //The absolute value deadzone that the value should be larger than before reporting the event.

            InputItem(eInputName inputName, float multiplier, float deadZone) :
                m_inputName(inputName), m_multiplier(multiplier), m_deadZone(deadZone) {}
        };

        //This is what the listener will get.
        struct InputEvent
        {
            HashedString m_name;
            float m_value;

            InputEvent(const HashedString& name, float value) : m_name(name), m_value(value) {}
        };

    public:
        InputComponent() = default;
        ~InputComponent() {};

        //May return nullptr to say that there's event name hashing collision (ultra rare)
        HashedString* AddEventNameHelper(const char * eventName);
        HashedString* AddEventNameHelper(const std::string& eventName) { return AddEventNameHelper(eventName.c_str()); }

        //returns false if inputName is already listed in eventName.
        bool AddEventInputItemHelper(const HashedString* eventName, eInputName inputName, float multiplier, float deadZone);

        //The HashedString represents a regular event name String but it's been hashed so you
        //can use the hash for a fast switch case.
        std::vector<HashedString> m_eventNames;
        std::map<StringHash, std::vector<InputItem>> m_eventInfoMap;
    };

}; //namespace GAL

