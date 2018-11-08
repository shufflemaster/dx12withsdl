#pragma once

#include "d3d12renderer/D3D12Renderer.h"

//This component is used to store the EntityId created by the Renderer,
//So when an engine entity is removed we can also send a message to the Renderer
//to remove the Renderer counter part EntityId.
namespace GAL
{
    class RendererIdComponent
    {
    public:
        RendererIdComponent();
        ~RendererIdComponent();

        RenderEntityId m_rendererEntityId;
    };
}; //namespace GAL
