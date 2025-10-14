// WorldAsset.h
#pragma once
#include "EntityRegistry.h"
#include "Nessie/Asset/AssetBase.h"

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
        EntityRegistry&         GetRegistry() { return m_entityRegistry; }
        
    protected:
        virtual ELoadResult     LoadFromFile(const std::filesystem::path& path) override;
        bool                    LoadEntities(const YAML::Node& entities);
        
    private:
        EntityRegistry          m_entityRegistry{};

        // [TODO]: Assets that need to be loaded.
        //std::vector<AssetID>    m_requiredAssets{}; // All assets that must be loaded for the world 
    };
    
    static_assert(ValidAssetType<WorldAsset>);
}
