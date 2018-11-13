#include "pch.h"

#include "../ResourceTypes/Mesh.h"
#include "SphereMeshLoader.h"
#include "VertexTypes.h"
#include "LogUtils.h"
#include "../CommonHelpers.h"

namespace GAL
{
    /*                                            +Z
    Case of stepSizeDegrees = 60                  |
    0-4-8--12-16-20-0                             0
    |\|\|\ |\ |\ |\ |                          /  |  \
    1-5-9--13-17-21-1                         20  |   4
    |\|\|\ |\ |\ |\ |        top view:  -X ---|---|---|--- +X 
    2-6-10-14-18-22-2                         16  |   8 
    |\|\|\ |\ |\ |\ |                          \  |  /   
    3-7-11-15-19-23-3                             12
                                                  | 
                                                  -Z
    */                          
    //{\displaystyle (x-x_{0})^{2}+(y-y_{0})^{2}+(z-z_{0})^{2}=r^{2}.}
    //(x, y,z) = (r*cosTheta*sinSigma, r*sinTheta*sinSigma, r*cosSigma)
    //Theta [0, 2pi), Sigma [0, pi]
    std::shared_ptr<Mesh> SphereMeshLoader::Load(const std::string& filename, float radius, float stepSizeDegrees, float red, float green, float blue) const
    {
        std::shared_ptr<Mesh> retMesh = std::make_shared<Mesh>(filename);

        //Calculate steps around Theta.
        int numColsTheta = (int)(360.0f / stepSizeDegrees);
        int numRowsSigma = (int)(180.0f / stepSizeDegrees);

        //Now, regarding Sigma, Point 0 is at angle 0, which is common across all steps in Theta. the same goes
        //for the last point in Sigma. We need to remove those points for an efficient sphere and avoid redundancies.
        //numPointsSigma -= 2;
        int numVertices = numColsTheta * (numRowsSigma + 1);
        int numIndices = numColsTheta * numRowsSigma * 6;
        retMesh->Allocate(numVertices, numIndices);

        float theta = 0.0f;
        const float stepSizeRads = stepSizeDegrees * (3.14159f / 180.0f);
        int vertIdx = 0;
        for (int cntTheta = 0; cntTheta < numColsTheta; ++cntTheta)
        {
            float sigma = 0;
            for (int cntSigma = 0;
                cntSigma <= numRowsSigma;
                ++cntSigma)
            {
                float nX = sinf(sigma)*sinf(theta);
                float nY = cosf(sigma);
                float nZ = sinf(sigma)*cosf(theta);
                float x = radius * nX;
                float y = radius * nY;
                float z = radius * nZ;
                float rndRed = red < 0.0f ? randFloat(0.0f, 1.0f) : red;
                float rndGreen = green < 0.0f ? randFloat(0.0f, 1.0f) : green;
                float rndBlue = blue < 0.0f ? randFloat(0.0f, 1.0f) : blue;
                retMesh->m_vertices[vertIdx] = { {  x,  y, z}, { nX, nY, nZ}, {rndRed, rndGreen, rndBlue, 1.0f} };
                //ODINFO("idx=%d, Theta=%f, Sigma=%f, x=%f, y=%f, z=%f", vertIdx, theta, sigma, x, y, z);
                sigma += stepSizeRads;
                ++vertIdx;
            }
            theta += stepSizeRads;
        }

        //Now, let's generate the indices.
        int colAstart = 0;
        int colBstart = numRowsSigma + 1;
        int idx = 0;
        for (int col = 0; col < numColsTheta; col++)
        {
            for (int row = 0; row < numRowsSigma; row++)
            {
                retMesh->m_indices[idx++] = colAstart;
                retMesh->m_indices[idx++] = colAstart + 1;
                retMesh->m_indices[idx++] = colBstart + 1;
                retMesh->m_indices[idx++] = colAstart;
                retMesh->m_indices[idx++] = colBstart + 1;
                retMesh->m_indices[idx++] = colBstart;
                colAstart++;
                colBstart++;
            }
            colAstart++;
            colBstart++;
            if (colBstart >= numVertices)
            {
                colBstart = 0;
            }
        }

        return retMesh;
    }
};//namespace GAL