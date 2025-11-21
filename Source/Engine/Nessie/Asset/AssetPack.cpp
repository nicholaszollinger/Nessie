// AssetPack.cpp
#include "AssetPack.h"
#include "Nessie/Graphics/Shader.h"

namespace nes
{
    void AssetPack::AddAsset(const AssetMetadata& metaData)
    {
        if (!Contains(metaData.m_assetID))
        {
            m_assets.emplace_back(metaData);
            m_idMap.emplace(metaData.m_assetID, m_assets.size() - 1);
        }
    }

    std::vector<AssetMetadata> AssetPack::Combine(const AssetPack& other)
    {
        AssetMetaDataArray addedAssets;
        addedAssets.reserve(other.m_assets.size());

        for (const auto& metaData : other.m_assets)
        {
            if (!Contains(metaData.m_assetID))
            {
                AddAsset(metaData);
                addedAssets.emplace_back(metaData);
            }
        }
        
        return addedAssets;
    }

    bool AssetPack::Contains(const AssetID id) const
    {
        return m_idMap.contains(id);
    }

    AssetMetadata& AssetPack::GetAsset(const AssetID id)
    {
        NES_ASSERT(m_idMap.contains(id));
        return m_assets[m_idMap.at(id)];
    }

    std::vector<AssetMetadata> AssetPack::GetDifference(const AssetPack& other) const
    {
        AssetMetaDataArray result;
        result.reserve(m_assets.size());
        
        for (const auto& metaData : m_assets)
        {
            if (!other.Contains(metaData.m_assetID))
            {
                result.emplace_back(metaData);
            }
        }

        return result;
    }

    bool AssetPack::Deserialize(const YamlNode& node, AssetPack& outPack)
    {
        std::string path;
        for (auto assetNode : node)
        {
            // AssetID
            AssetMetadata metaData;
            assetNode["AssetID"].Read(metaData.m_assetID, kInvalidAssetID);
            
            // TypeID
            // - If invalid, it will be caught by the AssetManager when trying to load.
            assetNode["TypeID"].Read(metaData.m_typeID);

            // Path
            assetNode["Path"].Read<std::string>(path);
            metaData.m_relativePath = path;
            
            if (!metaData.m_relativePath.has_extension())
            {
                NES_ERROR("Failed to load AssetPack! Invalid relative path for Asset: '{}'", metaData.m_relativePath.string());
                return false;
            }

            // Set the filename (no extension) as the asset name.
            metaData.m_assetName = metaData.m_relativePath.stem().string();
            outPack.AddAsset(metaData);
        }

        return true;
    }

    void AssetPack::Serialize(YamlOutStream& writer, const AssetPack& assetPack)
    {
        writer.BeginSequence("Assets", assetPack.m_assets.empty());
        for (auto& metaData : assetPack.m_assets)
        {
            writer.BeginMap();
            writer.Write("AssetID", metaData.m_assetID);
            writer.Write("TypeID", metaData.m_typeID);
            writer.Write("Path", metaData.m_relativePath.string());
            writer.EndMap();
        }
        writer.EndSequence();
    }
}
