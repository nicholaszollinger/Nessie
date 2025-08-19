// AssetManager.cpp
#include "AssetManager.h"
#include "Nessie/Application/Platform.h"

namespace nes
{
    static AssetManager* g_pInstance = nullptr;

    LoadRequest::LoadRequest(AssetManager& manager, const LoadRequestID id)
        : m_pAssetManager(&manager)
        , m_requestId(id)
    {
        //
    }
    
    LoadRequest::LoadRequest(LoadRequest&& other) noexcept
        : m_pAssetManager(other.m_pAssetManager)
        , m_requestId(other.m_requestId)
        , m_jobs(std::move(other.m_jobs))
        , m_onComplete(other.m_onComplete)
        , m_onAssetLoaded(other.m_onAssetLoaded)
    {
        other.m_requestId = kInvalidRequestID;
        other.m_pAssetManager = nullptr;
        other.m_onComplete = nullptr;
        other.m_onAssetLoaded = nullptr;
    }
    
    TypeID AsyncLoadResult::GetAssetTypeID() const
    {
        return m_assetInfo.m_typeID;
    }

    ELoadResult AsyncLoadResult::GetResult() const
    {
        return m_assetInfo.m_loadResult;
    }

    bool AsyncLoadResult::IsValid() const
    {
        return m_assetInfo.IsValid();
    }

    LoadRequest& LoadRequest::operator=(const LoadRequest& other)
    {
        if (this != &other)
        {
            m_pAssetManager = other.m_pAssetManager;
            m_jobs = other.m_jobs;
            m_requestId = other.m_requestId;
            m_onComplete = other.m_onComplete;
            m_onAssetLoaded = other.m_onAssetLoaded;
        }

        return *this;
    }

    LoadRequest& LoadRequest::operator=(LoadRequest&& other) noexcept
    {
        if (this != &other)
        {
            m_jobs = std::move(other.m_jobs);
            m_requestId = other.m_requestId; // Copy is fine.
            m_pAssetManager = other.m_pAssetManager;
            m_onComplete = other.m_onComplete;
            m_onAssetLoaded = other.m_onAssetLoaded;

            other.m_requestId = kInvalidRequestID;
            other.m_pAssetManager = nullptr;
            other.m_onComplete = nullptr;
            other.m_onAssetLoaded = nullptr;
        }
        
        return *this;
    }
    
    void LoadRequest::AddJob(const ThreadLoadFunc& job)
    {
        m_jobs.emplace_back(job);
    }

    AssetManager::AssetManager()
    {
        NES_ASSERT(g_pInstance == nullptr);
        g_pInstance = this;
    }

    AssetManager::~AssetManager()
    {
        NES_ASSERT(g_pInstance == this);
        g_pInstance = nullptr;
    }

    LoadRequest AssetManager::BeginLoadRequest()
    {
        NES_ASSERT(IsMainThread(), "Load Requests can only be made on the Main Thread.");
        
        AssetManager& instance = GetInstance();
        
        // Create the new Request:
        LoadRequest request(instance, instance.m_nextRequestID);

        // Increment the ID.
        ++instance.m_nextRequestID;

        // Ensure the next ID is valid.
        if (instance.m_nextRequestID == kInvalidRequestID)
            ++instance.m_nextRequestID;
        
        return request;
    }

    void AssetManager::SubmitLoadRequest(LoadRequest&& request)
    {
        if (request.m_jobs.empty())
            return;

#ifndef NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
        AssetManager& instance = GetInstance();
        NES_ASSERT(!instance.m_requestStatusMap.contains(request.m_requestId));
        
        // Create a new Status object for this request:
        LoadRequestStatus status{};
        status.m_onCompleted = request.m_onComplete;
        status.m_onAssetLoaded = request.m_onAssetLoaded;
        status.m_numLoads = static_cast<uint16>(request.m_jobs.size());
        instance.m_requestStatusMap.emplace(request.m_requestId, std::move(status));
        
        // Enqueue the request to be processed on the asset thread
        instance.m_threadJobQueue.EnqueueLocked(std::move(request));
        
        // Wake the asset thread.
        instance.m_assetThread.SendInstruction(EAssetThreadInstruction::ProcessLoadOperations);

#else
        LoadRequest localRequest = std::move(request);
        ELoadResult result = ELoadResult::Success;
        const size_t total = localRequest.m_jobs.size();
        size_t completed = 0;
        
        for (auto& job : localRequest.m_jobs)
        {
            result = job();

            // Break on the first error:
            if (result != ELoadResult::Success)
                break;

            ++completed;

            if (localRequest.m_onAssetLoaded)
            {
                const float progress = static_cast<float>(completed) / static_cast<float>(total);
                localRequest.m_onAssetLoaded(progress);
            }
        }

        if (localRequest.m_onComplete)
        {
            localRequest.m_onComplete(result);
        }
#endif
    }

