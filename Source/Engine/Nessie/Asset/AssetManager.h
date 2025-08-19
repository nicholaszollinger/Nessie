// AssetManager.h
#pragma once
#include "AssetBase.h"
#include "Nessie/Jobs/JobSystemWorkerThread.h"

#ifdef NES_FORCE_SINGLE_THREADED
#define NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
#else
//----------------------------------------------------------------------------------------------------
/// @brief : If defined, asset loading will be single threaded.  
//----------------------------------------------------------------------------------------------------
//#define NES_FORCE_ASSET_MANAGER_SINGLE_THREADED
#endif

namespace nes
{
    NES_DEFINE_LOG_TAG(kAssetLogTag, "Asset", Warn);

    template <ValidAssetType Type>
    class AssetPtr;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Manages the lifetime of assets. Assets are explicitly loaded and freed.
    ///     - Provides a static API for loading and unloading assets.
    ///     - Assets can be loaded asynchronously with LoadAsync().
    ///     - Multiple load operations can be grouped together in a single LoadRequest. See BeginLoadRequest() for more details.
    ///
    /// @note : When implementing a load function for an asset, if you need to load an asset within that function,
    ///     (like a Mesh needed to load a texture map), use LoadSync(). It will load that dependent Asset synchronously
    ///     on the asset thread.
    //----------------------------------------------------------------------------------------------------
    class AssetManager
    {
        /// Load function used on the Asset Thread. This is used to wrap an Assets load implementation to be performed on the Asset Thread.
        using ThreadLoadFunc = std::function<ELoadResult()>;

        /// Value given to Load Requests to track their completion status.
        using LoadRequestID = uint16;
        struct AssetInfo;

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Result object passed into an OnComplete callback for an Async Load. 
        //----------------------------------------------------------------------------------------------------
        class AsyncLoadResult
        {
        public:
            //----------------------------------------------------------------------------------------------------
            /// @brief : Get the ID for the asset that was trying to load. 
            //----------------------------------------------------------------------------------------------------
            AssetID                     GetAssetID() const      { return m_assetID; }
        
            //----------------------------------------------------------------------------------------------------
            /// @brief : Get the type ID for the asset.
            //----------------------------------------------------------------------------------------------------
            TypeID                      GetAssetTypeID() const;
        
            //----------------------------------------------------------------------------------------------------
            /// @brief : Get the enum result value.
            //----------------------------------------------------------------------------------------------------
            ELoadResult                 GetResult() const;
        
            //----------------------------------------------------------------------------------------------------
            /// @brief : Get whether this load was successful.
            //----------------------------------------------------------------------------------------------------
            bool                        IsValid() const;

            //----------------------------------------------------------------------------------------------------
            /// @brief : The request progress is a value between [0, 1] and is equal to:
            ///     'number of completed loads for the request' / 'total number of loads for the request'.
            //----------------------------------------------------------------------------------------------------
            float                       GetRequestProgress() const { return m_requestProgress; }
            
        private:
            friend class AssetManager;
        
            //----------------------------------------------------------------------------------------------------
            /// @brief : Private constructor for the result. 
            //----------------------------------------------------------------------------------------------------
            explicit                    AsyncLoadResult(const AssetID& id, const AssetInfo& info, const float progress) : m_assetInfo(info), m_assetID(id), m_requestProgress(progress) {}

            const AssetInfo&            m_assetInfo;                // Info about the loaded asset.
            AssetID                     m_assetID;                  // ID of the Asset.
            float                       m_requestProgress = 0.f;    // 
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : A load request is a group of 1 or more load operations that are processed asynchronously.
        //----------------------------------------------------------------------------------------------------
        class LoadRequest
        {
        public:
            using OnAssetLoaded         = std::function<void(const AsyncLoadResult& /*assetResult*/)>;
            using OnComplete            = std::function<void(bool /*wasSuccessful*/)>;
            
        private:
            friend class AssetManager;

            // Private Copy Construct/Assign.
            LoadRequest(const LoadRequest& other) = default;
            LoadRequest& operator=(const LoadRequest& other);
            
        public:
            LoadRequest(LoadRequest&& other) noexcept;
            LoadRequest& operator=(LoadRequest&& other) noexcept;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Add a load operation to this Request.
            ///	@tparam Type : Asset Type you are trying to load.
            /// @param id: Represents the AssetID that will be assigned to the loaded asset. If set to nes::kInvalidAssetID,
            ///     a new ID will be generated and stored in the reference.
            /// @param path : Path to the asset on disk.
            //----------------------------------------------------------------------------------------------------
            template <ValidAssetType Type>
            void                        AppendLoad(AssetID& id, const std::filesystem::path& path);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Set the callback invoked when the request has completed.
            //----------------------------------------------------------------------------------------------------
            void                        SetOnCompleteCallback(const OnComplete& callback)           { m_onComplete = callback; }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Set the callback invoked when a single load operation in the request is completed.
            //----------------------------------------------------------------------------------------------------
            void                        SetOnAssetLoadedCallback(const OnAssetLoaded& callback)     { m_onAssetLoaded = callback; }

