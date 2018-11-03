#pragma once

#include "ResourceManager.h"

namespace GAL
{
    class System;

    class Universe
    {
        friend class Engine;
        
    public:
        Universe() = default;

        ~Universe();

        //The order of calling methods is as follow.

        template<typename T, typename... Args>
        System* CreateAndAddSystem(Args &&... args)
        {
            System* newSys = new T(this, std::forward<Args>(args)...);
            m_systems.push_back(newSys);
            return newSys;
        }

        void InitializeSystems();
        void UpdateSystems(float deltaTime);

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

        EventDispatcher& GetEventDispatcher()
        {
            return m_eventDispatcher;
        }

    private:
        Registry m_registry;
        ResourceManager m_resourceManager;
        std::vector<System*> m_systems;
        EventDispatcher m_eventDispatcher;
    };

}; //namespace GAL
