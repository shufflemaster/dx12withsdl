#pragma once

#include "ResourceLoader.hpp"

namespace GAL
{
    class Mesh;

    class TriangleMeshLoader : public ResourceLoader<TriangleMeshLoader, Mesh>
    {
    public:
        TriangleMeshLoader() = default;
        std::shared_ptr<Mesh> Load(const std::string& filename, float size, float red, float green, float blue) const;
    };
} //namespace GAL;

