#include "pch.h"

#include "../ResourceTypes/Mesh.h"
#include "PlaneMeshLoader.h" //We use PlaneMeshLoader to create each face of the cube.
#include "CubeMeshLoader.h"
#include "VertexTypes.h"
#include "LogUtils.h"
#include "../CommonHelpers.h"

using namespace DirectX;

namespace GAL
{
    static inline void GetFaceColor(int faceId, float* redOut, float* greenOut, float* blueOut,
        const XMFLOAT3* faceColors = nullptr, int numFaceColors = 0)
    {
        if ((faceColors == nullptr) || (faceId >= numFaceColors)) 
        {
            *redOut = *greenOut = *blueOut = -1.0f;
            return;
        }
        *redOut = faceColors[faceId].x;
        *greenOut = faceColors[faceId].y;
        *blueOut = faceColors[faceId].z;
    }

    /*   
    SEE NOTES on CubeMeshLoader.h                                
    */                          
    std::shared_ptr<Mesh> CubeMeshLoader::Load(const std::string& filename,
        float width,  uint32_t numColumnsX,
        float height, uint32_t numRowsY,
        float depth,  uint32_t numColumnsZ,
        const XMFLOAT3* faceColors, int numFaceColors) const
    {
        assert(width > 0.0f && height > 0.0f && depth > 0.0f);
        assert(numColumnsX > 0 && numRowsY > 0 && numColumnsZ > 0);

        float red, green, blue;
        PlaneMeshLoader planeCreator;
        XMFLOAT3 planeNormal;
        XMFLOAT3 planePosition;

        GetFaceColor(0, &red, &green, &blue, faceColors, numFaceColors);
        planeNormal = XMFLOAT3{ 0.0f, 0.0f, 1.0f };
        std::shared_ptr<Mesh> meshFace0 = planeCreator.Load("face0", width, numColumnsX, height, numRowsY,
            planeNormal, red, green, blue);
        planePosition = XMFLOAT3{ width * 0.5f * planeNormal.x, height * 0.5f * planeNormal.y, depth * 0.5f * planeNormal.z };
        ApplyTranslationToVertices(planePosition, meshFace0->m_vertices, meshFace0->m_numVertices);

        GetFaceColor(1, &red, &green, &blue, faceColors, numFaceColors);
        planeNormal = XMFLOAT3{ 1.0f, 0.0f, 0.0f };
        std::shared_ptr<Mesh> meshFace1 = planeCreator.Load("face1", depth, numColumnsZ, height, numRowsY,
            planeNormal, red, green, blue);
        planePosition = XMFLOAT3{ width * 0.5f * planeNormal.x, height * 0.5f * planeNormal.y, depth * 0.5f * planeNormal.z };
        ApplyTranslationToVertices(planePosition, meshFace1->m_vertices, meshFace1->m_numVertices);

        GetFaceColor(2, &red, &green, &blue, faceColors, numFaceColors);
        planeNormal = XMFLOAT3{ 0, 0, -1 };
        std::shared_ptr<Mesh> meshFace2 = planeCreator.Load("face2", width, numColumnsX, height, numRowsY,
            planeNormal, red, green, blue);
        planePosition = XMFLOAT3{ width * 0.5f * planeNormal.x, height * 0.5f * planeNormal.y, depth * 0.5f * planeNormal.z };
        ApplyTranslationToVertices(planePosition, meshFace2->m_vertices, meshFace2->m_numVertices);

        GetFaceColor(3, &red, &green, &blue, faceColors, numFaceColors);
        planeNormal = XMFLOAT3{ -1, 0, 0 };
        std::shared_ptr<Mesh> meshFace3 = planeCreator.Load("face3", depth, numColumnsZ, height, numRowsY,
            planeNormal, red, green, blue);
        planePosition = XMFLOAT3{ width * 0.5f * planeNormal.x, height * 0.5f * planeNormal.y, depth * 0.5f * planeNormal.z };
        ApplyTranslationToVertices(planePosition, meshFace3->m_vertices, meshFace3->m_numVertices);

        GetFaceColor(4, &red, &green, &blue, faceColors, numFaceColors);
        planeNormal = XMFLOAT3{ 0, 1, 0 };
        std::shared_ptr<Mesh> meshFace4 = planeCreator.Load("face4", width, numColumnsX, depth, numColumnsZ,
            planeNormal, red, green, blue);
        planePosition = XMFLOAT3{ width * 0.5f * planeNormal.x, height * 0.5f * planeNormal.y, depth * 0.5f * planeNormal.z };
        ApplyTranslationToVertices(planePosition, meshFace4->m_vertices, meshFace4->m_numVertices);

        GetFaceColor(5, &red, &green, &blue, faceColors, numFaceColors);
        planeNormal = XMFLOAT3{ 0, -1, 0 };
        std::shared_ptr<Mesh> meshFace5 = planeCreator.Load("face5", width, numColumnsX, depth, numColumnsZ,
            planeNormal, red, green, blue);
        planePosition = XMFLOAT3{ width * 0.5f * planeNormal.x, height * 0.5f * planeNormal.y, depth * 0.5f * planeNormal.z };
        ApplyTranslationToVertices(planePosition, meshFace5->m_vertices, meshFace5->m_numVertices);

        Mesh* allMeshes[6] = { meshFace0.get(), meshFace1.get(),
                               meshFace2.get(), meshFace3.get(),
                               meshFace4.get(), meshFace5.get() };

        uint32_t numMeshes = sizeof(allMeshes) / sizeof(allMeshes[0]);

        uint32_t totalVertices = 0;
        uint32_t totalIndices = 0;
        for (uint32_t i = 0; i < numMeshes; i++)
        {
            totalVertices += allMeshes[i]->GetNumVertices();
            totalIndices += allMeshes[i]->GetNumIndices();
        }

        //Finally merge the meshes.
        std::shared_ptr<Mesh> retMesh = std::make_shared<Mesh>(filename);
        retMesh->Allocate(totalVertices, totalIndices);
        P3F_C4F* dstVertices = retMesh->m_vertices;
        DWORD* dstIndices = retMesh->m_indices;
        DWORD indexOffset = 0;
        for (uint32_t i = 0; i < numMeshes; i++)
        {
            memcpy(dstVertices, allMeshes[i]->GetVertices(), sizeof(P3F_C4F) * allMeshes[i]->GetNumVertices());

            //The indices need shifting!
            uint32_t meshNumIndices = allMeshes[i]->GetNumIndices();
            const DWORD* meshIndices = allMeshes[i]->GetIndices();
            for (uint32_t index = 0; index < meshNumIndices; index++)
            {
                dstIndices[index] = meshIndices[index] + indexOffset;
            }

            indexOffset += allMeshes[i]->GetNumVertices();
            dstVertices += allMeshes[i]->GetNumVertices();
            dstIndices += allMeshes[i]->GetNumIndices();
        }

        return retMesh;
    }
};//namespace GAL