// AssetManager.inl
#pragma once
#include "AssetBase.h"

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
    void AssetManager::LoadAsync(AssetID& id, const std::filesystem::path& path, const LoadRequest::OnAssetLoaded& onComplete)
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
                        AsyncLoadResult result(id, *pInfo);
                        onComplete(result, 1.f);
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
        request.SetOnAssetLoadedCallback(onComplete);
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

                // If the Asset Thread was tasked with loading the Asset, perform the load now.
                // - The Asset Thread will also perform the load; the duplicate asset will be destroyed on Frame Sync.
                case EAssetState::Loading:
                // If Invalid or Freed, perform the load.
                case EAssetState::Invalid:
                case EAssetState::Freed:
                    break;

                // Should not occur on the main thread. This state is only used on the Asset Thread, to denote that
                // it is actively being loaded. 
                case EAssetState::ThreadLoading:
                {
                    NES_ASSERT(false, "Main thread AssetState should never have 'ThreadLoading' set!");
                    break;
                }
            }
        }

        else
        {
            // Create a new entry:
            m_infoMap[id] = AssetInfo(kInvalidLoadIndex, Type::GetStaticTypeID(), EAssetState::Loading, ELoadResult::Pending);
        }
        
        m_threadInfoMapNeedsSync = true;
        
        // Load the Asset: 
        AssetBase* pAsset = NES_NEW(Type());
        const ELoadResult result = pAsset->LoadFromFile(path);

        // Process the loaded result.
        ProcessLoadedAsset(pAsset, Type::GetStaticTypeID(), id, result);

        // A synchronous load operation must have a completed result.
        NES_ASSERT(result != ELoadResult::Pending);
        return result;
    }

    template <ValidAssetType Type>
    ELoadResult AssetManager::ThreadLoadSync(const AssetID& id, const std::filesystem::path& path, const LoadRequestID requestID)
    {
        NES_ASSERT(IsAssetThread());

        // New memory asset that will be added to our buffer. Even if we don't need to perform a load operation,
        // this memory asset will be used to notify the load request.
        LoadedMemoryAsset memoryAsset
        {
            .m_pAsset = nullptr, // Only created if we need to perform the load.
            .m_id = id,
            .m_typeID = Type::GetStaticTypeID(),
            .m_requestID = requestID,
            .m_result = ELoadResult::Pending,
        };

        // Check if we need to perform a load:
        bool needsToLoad = false;
        {
            for (;;)
            {
                if (!ThreadCanProceed(id, Type::GetStaticTypeID(), memoryAsset.m_result, needsToLoad))
                {
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                    continue;
                }

                break;
            }
        }

        // Perform the load if necessary:
        if (needsToLoad)
        {
            // Create the asset on the heap and load:
            memoryAsset.m_pAsset = NES_NEW(Type());
            memoryAsset.m_result = memoryAsset.m_pAsset->LoadFromFile(path);
            
            // Set the result of the load in the map.
            std::lock_guard lock(m_threadInfoMapMutex);
            
            auto& info = m_threadInfoMap.at(id); 
            info.m_loadResult = memoryAsset.m_result;
            info.m_state = EAssetState::Loaded;
        }

        // Add the loaded memory asset to thread's buffer to be synced with the main thread.
        // The asset memory will be freed if it failed to load.
        // Note: Even if the Asset was previously loaded, we still add a memory asset object (it's value will be null, but the result can be Success).
        // This ensures that separate requests for the same asset have their callbacks invoked correctly. 
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
