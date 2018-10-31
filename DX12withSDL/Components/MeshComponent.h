#pragma once

namespace GAL
{
    class Mesh;

    class MeshComponent
    {
    public:
        MeshComponent();
        ~MeshComponent();

        std::string m_meshName; //Resource Filename

        //To get the resource (Mesh*)
        //Mesh* m = GAL::ResourceManager::GetMeshCache().handle(m_meshId).get()
        entt::hashed_string::hash_type m_meshId;
    };
}; //namespace GAL
