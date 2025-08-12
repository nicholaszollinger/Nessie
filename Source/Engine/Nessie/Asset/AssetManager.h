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
    // NOTES:
    // I got async asset loading to work, but when I have time, I'd like to make some changes (or at least consider them).
    //  1. Change LoadSync() and LoadAsync() to simply Load(). Always defer to loading asynchronously. If you
    //     do this, you need to have a way to wait for a particular load operation (WaitForLoad(AsyncLoadRequest/Result/...)).
    //  2. 'AssetLoadStatus'. I want a way to know how far along a load operation is for things like a loading bar.
    //     Using the BarrierImpl's 'm_numLeftToAcquire' (more on a Job System below) would be a good
    //     value to track. Every increment to that value would increase the 'total' value, and every decrement would decrease
    //     the 'remaining' value. This would have to be locked by mutex, of course.
    //  3. Figure out what to return from the LoadAsync. The return value should have an implicit boolean conversion
    //     to denote if the Asset is ready to go, and be able to be used to get information about the load status.
    //  4. Asset Listeners. Dispatch an event to those who are waiting on a particular asset to load.
    //  5. Use the Job System classes. I spent a long time trying to figure this out, but I just went with the
    //     current implementation to get some momentum again. I believe that a Barrier can be tied to a single
    //     call to Load(). The main thread could then use that barrier to wait for a particular operation to finish.
    //     Plus, dependent jobs, like loading texture maps when loading a mesh, could use this same barrier to add
    //     Jobs to. A LoadContext object that contains the AssetID of the original load request and the barrier could
    //     be passed to an overload of the Load function to help denote dependent load operations.
    //
    /// @brief : Manages the lifetime of assets. Assets are explicitly loaded and freed.
    ///     Provides a static API for loading and unloading assets.
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
        /// @brief : Load an asset, synchronously (meaning this function will not return until complete).
        ///	@tparam Type : Asset Type you are trying to load.
        /// @param id: Represents the AssetID that will be assigned to the loaded asset. If set to nes::kInvalidAssetID,
        ///     a new ID will be generated.
        /// @param path : Path to the asset on disk.
        /// @returns : Result of the load.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        static ELoadResult          LoadSync(AssetID& id, const std::filesystem::path& path);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load an asset, asynchronously. This queues the asset to be loaded on a separate thread.
        ///	@tparam Type : Asset Type you are trying to load.
        /// @param id: Represents the AssetID that will be assigned to the loaded asset. If set to nes::kInvalidAssetID,
        ///     a new ID will be generated.
        /// @param path : Path to the asset on disk.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        static ELoadResult          LoadAsync(AssetID& id, const std::filesystem::path& path);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Add an asset that was constructed outside the AssetManager. This will take ownership of
        ///     the object.
        /// @param id: Represents the AssetID that will be assigned to the loaded asset. If set to nes::kInvalidAssetID,
        ///     a new ID will be generated.
        /// @param asset : Asset that will be stored in the appropriate asset pool.
        /// @returns : If the AssetPool is full, this will return EAssetResult::OutOfMemory and the Asset will be freed.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        static ELoadResult          AddMemoryAsset(AssetID& id, Type&& asset);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a loaded asset. If the asset has not been loaded, the reference will be invalid.
        /// @note : Assets can only be accessed on the main thread.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        static AssetPtr<Type>       GetAsset(const AssetID& id);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks if the asset has been loaded.
        //----------------------------------------------------------------------------------------------------
        static bool                 IsValidAsset(const AssetID& id);

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
        /// @brief : Returns true if the current thread is the Asset Thread.
        //----------------------------------------------------------------------------------------------------
        static bool                 IsAssetThread();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the AssetManager - this will start the Asset Thread.
        //----------------------------------------------------------------------------------------------------
        bool                        Init();

        //----------------------------------------------------------------------------------------------------
        /// @brief : This will process all finished load operations processed on the Asset thread.
        ///     Any failed loads will have their assets destroyed.
        //----------------------------------------------------------------------------------------------------
        void                        SyncFrame();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Shutdown the AssetManager. This will destroy all loaded assets.
        //----------------------------------------------------------------------------------------------------
        void                        Shutdown();
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : State information about the Asset, including Type, State flags, and Load result. 
        //----------------------------------------------------------------------------------------------------
        struct AssetInfo
        {
            TypeID                  m_typeID        = 0;                        // Type of asset.
            EAssetState             m_state         = EAssetState::Invalid;     // The current status of the Asset.
            ELoadResult             m_loadResult    = ELoadResult::Invalid;     // The result of the load operation.

            //----------------------------------------------------------------------------------------------------
            /// @brief : An asset is considered 'valid' if it is in the Loaded state and the load was successful.
            //----------------------------------------------------------------------------------------------------
            bool                    IsValid() const   { return m_state == EAssetState::Loaded && m_loadResult == ELoadResult::Success; }
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Contains the Asset Location and whether the load operation was successful.
        //----------------------------------------------------------------------------------------------------
        struct alignas (NES_CACHE_LINE_SIZE) AssetLoadResult
        {
            // Number of Assets that have been loaded as a part of the operation.
            uint32                  m_numMemoryAssets = 0;

            // Result of the load operation.
            ELoadResult             m_result = ELoadResult::Invalid;

            // Boolean operator that returns true if the load was successful.
            inline                  operator bool() const { return m_result == ELoadResult::Success; }
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : A LoadMemoryAsset is created when an asset load operation needs to load other assets as
        ///     a part of the load process. Until the FrameSync(), this loaded asset does not exist in the
        ///     loaded asset map.
        //----------------------------------------------------------------------------------------------------
        struct LoadedMemoryAsset
        {
            // The loaded asset.
            AssetBase*              m_pAsset = nullptr;

            // ID of the Asset.
            AssetID                 m_id = kInvalidAssetID;

            // Type of Asset.
            TypeID                  m_typeID = 0;
            
            // The index of the result that was responsible for loading this asset. Used to process all 
            // memory assets tied to a single load operation during Sync.
            uint32                  m_resultIndex = std::numeric_limits<uint32>::max();

            // The Result from the load operation.
            ELoadResult             m_result = ELoadResult::Success;
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

        // Result Buffer Type: An array of results managed as a ring buffer.
        static constexpr uint32     kMaxLoadOperations = 256;
        using ResultBuffer = std::array<AssetLoadResult, kMaxLoadOperations>;

        // Map of loaded assets.
        using AssetMap = std::unordered_map<AssetID, AssetBase*, UUIDHasher>;

        // Asset Location Map: Maps an AssetID to the Asset's Info.
        using AssetInfoMap = std::unordered_map<AssetID, AssetInfo, UUIDHasher>;

        // Load function. This is used to wrap an Assets load implementation to be performed on the Asset Thread.
        using ThreadLoadFunc = std::function<ELoadResult()>;

        // Queue of Load functions to be executed on the Asset Thread.
        using ThreadJobQueue = ThreadSafeQueue<ThreadLoadFunc>;

        // Buffer of Memory Assets that have been loaded, but not added to the Asset Map.
        using MemoryAssetBuffer = std::vector<LoadedMemoryAsset>;
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper to assert that the instance is valid, and return as a reference. Only needed
        ///     internally.
        //----------------------------------------------------------------------------------------------------
        static AssetManager&        GetInstance();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if this thread is the main thread.
        //----------------------------------------------------------------------------------------------------
        static bool                 IsMainThread();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Main thread synchronous load implementation.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        ELoadResult                 MainLoadSync(const AssetID& id, const std::filesystem::path& path);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Asset thread synchronous load implementation.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        ELoadResult                 ThreadLoadSync(const AssetID& id, const std::filesystem::path& path);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Queues an asset to be loaded on the Asset Thread.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        ELoadResult                 QueueLoadAsset(AssetID& id, const std::filesystem::path& path);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add memory asset implementation.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType Type>
        ELoadResult                 AddMemoryAssetImpl(const AssetID& id, Type&& asset);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Asset thread's instruction handler.
        //----------------------------------------------------------------------------------------------------
        bool                        AssetThreadProcessInstruction(const EAssetThreadInstruction instruction);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Runs available load operations, until there is no more work, or if we have caught up to
        ///     the main thread in the ring buffer and are waiting.
        //----------------------------------------------------------------------------------------------------
        void                        AssetThreadProcessLoadOperations();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Queues the asset to be freed on the next Frame Sync.
        //----------------------------------------------------------------------------------------------------
        void                        QueueFreeAsset(const AssetID& id);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free any assets that have been queued to free and contain no more locks. 
        //----------------------------------------------------------------------------------------------------
        void                        ProcessFreeQueue();
        
    private:
        AssetInfoMap                m_infoMap{};                    // Owned by the main thread. Maps an AssetID to the state of the Asset.
        AssetInfoMap                m_threadInfoMap{};              // Owned by the asset thread. Maps the AssetID to the state of the Asset.
        AssetMap                    m_loadedAssets{};               // Owned by the main thread. Maps an AssetID to the loaded Asset object.
        std::vector<AssetID>        m_assetsToFree{};               // Array of Assets that are waiting to be freed.
        AssetThread                 m_assetThread;                  // Asset thread object.            
        ResultBuffer                m_results{};                    // Used as a ring buffer.
        ThreadJobQueue              m_threadJobQueue{};             // Queue of jobs to process. Both threads read and write to this queue.
        MemoryAssetBuffer           m_threadMemoryAssets{};         // Buffer of Assets that have been loaded as a part of a single load operation. They will be stored in the AssetManager on FrameSync().         
        Mutex                       m_threadMemoryAssetsMutex;      // Mutex used to restrict access to the Asset Thread's memory asset buffer.
        Mutex                       m_threadInfoMapMutex;           // Mutex to restrict access to the thread's asset info map. 
        std::atomic<uint32>         m_resultReadIndex = 0;          // Position for the main thread to read completed results. Updated by the Main Thread.
        std::atomic<uint32>         m_resultWriteIndex = 0;         // Position for the Asset thread to write results. Updated by the Asset Thread.
        bool                        m_threadInfoMapNeedsSync = false; // Flag to indicate that the Asset Thread needs a new copy of the asset info map. 
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
        /* Ctor */                  AssetPtr() = default;
        /* Nullptr Ctor*/           AssetPtr(std::nullptr_t) : AssetPtr() {}
        /* Copy Ctor */             AssetPtr(const AssetPtr& other);
        /* Move Ctor */             AssetPtr(AssetPtr&& other) noexcept;
        /* Copy Assign */           AssetPtr& operator=(const AssetPtr& other);
        /* Move Assign */           AssetPtr& operator=(AssetPtr&& other) noexcept;
        /* Nullptr Assign */        AssetPtr& operator=(nullptr_t);
        /* Destructor */            ~AssetPtr() { RemoveLock(); }

        [[nodiscard]] inline        operator Type*() const                      { NES_ASSERT(IsValid()); return m_pAsset; }
        [[nodiscard]] inline        operator bool() const                       { NES_ASSERT(IsValid()); return m_pAsset != nullptr; }
        [[nodiscard]] inline bool   operator==(const AssetPtr& other) const     { NES_ASSERT(IsValid()); return m_pAsset == other.m_pAsset; }
        [[nodiscard]] inline bool   operator!=(const AssetPtr& other) const     { return !(*this == other); }
        [[nodiscard]] inline bool   operator==(const std::nullptr_t) const      { return m_pAsset == nullptr; }
        [[nodiscard]] inline bool   operator!=(const std::nullptr_t) const      { return m_pAsset != nullptr; }
        [[nodiscard]] inline Type*  operator->() const                          { NES_ASSERT(IsValid()); return m_pAsset; }
        [[nodiscard]] inline Type&  operator*() const                           { NES_ASSERT(IsValid()); return *m_pAsset; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get this AssetPtr cast to base or derived class.
        //----------------------------------------------------------------------------------------------------
        template <ValidAssetType OtherType> requires TypeIsBaseOrDerived<Type, OtherType>
        AssetPtr<OtherType>         Cast() const;
        
    private:
        friend class AssetManager;
        
        /// Friend declaration to allow Cast operation to use the private Ctor. 
        template <ValidAssetType OtherType>
        friend class AssetPtr;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Private Ctor to set the Asset reference. This is only usable by the AssetManager.
        //----------------------------------------------------------------------------------------------------
        explicit                    AssetPtr(Type* pAsset);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a lock to the underlying Asset. 
        //----------------------------------------------------------------------------------------------------
        void                        AddLock() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove a lock to the underlying Asset.
        //----------------------------------------------------------------------------------------------------
        void                        RemoveLock() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks that the Asset is either null or has not been freed from the AssetManager.
        ///     If the pointer is not null and not in the AssetManager, then the pointer is dangling!
        //----------------------------------------------------------------------------------------------------
        bool                        IsValid() const { return m_pAsset == nullptr || AssetManager::IsValidAsset(m_id); }

    private:
        Type*                       m_pAsset = nullptr;         // The Asset reference.
        AssetID                     m_id = kInvalidAssetID;     // The Asset's ID. This is used to ensure that the pointer still points to an asset.
    };
}

#include "AssetManager.inl"
