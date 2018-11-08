#include "pch.h"

#include "d3d12renderer/GpuResourceManager.h"

namespace GAL
{
    GpuResourceManager::GpuResourceManager()
    {
    }

    GpuResourceManager::~GpuResourceManager()
    {
    }

    void GpuResourceManager::Clear()
    {
        m_gpuMeshCache.EmptyCache();
    }

}; //namespace GAL