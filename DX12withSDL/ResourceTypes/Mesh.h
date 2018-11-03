#pragma once

#include "ResourceBase.hpp"

namespace GAL
{
    struct P3F_C4F;

    class Mesh : public ResourceBase<Mesh>
    {
        friend class TriangleMeshLoader;

    public:
        Mesh(const std::string& filename);
        Mesh() = delete;
        Mesh(const Mesh&) = delete;
        Mesh(Mesh&& other);
        ~Mesh();

        size_t GetNumVertices() { return m_numVertices; }
        size_t GetNumIndices() { return m_numIndices; }
        const P3F_C4F* GetVertices() { return m_vertices; }
        const DWORD* GetIndices() { return m_indices; }
        
    private:
        void Allocate(size_t numVertices, size_t numIndices);

        size_t m_numVertices;
        size_t m_numIndices;

        P3F_C4F* m_vertices;
        DWORD* m_indices;

        //Data for the Renderer
        ComPtr <ID3D12Resource> m_vertexBuffer; // a default buffer in GPU memory that we will hold mesh vertex data.
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView; // a structure containing a pointer to the vertex data in gpu memory
                                                   // the total size of the buffer, and the size of each element (vertex)

        ComPtr <ID3D12Resource> m_indexBuffer; // a default buffer in GPU memory that we will hold mesh index data.
        D3D12_INDEX_BUFFER_VIEW m_indexBufferView; // a structure containing a pointer to the index data in gpu memory
                                                   // the total size of the buffer, and the size of each element (index)
    };

};//namespace GAL

