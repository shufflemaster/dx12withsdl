#pragma once

#include "ResourceLoader.hpp"

namespace GAL
{
    template<typename ResourceType>
    class ResourceBase;

    //-----------------------------------------------------------------------------
    // Resource Manager Class
    //-----------------------------------------------------------------------------
    template<typename ResourceType>
    class ResourceCache
    {
    public:
        //-------------------------------------------------------------------------
        // Use default constructor
        //-------------------------------------------------------------------------
        ResourceCache() = default;

        /*! @brief Copying a cache isn't allowed. */
        ResourceCache(const ResourceCache &) = delete;

        /*! @brief Default move constructor. */
        ResourceCache(ResourceCache &&) noexcept = default;

        //Creates, Loads the resource and adds it to the cache using a custom ResourceLoader
        //Returns in newHandle the resource handle.
        template<typename Loader, typename... Args>
        ResourceHandle AddResource(const std::string& name, Args &&... args) {
            static_assert(std::is_base_of_v<ResourceLoader<Loader, ResourceType>, Loader>);
            static_assert(std::is_base_of_v<ResourceBase<ResourceType>, ResourceType>);

            //Do we have already a resource with such name?
            auto itor = m_nameToHandle.find(name);
            if (itor != m_nameToHandle.end())
            {
                //Already exists
                return itor->second;
            }

            std::shared_ptr<ResourceType> resource = Loader{}.Load(name, std::forward<Args>(args)...);
            if (resource == nullptr)
            {
                return -1;
            }

            //Find the next available Handle
            bool handleAvailable = !m_handles.empty();
            ResourceHandle handle;
            if (handleAvailable)
            {
                handle = m_handles.top();
                m_handles.pop();
            }
            else handle = static_cast<ResourceHandle>(m_collection.size());

            //Add the resource to the manager. If there is an available handle, then 
            //we store the resource using the handle. Otherwise we add it to the vector.
            if (handleAvailable)
                m_collection[handle] = resource;
            else
                m_collection.push_back(resource);

            resource->m_handle = handle;
            m_nameToHandle[name] = handle;
            
            return handle;
        }

        //-------------------------------------------------------------------------
        // Clears the resource list.
        //-------------------------------------------------------------------------
        void EmptyCache()
        {
            while (!m_handles.empty())
                m_handles.pop();

            m_collection.clear();
            m_nameToHandle.clear();
        }

        //-------------------------------------------------------------------------
        // Removes the given resource from the manager.
        //-------------------------------------------------------------------------
        void Remove(ResourceHandle handle)
        {
            // Ensure the resource to be removed and the list is valid.
            if (handle < 0 || handle >= m_collection.size() || m_collection[handle] == nullptr)
                return;

            std::shared_ptr<ResourceType> resPtr = m_collection[handle];
            auto itor = m_nameToHandle.find(resPtr->m_filename);
            if (itor != m_nameToHandle.end())
            {
                m_nameToHandle.erase(itor);
            }

            m_collection[handle] = nullptr;
            //add the handle to the stack of free handles
            m_handles.push(handle);
        }

        //-------------------------------------------------------------------------
        // Returns a resource by its handle.
        //-------------------------------------------------------------------------
        std::shared_ptr<ResourceType> GetResource(ResourceHandle handle)
        {
            if ((handle >= 0) && (handle < m_collection.size()))
                return m_collection[handle];

            return nullptr;
        }

        //-------------------------------------------------------------------------
        // Overloaded [] operator for easier access
        //-------------------------------------------------------------------------
        std::shared_ptr<ResourceType> operator [](ResourceHandle handle)
        {
            if ((handle >= 0) && (handle < m_collection.size()))
                return m_collection[handle];

            return nullptr;
        }

        bool HasResource(const std::string& resName)
        {
            auto itor = m_nameToHandle.find(resName);
            return (itor != m_nameToHandle.end());
        }

        ResourceHandle GetHandle(const std::string& resName)
        {
            auto itor = m_nameToHandle.find(resName);
            if (itor == m_nameToHandle.end())
                return -1;
            return itor->second;
        }

    private:
        std::stack<ResourceHandle> m_handles;
        std::vector<std::shared_ptr<ResourceType>> m_collection;	//collection of resources
        std::unordered_map<std::string, ResourceHandle> m_nameToHandle;
    };

}; // namespace GAL;