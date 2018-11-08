#pragma once

#include "ResourceBase.hpp"

using Microsoft::WRL::ComPtr;

namespace GAL
{
    struct P3F_C4F;

    class GpuMesh : public ResourceBase<GpuMesh>
    {
        friend class GpuMeshLoader;

    public:
        GpuMesh(const std::string& filename);
        GpuMesh() = delete;
        GpuMesh(const GpuMesh&) = delete;
        GpuMesh(GpuMesh&& other);
        ~GpuMesh();

        inline const ID3D12Resource* GetVertexBufferGpuRes() { return m_vertexBuffer.Get(); }
        inline const ID3D12Resource* GetIndexBufferGpuRes() { return m_vertexBuffer.Get(); }
        inline const D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() { return &m_vertexBufferView; }
        inline const D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView() { return &m_indexBufferView; }
        inline uint32_t GetNumIndices() { return m_numIndices; }

    private:
        //Data for the Renderer
        ComPtr <ID3D12Resource> m_vertexBuffer; // a default buffer in GPU memory that we will hold mesh vertex data.
        // a structure containing a pointer to the vertex data in gpu memory
        // the total size of the buffer, and the size of each element (vertex)
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
        
        ComPtr <ID3D12Resource> m_indexBuffer; // a default buffer in GPU memory that we will hold mesh index data.
        // a structure containing a pointer to the index data in gpu memory
        // the total size of the buffer, and the size of each element (index)
        D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

        //Required by DrawIndexed()
        uint32_t m_numIndices;
    };

};//namespace GAL

