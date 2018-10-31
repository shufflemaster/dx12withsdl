#pragma once

#include "ResourceCache.hpp"
#include "ResourceTypes\Mesh.h"

namespace GAL
{
    using MeshCache = ResourceCache<Mesh>;

    class ResourceManager
    {
    public:
        ResourceManager();
        ~ResourceManager();

        MeshCache& GetMeshCache() { return m_meshCache; }

    private:
        MeshCache m_meshCache;
    };
}; //namespace GAL

