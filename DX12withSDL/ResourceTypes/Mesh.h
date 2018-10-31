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
    };

};//namespace GAL

