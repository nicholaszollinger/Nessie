// AssetManager.inl
#pragma once

namespace nes
{
    template <ValidAssetType Type>
    void LoadRequest::AppendLoad(AssetID& id, const std::filesystem::path& path)
    {
        NES_ASSERT(m_pAssetManager != nullptr);

        // Create the asset id if necessary:
        if (id == kInvalidAssetID)
        {
            id = GenerateAssetIDFromPath(path);
        }

#ifdef NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
        ThreadLoadFunc func = [&id, path]()
        {
            return AssetManager::LoadSync<Type>(id, path);
        };
#else
        // If we don't have an entry already, create one.
        if (!m_pAssetManager->GetAssetInfo(id))
        {
            //m_pAssetManager->m_infoMap[id] = AssetInfo(kInvalidLoadIndex, Type::GetStaticTypeID(), EAssetState::Invalid, ELoadResult::Pending);
            m_pAssetManager->m_infoMap[id] = AssetInfo(kInvalidLoadIndex, Type::GetStaticTypeID(), EAssetState::Loading, ELoadResult::Pending);
        }
        
        AssetManager* pAssetManager = m_pAssetManager;
        LoadRequestID requestId = m_requestId;
        ThreadLoadFunc func = [pAssetManager, requestId, &id, path]()
        {
            return pAssetManager->ThreadLoadSync<Type>(id, path, requestId);  
        };
#endif  

        AddJob(func);
    }

    template <ValidAssetType Type>
    ELoadResult AssetManager::LoadSync(AssetID& id, const std::filesystem::path& path)
    {
        // Create the asset id if necessary:
        if (id == kInvalidAssetID)
        {
            id = GenerateAssetIDFromPath(path);
        }
        
        AssetManager& instance = GetInstance();

#ifdef NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
        return instance.MainLoadSync<Type>(id, path);
#else
        if (IsMainThread())
            return instance.MainLoadSync<Type>(id, path);

        // Asset Thread:
        return instance.ThreadLoadSync<Type>(id, path);
#endif
    }
    
    template <ValidAssetType Type>
    void AssetManager::LoadAsync(AssetID& id, const std::filesystem::path& path, const LoadRequest::OnComplete& onComplete)
    {
#ifdef NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
        const ELoadResult result = LoadSync<Type>(id, std::filesystem::path(path));
        if (onComplete)
        {
            onComplete(result);
        }
#else
        // Make sure this is the main thread.
        NES_ASSERT(IsMainThread(), "LoadAsync() can only be called on the Main thread! You should call LoadSync() in an Asset's Load function, because it is a synchronous operation on the Asset Thread");

        // Create the asset id if necessary:
        if (id == kInvalidAssetID)
        {
            id = GenerateAssetIDFromPath(path);
        }

        AssetManager& instance = GetInstance();
        
        // Determine if we need to perform the load:
        if (AssetInfo* pInfo = instance.GetAssetInfo(id))
        {
            switch (pInfo->m_state)
            {
                // We have finished or if we were freeing the asset, mark it as Loaded (this will make sure it isn't freed)
                // and call the OnComplete callback immediately. 
                case EAssetState::Loaded:
                case EAssetState::Freeing:
                {
                    pInfo->m_state = EAssetState::Loaded;

                    if (onComplete)
                    {
                        onComplete(pInfo->m_loadResult);
                    }
                    return;
                }

                // If invalid or freed, perform the load.
                // We still make a request if we are loading so that the callbacks are handled appropriately.
                case EAssetState::Loading:
                case EAssetState::Invalid:
                case EAssetState::Freed:
                    break;
            }
        }
        
        // Submit a new load request with a single asset.
        LoadRequest request = BeginLoadRequest();
        request.SetOnCompleteCallback(onComplete);
        request.AppendLoad<Type>(id, path);
        SubmitLoadRequest(std::move(request));
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
        
        // Create the new Asset:
        // Set the handle and ID of the asset.
        AssetBase* pAsset = NES_NEW(Type(std::forward<Type>(asset)));
        assetManager.ProcessLoadedAsset(pAsset, Type::GetStaticTypeID(), id, ELoadResult::Success);

        return ELoadResult::Success;
    }

    template <ValidAssetType Type>
    AssetPtr<Type> AssetManager::GetAsset(const AssetID& id)
    {
        NES_ASSERT(IsMainThread(), "Assets can only be accessed on the Main Thread!");
        
        AssetManager& assetManager = GetInstance();
        if (auto it = assetManager.m_infoMap.find(id); it != assetManager.m_infoMap.end())
        {
            auto& info = it->second;
            NES_ASSERT(info.m_typeID == Type::GetStaticTypeID());

            // If we are queued to free, but requested, cancel the free operation.
            // Because we only get assets from the main thread, we won't be freeing an asset while
            // changing this flag.
            if (info.m_state == EAssetState::Freeing)
            {
                info.m_state = EAssetState::Loaded;
            }
            
            // If not loaded, then return nullptr.
            else if (info.m_state != EAssetState::Loaded)
            {
                return nullptr;
            }

            NES_ASSERT(info.m_loadedIndex < assetManager.m_loadedAssets.size());
            NES_ASSERT(assetManager.m_loadedAssets[info.m_loadedIndex] != nullptr);
            NES_ASSERT(assetManager.m_loadedAssets[info.m_loadedIndex]->GetTypeID() == Type::GetStaticTypeID());

            // Otherwise, return a pointer to the asset.
            return AssetPtr<Type>(checked_cast<Type*>(assetManager.m_loadedAssets[info.m_loadedIndex]));
        }

        return nullptr;
    }

