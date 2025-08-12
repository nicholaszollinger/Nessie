// AssetManager.cpp
#include "AssetManager.h"
#include "Nessie/Application/Platform.h"

namespace nes
{
    static AssetManager* g_pInstance = nullptr;

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
        // Get the read range for this sync event:
        uint32 current = m_resultReadIndex.load(std::memory_order_relaxed);
        const uint32 end = m_resultWriteIndex.load(std::memory_order_relaxed);

        uint32 memoryAssetIndex = 0;

        // Take ownership of loaded memory assets to process on the main thread.
        MemoryAssetBuffer memoryAssets{};
        memoryAssets.reserve(64);
        {
            std::lock_guard lock(m_threadMemoryAssetsMutex);
            memoryAssets.swap(m_threadMemoryAssets);
        }
        
        // If there are new results:
        while (current != end)
        {
            m_threadInfoMapNeedsSync = true;
            auto& result = m_results[current];

            // Process the number of loaded assets for this Job.
            for (uint32 i = 0; i < result.m_numMemoryAssets; ++i, ++memoryAssetIndex)
            {
                auto& memoryAsset = memoryAssets[memoryAssetIndex];
                NES_ASSERT(memoryAsset.m_resultIndex == current);

                AssetInfo info
                {
                    .m_typeID = memoryAsset.m_typeID,
                    .m_state = EAssetState::Loaded,
                    .m_loadResult = memoryAsset.m_result,
                };

                // Handle special cases:
                if (auto it = m_infoMap.find(memoryAsset.m_id); it != m_infoMap.end())
                {
                    auto& currentInfo = it->second;
                    
                    // Case: Asset Thread loaded a dependent asset which was loaded synchronously by the
                    // Main Thread before syncing. Shouldn't happen, but possible.
                    if (currentInfo.IsValid())
                    {
                        // Delete the memory without storing it.
                        NES_SAFE_DELETE(memoryAsset.m_pAsset);
                        continue;
                    }

                    // Case: Asset was requested to be freed when it was loading.
                    if (currentInfo.m_state == EAssetState::Freeing)
                    {
                        // Keep the free state.
                        info.m_state = EAssetState::Freeing;

                        // Add it to the queue to be freed.
                        // When calling FreeAsset(), assets that are in the Loading state are not
                        // immediately queued. 
                        m_assetsToFree.emplace_back(memoryAsset.m_id);
                    }
                }
                
                // If the load failed, then free the asset.
                if (memoryAsset.m_result != ELoadResult::Success)
                {
                    info.m_state = EAssetState::Freed;
                    NES_SAFE_DELETE(memoryAsset.m_pAsset);
                }

                // Success!
                else
                {
                    m_loadedAssets[memoryAsset.m_id] = memoryAsset.m_pAsset;
                    memoryAsset.m_pAsset->m_id = memoryAsset.m_id;
                }

                // Update the Asset Info.
                m_infoMap[memoryAsset.m_id] = info;
            }

            // [TODO]: Print Error Message on a failed result.

            // [TODO]: Process asset listeners:
            
            // Increment (results are in a ring buffer).
            current = (current + 1) % kMaxLoadOperations;
        }
        memoryAssets.clear();

        // If the main thread assets have been updated
        if (m_threadInfoMapNeedsSync)
        {
            std::lock_guard lock(m_threadInfoMapMutex);
            m_threadInfoMap = m_infoMap; // Copy the map. Expensive, but starting simple for now.
            m_threadInfoMapNeedsSync = false;
        }
        
        // Store the `end` value as the new read position. The asset thread could have incremented the
        // 'm_threadResultIndex' value during this function, so we don't want to potentially miss loaded assets. 
        m_resultReadIndex.store(end, std::memory_order_relaxed);

        // Wake the asset thread to process any load operations.
        m_assetThread.SendInstruction(EAssetThreadInstruction::ProcessLoadOperations);
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
        
