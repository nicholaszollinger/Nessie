// AssetManager.h
#pragma once
#include "Nessie/Core/Memory/StrongPtr.h"
#include "AssetPool.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    // [TODO]: This needs to be multithreaded. Right now, this is all synchronous loading, which does the job.
    //
    /// @brief : Manages the lifetime of assets. Assets are explicitly loaded and freed.
    ///     Provides a static API for loading and unloading assets.
    ///     If you know the rough number needed for a specific type of asset, you can use InitializePool()
    ///     to set the size that you need. Pools will default to holding a max of 64 assets.
    //----------------------------------------------------------------------------------------------------
    class AssetManager
    {
    public:
        AssetManager();
        ~AssetManager();
        AssetManager(const AssetManager&) = delete;
        AssetManager& operator=(const AssetManager&) = delete;
        AssetManager(AssetManager&&) = delete;
        AssetManager& operator=(AssetManager&&) = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load an asset from a filepath path (if necessary) synchronously. Returns an invalid handle
        ///     if loading failed.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        static StrongPtr<Type>      LoadAssetFromPath(const std::filesystem::path& path);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add an asset that was created during runtime.
        /// @param asset : The created asset to store in the AssetManager.
        /// @param overrideID : If no id is provided, one will be generated.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        static StrongPtr<Type>      AddMemoryAsset(Type&& asset, AssetID overrideID = kInvalidAssetID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a loaded asset. If the asset has not been loaded, this will return nullptr.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        static StrongPtr<Type>      GetAsset(const AssetID& id);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free an asset associated with the given id.
        //----------------------------------------------------------------------------------------------------
        static void                 FreeAsset(const AssetID& id);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Generates a new, unique asset id. 
        //----------------------------------------------------------------------------------------------------
        static AssetID              GenerateAssetID();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Generate an asset id from a path. 
        //----------------------------------------------------------------------------------------------------
        static constexpr AssetID    GenerateAssetIDFromPath(const std::filesystem::path& path) { return HashString64(path.string().c_str()); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the static instance of the AssetManager. You only need this if you are managing the
        ///     AssetManager itself.
        //----------------------------------------------------------------------------------------------------
        static AssetManager&        Get();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize an asset pool with the given size constraints.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        void                        InitializePool(const AssetPoolCreateInfo& info);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Shutdown the AssetManager. This will destroy all loaded assets.
        //----------------------------------------------------------------------------------------------------
        void                        Shutdown();

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the asset pool of the associated type. 
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        AssetPool<Type>&            GetAssetPool();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Info about a created asset.
        //----------------------------------------------------------------------------------------------------
        struct AssetInfo
        {
            TypeID      m_typeID = 0;
            AssetHandle m_handle = kInvalidAssetHandle;
        };
        
        std::unordered_map<TypeID, AssetPoolBase*> m_assetPools{}; 
        std::unordered_map<AssetID, AssetInfo, UUIDHasher> m_idToInfoMap{};
    };
}

#include "AssetManager.inl"