    template <ValidAssetType Type>
    ELoadResult AssetManager::MainLoadSync(const AssetID& id, const std::filesystem::path& path)
    {
        // Should be valid at this point.
        NES_ASSERT(id != kInvalidAssetID);
        NES_ASSERT(IsMainThread());
        
        // Determine if we need to perform the load:
        if (AssetInfo* pInfo = GetAssetInfo(id))
        {
            switch (pInfo->m_state)
            {
                // We are already loading it:
                // This can happen if the Asset Thread was tasked with loading the
                // Asset, then LoadSync was called for the same asset.
                case EAssetState::Loading:
                {
                    return ELoadResult::Pending;
                }

                // We have finished, return the previous result.
                case EAssetState::Loaded:
                {
                    return pInfo->m_loadResult;    
                }
                
                // If we were freeing the asset, mark it as Loaded (this will make sure it isn't freed), and return success.
                case EAssetState::Freeing:
                {
                    pInfo->m_state = EAssetState::Loaded;
                    return ELoadResult::Success;
                }

                // If invalid or freed, perform the load.
                case EAssetState::Invalid:
                case EAssetState::Freed:
                    break;
            }
        }
        
        // Create a new entry:
        m_infoMap[id] = AssetInfo(kInvalidLoadIndex, Type::GetStaticTypeID(), EAssetState::Loading, ELoadResult::Pending);
        m_threadInfoMapNeedsSync = true;
        
        // Load the Asset: 
        AssetBase* pAsset = NES_NEW(Type());
        const ELoadResult result = pAsset->LoadFromFile(path);

        // Process the loaded result.
        ProcessLoadedAsset(pAsset, Type::GetStaticTypeID(), id, result);
        return result;         
    }

    template <ValidAssetType Type>
    ELoadResult AssetManager::ThreadLoadSync(const AssetID& id, const std::filesystem::path& path, const LoadRequestID requestID)
    {
        NES_ASSERT(IsAssetThread());

        // New MemoryAsset that will be added to our buffer.
        LoadedMemoryAsset memoryAsset{};
        memoryAsset.m_id = id;
        memoryAsset.m_typeID = Type::GetStaticTypeID();
        memoryAsset.m_requestID = requestID;
        memoryAsset.m_result = ELoadResult::Pending;
        memoryAsset.m_pAsset = nullptr; // Only created if we need to perform the load.

        // Updated info for the asset.
        AssetInfo newInfo
        {
            .m_loadedIndex = kInvalidLoadIndex,
            .m_typeID = Type::GetStaticTypeID(),
            .m_state = EAssetState::Loading,
            .m_loadResult = ELoadResult::Pending,
        };

        // Check if the asset is already loaded or not.
        if (AssetInfo* pInfo = GetAssetInfo(id))
        {
            switch (pInfo->m_state)
            {
                case EAssetState::Loading:
                {
                    // The asset will be loaded before this, and the result will be overwritten.
                    // Break out.
                    newInfo = *pInfo;
                    memoryAsset.m_result = ELoadResult::Success;
                    break;
                }
                
                // We have finished already, use the previous result.
                case EAssetState::Loaded:
                {
                    memoryAsset.m_result = pInfo->m_loadResult;
                    newInfo = *pInfo;
                    break;
                }

                // If we were freeing the asset, mark it as Loaded (this will make sure it isn't freed), and return success.
                case EAssetState::Freeing:
                {
                    newInfo.m_state = EAssetState::Loaded;
                    memoryAsset.m_result = ELoadResult::Success;
                    break;
                }

                // If queued, invalid or freed, set to the loading state, and perform the load.
                //case EAssetState::Queued:
                case EAssetState::Invalid:
                case EAssetState::Freed:
                {
                    // Create the asset on the heap and load:
                    newInfo.m_state = EAssetState::Loading;
                    memoryAsset.m_pAsset = NES_NEW(Type());
                    memoryAsset.m_result = memoryAsset.m_pAsset->LoadFromFile(path);
                    break;
                }
            }
        }

        // No Info, create the Asset:
        else
        {
            // Create the asset on the heap and load:
            memoryAsset.m_pAsset = NES_NEW(Type());
            memoryAsset.m_result = memoryAsset.m_pAsset->LoadFromFile(path);
            newInfo.m_loadResult = memoryAsset.m_result;
        }

        // Update the threadInfoMap:
        // This ensures that we don't try to load this asset multiple times on the asset thread.
        {
            std::lock_guard lock(m_threadInfoMapMutex);
            m_threadInfoMap[id] = newInfo;
        }

        // Add the loaded memory asset to thread's buffer to be synced with the main thread.
        // The asset will be freed if it failed to load.
        // Note: Even if the Asset was previously loaded, we still add a memory asset (it's value will be null, but the result can be Success).
        // This ensures that separete requests for the same asset have their callbacks called appropriately. 
        {
            std::lock_guard lock(m_threadMemoryAssetsMutex);
            m_threadMemoryAssets.emplace_back(memoryAsset);
        }

        return memoryAsset.m_result;
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
