#include "pch.h"

#include "Universe.h"
#include "ResourceManager.h"
#include "RenderSystem.h"

#include "../ResourceTypes/Mesh.h"
#include "../Components/TransformComponent.h"
#include "../Components/MeshComponent.h"
#include "../Components/RendererIdComponent.h"

#include "d3d12renderer/GpuResourceTypes/GpuMesh.h"
#include "d3d12renderer/GpuResourceLoaders/GpuMeshLoader.h"

namespace GAL
{
    RenderSystem::~RenderSystem()
    {
    }

    void RenderSystem::Initialize()
    {
        assert(m_renderer != nullptr);

        //For optimization reasons this prepare could be done even before adding entities.
        Registry& registry = m_universe->GetRegistry();
        registry.prepare_persistent_view<TransformComponent, MeshComponent>();

        GpuMeshCache& gpuMeshCache = m_renderer->GetGpuResourceManager().GetGpuMeshCache();
        MeshCache& cpuMeshCache = m_universe->GetResourceManager().GetMeshCache();

        //Loop through all the entities with transform and mesh components. Request the renderer to create a render node for each one.
        auto persView = registry.persistent_view<TransformComponent, MeshComponent>();
        for (auto entity : persView)
        {
            TransformComponent& tc = persView.get<TransformComponent>(entity);
            MeshComponent& mc = persView.get<MeshComponent>(entity);

            //Create the GpuMeshHandle resource.
            ResourceHandle gpuMeshHandle = gpuMeshCache.GetHandle(mc.m_filename);
            if (gpuMeshHandle < 0)
            {
                //If we are here, the GpuMesh has not been created.
                //Let's get the Mesh resource itself
                std::shared_ptr<Mesh> mesh = cpuMeshCache.GetResource(mc.m_meshHandle);
                //Let's create the resurce
                gpuMeshHandle = gpuMeshCache.AddResource<GpuMeshLoader>(mesh->GetFilename(), m_renderer->GetDevice(), m_renderer->GetCommandQueue(),
                    mesh->GetVertices(), mesh->GetNumVertices(), mesh->GetIndices(), mesh->GetNumIndices());
                if (gpuMeshHandle < 0)
                {
                    ODERROR("Failed to create GpuMesh for mesh %s", mesh->GetFilename().c_str());
                    return;
                }
            }

            RenderEntityId rendererId = m_renderer->AddRenderNode(gpuMeshHandle, tc.m_matrix);
            //Store the RenderEntityId in the engine entity.
            auto& riComponent = registry.assign<RendererIdComponent>(entity);
            riComponent.m_rendererEntityId = rendererId;
        }

    }
};