        private:
            //----------------------------------------------------------------------------------------------------
            /// @brief : Private constructor for the AssetManager.
            //----------------------------------------------------------------------------------------------------
            explicit                    LoadRequest(AssetManager& manager, const LoadRequestID id);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Adds a Job generated from a call to AppendLoad().
            //----------------------------------------------------------------------------------------------------
            void                        AddJob(const ThreadLoadFunc& job);

        private:
            AssetManager*               m_pAssetManager = nullptr;

            /// The ID unique for this request.
            LoadRequestID               m_requestId;

            /// The number of load jobs that need to be performed for this request. 
            std::vector<ThreadLoadFunc> m_jobs{};

            /// Called when a single asset load operation is completed.
            OnAssetLoaded               m_onAssetLoaded = nullptr;

            /// Called when all load operations on a request are complete, or on the first error. Passes the result of the operation.
            OnComplete                  m_onComplete = nullptr;
        };
    
    public:
        AssetManager();
        ~AssetManager();
        AssetManager(const AssetManager&) = delete;
        AssetManager& operator=(const AssetManager&) = delete;
        AssetManager(AssetManager&&) = delete;
        AssetManager& operator=(AssetManager&&) = delete;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Load an asset, synchronously (meaning this function will not return until complete).
        ///	@tparam Type : Asset Type you are trying to load.
        /// @param id: Represents the AssetID that will be assigned to the loaded asset. If set to nes::kInvalidAssetID,
        ///     a new ID will be generated.
        /// @param path : Path to the asset on disk.
        /// @returns : Result of the load.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        static ELoadResult              LoadSync(AssetID& id, const std::filesystem::path& path);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load an asset, asynchronously. This queues the asset to be loaded on a separate thread.
        ///	@tparam Type : Asset Type you are trying to load.
        /// @param id: Represents the AssetID that will be assigned to the loaded asset. If set to nes::kInvalidAssetID,
        ///     a new ID will be generated.
        /// @param path : Path to the asset on disk.
        /// @param onComplete : Optional callback function to be notified when the Asset is loaded.
        ///     If the asset is already loaded, and the onComplete callback is set, then it will be called immediately.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        static void                     LoadAsync(AssetID& id, const std::filesystem::path& path, const LoadRequest::OnAssetLoaded& onComplete = nullptr);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Begin a new load request that groups multiple load operations together.
        ///
        /// Example Usage: <code>
        ///     LoadRequest request = BeginLoadRequest();               // Create a new LoadRequest.
        ///     request.AppendLoad<Texture>("Test.png");                // Adds loading "Test.png" to this request.
        ///     request.AppendLoad<Mesh>("Mesh.gltf");                  // Adds loading "Mesh.gltf" to this request.
        ///     AssetManager::SubmitLoadRequest(std::move(request));    // Submits the load request to the Asset Thread.
        /// </code>
        //----------------------------------------------------------------------------------------------------
        static LoadRequest              BeginLoadRequest();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Submits a load request to the Asset Thread.
        /// @param request : Request object to be submitted. Should be moved into this function.
        ///
        /// Example Usage: <code>
        ///     LoadRequest request = BeginLoadRequest();               // Create a new LoadRequest.
        ///     request.AppendLoad<Texture>("Test.png");                // Adds loading "Test.png" to this request.
        ///     request.AppendLoad<Mesh>("Mesh.gltf");                  // Adds loading "Mesh.gltf" to this request.
        ///     AssetManager::SubmitLoadRequest(std::move(request));    // Submits the load request to the Asset Thread.
        /// </code>
        //----------------------------------------------------------------------------------------------------
        static void                     SubmitLoadRequest(LoadRequest&& request);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Add an asset that was constructed outside the AssetManager. This will take ownership of
        ///     the object.
        /// @param id: Represents the AssetID that will be assigned to the loaded asset. If set to nes::kInvalidAssetID,
        ///     a new ID will be generated.
        /// @param asset : Asset that will be stored in the Asset Manager.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        static ELoadResult              AddMemoryAsset(AssetID& id, Type&& asset);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a loaded asset. If the asset has not been loaded, the reference will be invalid.
        /// @note : Assets can only be accessed on the main thread.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        static AssetPtr<Type>           GetAsset(const AssetID& id);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks if the asset has been loaded and is usable.
        //----------------------------------------------------------------------------------------------------
        static bool                     IsValidAsset(const AssetID& id);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free an asset associated with the given id.
        //----------------------------------------------------------------------------------------------------
        static void                     FreeAsset(const AssetID& id);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Generates a new, unique asset id. 
        //----------------------------------------------------------------------------------------------------
        static AssetID                  GenerateAssetID();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Generate an asset id from a path. 
        //----------------------------------------------------------------------------------------------------
        static constexpr AssetID        GenerateAssetIDFromPath(const std::filesystem::path& path) { return HashString64(path.string().c_str()); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the current thread is the Asset Thread.
        //----------------------------------------------------------------------------------------------------
        static bool                     IsAssetThread();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the AssetManager - this will start the Asset Thread.
        //----------------------------------------------------------------------------------------------------
        bool                            Init();

        //----------------------------------------------------------------------------------------------------
        /// @brief : This will process all finished load operations processed on the Asset thread.
        ///     Any failed loads will have their assets destroyed.
        //----------------------------------------------------------------------------------------------------
        void                            SyncFrame();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Shutdown the AssetManager. This will destroy all loaded assets.
        //----------------------------------------------------------------------------------------------------
        void                            Shutdown();
    
    private:
        // Indicates a loaded memory asset does not belong to a request. This can be used to indicate that an Asset
        // loaded another asset as part of its load function. Ex: A Mesh loading a Texture.
        static constexpr LoadRequestID  kInvalidRequestID = std::numeric_limits<LoadRequestID>::max();
        static constexpr uint64         kInvalidLoadIndex = std::numeric_limits<uint64>::max();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : State information about the Asset, including Type, State flags, and Load result. 
        //----------------------------------------------------------------------------------------------------
        struct AssetInfo
        {
            uint64                      m_loadedIndex   = kInvalidLoadIndex;        // Index in the Loaded Assets array. Only set if successfully loaded.
            TypeID                      m_typeID        = 0;                        // Type of asset.
            EAssetState                 m_state         = EAssetState::Invalid;     // The current status of the Asset.
            ELoadResult                 m_loadResult    = ELoadResult::Pending;     // The result of the load operation.

            //----------------------------------------------------------------------------------------------------
            /// @brief : An asset is considered 'valid' if it is in the Loaded state and the load was successful.
            //----------------------------------------------------------------------------------------------------
            bool                        IsValid() const   { return m_loadedIndex != kInvalidLoadIndex && m_state == EAssetState::Loaded && m_loadResult == ELoadResult::Success; }
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Information about the status of an open Load Request. Owned by the main thread.
        //----------------------------------------------------------------------------------------------------
        struct LoadRequestStatus
        {
            LoadRequest::OnComplete     m_onCompleted{};        // Callback for responding to the request completed.
            LoadRequest::OnAssetLoaded  m_onAssetLoaded{};      // Callback for responding to individual asset loads.
            uint16                      m_numLoads = 1;         // Number of loads that are needed to complete the request
            uint16                      m_completedLoads = 0;   // Number of loads completed so far.
            uint16                      m_successfulLoads = 0;  // Number of loads that returned ELoadResult::Success.
            
            //----------------------------------------------------------------------------------------------------
            /// @brief : Returns a value from [0, 1] for the current progress of an operation.
            //----------------------------------------------------------------------------------------------------
            float                       GetProgress() const { return static_cast<float>(m_completedLoads) / static_cast<float>(m_numLoads); }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Returns true if all load operations have completed. 
            //----------------------------------------------------------------------------------------------------
            bool                        IsComplete() const { return m_completedLoads == m_numLoads; }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Returns if all assets were loaded successfully.
            //----------------------------------------------------------------------------------------------------
            bool                        IsSuccessful() const { return m_numLoads == m_successfulLoads; }
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : A LoadMemoryAsset is created on the Asset Thread, and processed on the Main Thread.
        ///     - The Asset itself will be destroyed on the main thread only.
        ///     - Attached to a request ID for load status tracking.
        //----------------------------------------------------------------------------------------------------
        struct LoadedMemoryAsset
        {
            AssetBase*                  m_pAsset = nullptr;                 // The Loaded Asset itself.
            AssetID                     m_id = kInvalidAssetID;             // Unique ID of the Asset.
            TypeID                      m_typeID = 0;                       // TypeID of the asset.
            LoadRequestID               m_requestID = kInvalidRequestID;    // Which Load Request this asset was loaded for.
            ELoadResult                 m_result = ELoadResult::Success;    // The result of the load operation.
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Instructions for the Asset Thread. 
        //----------------------------------------------------------------------------------------------------
        enum class EAssetThreadInstruction
        {
            ProcessLoadOperations,  // Signal that the Asset Thread should wake and begin the load operations (if available).
        };

        // Asset Thread Type:
        using AssetThread = WorkerThread<EAssetThreadInstruction>;

        // Array of loaded assets. The index for a particular asset is located in its AssetInfo.
        using AssetArray = std::vector<AssetBase*>;

        // Asset Info Map: Maps an AssetID to the Asset's Info.
        using AssetInfoMap = std::unordered_map<AssetID, AssetInfo, UUIDHasher>;

        // Load function. This is used to wrap an Assets load implementation to be performed on the Asset Thread.
        using ThreadLoadFunc = std::function<ELoadResult()>;

        // Queue of Load functions to be executed on the Asset Thread.
        using ThreadJobQueue = ThreadSafeQueue<LoadRequest>;

        // Buffer of Memory Assets that have been loaded, but not processed on the main thread.
        using MemoryAssetBuffer = std::vector<LoadedMemoryAsset>;

        // Maps a Request ID to its status.
        using RequestMap = std::unordered_map<LoadRequestID, LoadRequestStatus>;
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper to assert that the instance is valid, and return as a reference. Only needed
        ///     internally.
        //----------------------------------------------------------------------------------------------------
        static AssetManager&            GetInstance();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if this thread is the main thread.
        //----------------------------------------------------------------------------------------------------
        static bool                     IsMainThread();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Asset Info for an asset if it exists. 
        //----------------------------------------------------------------------------------------------------
        AssetInfo*                      GetAssetInfo(const AssetID& id);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Main thread synchronous load implementation.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        ELoadResult                     MainLoadSync(const AssetID& id, const std::filesystem::path& path);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Asset thread synchronous load implementation.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        ELoadResult                     ThreadLoadSync(const AssetID& id, const std::filesystem::path& path, const LoadRequestID requestID = kInvalidRequestID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Asset thread's instruction handler.
        //----------------------------------------------------------------------------------------------------
        bool                            AssetThreadProcessInstruction(const EAssetThreadInstruction instruction);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Runs available load operations, until there is no more work, or if we have caught up to
        ///     the main thread in the ring buffer and are waiting.
        //----------------------------------------------------------------------------------------------------
        void                            AssetThreadProcessLoadOperations();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Queues the asset to be freed on the next Frame Sync.
        //----------------------------------------------------------------------------------------------------
        void                            QueueFreeAsset(const AssetID& id);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free any assets that have been queued to free and contain no more locks. 
        //----------------------------------------------------------------------------------------------------
        void                            ProcessFreeQueue();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Update the Asset Info of a newly loaded asset. This will update the Load Request Result if
        ///     the requestID is valid as well. The asset is assumed to be valid, and will be deleted if
        ///     the result has failed.
        //----------------------------------------------------------------------------------------------------
        void                            ProcessLoadedAsset(AssetBase*& pAsset, const TypeID typeID, const AssetID& id, const ELoadResult result, const LoadRequestID requestID = kInvalidRequestID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks to see if the thread can proceed with the current Asset, and determines a load needs to occur.
        ///     If another thread is actively loading this Asset, this will return false.
        /// @param id : The ID of the Asset to load. 
        /// @param typeID : The typeID of the Asset. 
        ///	@param outResult : If 'outShouldLoad' is false, then this is the existing result value.
        ///	@param outShouldLoad : Whether a load needs to be performed. If false, then 'outResult' contains the
        ///     previous load result.
        ///	@returns : If false, then another thread is attempting to load this Asset.
        //----------------------------------------------------------------------------------------------------
        bool                            ThreadCanProceed(const AssetID& id, const TypeID& typeID, ELoadResult& outResult, bool& outShouldLoad);
        
    private:
        AssetInfoMap                    m_infoMap{};                        // Owned by the main thread. Maps an AssetID to the state of the Asset.
        AssetInfoMap                    m_threadInfoMap{};                  // Owned by the asset thread. Maps the AssetID to the state of the Asset.
        RequestMap                      m_requestStatusMap{};               // Owned by the main thread. Maps a RequestID to the request's status.
        AssetArray                      m_loadedAssets{};                   // Owned by the main thread. Array of loaded assets.
        std::vector<AssetID>            m_assetsToFree{};                   // Array of Assets that are waiting to be freed.
        AssetThread                     m_assetThread;                      // Asset thread object.            
        ThreadJobQueue                  m_threadJobQueue{};                 // Queue of jobs to process. Both threads read and write to this queue.
        MemoryAssetBuffer               m_threadMemoryAssets{};             // Buffer of Assets that have been loaded as on the Asset Thread.         
        Mutex                           m_threadMemoryAssetsMutex;          // Mutex used to restrict access to the Asset Thread's memory asset buffer.
        Mutex                           m_threadInfoMapMutex;               // Mutex to restrict access to the thread's asset info map. 
        LoadRequestID                   m_nextRequestID = 0;                // Counter incremented when a new request is made, used to set the ID of a LoadRequest.
        bool                            m_threadInfoMapNeedsSync = false;   // Flag to indicate that the Asset Thread needs an updated copy of the asset info map. 
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : An AssetPtr is a non-owning reference to an Asset. To get a valid AssetPtr, you must request it
    ///     from the AssetManager by calling GetAsset().
    ///
    ///     Creating an AssetPtr will add a "lock" on the asset. Assets will not be freed if there are any
    ///     locks present. An AssetPtr is not meant to be stored long term - just for use within a scope.
    ///     You can store an AssetID as a member variable and use that to create an AssetPtr when you need to.
    //----------------------------------------------------------------------------------------------------
    template <ValidAssetType Type>
    class AssetPtr
    {
    public:
        /* Ctor */                      AssetPtr() = default;
        /* Nullptr Ctor*/               AssetPtr(std::nullptr_t) : AssetPtr() {}
        /* Copy Ctor */                 AssetPtr(const AssetPtr& other);
        /* Move Ctor */                 AssetPtr(AssetPtr&& other) noexcept;
        /* Copy Assign */               AssetPtr& operator=(const AssetPtr& other);
        /* Move Assign */               AssetPtr& operator=(AssetPtr&& other) noexcept;
        /* Nullptr Assign */            AssetPtr& operator=(nullptr_t);
        /* Destructor */                ~AssetPtr() { RemoveLock(); }

        [[nodiscard]] inline            operator Type*() const                      { NES_ASSERT(IsValid()); return m_pAsset; }
        [[nodiscard]] inline            operator bool() const                       { NES_ASSERT(IsValid()); return m_pAsset != nullptr; }
        [[nodiscard]] inline bool       operator==(const AssetPtr& other) const     { NES_ASSERT(IsValid()); return m_pAsset == other.m_pAsset; }
        [[nodiscard]] inline bool       operator!=(const AssetPtr& other) const     { return !(*this == other); }
        [[nodiscard]] inline bool       operator==(const std::nullptr_t) const      { return m_pAsset == nullptr; }
        [[nodiscard]] inline bool       operator!=(const std::nullptr_t) const      { return m_pAsset != nullptr; }
        [[nodiscard]] inline Type*      operator->() const                          { NES_ASSERT(IsValid()); return m_pAsset; }
        [[nodiscard]] inline Type&      operator*() const                           { NES_ASSERT(IsValid()); return *m_pAsset; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get this AssetPtr cast to base or derived class.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType OtherType> requires TypeIsBaseOrDerived<Type, OtherType>
        AssetPtr<OtherType>             Cast() const;
        
    private:
        friend class AssetManager;
        
        /// Friend declaration to allow Cast operation to use the private Ctor. 
        template <ValidAssetType OtherType>
        friend class AssetPtr;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Private Ctor to set the Asset reference. This is only usable by the AssetManager.
        //----------------------------------------------------------------------------------------------------
        explicit                        AssetPtr(Type* pAsset);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a lock to the underlying Asset. 
        //----------------------------------------------------------------------------------------------------
        void                            AddLock() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove a lock to the underlying Asset.
        //----------------------------------------------------------------------------------------------------
        void                            RemoveLock() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks that the Asset is either null or has not been freed from the AssetManager.
        ///     If the pointer is not null and not in the AssetManager, then the pointer is dangling!
        //----------------------------------------------------------------------------------------------------
        bool                            IsValid() const { return m_pAsset == nullptr || AssetManager::IsValidAsset(m_id); }

    private:
        Type*                           m_pAsset = nullptr;         // The Asset reference.
        AssetID                         m_id = kInvalidAssetID;     // The Asset's ID. This is used to ensure that the pointer still points to an asset.
    };
    
    using LoadRequest = AssetManager::LoadRequest;
    using AsyncLoadResult = AssetManager::AsyncLoadResult;
}


#include "AssetManager.inl"
