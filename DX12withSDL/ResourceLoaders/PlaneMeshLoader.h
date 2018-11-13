#pragma once

#include "ResourceLoader.hpp"

namespace DirectX
{
    struct XMFLOAT3;
}

namespace GAL
{
    class Mesh;


    class PlaneMeshLoader : public ResourceLoader<PlaneMeshLoader, Mesh>
    {
    public:
        enum class ePlaneNormal
        {

        };

        PlaneMeshLoader() = default;

        //WHAT YOU NEED TO KNOW.
        //The natural forward orientation in D3D12 is (0, 0, 1).
        //This means that when designing a plane, you are NOT by default facing
        //the default normal, because the plane and your face are looking in the
        //same direction.

        //So imaging the plane is centered at 0,0,0. Your camera location would be
        // let's say at (0, 0, -10), but the camera Forward vector should be (0, 0, 1).

        //We need the plane to be facing in (0, 0, -1).
        //In this design the default plane normal is facing you X = 0, y = 0, z = -1.
        //Columns would increase from left to right (From -X to +X).
        //Rows would increase from top to bottom(From +Y to -Y).
        //Negative color means random color.

        //Regarding  "planeNormal":
        std::shared_ptr<Mesh> Load(const std::string& filename,
            float width, uint32_t numColumns,
            float height, uint32_t numRows,
            DirectX::XMFLOAT3 planeNormal = {0.0f, 0.0f, -1.0f},
            float red = -1.0f, float green = -1.0f, float blue = -1.0f) const;
    };
} //namespace GAL;

