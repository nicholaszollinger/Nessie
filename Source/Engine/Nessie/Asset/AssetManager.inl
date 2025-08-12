// AssetManager.inl
#pragma once

namespace nes
{
    template <ValidAssetType Type>
    ELoadResult AssetManager::LoadSync(AssetID& id, const std::filesystem::path& path)
    {
        // Create the asset id if necessary:
        if (id == kInvalidAssetID)
        {
            id = GenerateAssetIDFromPath(path);
        }
        
        // If this Asset is already loaded, return.
        if (IsValidAsset(id))
        {
            return ELoadResult::Success;
        }

        // Load the Asset:
        AssetManager& assetManager = GetInstance();

#ifdef NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
        return assetManager.MainLoadSync<Type>(id, path);
#else
        if (IsMainThread())
            return assetManager.MainLoadSync<Type>(id, path);

        // Asset Thread:
        return assetManager.ThreadLoadSync<Type>(id, path);
#endif
    }

    template <ValidAssetType Type>
    ELoadResult AssetManager::LoadAsync(AssetID& id, const std::filesystem::path& path)
    {
#ifdef NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
        return LoadSync<Type>(id, std::filesystem::path(path));
#else
        
        // If we are already on the asset thread, Load synchronously.
        if (IsAssetThread())
        {
            NES_WARN(kAssetLogTag, "You should not call Load Async in an Asset's Load function. You should call LoadSync().");
            return LoadSync<Type>(id, path);
        }
        
        // Create the asset id if necessary:
        if (id == kInvalidAssetID)
        {
            id = GenerateAssetIDFromPath(path);
        }
        
        // If this Asset is already loaded, return.
        if (IsValidAsset(id))
        {
            return ELoadResult::Success;
        }

        AssetManager& assetManager = GetInstance();
        return assetManager.QueueLoadAsset<Type>(id, path);
#endif
    }

    template <ValidAssetType Type>
    ELoadResult AssetManager::AddMemoryAsset(AssetID& id, Type&& asset)
    {
        AssetManager& assetManager = GetInstance();

        // Ensure that this is not a valid AssetID to begin with.
        NES_ASSERT(!IsValidAsset(id));

        // Create the asset id if necessary:
        if (id == kInvalidAssetID)
        {
            id = GenerateAssetID();
        }

        return assetManager.AddMemoryAssetImpl<Type>(id, std::forward<Type>(asset));
    }

    template <ValidAssetType Type>
    AssetPtr<Type> AssetManager::GetAsset(const AssetID& id)
    {
        NES_ASSERT(IsMainThread(), "Assets can only be accessed on the Main Thread!");
        
        AssetManager& assetManager = GetInstance();
        if (auto it = assetManager.m_infoMap.find(id); it != assetManager.m_infoMap.end())
        {
            NES_ASSERT(it->second.m_typeID == Type::GetStaticTypeID());

            // If we are queued to free, but requested, cancel the free operation.
            // Because we only get assets from the main thread, we won't be freeing an asset while
            // changing this flag.
            if (it->second.m_state == EAssetState::Freeing)
            {
                it->second.m_state = EAssetState::Loaded;
            }
            
            // If not loaded, then return nullptr.
            else if (it->second.m_state != EAssetState::Loaded)
            {
                return nullptr;
            }

            // Otherwise, return a pointer to the asset.
            return AssetPtr<Type>(checked_cast<Type*>(assetManager.m_loadedAssets[id]));
        }

        return nullptr;
    }

    template <ValidAssetType Type>
    ELoadResult AssetManager::MainLoadSync(const AssetID& id, const std::filesystem::path& path)
    {
        // Should be valid at this point.
        NES_ASSERT(id != kInvalidAssetID);
        NES_ASSERT(IsMainThread());
        
        // Create the initial entry:
        m_infoMap[id] = AssetInfo(Type::GetStaticTypeID(), EAssetState::Loading, ELoadResult::Invalid);
        m_threadInfoMapNeedsSync = true;
        
        // Load the Asset: 
        AssetBase* pAsset = NES_NEW(Type()); //&assetPool.Get(handle);
        const ELoadResult result = pAsset->LoadFromFile(path);

        // Update the loaded asset map, including the result.
        auto& info = m_infoMap.at(id);
        info.m_loadResult = result;
        info.m_state = EAssetState::Loaded;
        
        if (result != ELoadResult::Success)
        {
            NES_SAFE_DELETE(pAsset);
            return result;
        }

        // Update the loaded assets map and set the Asset's ID.
        m_loadedAssets[id] = pAsset;
        pAsset->m_id = id;
        
        return ELoadResult::Success;
        
    }

