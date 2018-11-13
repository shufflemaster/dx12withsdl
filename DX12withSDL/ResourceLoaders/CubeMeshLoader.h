#pragma once

#include "ResourceLoader.hpp"

namespace DirectX
{
    struct XMFLOAT3;
}

namespace GAL
{
    class Mesh;

    class CubeMeshLoader : public ResourceLoader<CubeMeshLoader, Mesh>
    {
    public:
        CubeMeshLoader() = default;

        //WHAT YOU NEED TO KNOW.
        //The cube is centered at 0,0,0
        //Face0 normal is 0, 0, 1
        //Face1 normal is 1, 0, 0
        //Face2 normal is 0, 0, -1
        //Face3 normal is -1, 0, 0
        //Face4 normal is 0, 1, 0
        //Face5 normal is 0, -1, 0.
        //@param width: How large in X.
        //@param numColumnsX: How many columns in X dimesion.
        //@param height: How large in Y.
        //@param numRowsY: How many rows in Y dimesion.
        //@param depth: How large in Z.
        //@param numColumnsZ: How many columns in Z dimension.
        //@param faceColors: colors for face 0, 1, 2... 5. nullptr means use random colors.
        //@param numFaceColors: How many faces are in faceColors.
        //                      0 means use random colors, max value is 6.
        std::shared_ptr<Mesh> Load(const std::string& filename,
            float width = 1.0f, uint32_t numColumnsX = 1,
            float height = 1.0f, uint32_t numRowsY = 1,
            float depth = 1.0f, uint32_t numColumnsZ = 1,
            const DirectX::XMFLOAT3* faceColors = nullptr, int numFaceColors = 0 ) const;
    };
} //namespace GAL;

