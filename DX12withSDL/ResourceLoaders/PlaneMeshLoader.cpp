#include "pch.h"

#include "../ResourceTypes/Mesh.h"
#include "PlaneMeshLoader.h"
#include "VertexTypes.h"
#include "LogUtils.h"
#include "../CommonHelpers.h"

using namespace DirectX;

namespace GAL
{
    /*   
    SEE NOTES on PlaneMeshLoader.h
    Case of numColumns = 3, numRows = 3.
    Plane Normal = {0.0f, 0.0f, -1.0f}.
    Center at 0,0,0
    Visible if the camera is at (0, 0, -10), with forward (0, 0, 1).
    0-4-8--12        
    |\|\|\ |      
    1-5-9--13        
    |\|\|\ |
    2-6-10-14         
    |\|\|\ |         
    3-7-11-15                                  
    */                          
    std::shared_ptr<Mesh> PlaneMeshLoader::Load(const std::string& filename,
        float width, uint32_t numColumns,
        float height, uint32_t numRows,
        XMFLOAT3 planeNormal,
        float red, float green, float blue) const
    {
        assert(numColumns > 0 && numRows > 0);
        std::shared_ptr<Mesh> retMesh = std::make_shared<Mesh>(filename);

        //Now, regarding Sigma, Point 0 is at angle 0, which is common across all steps in Theta. the same goes
        //for the last point in Sigma. We need to remove those points for an efficient sphere and avoid redundancies.
        //numPointsSigma -= 2;
        int numVertices = (numColumns + 1) * (numRows + 1);
        int numIndices = numColumns * numRows * 6;
        retMesh->Allocate(numVertices, numIndices);

        const float colWidth = width/numColumns;
        const float rowHeight= height/numRows;
        int vertIdx = 0;
        float x = -width * 0.5f;
        for (uint32_t colIdx = 0; colIdx <= numColumns; ++colIdx)
        {
            float y = height * 0.5f;
            for (uint32_t rowIdx = 0; rowIdx <= numRows; ++rowIdx)
            {
                float rndRed = red < 0.0f ? randFloat(0.0f, 1.0f) : red;
                float rndGreen = green < 0.0f ? randFloat(0.0f, 1.0f) : green;
                float rndBlue = blue < 0.0f ? randFloat(0.0f, 1.0f) : blue;
                retMesh->m_vertices[vertIdx] = { {  x,  y, 0}, {planeNormal.x, planeNormal.y, planeNormal.z},  {rndRed, rndGreen, rndBlue, 1.0f} };
                //ODINFO("idx=%d, Theta=%f, Sigma=%f, x=%f, y=%f, z=%f", vertIdx, theta, sigma, x, y, z);
                y -= rowHeight;
                ++vertIdx;
            }
            x += colWidth;
        }

        //Now, let's generate the indices.
        int colAstart = 0;
        int colBstart = numRows + 1;
        int idx = 0;
        for (uint32_t col = 0; col < numColumns; col++)
        {
            for (uint32_t row = 0; row < numRows; row++)
            {
                retMesh->m_indices[idx++] = colAstart;
                retMesh->m_indices[idx++] = colBstart + 1;
                retMesh->m_indices[idx++] = colAstart + 1;
                retMesh->m_indices[idx++] = colAstart;
                retMesh->m_indices[idx++] = colBstart;
                retMesh->m_indices[idx++] = colBstart+1;
                colAstart++;
                colBstart++;
            }
            colAstart++;
            colBstart++;
        }

        //Transform the vertices according to the desired plane normal.
        XMVECTOR defaultNormal = XMVectorSet(0, 0, -1, 0);
        XMVECTOR targetNormal = XMLoadFloat3(&planeNormal);
        targetNormal = XMVector3Normalize(targetNormal);

        //Let's get the cosine of the angle.
        XMVECTOR dot = XMVector3Dot(defaultNormal, targetNormal);
        XMVECTOR epsilon = XMVectorSet(0.0001, 0.0001, 0.0001, 0.0001);
        if (XMVector2NearEqual(dot, XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f), epsilon))
        {
            return retMesh; //No need to transform vertices.
        }
        XMMATRIX rotMatrix;
        if (XMVector2NearEqual(dot, XMVectorSet(-1.0f, -1.0f, -1.0f, 0.0f), epsilon))
        {
            rotMatrix = XMMatrixRotationNormal(XMVectorSet(1, 0, 0, 0), 3.14159f);
        }
        else
        {
            XMVECTOR rotAxis = XMVector3Cross(defaultNormal, targetNormal);
            rotAxis = XMVector3Normalize(rotAxis);
            XMVECTOR angle = XMVector3AngleBetweenNormals(defaultNormal, targetNormal);
            rotMatrix = XMMatrixRotationNormal(rotAxis, XMVectorGetX(angle));
        }
       
        for (int i = 0; i < numVertices; i++)
        {
            XMVECTOR v = XMLoadFloat3((const XMFLOAT3 *)&(retMesh->m_vertices[i].position[0]));
            v = XMVector3Transform(v, rotMatrix);
            XMStoreFloat3((XMFLOAT3 *)&(retMesh->m_vertices[i].position[0]), v);
#if 0
            XMVECTOR n = XMLoadFloat3((const XMFLOAT3 *)&(retMesh->m_vertices[i].normal[0]));
            n = XMVector3TransformNormal(n, rotMatrix);
            XMStoreFloat3((XMFLOAT3 *)&(retMesh->m_vertices[i].normal[0]), n);
#endif
        }
        
        return retMesh;
    }
};//namespace GAL