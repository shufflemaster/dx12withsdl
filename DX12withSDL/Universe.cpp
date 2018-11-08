#include "pch.h"

#include "Systems\System.h"
#include "Universe.h"

namespace GAL
{
    Universe::~Universe()
    {
        for (auto system : m_systems)
        {
            delete system;
        }
    }

    void Universe::InitializeSystems()
    {
        for (auto system : m_systems)
        {
            system->Initialize();
        }
    }

    void Universe::UpdateSystems(float deltaTime)
    {
        for (auto system : m_systems)
        {
            if (!system->GetWantsUpdate())
                continue;
            system->TickUpdate(m_registry, deltaTime);
        }
    }

}; //namespace GAL
