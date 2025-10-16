// AssetPack.cpp
#include "AssetPack.h"
#include "Nessie/Graphics/Shader.h"

namespace nes
{
    // [TODO]: Temp Fix. Right now, I have shaders in their own folder.
    // - When I have all Assets in a single folder, then I can just prepend the NES_CONTENT_DIR
    //   directory.
    // - I will also have to update my shader compilation script.
    // - NOTE: You will probably want to load Shaders separately from world assets, so that you
    //   can immediately start rendering and don't stall the update loop.
    static void ResolveAssetPath(AssetMetadata& metaData)
    {
        if (metaData.m_typeID == Shader::GetStaticTypeID())
        {
            metaData.m_path = std::filesystem::path(NES_SHADER_DIR) / metaData.m_path;
        }
        else
        {
            metaData.m_path = std::filesystem::path(NES_CONTENT_DIR) / metaData.m_path;
        }
    }
    
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

    bool AssetPack::LoadFromYAML(const YAML::Node& node, AssetPack& outPack)
    {
        std::filesystem::path path{};
        
        for (auto assetNode : node)
        {
            // AssetID
            AssetMetadata metaData;
            metaData.m_assetID = assetNode["AssetID"].as<uint64>();
            
            // TypeID
            // - If invalid, it will be caught by the AssetManager when trying to load.
            metaData.m_typeID = assetNode["TypeID"].as<uint64>();

            // Path
            metaData.m_path = assetNode["Path"].as<std::string>();
            ResolveAssetPath(metaData);
            if (!metaData.m_path.has_extension())
            {
                NES_ERROR("Failed to load AssetPack! Invalid relative path for Asset: '{}'", metaData.m_path.string());
                return false;
            }
            
            outPack.AddAsset(metaData);
        }

        return true;
    }
}