    bool AssetManager::IsValidAsset(const AssetID& id)
    {
        // Invalid ID:
        if (id == kInvalidAssetID)
            return false;

        AssetManager& assetManager = GetInstance();
        
        if (IsAssetThread())
        {
            std::lock_guard lock(assetManager.m_threadInfoMapMutex);
            if (const auto it = assetManager.m_threadInfoMap.find(id); it != assetManager.m_threadInfoMap.end())
            {
                return it->second.IsValid();
            }
        }

        // Main Thread:
        else
        {
            if (const auto it = assetManager.m_infoMap.find(id); it != assetManager.m_infoMap.end())
            {
                return it->second.IsValid();
            }
        }
        
        return false;
    }

    void AssetManager::FreeAsset(const AssetID& id)
    {
        NES_ASSERT(IsMainThread(), "Assets can only be freed on the main thread!");
        
        if (id == kInvalidAssetID)
            return;
        
        AssetManager& assetManager = GetInstance();
        assetManager.QueueFreeAsset(id);
    }

    AssetID AssetManager::GenerateAssetID()
    {
        static UUIDGenerator generator{};
        return generator.GenerateUUID();
    }

    bool AssetManager::IsAssetThread()
    {
        return std::this_thread::get_id() == GetInstance().m_assetThread.GetThreadId();
    }

    AssetManager& AssetManager::GetInstance()
    {
        NES_ASSERT(g_pInstance != nullptr);
        return *g_pInstance;
    }

    bool AssetManager::IsMainThread()
    {
        return Platform::IsMainThread();
    }

    bool AssetManager::Init()
    {
#ifndef NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
        // Start the Asset thread:
        m_assetThread.Start([this](const EAssetThreadInstruction instruction)
        {
            return AssetThreadProcessInstruction(instruction);
        }, "Asset Thread");
#endif
        
        return true;
    }

    void AssetManager::SyncFrame()
    {
        // Free any assets that can be freed.
        ProcessFreeQueue();
        
#ifndef NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
        // Take ownership of loaded memory assets to process on the main thread.
        MemoryAssetBuffer memoryAssets{};
        memoryAssets.reserve(64);
        {
            std::lock_guard lock(m_threadMemoryAssetsMutex);
            memoryAssets.swap(m_threadMemoryAssets);
        }
        
        // If there are new results:
        if (!memoryAssets.empty())
        {
            m_threadInfoMapNeedsSync = true;

            // Process loaded memory assets:
            for (auto& memoryAsset : memoryAssets)
            {
                ProcessLoadedAsset(memoryAsset.m_pAsset, memoryAsset.m_typeID, memoryAsset.m_id, memoryAsset.m_result, memoryAsset.m_requestID);
            }
            memoryAssets.clear();
        }

        // If the thread info map needs to be updated, do it while the asset thread is idle, to ensure that
        // the Asset Info doesn't change during a load. 
        if (m_threadInfoMapNeedsSync && m_assetThread.IsIdle())
        {
            std::lock_guard lock(m_threadInfoMapMutex);
            m_threadInfoMap = m_infoMap; // Copy the map. Expensive, but starting simple for now.
            m_threadInfoMapNeedsSync = false;
        }
#endif
    }

    void AssetManager::Shutdown()
    {
        NES_ASSERT(g_pInstance != nullptr);

#ifndef NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
        // Shutdown the Asset Thread.
        m_assetThread.WaitUntilDone();
        m_assetThread.Terminate();

        // Clear the operation queue.
        m_threadJobQueue.Clear();
        
        // Destroy any memory assets that weren't processed in the sync yet.
        for (auto& memoryAsset : m_threadMemoryAssets)
        {
            NES_SAFE_DELETE(memoryAsset.m_pAsset);
        }
        m_threadMemoryAssets.clear();
        m_threadInfoMap.clear();
        
#endif
        
        // Free all remaining assets:
        for (auto& pAsset : m_loadedAssets)
        {
            NES_SAFE_DELETE(pAsset);
        }
        
        m_loadedAssets.clear();
        m_infoMap.clear();
    }

    bool AssetManager::AssetThreadProcessInstruction(const EAssetThreadInstruction instruction)
    {
        switch (instruction)
        {
            case EAssetThreadInstruction::ProcessLoadOperations:
            {
                AssetThreadProcessLoadOperations();
                return true;
            }
        }

        NES_ERROR(kAssetLogTag, "Unhandled Asset Thread Instruction!");
        return false;
    }

