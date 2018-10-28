#pragma once

#include "System.h"

namespace GAL
{
    class Universe
    {
    public:
        ~Universe()
        {
            for (auto system : m_systems)
            {
                delete system;
            }
        }

        //The order of calling methods is as follow.

        template<typename T>
        System* CreateAndAddSystem()
        {
            System* newSys = new T();
            m_systems.push_back(new T());
        }

        void InitializeSystems()
        {
            for (auto system : m_systems)
            {
                system->Initialize(this);
            }
        }

        void UpdateSystems(float deltaTime)
        {
            for (auto system : m_systems)
            {
                system->Update(m_registry, deltaTime);
            }
        }

        EntityId CreteEntity()
        {
            return m_registry.create();
        }

        EntityId DestroyEntity()
        {
            return m_registry.create();
        }

        Registry* GetRegistry()
        {
            return &m_registry;
        }


    private:
        std::vector<System*> m_systems;
        Registry m_registry;
    };

}; //namespace GAL
