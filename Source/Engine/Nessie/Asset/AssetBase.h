// AssetBase.h
#pragma once
#include <filesystem>
#include "Nessie/Core/Memory/RefCounter.h"
#include "Nessie/Core/TypeInfo.h"
#include "Nessie/Random/UUID.h"

namespace nes
{
    using AssetHandle = uint32;
    using AssetID = UUID;

    static constexpr AssetID kInvalidAssetID = std::numeric_limits<UUID::ValueType>::max();
    static constexpr AssetHandle kInvalidAssetHandle = std::numeric_limits<AssetHandle>::max();

    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //  Assets maintain a ref count (managed by the RefTarget classes), but they don't destroy themselves
    //  on RefCount becoming zero. I don't want to reload a resource if it isn't in use for a short
    //  period.
    //		
    /// @brief : Base class for all Assets.  
    //----------------------------------------------------------------------------------------------------
    class AssetBase : public RefTarget<AssetBase>
    {
    public:
        AssetBase() = default;
        AssetBase(const AssetBase&) = delete;
        AssetBase& operator=(const AssetBase&) = delete;
        AssetBase(AssetBase&&) noexcept = default;
        AssetBase& operator=(AssetBase&&) noexcept = default;

        bool                operator==(const AssetBase& other) const    { return m_id == other.m_id && m_handle == other.m_handle; }
        bool                operator!=(const AssetBase& other) const    { return m_id != other.m_id || m_handle != other.m_handle; }

        virtual const char* GetTypename() const = 0;
        virtual TypeID      GetTypeID() const = 0;
        AssetHandle         GetAssetHandle() const                      { return m_handle; }
        AssetID             GetAssetID() const                          { return m_id; }

    private:
        friend class AssetManager;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Override to support loading an asset from a filepath. Return false if there was an error
        ///     loading the asset.
        //----------------------------------------------------------------------------------------------------
        virtual bool        LoadFromFile([[maybe_unused]] const std::filesystem::path& path) { return false; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free the asset from memory.
        //----------------------------------------------------------------------------------------------------
        virtual void        Free() = 0;

    private:
        AssetID             m_id = kInvalidAssetID;         /// Unique identifier for this specific asset.
        AssetHandle         m_handle = kInvalidAssetHandle; /// Index into the AssetPool.
    };

    template <typename Type>
    concept ValidAssetType = TypeIsDerivedFrom<Type, AssetBase>
        && std::is_default_constructible_v<Type>
        && HasValidTypeInfo<Type>;
}
