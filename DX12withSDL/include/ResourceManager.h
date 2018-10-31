#pragma once

namespace GAL
{
    class Mesh;
    
    using MeshCache = entt::resource_cache<Mesh>;

    class ResourceManager
    {
    public:
        ResourceManager();
        ~ResourceManager();

        static MeshCache& GetMeshCache() { return m_meshCache; }

    private:
        static MeshCache m_meshCache;
    };
}; //namespace GAL

