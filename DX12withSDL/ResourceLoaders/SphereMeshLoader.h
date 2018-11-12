#pragma once

#include "ResourceLoader.hpp"

namespace GAL
{
    class Mesh;

    class SphereMeshLoader : public ResourceLoader<SphereMeshLoader, Mesh>
    {
    public:
        SphereMeshLoader() = default;

        //Negative color means random color.
        std::shared_ptr<Mesh> Load(const std::string& filename, float radius, float stepSizeDegrees, float red = -1.0f, float green = -1.0f, float blue = -1.0f) const;
    };
} //namespace GAL;

