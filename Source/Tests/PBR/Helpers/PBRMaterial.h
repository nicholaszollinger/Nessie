// PBRMaterial.h
#pragma once
#include "Nessie/Asset/AssetBase.h"
#include "Nessie/Asset/AssetManager.h"
#include <yaml-cpp/yaml.h>

namespace helpers
{
    static constexpr uint32 kInvalidIndex = std::numeric_limits<uint32>::max();
    static constexpr uint8  kMinSRGBValue = 30;
    static constexpr uint8  kMaxSRGBValue = 240;
    static constexpr float  kMinLinearColorValue = kMinSRGBValue / 255.0f;
    static constexpr float  kMaxLinearColorValue = kMaxSRGBValue / 255.0f;
}

enum class ETextureMapBits : uint8
{
    None        = 0,
    BaseColor   = NES_BIT(0),
    Normal      = NES_BIT(1),
    Roughness   = NES_BIT(2),
    Metallic    = NES_BIT(3),
    Emission    = NES_BIT(4),
};
NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(ETextureMapBits)

enum class EDefaultTextures : uint32
{
    Error,      // Magenta and Black Checkerboard
    Black,      // All pixels Black
    White,      // All pixels White
    FlatNormal, // All pixels = (127, 127, 255)
};

//----------------------------------------------------------------------------------------------------
/// @brief : Material Data for a PBR Material Asset. 
//----------------------------------------------------------------------------------------------------
struct PBRMaterialDesc
{
    // Texture Maps:
    nes::AssetID                m_baseColorMap          = nes::kInvalidAssetID;
    nes::AssetID                m_normalMap             = nes::kInvalidAssetID;
    nes::AssetID                m_roughnessMetallicMap  = nes::kInvalidAssetID; // Roughness = G channel, Metallic = B channel.
    nes::AssetID                m_emissionMap           = nes::kInvalidAssetID;
    
    // Base Color scale, with alpha.
    nes::Float4                 m_baseColor     = nes::Float4(helpers::kMaxLinearColorValue, helpers::kMaxLinearColorValue, helpers::kMaxLinearColorValue, 1.f);
    // Emission color scale.
    nes::Float3                 m_emission      = nes::Float3(0.f);                
    
    // Metallic describes if your material should behave like a metal or not.
    // Should be thought of as a binary option, either 0 or 1, nonmetal or metal.
    // The metallic model has no diffuse component and also has a tinted incident specular, equal to the base color.
    float                       m_metallic        = 0.f;
    
    // Roughness describes how smooth a surface is. 0 = rough, 1 = smooth.
    // The Roughness should be the value that you use to determine how reflective a
    // surface is.
    float                       m_roughness       = 0.5f;
    
    // Whether this Material should be considered Transparent or not. 
    bool                        m_isTransparent    = false;
};

//----------------------------------------------------------------------------------------------------
/// @brief : PBR Material Asset loaded from disk. 
//----------------------------------------------------------------------------------------------------
class PBRMaterial final : public nes::AssetBase
{
    NES_DEFINE_TYPE_INFO(PBRMaterial)
    
public:
    PBRMaterial() = default;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Creates a runtime PBR Material Asset. Must be manually added to Asset Manager. 
    //----------------------------------------------------------------------------------------------------
    PBRMaterial(const PBRMaterialDesc& desc);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the properties of the Material.
    //----------------------------------------------------------------------------------------------------
    const PBRMaterialDesc&      GetDesc() const  { return m_desc; }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the properties of the Material. 
    //----------------------------------------------------------------------------------------------------
    PBRMaterialDesc&            GetDesc()        { return m_desc; }

private:
    virtual nes::ELoadResult    LoadFromFile(const std::filesystem::path& path) override;
    nes::ELoadResult            LoadFromYAML(const YAML::Node& node);
    
    PBRMaterialDesc             m_desc{};
};

static_assert(nes::ValidAssetType<PBRMaterial>);