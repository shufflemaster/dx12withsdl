#include "pch.h"

#include "..\ResourceTypes\Mesh.h"
#include "TriangleMeshLoader.h"
#include "VertexTypes.h"

namespace GAL
{
    std::shared_ptr<Mesh> TriangleMeshLoader::Load(const std::string& filename, float size, float red, float green, float blue) const
    {
        std::shared_ptr<Mesh> retMesh = std::make_shared<Mesh>(filename);

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