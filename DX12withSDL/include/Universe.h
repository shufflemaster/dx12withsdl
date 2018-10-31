#pragma once

#include "Systems\System.h"

namespace GAL
{
    class ResourceManager;

    class Universe
    {
    public:
        Universe() = default;

        ~Universe()
        {
            for (auto system : m_systems)
            {
                delete system;
            }
        }

        //The order of calling methods is as follow.

        template<typename T, typename... Args>
        System* CreateAndAddSystem(Args &&... args)
        {
            System* newSys = new T(this, std::forward<Args>(args)...);
            m_systems.push_back(newSys);
        }

        void InitializeSystems()
        {
            for (auto system : m_systems)
            {
                system->Initialize();
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

        void DestroyEntity(EntityId entity)
        {
            m_registry.destroy(entity);
        }

        Registry& GetRegistry()
        {
            return m_registry;
        }

        ResourceManager& GetResourceManager()
        {
            return m_resourceManager;
        }

    private:
        Registry m_registry;
        ResourceManager m_resourceManager;
        std::vector<System*> m_systems;
    };

}; //namespace GAL
