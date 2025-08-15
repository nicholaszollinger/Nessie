// AssetBase.h
#pragma once
#include <filesystem>
#include "Nessie/Core/Memory/RefCounter.h"
#include "Nessie/Core/TypeInfo.h"
#include "Nessie/Random/UUID.h"

namespace nes
{
    using AssetID = UUID;
    static constexpr AssetID kInvalidAssetID = std::numeric_limits<UUID::ValueType>::max();

    //----------------------------------------------------------------------------------------------------
    /// @brief : Current status of the Asset. 
    //----------------------------------------------------------------------------------------------------
    enum class EAssetState : uint8
    {
        Invalid,            // Initial state.
        Loading,            // The Asset is currently being loaded on the Asset Thread.
        Loaded,             // The Asset has been loaded. However, the load could have failed. This just says that the load operation has completed. See EAssetResult.
        //NeedsReload,      // TODO: If the asset on disk has been updated, this would be set as the status.
        Freeing,            // The Asset has been requested to be freed. Once the Asset has no more locks, it will be freed.
        Freed,              // The Asset has been freed from memory. It can be loaded again.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Possible results when trying to load an Asset.
    //----------------------------------------------------------------------------------------------------
    enum class ELoadResult : uint8
    {
        Pending,            // Initial State. Used to indicate that the Asset is being loaded.
        Success,            // The Load was successful.
        Failure,            // The Load failed.
        MissingDependency,  // The Asset failed to load a dependency.
        InvalidArgument,    // Input params for the load were invalid.
    };
    // [TODO]: 
    //constexpr const char* GetAssetResultString(const EAssetResult result);
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for all Assets. The destructor should handle all clean up.
    //----------------------------------------------------------------------------------------------------
    class AssetBase
    {
    public:
        /* Default Ctor */  AssetBase() = default;
        /* No Copy */       AssetBase(const AssetBase&) = delete;
        /* No Copy Assign*/ AssetBase& operator=(const AssetBase&) = delete;
        /* Move Ctor */     AssetBase(AssetBase&& other) noexcept;
        /* Move Assign */   AssetBase& operator=(AssetBase&& other) noexcept;
        virtual             ~AssetBase() = default;

        bool                operator==(const AssetBase& other) const    { return m_id == other.m_id; } // && m_handle == other.m_handle; }
        bool                operator!=(const AssetBase& other) const    { return m_id != other.m_id; } // || m_handle != other.m_handle; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the name of the Asset Type.
        //----------------------------------------------------------------------------------------------------
        virtual const char* GetTypename() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the ID of the type.
        //----------------------------------------------------------------------------------------------------
        virtual TypeID      GetTypeID() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Asset's unique ID.
        //----------------------------------------------------------------------------------------------------
        AssetID             GetAssetID() const                          { return m_id; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of locks on this Asset.
        //----------------------------------------------------------------------------------------------------
        uint32              GetNumLocks() const                         { return m_lockCount.load(std::memory_order_relaxed); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if there are locks on this Asset. An asset cannot be freed if there are locks present.
        ///     The lock count is managed by the AssetPtr class.
        //----------------------------------------------------------------------------------------------------
        bool                HasLocks() const                            { return GetNumLocks() > 0; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a lock to this Asset. An asset cannot be freed if there are locks present.
        ///     The lock count is managed by the AssetPtr class.
        //----------------------------------------------------------------------------------------------------
        void                AddLock() const                             { m_lockCount.fetch_add(1, std::memory_order_relaxed); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove a lock to this Asset. An asset cannot be freed if there are locks present.
        ///     The lock count is managed by the AssetPtr class.
        //----------------------------------------------------------------------------------------------------
        void                RemoveLock() const;
    

    private:
        friend class AssetManager;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Override to support loading an asset from a filepath. If the result != Success,
        ///     then the Asset will be destroyed.
        //----------------------------------------------------------------------------------------------------
        virtual ELoadResult LoadFromFile([[maybe_unused]] const std::filesystem::path& path) { return ELoadResult::Failure; }

    private:
        mutable std::atomic<uint32> m_lockCount = 0;
        AssetID             m_id = kInvalidAssetID;         /// Unique identifier for this specific asset.
        //AssetHandle         m_handle = kInvalidAssetHandle; /// Index into the AssetPool.
    };

    template <typename Type>
    concept ValidAssetType = TypeIsDerivedFrom<Type, AssetBase>
        && std::is_default_constructible_v<Type>
        && HasValidTypeInfo<Type>;
}