    void AssetManager::AssetThreadProcessLoadOperations()
    {
        while (!m_threadJobQueue.IsEmptyLocked())
        {
            // Grab the next request:
            m_threadJobQueue.Lock();
            const auto loadRequest = m_threadJobQueue.Front();
            m_threadJobQueue.Pop();
            m_threadJobQueue.Unlock();
            
            // Run each job.
            for (auto& job : loadRequest.m_jobs)
            {
                // We don't use the result. Even if there is an error, we will
                // try to load all Assets, so that all entries in the main thread's info
                // map are updated appropriately when processing the loaded results.
                [[maybe_unused]] const auto result = job();
            }
        }
    }

    void AssetManager::QueueFreeAsset(const AssetID& id)
    {
        NES_ASSERT(id != kInvalidAssetID);
        
        if (auto* pInfo = GetAssetInfo(id))
        {
            // If the asset has not been freed or queued to free, queue it.
            if (pInfo->m_state < EAssetState::Freeing)
            {
                // Only enqueue the asset if it was fully loaded. If it is being loaded,
                // then we need to wait for it to complete.
                if (pInfo->m_state == EAssetState::Loaded)
                {
                    NES_ASSERT(pInfo->m_loadedIndex < m_loadedAssets.size());
                    m_assetsToFree.emplace_back(id);
                }
                
                pInfo->m_state = EAssetState::Freeing;
            }
            
            return;
        }

        NES_WARN(kAssetLogTag, "Attempted to free an asset that doesn't exist. ID: {}", id.GetValue());
    }

    AssetManager::AssetInfo* AssetManager::GetAssetInfo(const AssetID& id)
    {
        if (IsMainThread())
        {
            if (auto it = m_infoMap.find(id); it != m_infoMap.end())
            {
                return &it->second;   
            }
        }

        else
        {
            NES_ASSERT(IsAssetThread());
            
            std::lock_guard lock(m_threadInfoMapMutex);
            if (const auto it = m_threadInfoMap.find(id); it != m_threadInfoMap.end())
            {
                return &it->second;
            }
        }
        
        return nullptr;
    }

    void AssetManager::ProcessFreeQueue()
    {
        if (m_assetsToFree.empty())
            return;

        if (m_loadedAssets.empty())
        {
            m_assetsToFree.clear();
            return;
        }

        // We are updating asset state, so we will have to sync.
        m_threadInfoMapNeedsSync = true;
        
        for (size_t i = 0; i < m_assetsToFree.size();)
        {
            const auto& id = m_assetsToFree[i];
            NES_ASSERT(id != kInvalidAssetID);
            NES_ASSERT(m_infoMap.contains(id));
            
            auto& assetInfo = m_infoMap.at(id);
            
            // If the asset has been requested again, or it has been freed already, remove from the array.
            if (assetInfo.m_state != EAssetState::Freeing)
            {
                std::swap(m_assetsToFree[i], m_assetsToFree.back());
                m_assetsToFree.pop_back();
                continue;
            }
            
            NES_ASSERT(assetInfo.m_loadedIndex < m_loadedAssets.size());
            NES_ASSERT(assetInfo.m_typeID == m_loadedAssets[assetInfo.m_loadedIndex]->GetTypeID());

            const uint64 freeIndex = assetInfo.m_loadedIndex;
            auto*& pAsset = m_loadedAssets[freeIndex];

            // If the Asset has no more locks, we can free it:
            if (!pAsset->HasLocks())
            {
                // Delete the asset.
                NES_SAFE_DELETE(pAsset);
                
                // Remove the asset from the loaded array:
                if (freeIndex != m_loadedAssets.size() - 1)
                {
                    const AssetID lastAssetID = m_loadedAssets.back()->m_id;
                    auto& lastAssetInfo = m_infoMap.at(lastAssetID);

                    // Update the loaded index for the swapped asset:
                    lastAssetInfo.m_loadedIndex = freeIndex;
                    
                    // Swap:
                    std::swap(m_loadedAssets[freeIndex], m_loadedAssets.back());
                }
                m_loadedAssets.pop_back();

                // Set the Freed State:
                assetInfo.m_state = EAssetState::Freed;
                assetInfo.m_loadedIndex = kInvalidLoadIndex;

                // Remove from the free array.
                std::swap(m_assetsToFree[i], m_assetsToFree.back());
                m_assetsToFree.pop_back();
                continue;
            }

            ++i;
        }
    }

