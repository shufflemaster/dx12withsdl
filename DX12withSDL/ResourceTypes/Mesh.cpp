#include "pch.h"
#include "Mesh.h"

#include "VertexTypes.h"

namespace GAL
{
    Mesh::Mesh(const std::string& filename) : ResourceBase(filename)
    {
    }

    Mesh::Mesh(Mesh&& other)
    {
        m_numVertices = other.m_numVertices;
        m_numIndices = other.m_numIndices;
        m_vertices = other.m_vertices;
        m_indices = other.m_indices;

        other.m_numVertices = 0;
        other.m_numIndices = 0;
        other.m_vertices = nullptr;
        other.m_indices = nullptr;
    }


    Mesh::~Mesh()
    {
        if (m_vertices)
        {
            delete[] m_vertices;
            m_vertices = nullptr;
        }
        if (m_indices)
        {
            delete[] m_indices;
            m_indices = nullptr;
        }
    }

    void Mesh::Allocate(size_t numVertices, size_t numIndices)
    {
        assert(m_vertices == nullptr);
        assert(m_indices == nullptr);
        m_numVertices = numVertices;
        m_numIndices = numIndices;
        m_vertices = new P3F_C4F[numVertices];
        m_indices = new DWORD[numIndices];
    }

};//namespace GAL

