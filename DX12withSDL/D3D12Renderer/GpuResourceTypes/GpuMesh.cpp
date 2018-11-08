#include "pch.h"
#include "d3d12renderer/GpuResourceTypes/GpuMesh.h"

namespace GAL
{
    GpuMesh::GpuMesh(const std::string& filename) : ResourceBase(filename)
    {
    }

    GpuMesh::GpuMesh(GpuMesh&& other) : ResourceBase(std::move(other))
    {
        m_vertexBuffer.Swap(other.m_vertexBuffer); // a default buffer in GPU memory that we will hold mesh vertex data.
        m_vertexBufferView = other.m_vertexBufferView;
        memset(&other.m_vertexBufferView, 0, sizeof(D3D12_VERTEX_BUFFER_VIEW)); // a structure containing a pointer to the vertex data in gpu memory
                                                   // the total size of the buffer, and the size of each element (vertex)
        m_indexBuffer.Swap(other.m_indexBuffer); // a default buffer in GPU memory that we will hold mesh index data.
        m_indexBufferView = other.m_indexBufferView;
        memset(&other.m_indexBufferView, 0, sizeof(D3D12_INDEX_BUFFER_VIEW));  
    }


    GpuMesh::~GpuMesh()
    {

    }

};//namespace GAL

