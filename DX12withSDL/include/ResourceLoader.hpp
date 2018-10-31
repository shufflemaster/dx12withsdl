#pragma once

namespace GAL
{
    class ResourceCache;

    template<typename Loader, typename ResourceType>
    class ResourceLoader
    {
        /*! @brief Resource loaders are friends of their caches. */
        friend class ResourceCache<ResourceType>;

        template<typename... Args>
        std::shared_ptr<ResourceType> Load(const std::string& filename, Args &&... args) const {
            return static_cast<const Loader *>(this)->Load(filename, std::forward<Args>(args)...);
        }
    };
}; //namespace GAL