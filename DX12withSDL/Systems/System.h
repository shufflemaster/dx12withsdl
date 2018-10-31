#pragma once

namespace GAL
{
    class Universe;

    class System
    {
    public:
        System() = delete;
        System(const System&) = delete;
        System(const System&&) = delete;

        System(Universe* universe) : m_universe(universe), m_wantsUpdate(true) {}

        virtual ~System(){}

        virtual void Initialize() {} //Suitable to avoid making ugly constructors.
        virtual void Update(Registry& registry, float deltaTime) = 0;

        bool GetWantsUpdate() { return m_wantsUpdate; }

    protected:
        bool m_wantsUpdate; //Set to false if you don't want Update() to be called.
        Universe* m_universe;
    };

} //namespace GAL
