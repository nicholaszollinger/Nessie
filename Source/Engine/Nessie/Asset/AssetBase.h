// AssetBase.h
#pragma once
#include <filesystem>
#include "Nessie/Core/Memory/RefCounter.h"
#include "Nessie/Core/TypeInfo.h"
#include "Nessie/Random/UUID.h"
#include "Nessie/FileIO/YAML/Serializers/YamlCoreSerializers.h"

namespace nes
{
    using AssetID = UUID;
    static constexpr AssetID kInvalidAssetID = UUID(); // Default constructs with std::numeric_limits<uint64>::max().

    //----------------------------------------------------------------------------------------------------
    /// @brief : Current status of the Asset. 
    //----------------------------------------------------------------------------------------------------
    enum class EAssetState : uint8
    {
        Invalid,                // Initial state.
        Loading,                // The Asset is queued to load on the Asset Thread.
        ThreadLoading,          // The Asset is currently being loaded by the Asset Thread.
        Loaded,                 // The Asset has completed its load operation. If the load was successful, this Asset is now considered valid and can be used.  
        //NeedsReload,          // TODO: If the asset on disk has been updated, this would be set as the status.
        Freeing,                // The Asset has been requested to be freed. Once the Asset has no more locks, it will be freed.
        Freed,                  // The Asset has been freed from memory. It can be loaded again.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Possible results when trying to load an Asset.
    //----------------------------------------------------------------------------------------------------
    enum class ELoadResult : int8
    {
        Pending = -1,           // Initial State. Used to indicate that the Asset is being loaded.
        Success = 0,            // The Load was successful.
        Failure = 1,            // The Load failed.
        MissingDependency = 2,  // The Asset failed to load a dependency.
        InvalidArgument = 3,    // Input params for the load were invalid.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Returns a string representation of the ELoadResult value.
    //----------------------------------------------------------------------------------------------------
    constexpr const char* GetLoadResultString(const ELoadResult result)
    {
        switch (result)
        {
            case ELoadResult::Pending: return "Pending";
            case ELoadResult::Success: return "Success";
            case ELoadResult::Failure: return "Failure";
            case ELoadResult::MissingDependency: return "MissingDependency";
            case ELoadResult::InvalidArgument: return "InvalidArgument";
        }
        
        return "Unknown";
    }
    
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
        /// @brief : Save an Asset to the given filepath.
        //----------------------------------------------------------------------------------------------------
        virtual void        SaveToFile(const std::filesystem::path&) {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Override to support loading an asset from a filepath. If the result != Success,
        ///     then the Asset will be destroyed.
        //----------------------------------------------------------------------------------------------------
        virtual ELoadResult LoadFromFile([[maybe_unused]] const std::filesystem::path& path) { return ELoadResult::Failure; }

    private:
        mutable std::atomic<uint32> m_lockCount = 0;
        AssetID             m_id = kInvalidAssetID;         /// Unique identifier for this specific asset.
    };

    template <typename Type>
    concept ValidAssetType = std::same_as<Type, AssetBase> ||
        (TypeIsDerivedFrom<Type, AssetBase>
        && std::is_default_constructible_v<Type>
        && std::is_move_assignable_v<Type>
        && std::is_move_constructible_v<Type>
        && HasValidTypeInfo<Type>);

    static_assert(ValidAssetType<AssetBase>);
    
    template <ValidAssetType Type>
    class AssetPtr;
}