        // Free and Destruct any resulting operations that weren't processed in the sync yet.
        uint32 current = m_resultReadIndex.load(std::memory_order_relaxed);
        const uint32 end = m_resultWriteIndex.load(std::memory_order_relaxed);
        uint32 memoryAssetIndex = 0;
        while (current != end)
        {
            auto& result = m_results[current];

            // Free any loaded memory assets:
            // Process the number of loaded assets:
            for (uint32 i = 0; i < result.m_numMemoryAssets; ++i, ++memoryAssetIndex)
            {
                auto& memoryAsset = m_threadMemoryAssets[memoryAssetIndex];
                NES_ASSERT(memoryAsset.m_resultIndex == current);

                NES_SAFE_DELETE(memoryAsset.m_pAsset);
            }
            
            // Increment (results are in a ring buffer).
            current = (current + 1) % kMaxLoadOperations;
        }
#endif
        // Free all remaining assets:
        for (auto& [_, pAsset] : m_loadedAssets)
        {
            if (pAsset)
            {
                NES_SAFE_DELETE(pAsset);
            }
        }
        m_loadedAssets.clear();
        m_infoMap.clear();
        m_threadInfoMap.clear();
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
        // While we have not caught up to the Main Thread,
        // and we have work to do, process the next load operation.
        uint32 current = m_resultWriteIndex.load(std::memory_order_relaxed);
        uint32 next = (current + 1) % kMaxLoadOperations;
        
        while (!m_threadJobQueue.IsEmptyLocked() && next != m_resultReadIndex.load(std::memory_order_relaxed))
        {
            // Get the next result to process:
            auto& nextResult = m_results[current];

            m_threadJobQueue.Lock();
            const auto loadOperation = m_threadJobQueue.Front();
            m_threadJobQueue.Pop();
            m_threadJobQueue.Unlock();
            
            // Run the Load Operation, and store the result.
            nextResult.m_result = loadOperation();

            // Increment to the next result index
            current = next;
            next = (current + 1) % kMaxLoadOperations;
            
            // Update the tail. On the next call to ProcessLoadedAssets on the main thread,
            // the main thread will be able to access the result at current.
            m_resultWriteIndex.store(current, std::memory_order_relaxed);
        }
    }

    void AssetManager::QueueFreeAsset(const AssetID& id)
    {
        NES_ASSERT(id != kInvalidAssetID);
        
        if (auto it = m_infoMap.find(id); it != m_infoMap.end())
        {
            auto& info = it->second;
            
            // If the asset has not been freed or queued to free, queue it.
            if (info.m_state < EAssetState::Freeing)
            {
                // Only enqueue the asset if it was fully loaded. If it is being loaded,
                // then we need to wait for it to complete.
                if (info.m_state == EAssetState::Loaded)
                {
                    NES_ASSERT(m_loadedAssets.contains(id));
                    m_assetsToFree.emplace_back(id);
                }
                
                info.m_state = EAssetState::Freeing;
            }
            
            return;
        }

        NES_WARN(kAssetLogTag, "Attempted to free an asset that doesn't exist. ID: {}", id.GetValue());
    }

    void AssetManager::ProcessFreeQueue()
    {
        if (m_assetsToFree.empty())
            return;

        // We are updating asset state, so we will have to sync.
        m_threadInfoMapNeedsSync = true;
        
        for (size_t i = 0; i < m_assetsToFree.size();)
        {
            const auto& id = m_assetsToFree[i];
            NES_ASSERT(id != kInvalidAssetID);
            NES_ASSERT(m_infoMap.contains(id));
            
            auto& assetInfo = m_infoMap.at(id);
            
            // If the asset has been requested again before freeing, remove from the array.
            if (assetInfo.m_state < EAssetState::Freeing)
            {
                std::swap(m_assetsToFree[i], m_assetsToFree.back());
                m_assetsToFree.pop_back();
                continue;
            }

            // If the asset has no more locks, free it.
            auto it = m_loadedAssets.find(id);
            NES_ASSERT(it != m_loadedAssets.end());
            
            if (!it->second->HasLocks())
            {
                // Delete the asset.
                NES_SAFE_DELETE(it->second);
                
                // Remove the asset from the loaded map:
                m_loadedAssets.erase(it);

                // Update the state.
                assetInfo.m_state = EAssetState::Freed;

                // Remove from the array.
                std::swap(m_assetsToFree[i], m_assetsToFree.back());
                m_assetsToFree.pop_back();
                continue;
            }

            ++i;
        }
    }
}
