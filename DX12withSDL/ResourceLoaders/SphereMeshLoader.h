#pragma once

#include "ResourceLoader.hpp"

namespace GAL
{
    class Mesh;

    class SphereMeshLoader : public ResourceLoader<SphereMeshLoader, Mesh>
    {
    public:
        SphereMeshLoader() = default;
        std::shared_ptr<Mesh> Load(const std::string& filename, float radius, float red, float green, float blue) const;
    };
} //namespace GAL;

