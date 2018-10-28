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

        static entt::hashed_string GetResourceId(const char * resourcePath) { return entt::hashed_string{ resourcePath }; }
        static MeshCache& GetMeshCache() { return m_meshCache; }

    private:
        static MeshCache m_meshCache;
    };
}; //namespace GAL

