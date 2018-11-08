#pragma once

#include "System.h"

namespace GAL
{
    class D3D12Renderer;

    class RenderSystem : public System
    {
    public:
        ~RenderSystem();

        RenderSystem(Universe* universe, D3D12Renderer* renderer) : System(universe), m_renderer(renderer)
        {
            m_wantsUpdate = false;
        }

        RenderSystem() = delete;
        RenderSystem(const RenderSystem&) = delete;
        RenderSystem(RenderSystem&&) = delete;

        void Initialize() override;
        void TickUpdate(Registry& /*registry*/, float /*deltaTimeMillis*/) override {};

    private:
        D3D12Renderer* m_renderer;
    };
}; //namespace GAL
