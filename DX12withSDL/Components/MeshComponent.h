#pragma once

namespace GAL
{
    class MeshComponent
    {
    public:
        MeshComponent() : m_meshHandle(-1) {}
        ~MeshComponent();

        //Defined at design time.
        std::string m_filename;

        //Assigned realtime from the ResourceManager::MeshCache
        ResourceHandle m_meshHandle;
    };
}; //namespace GAL