    template <ValidAssetType Type>
    ELoadResult AssetManager::ThreadLoadSync(const AssetID& id, const std::filesystem::path& path)
    {
        NES_ASSERT(IsAssetThread());

        // Create the asset on the heap and load:
        AssetBase* pAsset = NES_NEW(Type());
        const ELoadResult result = pAsset->LoadFromFile(path);

        // Add this to the loaded asset map for the thread, regardless of the result.
        // This ensures that we don't try to load this asset multiple times on the asset thread.
        {
            std::lock_guard lock(m_threadInfoMapMutex);
            m_threadInfoMap.emplace(id, AssetInfo(Type::GetStaticTypeID(), EAssetState::Loaded, result));
        }

        // Update the load result info based on the result:
        const uint32 loadResultIndex = m_resultWriteIndex.load(std::memory_order_relaxed);
        auto& loadResult = m_results[loadResultIndex];
            
        if (result != ELoadResult::Success)
        {
            loadResult.m_result = result;
        }
            
        // Update the number of memory assets for the whole operation:
        loadResult.m_numMemoryAssets += 1;
            
        // Add the loaded memory asset to thread's buffer to be synced with the main thread.
        // The asset will be freed if it failed to load.
        {
            std::lock_guard lock(m_threadMemoryAssetsMutex);
            m_threadMemoryAssets.emplace_back(pAsset, id, Type::GetStaticTypeID(), loadResultIndex, result);
        }

        return result;
    }

    template <ValidAssetType Type>
    ELoadResult AssetManager::QueueLoadAsset(AssetID& id, const std::filesystem::path& path)
    {
        // Should be valid at this point.
        NES_ASSERT(id != kInvalidAssetID);

        // Create the initial entry:
        m_infoMap[id] = AssetInfo(Type::GetStaticTypeID(), EAssetState::Loading, ELoadResult::Invalid);
        m_threadInfoMapNeedsSync = true;
        
        ThreadLoadFunc func = [&id, path]()
        {
            return AssetManager::LoadSync<Type>(id, path);
        };
        m_threadJobQueue.EnqueueLocked(func);

        // [TODO]: What should be returned here?
        return ELoadResult::Success;   
    }

    template <ValidAssetType Type>
    ELoadResult AssetManager::AddMemoryAssetImpl(const AssetID& id, Type&& asset)
    {
        // Create the new Asset:
        // Set the handle and ID of the asset.
        AssetBase* pAsset = NES_NEW(Type(std::forward<Type>(asset))); //&assetPool.Get(handle);
        pAsset->m_id = id;

        // Update the id to info map. The Asset can now be returned by GetAsset().
        m_infoMap[id] = AssetInfo(Type::GetStaticTypeID(), EAssetState::Loaded, ELoadResult::Success);

        return ELoadResult::Success;
    }
    
    template <ValidAssetType Type>
    AssetPtr<Type>::AssetPtr(const AssetPtr& other)
        : m_pAsset(other.m_pAsset)
        , m_id(other.m_id)
    {
        AddLock();
    }

    template <ValidAssetType Type>
    AssetPtr<Type>::AssetPtr(AssetPtr&& other) noexcept
        : m_pAsset(other.m_pAsset)
        , m_id(other.m_id)
    {
        other.m_pAsset = nullptr;
        other.m_id = kInvalidAssetID;
    }

    template <ValidAssetType Type>
    AssetPtr<Type>& AssetPtr<Type>::operator=(const AssetPtr& other)
    {
        if (this != &other && m_pAsset != other.m_pAsset)
        {
            RemoveLock();
            m_id = other.m_id;
            m_pAsset = other.m_pAsset;
            AddLock();
        }

        return *this;
    }

    template <ValidAssetType Type>
    AssetPtr<Type>& AssetPtr<Type>::operator=(AssetPtr&& other) noexcept
    {
        if (this != &other)
        {
            RemoveLock();
            m_id = other.m_id;
            m_pAsset = other.m_pAsset;
            
            other.m_pAsset = nullptr;
            other.m_id = kInvalidAssetID;
        }

        return *this;
    }

    template <ValidAssetType Type>
    AssetPtr<Type>& AssetPtr<Type>::operator=(std::nullptr_t)
    {
        RemoveLock();
        m_id = kInvalidAssetID;
        m_pAsset = nullptr;

        return *this;
    }

    template <ValidAssetType Type>
    template <ValidAssetType OtherType> requires TypeIsBaseOrDerived<Type, OtherType>
    AssetPtr<OtherType> AssetPtr<Type>::Cast() const
    {
        return checked_cast<OtherType*>(m_pAsset);
    }

    template <ValidAssetType Type>
    AssetPtr<Type>::AssetPtr(Type* pAsset)
        : m_pAsset(pAsset)
    {
        // Set the ID, if valid.
        if (m_pAsset != nullptr)
            m_id = pAsset->GetAssetID();

        // Add a reference to the Asset.
        AddLock();
    }

    template <ValidAssetType Type>
    void AssetPtr<Type>::AddLock() const
    {
        if (m_pAsset)
        {
            NES_ASSERT(IsValid());
            m_pAsset->AddLock();
        }
    }

    template <ValidAssetType Type>
    void AssetPtr<Type>::RemoveLock() const
    {
        if (m_pAsset)
        {
            NES_ASSERT(IsValid());
            m_pAsset->RemoveLock();
        }
    }
}
