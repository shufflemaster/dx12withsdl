#pragma once

#include <stdint.h>

#include <memory>
using namespace std;

#include <d3d12.h>
#include <dxgi1_4.h>
#include "Shader.h"
using namespace Microsoft::WRL::ComPtr;

namespace GAL
{
    struct P3F_C4F;

    class RenderNode
    {
    public:
        RenderNode();
        virtual ~RenderNode();

        bool InitWithVertices(const P3F_C4F* vertices, uint32_t numVertices,
                              const DWORD* indices, uint32_t numIndices);

        ComPtr <ID3D12Resource> m_vertexBuffer; // a default buffer in GPU memory that we will hold mesh vertex data.
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView; // a structure containing a pointer to the vertex data in gpu memory
                                                   // the total size of the buffer, and the size of each element (vertex)

        ComPtr <ID3D12Resource> m_indexBuffer; // a default buffer in GPU memory that we will hold mesh index data.
        D3D12_INDEX_BUFFER_VIEW m_indexBufferView; // a structure containing a pointer to the index data in gpu memory
                                                   // the total size of the buffer, and the size of each element (index)
    };
};
