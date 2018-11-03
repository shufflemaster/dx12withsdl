#include "pch.h"
#include "InputComponent.h"

#include "LogUtils.h"

namespace GAL
{
    //std::vector<HashedString> m_eventNames;
    //std::map<StringHash, std::vector<InputItem>> m_eventInfoMap;
    HashedString* InputComponent::AddEventNameHelper(const char * eventName)
    {
        HashedString hashedString(eventName);
        if (m_eventInfoMap.find(hashedString.GetHash()) != m_eventInfoMap.end())
        {
            ODERROR("Event with name %s already added", eventName);
            return nullptr;
        }
        m_eventNames.push_back(hashedString);
        m_eventInfoMap.insert(std::pair<StringHash, std::vector<InputItem>>(hashedString.GetHash(), std::vector<InputItem>()));
        return &m_eventNames[m_eventNames.size() - 1];
    }

    //returns false if inputName is already listed in eventName.
    bool InputComponent::AddEventInputItemHelper(const HashedString* eventName, eInputName inputName, float multiplier, float deadZone)
    {
        if (m_eventInfoMap.find(eventName->GetHash()) == m_eventInfoMap.end())
        {
            ODERROR("event with name %s and hash %llu doesn't exist", eventName->GetString().c_str(), eventName->GetHash());
            return false;
        }
        m_eventInfoMap[eventName->GetHash()].push_back(InputItem(inputName, multiplier, deadZone));
        return true;
    }

}; //namespace GAL
