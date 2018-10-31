#include "pch.h"
#include "InputComponent.h"

namespace GAL
{
    InputComponent::InputComponent()
    {
    }


    InputComponent::~InputComponent()
    {
    }

    HashedString* AddEventNameHelper(const char * eventName);
    HashedString* AddEventNameHelper(const std::string& eventName);
    //returns false if inputName is already listed in eventName.
    bool AddEventInputItemHelper(const HashedString* eventName, eInputName inputName, float multiplier, float deadZone);

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
}; //namespace GAL
