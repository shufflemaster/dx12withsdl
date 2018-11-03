#pragma once

namespace GAL
{
    template<typename ResourceType>
    class ResourceCache;

    //-------------------------------------------------------------------------
    // The Resource class.
    //-------------------------------------------------------------------------
    template<typename ResourceType>
    class ResourceBase
    {
        friend class ResourceCache<ResourceType>;

    protected:
        //-------------------------------------------------------------------------
        // The Resource class constructor.
        // m_handle starts invalid, but eventually ResourceCache will give it a value.
        //-------------------------------------------------------------------------
        ResourceBase(const std::string& filename) :
            m_handle(-1), m_filename(filename)
        {
        }

    public:
        ResourceBase() = default;
        ResourceBase(const ResourceBase &) = default;
        ResourceBase(ResourceBase &&) = default;


        //-------------------------------------------------------------------------
        // Returns the complete path filename of the resource.
        //-------------------------------------------------------------------------
        const std::string& GetFilename()
        {
            return m_filename;
        }

        //-------------------------------------------------------------------------
        // Returns the handle of the resource.
        //-------------------------------------------------------------------------
        ResourceHandle GetHandle()
        {
            return m_handle;
        }

    private:
        ResourceHandle m_handle; //Filled by the ResourceCache<ResourceType>
        std::string	m_filename;
    };

}; //namespace GAL