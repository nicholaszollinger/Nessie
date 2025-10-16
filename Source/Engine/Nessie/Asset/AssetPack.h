// AssetPack.h
#pragma once
#include "AssetBase.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Contains the type, ID, and path for an Asset. Can be used to load an Asset, only if the
    /// Asset Type has been registered with the AssetManager first.
    //----------------------------------------------------------------------------------------------------
    struct AssetMetadata
    {
        std::filesystem::path   m_path;
        TypeID                  m_typeID = 0;   
        AssetID                 m_assetID = kInvalidAssetID;
    };
    using AssetMetaDataArray = std::vector<AssetMetadata>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : An Asset Pack is a container of AssetMetaData.
    //----------------------------------------------------------------------------------------------------
    class AssetPack
    {
    public:
        AssetPack() = default;
        AssetPack(const AssetPack&) = delete;
        AssetPack(AssetPack&&) noexcept = default;
        AssetPack& operator=(const AssetPack&) = delete;
        AssetPack& operator=(AssetPack&&) noexcept = default;
        ~AssetPack() = default;

        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Add an asset's metadata to the pack, if it isn't within it already.
        //----------------------------------------------------------------------------------------------------
        void                        AddAsset(const AssetMetadata& metaData);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Adds all asset metadata from the other pack not found in this pack already.
        /// @returns : All asset metadata that were added to this pack.
        //----------------------------------------------------------------------------------------------------
        AssetMetaDataArray          Combine(const AssetPack& other);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the given AssetID is in this pack.
        //----------------------------------------------------------------------------------------------------
        bool                        Contains(const AssetID id) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the asset metadata for the given AssetID. 
        //----------------------------------------------------------------------------------------------------
        AssetMetadata&              GetAsset(const AssetID id);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the metadata for all assets in the pack.
        //----------------------------------------------------------------------------------------------------
        const AssetMetaDataArray&   GetAssets() const { return m_assets; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get a mutable array of metadata for all assets in the pack. 
        //----------------------------------------------------------------------------------------------------
        AssetMetaDataArray&         GetAssets() { return m_assets; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the metadata for all assets that are in this pack, but *not* the other.
        //----------------------------------------------------------------------------------------------------
        AssetMetaDataArray          GetDifference(const AssetPack& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load an Asset Pack from YAML.
        //----------------------------------------------------------------------------------------------------
        static bool                 LoadFromYAML(const YAML::Node& node, AssetPack& outPack);
        
    private:
        using IDToIndexMap = std::unordered_map<AssetID, size_t, UUIDHasher>;
        IDToIndexMap                m_idMap{};
        AssetMetaDataArray          m_assets{};
    };
}
