#pragma once

#include "ResourceLoader.hpp"

namespace GAL
{
    class GpuMesh;

    class GpuMeshLoader : public ResourceLoader<GpuMeshLoader, GpuMesh>
    {
    public:
        GpuMeshLoader() = default;
        std::shared_ptr<GpuMesh> Load(const std::string& filename,
            ID3D12Device* device, ID3D12CommandQueue* commandQueue,
            const P3F_C4F* vertices, uint32_t numVertices, const DWORD* indices, uint32_t numIndices) const;
    };
} //namespace GAL;