    void AssetManager::ProcessLoadedAsset(AssetBase*& pAsset, const TypeID typeID, const AssetID& id, const ELoadResult result, const LoadRequestID requestID)
    {
        // Get the current info, or create a new entry:
        AssetInfo* pInfo = GetAssetInfo(id);
        if (pInfo == nullptr)
        {
            m_infoMap[id] = AssetInfo
            {
                .m_loadedIndex = kInvalidLoadIndex,
                .m_typeID = typeID,
                .m_state = EAssetState::Loading, // Not immediately set to loaded, so that it will not be queued to free.
                .m_loadResult = result
            };
            pInfo = &m_infoMap.at(id);
        }
        
        // Load Success
        if (result == ELoadResult::Success)
        {
            // If the load was successful and the Asset is not null, add it to the loaded assets.
            // The asset can be null and still be a successful load when two async load requests ask for the same asset at the same time.
            // We still create a memory asset, we just don't load the actual asset. This ensures that the
            // request's callbacks are handled appropriately.
            // - Why not have an array of callbacks? Because a LoadRequest can be used for multiple assets, so the callbacks only
            //   make sense per request.
            if (pAsset != nullptr)
            {
                // Case: Asset Thread loaded an asset which was then loaded synchronously by the
                // Main Thread before syncing. The Asset can be fully loaded or Freed. Either way, our asset is not needed.
                if (pInfo->IsValid() || pInfo->m_state == EAssetState::Freed)
                {
                    // Delete the memory without storing it.
                    NES_SAFE_DELETE(pAsset);
                }

                // Asset will be added to the loaded assets map:
                else
                {
                    // Set the loaded index and add to the array.
                    pInfo->m_loadedIndex = m_loadedAssets.size();
                    m_loadedAssets.emplace_back(pAsset);
                    pInfo->m_loadResult = result;
            
                    // Set the Asset's internal ID.
                    pAsset->m_id = id;

                    // Case: The Asset was requested to be freed when it was loading.
                    // Queue it to be freed on the next sync.
                    if (pInfo->m_state == EAssetState::Freeing)
                    {
                        // Add it to the queue to be freed.
                        // When calling FreeAsset(), assets that are in the Loading state are not
                        // immediately queued. They wait for this case.
                        m_assetsToFree.emplace_back(id);
                    }
                    else
                    {
                        pInfo->m_state = EAssetState::Loaded;
                    }
                }
            }
        }

        // If the load failed, free the temporary asset now.
        else
        {
            if (!pInfo->IsValid())
            {
                pInfo->m_state = EAssetState::Freed;
                pInfo->m_loadedIndex = kInvalidLoadIndex;
                pInfo->m_loadResult = result;
            }
            
            NES_SAFE_DELETE(pAsset);
        }
        
        // If the asset belongs to a load request, update it:
        if (requestID != kInvalidRequestID)
        {
            if (auto it = m_requestStatusMap.find(requestID); it != m_requestStatusMap.end())
            {
                auto& requestResult = it->second;

                // Completed Count
                requestResult.m_completedLoads += 1;
                NES_ASSERT(requestResult.m_completedLoads <= requestResult.m_numLoads);

                // If the load was successful, increment the success count.
                if (result == ELoadResult::Success)
                    requestResult.m_successfulLoads += 1;
                
                // Call on Asset Loaded, if provided
                if (requestResult.m_onAssetLoaded)
                {
                    const AsyncLoadResult asyncResult(id, *pInfo, requestResult.GetProgress());
                    requestResult.m_onAssetLoaded(asyncResult);
                }

                // Check for completed:
                if (requestResult.IsComplete())
                {
                    // Call the completed callback if valid.
                    if (requestResult.m_onCompleted)
                    {
                        requestResult.m_onCompleted(requestResult.IsSuccessful());
                    }

                    // Remove the Request Result from the map:
                    m_requestStatusMap.erase(it);
                }
            }
        }
    }

    bool AssetManager::ThreadCanProceed(const AssetID& id, const TypeID& typeID, ELoadResult& outResult, bool& outShouldLoad)
    {
        std::lock_guard lock(m_threadInfoMapMutex);

        auto it = m_threadInfoMap.find(id);
        
        // No current info exists, we need to perform the load!
        if (it == m_threadInfoMap.end())
        {
            m_threadInfoMap[id] = AssetInfo(kInvalidLoadIndex, typeID, EAssetState::ThreadLoading, ELoadResult::Pending);
            outShouldLoad = true;
        }

        // We have existing info:
        else
        {
            AssetInfo& info = it->second;

            switch (info.m_state)
            {
                // Another thread is currently trying to load this asset. The result is not
                // valid to read in this case.
                case EAssetState::ThreadLoading:
                {
                    return false;
                }
                    
                // We have finished already, use the previous result.
                case EAssetState::Loaded:
                {
                    outResult = info.m_loadResult;
                    break;
                }

                case EAssetState::Loading:
                case EAssetState::Invalid:
                case EAssetState::Freeing:
                case EAssetState::Freed:
                {
                    // Claim the Asset to load on this thread.
                    info.m_state = EAssetState::ThreadLoading;
                    outShouldLoad = true;
                    break;
                } 
            }
        }

        // No other thread is trying to load this asset. We either need to perform the load or
        // we can use an existing result.
        return true;
    }
}
