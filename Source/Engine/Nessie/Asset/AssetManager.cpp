// AssetManager.cpp
#include "AssetManager.h"
#include "Nessie/Application/Application.h"
#include "Nessie/Graphics/Shader.h"

namespace nes
{
    static AssetManager* g_pInstance = nullptr;

    // [TODO]: Temp Fix. Right now, I have shaders in their own folder.
    // - When I have all Assets in a single folder, then I can just prepend the NES_CONTENT_DIR
    //   directory.
    // - I will also have to update my shader compilation script.
    // - NOTE: You will probably want to load Shaders separately from world assets, so that you
    //   can immediately start rendering and don't stall the update loop.
    static std::filesystem::path ResolveAbsoluteAssetPath(const AssetMetadata& metadata)
    {
        if (metadata.m_typeID == Shader::GetStaticTypeID())
        {
            return std::filesystem::path(NES_SHADER_DIR) / metadata.m_relativePath;
        }
        else
        {
            return std::filesystem::path(NES_CONTENT_DIR) / metadata.m_relativePath;
        }
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
    
    LoadRequest::LoadRequest(LoadRequest&& other) noexcept
        : m_pAssetManager(other.m_pAssetManager)
        , m_requestId(other.m_requestId)
        , m_jobs(std::move(other.m_jobs))
        , m_onAssetLoaded(other.m_onAssetLoaded)
        , m_onComplete(other.m_onComplete)
    {
        other.m_requestId = kInvalidRequestID;
        other.m_pAssetManager = nullptr;
        other.m_onComplete = nullptr;
        other.m_onAssetLoaded = nullptr;
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

    void LoadRequest::AppendLoad(const AssetMetadata& metadata)
    {
        NES_ASSERT(m_pAssetManager != nullptr);
        NES_ASSERT(m_pAssetManager->IsTypeRegistered(metadata.m_typeID), "Asset Type must be registered before being able to use AssetMetadata parameters directly!");
        NES_ASSERT(!metadata.m_relativePath.empty(), "Failed to AppendLoad to LoadRequest! AssetMetadata must have a valid path!");

        // If we don't have an entry already, create one.
        if (!m_pAssetManager->GetAssetDesc(metadata.m_assetID))
        {
            m_pAssetManager->m_loadedAssetMap[metadata.m_assetID] = LoadedAssetDesc
            {
                .m_metadata = metadata,
                .m_loadedIndex = kInvalidLoadIndex,
                .m_state = EAssetState::Loading,
                .m_loadResult = ELoadResult::Pending,
            };
        }

    #ifdef NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
        ThreadLoadFunc func = [pAssetManager, metadata]()
        {
            return pAssetManager->MainLoadSync(metadata);
        };
    #else

        AssetManager* pAssetManager = m_pAssetManager;
        LoadRequestID requestId = m_requestId;
        ThreadLoadFunc func = [pAssetManager, requestId, metadata]()
        {
            return pAssetManager->ThreadLoadSync(metadata, requestId);  
        };
    #endif

        AddJob(func);
    }

    LoadRequest::LoadRequest(AssetManager& manager, const LoadRequestID id)
        : m_pAssetManager(&manager)
        , m_requestId(id)
    {
        //
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

    ELoadResult AssetManager::LoadSync(AssetMetadata& metadata)
    {
        // Create the asset id if necessary:
        if (metadata.m_assetID == kInvalidAssetID)
        {
            metadata.m_assetID = GenerateAssetIDFromPath(metadata.m_relativePath);
        }

        AssetManager& instance = GetInstance();

#ifdef NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
        return instance.MainLoadSync(metadata);
        
#else
        if (IsMainThread())
            return instance.MainLoadSync(metadata);

        return instance.ThreadLoadSync(metadata);
#endif
    }

    ELoadResult AssetManager::LoadAssetPackSync(AssetPack& pack)
    {
        auto& assets = pack.GetAssets();
        for (auto& metadata : assets)
        {
            const ELoadResult result = LoadSync(metadata);
            if (result != ELoadResult::Success)
            {
                NES_ERROR(kAssetLogTag, "Failed to load Asset Pack!");
                return result;
            }
        }
        
        return ELoadResult::Success;
    }

    void AssetManager::SaveAssetSync(const AssetID& id)
    {
        NES_ASSERT(IsMainThread(), "Assets can only be accessed on the Main Thread!");
        
        AssetManager& assetManager = GetInstance();
        if (auto it = assetManager.m_loadedAssetMap.find(id); it != assetManager.m_loadedAssetMap.end())
        {
            auto& desc = it->second;
            
            // If we are queued to free, but requested, cancel the free operation.
            // Because we only get assets from the main thread, we won't be freeing an asset while
            // changing this flag.
            if (desc.m_state == EAssetState::Freeing)
            {
                desc.m_state = EAssetState::Loaded;
            }
            
            // If not loaded, then return.
            else if (desc.m_state != EAssetState::Loaded)
            {
                NES_WARN("Attempted to save an Asset that was not loaded! \n- ID: {}\n- Path: {}", id.GetValue(), desc.m_metadata.m_relativePath.string());
                return;
            }

            // If no path is present, we can't save it.
            if (desc.m_metadata.m_relativePath.empty())
            {
                NES_WARN("Attempted to save an Asset that does not have a valid path! \n- ID: {}", id.GetValue());
                return;
            }

            NES_ASSERT(desc.m_loadedIndex < assetManager.m_loadedAssets.size());
            NES_ASSERT(assetManager.m_loadedAssets[desc.m_loadedIndex] != nullptr);

            // Perform the save:
            assetManager.m_loadedAssets[desc.m_loadedIndex]->SaveToFile(desc.m_metadata.m_relativePath);    
        }
    }

    void AssetManager::LoadAssetPackAsync(AssetPack& pack, const LoadRequest::OnComplete& onComplete, const LoadRequest::OnAssetLoaded& onSingleAssetLoaded)
    {
        LoadRequest request = BeginLoadRequest();
        request.SetOnCompleteCallback(onComplete);
        request.SetOnAssetLoadedCallback(onSingleAssetLoaded);
        
        auto& assets = pack.GetAssets();
        for (auto& metadata : assets)
        {
            // Create the asset id if necessary:
            if (metadata.m_assetID == kInvalidAssetID)
            {
                metadata.m_assetID = GenerateAssetIDFromPath(metadata.m_relativePath);
            }
            
            request.AppendLoad(metadata);
        }

        SubmitLoadRequest(std::move(request));
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
        {
            // Call the on complete immediately.
            if (request.m_onComplete)
                request.m_onComplete(true);
            
            return;
        }

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

    const AssetManager::AssetTypeDesc* AssetManager::GetAssetTypeDescByName(const std::string& name)
    {
        auto& assetManager = GetInstance();
        for (auto& [_, typeDesc] : assetManager.m_typeRegistry)
        {
            if (typeDesc.m_typename == name)
            {
                return &typeDesc;
            }
        }

        return nullptr;
    }

    bool AssetManager::IsValidAsset(const AssetID& id)
    {
        // Invalid ID:
        if (id == kInvalidAssetID)
            return false;

        AssetManager& assetManager = GetInstance();
        
        if (IsAssetThread())
        {
            std::lock_guard lock(assetManager.m_threadLoadedAssetMapMutex);
            if (const auto it = assetManager.m_threadLoadedAssetMap.find(id); it != assetManager.m_threadLoadedAssetMap.end())
            {
                return it->second.IsValid();
            }
        }

        // Main Thread:
        else
        {
            if (const auto it = assetManager.m_loadedAssetMap.find(id); it != assetManager.m_loadedAssetMap.end())
            {
                return it->second.IsValid();
            }
        }
        
        return false;
    }

    bool AssetManager::IsMemoryAsset(const AssetID& id)
    {
        // Invalid ID:
        if (id == kInvalidAssetID)
            return false;

        AssetManager& assetManager = GetInstance();
        
        if (IsAssetThread())
        {
            std::lock_guard lock(assetManager.m_threadLoadedAssetMapMutex);
            if (const auto it = assetManager.m_threadLoadedAssetMap.find(id); it != assetManager.m_threadLoadedAssetMap.end())
            {
                return it->second.IsValid() && it->second.m_metadata.m_relativePath.empty();
            }
        }

        // Main Thread:
        else
        {
            if (const auto it = assetManager.m_loadedAssetMap.find(id); it != assetManager.m_loadedAssetMap.end())
            {
                return it->second.IsValid() && it->second.m_metadata.m_relativePath.empty();
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

    void AssetManager::FreeAssets(const std::vector<AssetID>& ids)
    {
        NES_ASSERT(IsMainThread(), "Assets can only be freed on the main thread!");
        AssetManager& assetManager = GetInstance();

        for (const auto& id : ids)
        {
            if (id == kInvalidAssetID)
                continue;
            
            assetManager.QueueFreeAsset(id);
        }
    }

    void AssetManager::FreeAssets(const AssetMetaDataArray& assets)
    {
        NES_ASSERT(IsMainThread(), "Assets can only be freed on the main thread!");
        AssetManager& assetManager = GetInstance();

        for (const auto& metadata : assets)
        {
            if (metadata.m_assetID == kInvalidAssetID)
                continue;
            
            assetManager.QueueFreeAsset(metadata.m_assetID);
        }
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
                ProcessLoadedAsset(memoryAsset.m_pAsset, memoryAsset.m_metadata, memoryAsset.m_result, memoryAsset.m_requestID);
            }
            memoryAssets.clear();
        }

        // If the thread info map needs to be updated, do it while the asset thread is idle, to ensure that
        // the Asset Info doesn't change during a load. 
        if (m_threadInfoMapNeedsSync && m_assetThread.IsIdle())
        {
            std::lock_guard lock(m_threadLoadedAssetMapMutex);
            m_threadLoadedAssetMap = m_loadedAssetMap; // Copy the map. Expensive, but starting simple for now.
            m_threadInfoMapNeedsSync = false;
        }
#endif
    }

    void AssetManager::TerminateAssetThread()
    {
        NES_ASSERT(g_pInstance != nullptr);

#ifndef NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
        // Shutdown the Asset Thread.
        m_assetThreadShouldQuit = true;
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
        m_threadLoadedAssetMap.clear();
#endif
    }

    void AssetManager::Shutdown()
    {
        NES_ASSERT(g_pInstance != nullptr);

#ifndef NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
        if (!m_assetThread.IsTerminated())
            TerminateAssetThread();
#endif
        
        // Free all remaining assets:
        for (auto& pAsset : m_loadedAssets)
        {
            NES_SAFE_DELETE(pAsset);
        }
        
        m_loadedAssets.clear();
        m_loadedAssetMap.clear();
    }

    AssetManager& AssetManager::GetInstance()
    {
        NES_ASSERT(g_pInstance != nullptr);
        return *g_pInstance;
    }

    bool AssetManager::IsMainThread()
    {
        return Application::IsMainThread();
    }

    AssetManager::LoadedAssetDesc* AssetManager::GetAssetDesc(const AssetID& id)
    {
        if (IsMainThread())
        {
            if (auto it = m_loadedAssetMap.find(id); it != m_loadedAssetMap.end())
            {
                return &it->second;   
            }
        }

        else
        {
            NES_ASSERT(IsAssetThread());
            
            std::lock_guard lock(m_threadLoadedAssetMapMutex);
            if (const auto it = m_threadLoadedAssetMap.find(id); it != m_threadLoadedAssetMap.end())
            {
                return &it->second;
            }
        }
        
        return nullptr;
    }

    bool AssetManager::IsTypeRegistered(const TypeID typeID)
    {
        std::lock_guard lock(m_typeRegistryMutex);
        return m_typeRegistry[typeID].m_isRegistered;
    }

    void AssetManager::SetTypeDesc(AssetTypeDesc&& desc)
    {
        std::lock_guard lock(m_typeRegistryMutex);
        m_typeRegistry[desc.m_type] = std::move(desc);
    }

    AssetManager::AssetTypeDesc::CreateNewAsset AssetManager::GetCreateNewAsset(const TypeID typeID)
    {
        NES_ASSERT(IsTypeRegistered(typeID));
        std::lock_guard lock(m_typeRegistryMutex);
        return m_typeRegistry.at(typeID).m_createNewAsset;
    }

    AssetManager::AssetTypeDesc::CreateNewAssetMove AssetManager::GetCreateNewAssetMove(const TypeID typeID)
    {
        NES_ASSERT(IsTypeRegistered(typeID));
        std::lock_guard lock(m_typeRegistryMutex);
        return m_typeRegistry.at(typeID).m_createNewAssetMove;
    }

    ELoadResult AssetManager::MainLoadSync(const AssetMetadata& metadata)
    {
        NES_ASSERT(metadata.m_assetID != kInvalidAssetID);
        NES_ASSERT(IsMainThread());
        NES_ASSERT(IsTypeRegistered(metadata.m_typeID), "Attempted to load Asset that wasn't registered! Be sure to call NES_REGISTER_ASSET_TYPE(Type) for all Assets before attempting loads!");

        // Determine if we need to perform the load:
        if (LoadedAssetDesc* pInfo = GetAssetDesc(metadata.m_assetID))
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
            m_loadedAssetMap[metadata.m_assetID] = LoadedAssetDesc
            {
                .m_metadata = metadata,
                .m_loadedIndex = kInvalidLoadIndex,
                .m_state = EAssetState::Loading,
                .m_loadResult = ELoadResult::Pending
            };
        }

        m_threadInfoMapNeedsSync = true;
        
        // Load the Asset:
        AssetTypeDesc::CreateNewAsset createNew = GetCreateNewAsset(metadata.m_typeID);
        AssetBase* pAsset = createNew();

        auto absolutePath = ResolveAbsoluteAssetPath(metadata);
        ELoadResult result = pAsset->LoadFromFile(absolutePath);

        // Process the loaded result:
        ProcessLoadedAsset(pAsset, metadata, result);

        // A synchronous load operation must have a completed result.
        NES_ASSERT(result != ELoadResult::Pending);
        return result;
    }

    ELoadResult AssetManager::ThreadLoadSync(const AssetMetadata& metadata, const LoadRequestID requestID)
    {
        NES_ASSERT(IsAssetThread());

        // New memory asset that will be added to our buffer. Even if we don't need to perform a load operation,
        // this memory asset will be used to notify the load request.
        LoadedMemoryAsset memoryAsset
        {
            .m_metadata = metadata,
            .m_pAsset = nullptr, // Only created if we need to perform the load.
            .m_requestID = requestID,
            .m_result = ELoadResult::Pending,
        };

        // Check if we need to perform a load:
        bool needsToLoad = false;
        {
            for (;;)
            {
                if (!ThreadCanProceed(memoryAsset.m_metadata, memoryAsset.m_result, needsToLoad))
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
            // Load:
            AssetTypeDesc::CreateNewAsset createNew = GetCreateNewAsset(metadata.m_typeID);
            memoryAsset.m_pAsset = createNew();
            auto absolutePath = ResolveAbsoluteAssetPath(metadata);
            memoryAsset.m_result = memoryAsset.m_pAsset->LoadFromFile(absolutePath);
            
            // Set the result of the load in the map.
            std::lock_guard lock(m_threadLoadedAssetMapMutex);
            
            auto& info = m_threadLoadedAssetMap.at(metadata.m_assetID); 
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
    
    ELoadResult AssetManager::AddMemoryAsset(const AssetMetadata& metadata, AssetBase&& asset)
    {
        AssetTypeDesc::CreateNewAssetMove createAsset = GetCreateNewAssetMove(metadata.m_typeID);
        
        if (IsMainThread())
        {
            AssetBase* pAsset = createAsset(std::move(asset)); //NES_NEW(Type(std::forward<Type>(asset)));
            ProcessLoadedAsset(pAsset, metadata, ELoadResult::Success);
        }
        else
        {
            NES_ASSERT(IsAssetThread());

            // New memory asset that will be added to our buffer.
            LoadedMemoryAsset memoryAsset
            {
                .m_metadata = metadata,
                .m_pAsset = nullptr,
                .m_requestID = kInvalidRequestID, // Sub-load operations do not affect the Request's progress. 
                .m_result = ELoadResult::Pending,
            };

            // Check if we can set this asset:
            bool canAddAsset = false;
            {
                for (;;)
                {
                    if (!ThreadCanProceed(memoryAsset.m_metadata, memoryAsset.m_result, canAddAsset))
                    {
                        std::this_thread::sleep_for(std::chrono::microseconds(100));
                        continue;
                    }

                    break;
                }
            }

            // Add if necessary.
            if (canAddAsset)
            {
                // Create the asset on the heap.
                memoryAsset.m_pAsset = createAsset(std::move(asset));
                memoryAsset.m_result = ELoadResult::Success;
            
                // Set the result of the load in the map.
                std::lock_guard lock(m_threadLoadedAssetMapMutex);
            
                auto& info = m_threadLoadedAssetMap.at(metadata.m_assetID); 
                info.m_loadResult = memoryAsset.m_result;
                info.m_state = EAssetState::Loaded;
            }

            // Add to the array of memory assets:
            {
                std::lock_guard lock(m_threadMemoryAssetsMutex);
                m_threadMemoryAssets.emplace_back(std::move(memoryAsset));
            }
        }

        return ELoadResult::Success;
    }

    bool AssetManager::AssetThreadProcessInstruction(const EAssetThreadInstruction instruction)
    {
        switch (instruction)
        {
            case EAssetThreadInstruction::ProcessLoadOperations:
            {
                AssetThreadProcessLoadOperations();
                return !m_assetThreadShouldQuit;
            }
        }

        NES_ERROR(kAssetLogTag, "Unhandled Asset Thread Instruction!");
        return false;
    }

    void AssetManager::AssetThreadProcessLoadOperations()
    {
        while (!m_assetThreadShouldQuit && !m_threadJobQueue.IsEmptyLocked())
        {
            // Grab the next request:
            m_threadJobQueue.Lock();
            const auto loadRequest = m_threadJobQueue.Front();
            m_threadJobQueue.Pop();
            m_threadJobQueue.Unlock();
            
            // Run each job.
            for (auto& job : loadRequest.m_jobs)
            {
                if (m_assetThreadShouldQuit)
                    break;
                
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
        
        if (auto* pInfo = GetAssetDesc(id))
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
            NES_ASSERT(m_loadedAssetMap.contains(id));
            
            auto& assetDesc = m_loadedAssetMap.at(id);
            
            // If the asset has been requested again, or it has been freed already, remove from the array.
            if (assetDesc.m_state != EAssetState::Freeing)
            {
                std::swap(m_assetsToFree[i], m_assetsToFree.back());
                m_assetsToFree.pop_back();
                continue;
            }
            
            NES_ASSERT(assetDesc.m_loadedIndex < m_loadedAssets.size());
            NES_ASSERT(assetDesc.m_metadata.m_typeID == m_loadedAssets[assetDesc.m_loadedIndex]->GetTypeID());

            const uint64 freeIndex = assetDesc.m_loadedIndex;
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
                    auto& lastAssetInfo = m_loadedAssetMap.at(lastAssetID);

                    // Update the loaded index for the swapped asset:
                    lastAssetInfo.m_loadedIndex = freeIndex;
                    
                    // Swap:
                    std::swap(m_loadedAssets[freeIndex], m_loadedAssets.back());
                }
                m_loadedAssets.pop_back();

                // Set the Freed State:
                assetDesc.m_state = EAssetState::Freed;
                assetDesc.m_loadedIndex = kInvalidLoadIndex;

                // Remove from the free array.
                std::swap(m_assetsToFree[i], m_assetsToFree.back());
                m_assetsToFree.pop_back();
                continue;
            }

            ++i;
        }
    }

    void AssetManager::ProcessLoadedAsset(AssetBase*& pAsset, const AssetMetadata& metadata, const ELoadResult result, const LoadRequestID requestID)
    {
        // Get the current info, or create a new entry:
        LoadedAssetDesc* pDesc = GetAssetDesc(metadata.m_assetID);
        if (pDesc == nullptr)
        {
            m_loadedAssetMap[metadata.m_assetID] = LoadedAssetDesc
            {
                // [TODO]: Pass in metadata to this function.
                .m_metadata = metadata,
                .m_loadedIndex = kInvalidLoadIndex,
                .m_state = EAssetState::Loading, // Not immediately set to loaded, so that it will not be queued to free.
                .m_loadResult = result
            };
            pDesc = &m_loadedAssetMap.at(metadata.m_assetID);
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
                if (pDesc->IsValid() || pDesc->m_state == EAssetState::Freed)
                {
                    // Delete the memory without storing it.
                    NES_SAFE_DELETE(pAsset);
                }

                // Asset will be added to the loaded assets map:
                else
                {
                    // Set the loaded index and add to the array.
                    pDesc->m_loadedIndex = m_loadedAssets.size();
                    m_loadedAssets.emplace_back(pAsset);
                    pDesc->m_loadResult = result;
            
                    // Set the Asset's internal ID.
                    pAsset->m_id = metadata.m_assetID;

                    // Case: The Asset was requested to be freed when it was loading.
                    // Queue it to be freed on the next sync.
                    if (pDesc->m_state == EAssetState::Freeing)
                    {
                        // Add it to the queue to be freed.
                        // When calling FreeAsset(), assets that are in the Loading state are not
                        // immediately queued. They wait for this case.
                        m_assetsToFree.emplace_back(metadata.m_assetID);
                    }
                    else
                    {
                        pDesc->m_state = EAssetState::Loaded;
                    }
                }
            }
        }

        // If the load failed, free the temporary asset now.
        else
        {
            if (!pDesc->IsValid())
            {
                pDesc->m_state = EAssetState::Freed;
                pDesc->m_loadedIndex = kInvalidLoadIndex;
                pDesc->m_loadResult = result;
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
                    const AsyncLoadResult asyncResult(*pDesc, requestResult.GetProgress());
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

    bool AssetManager::ThreadCanProceed(const AssetMetadata& metadata, ELoadResult& outResult, bool& outShouldLoad)
    {
        std::lock_guard lock(m_threadLoadedAssetMapMutex);

        auto it = m_threadLoadedAssetMap.find(metadata.m_assetID);
        
        // No current info exists, we need to perform the load!
        if (it == m_threadLoadedAssetMap.end())
        {
            m_threadLoadedAssetMap[metadata.m_assetID] = LoadedAssetDesc
            {
                .m_metadata = metadata,
                .m_loadedIndex = kInvalidLoadIndex,
                .m_state = EAssetState::ThreadLoading,
                .m_loadResult = ELoadResult::Pending,
            };
            
            outShouldLoad = true;
        }

        // We have existing info:
        else
        {
            LoadedAssetDesc& info = it->second;

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
