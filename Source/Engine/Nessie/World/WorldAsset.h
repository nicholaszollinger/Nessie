// WorldAsset.h
#pragma once
#include "EntityRegistry.h"
#include "Nessie/Asset/AssetBase.h"
#include "Nessie/Asset/AssetPack.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A World Asset contains an Entity Registry that can be merged into a runtime world.
    //----------------------------------------------------------------------------------------------------
    class WorldAsset : public AssetBase
    {
        NES_DEFINE_TYPE_INFO(WorldAsset)
        
    public:
        WorldAsset() = default;
        WorldAsset(WorldAsset&& other) noexcept;
        WorldAsset& operator=(WorldAsset&& other) noexcept;

        AssetPack&              GetAssetPack()          { return m_assetPack; }
        const AssetPack&        GetAssetPack() const    { return m_assetPack; }
        EntityRegistry&         GetEntityRegistry()           { return m_entityRegistry; }
    
    protected:
        virtual ELoadResult     LoadFromFile(const std::filesystem::path& path) override;
        virtual void            SaveToFile(const std::filesystem::path&) override;
        bool                    LoadEntities(const YamlNode& entities);
        
    private:
        EntityRegistry          m_entityRegistry{};
        AssetPack               m_assetPack{};
    };
    
    static_assert(ValidAssetType<WorldAsset>);
}
