#pragma once

#include "ResourceBase.hpp"

namespace GAL
{
    struct P3F_C4F;

    class Mesh : public ResourceBase<Mesh>
    {
        friend class TriangleMeshLoader;
        friend class SphereMeshLoader;

    public:
        Mesh(const std::string& filename);
        Mesh() = delete;
        Mesh(const Mesh&) = delete;
        Mesh(Mesh&& other);
        ~Mesh();

        uint32_t GetNumVertices() { return m_numVertices; }
        uint32_t GetNumIndices() { return m_numIndices; }
        const P3F_C4F* GetVertices() { return m_vertices; }
        const DWORD* GetIndices() { return m_indices; }
        
    private:
        void Allocate(uint32_t numVertices, uint32_t numIndices);

        uint32_t m_numVertices;
        uint32_t m_numIndices;

        P3F_C4F* m_vertices;
        DWORD* m_indices;
    };

};//namespace GAL

