#pragma once

using namespace DirectX;

namespace GAL
{
    class RenderNode
    {
    public:
        RenderNode();
        RenderNode(ResourceHandle gpuMeshHandle, const XMFLOAT4X4A& worldMatrix) :
            m_gpuMeshHandle(gpuMeshHandle), m_worldMatrix(worldMatrix) {}
        virtual ~RenderNode();

        ResourceHandle m_gpuMeshHandle;
        XMFLOAT4X4A m_worldMatrix;
    };
};
