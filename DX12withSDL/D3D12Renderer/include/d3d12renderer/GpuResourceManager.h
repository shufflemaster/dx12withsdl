#pragma once

#include "ResourceCache.hpp"

namespace GAL
{
    class GpuMesh;

    using GpuMeshCache = ResourceCache<GpuMesh>;

    class GpuResourceManager
    {
    public:
        GpuResourceManager();
        ~GpuResourceManager();

        GpuMeshCache& GetGpuMeshCache() { return m_gpuMeshCache; }

        void Clear();

    private:
        GpuMeshCache m_gpuMeshCache;
    };
}; //namespace GAL

