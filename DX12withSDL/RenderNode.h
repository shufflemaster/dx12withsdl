#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

namespace GAL
{
    struct ColorVertex;

    class RenderNode
    {
    public:
        RenderNode();
        virtual ~RenderNode();

        bool InitWithVertices(const ColorVertex* vertices, uint32_t numVertices,
                              const DWORD* indices, uint32_t numIndices);

        ID3D12PipelineState* m_pipelineStateObject; // pso containing a pipeline state
        ID3D12RootSignature* m_rootSignature; // root signature defines data shaders will access
        ID3D12Resource* m_vertexBuffer; // a default buffer in GPU memory that we will load vertex data for our mesh into.
        ID3D12Resource* m_indexBuffer;
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView; // a structure containing a pointer to the vertex data in gpu memory
                                                   // the total size of the buffer, and the size of each element (vertex)
        D3D12_INDEX_BUFFER_VIEW m_indexBufferView; // a structure containing a pointer to the index data in gpu memory
                                                   // the total size of the buffer, and the size of each element (index)
    };
};
