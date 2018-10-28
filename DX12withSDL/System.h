#pragma once

namespace GAL
{
    class Universe;

    class System
    {
    public:
        Universe* m_universe;

        virtual ~System(){}

        virtual bool Initialize(Universe* universe)
        {
            m_universe = universe;
            return true;
        }

        virtual void Update(Registry& registry, float deltaTime) = 0;
    };

} //namespace GAL
