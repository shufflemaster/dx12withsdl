#pragma once

#include "System.h"

namespace GAL
{
    class RenderSystem : public System
    {
    public:
        RenderSystem();
        ~RenderSystem();

        RenderSystem(Universe* universe) : System(universe) {}

        RenderSystem() = delete;
        RenderSystem(const RenderSystem&) = delete;
        RenderSystem(RenderSystem&&) = delete;

        void Initialize() override;
        void Update(Registry& /*registry*/, float /*deltaTime*/) override {};
    };
}; //namespace GAL
