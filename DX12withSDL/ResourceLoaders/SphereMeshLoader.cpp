#include "pch.h"

#include "..\ResourceTypes\Mesh.h"
#include "TriangleMeshLoader.h"
#include "VertexTypes.h"

namespace GAL
{
    //{\displaystyle (x-x_{0})^{2}+(y-y_{0})^{2}+(z-z_{0})^{2}=r^{2}.}
    //(x, y,z) = (r*cosTheta*sinSigma, r*sinTheta*sinSigma, r*cosSigma)
    //Theta [0, 2pi), Sigma [0, pi]
    std::shared_ptr<Mesh> TriangleMeshLoader::Load(const std::string& filename, float radius, float stepSizeDegrees, float red, float green, float blue) const
    {
        std::shared_ptr<Mesh> retMesh = std::make_shared<Mesh>(filename);

        retMesh->Allocate(3, 3);
        
        retMesh->m_vertices[0] = { {  0.0f,  size, 0.0f}, {red, green, blue, 1.0f} };
        retMesh->m_vertices[1] = { {  size, -size, 0.0f}, {red, green, blue, 1.0f} };
        retMesh->m_vertices[2] = { { -size, -size, 0.0f}, {red, green, blue, 1.0f} };
        
        retMesh->m_indices[0] = 0;
        retMesh->m_indices[1] = 1;
        retMesh->m_indices[2] = 2;

        //Calculate steps around Theta.
        int numPointsTheta = (int)(360.0f / stepSizeDegrees);
        int numPointsSigma = (int)(180.0f / stepSizeDegrees) + 1;

        //Now, regarding Sigma, Point 0 is at angle 0, which is common across all steps in Theta. the same goes
        //for the last point in Sigma. We need to remove those points for an efficient sphere and avoid redundancies.
        //numPointsSigma -= 2;

        int numVertices = (numPointsSigma * numPointsTheta);// +2; //The last 2 points is for 0deg and 180deg.
        int numIndices = (numPointsSigma - 1) * 6 * (numPointsTheta - 1);// +(3 * numPointsTheta) * 2;
        retMesh->Allocate(numVertices, numIndices);

        float theta = 0.0f;
        const float stepSizeRads = stepSizeDegrees * (3.14159 / 180.0f);
        for (int cntTheta = 0; cntTheta < numPointsTheta; ++cntTheta)
        {
            float sigma = stepSizeRads;
            int vertIdx = numPointsSigma * cntTheta;
            for (int cntSigma = 0; cntSigma < numPointsSigma; ++cntSigma, ++vertIdx)
            {
                float x = radius * cosf(theta) * sinf(sigma);
                float y = radius * sinf(theta) * sinf(sigma);
                float z = radius * cosf(sigma);
                retMesh->m_vertices[vertIdx] = { {  x,  y, z}, {red, green, blue, 1.0f} };
            }
        }

        //Now, let's generate the indices.


        return retMesh;
    }
};//namespace GAL