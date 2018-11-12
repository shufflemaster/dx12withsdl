#pragma once

#include <DirectXMath.h>

using namespace DirectX;

//helper for LoadLevel
namespace GAL
{
    static inline float randFloat(float min, float max)
    {
        int random_number = std::rand();
        float normalized = (float)random_number / (float)RAND_MAX;
        float retVal = (max - min)*normalized + min;
        return retVal;
    }

    //helper for LoadLevel
    static inline void GenerateRandomWorldMatrix(XMFLOAT4X4& worldMatOut,
        float minX, float maxX,
        float minY, float maxY,
        float minZ, float maxZ)
    {
        XMMATRIX tmpMat = XMMatrixTranslation(randFloat(minX, maxX), randFloat(minY, maxY), randFloat(minZ, maxZ));
        XMStoreFloat4x4(&worldMatOut, tmpMat);
    }

};//namespace GAL
