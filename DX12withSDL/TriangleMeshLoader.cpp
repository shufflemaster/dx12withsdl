#include "pch.h"
#include "TriangleMeshLoader.h"
#include "VertexTypes.h"
#include "Mesh.h"

namespace GAL
{
    TriangleMeshLoader::TriangleMeshLoader()
    {
    }


    TriangleMeshLoader::~TriangleMeshLoader()
    {
    }

    std::shared_ptr<Mesh> TriangleMeshLoader::load(float size, float red, float green, float blue) const
    {
        std::shared_ptr<Mesh> retMesh = std::make_shared<Mesh>();

        retMesh->Allocate(3, 3);
        
        retMesh->m_vertices[0] = { {  0.0f,  size, 0.0f}, {red, green, blue, 1.0f} };
        retMesh->m_vertices[1] = { {  size, -size, 0.0f}, {red, green, blue, 1.0f} };
        retMesh->m_vertices[2] = { { -size, -size, 0.0f}, {red, green, blue, 1.0f} };
        
        retMesh->m_indices[0] = 0;
        retMesh->m_indices[1] = 1;
        retMesh->m_indices[2] = 2;

        return retMesh;
    }
};//namespace GAL