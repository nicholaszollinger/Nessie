// PBRMaterial.h
#pragma once
#include "Nessie/Asset/AssetBase.h"
#include "Nessie/Asset/AssetManager.h"

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

//----------------------------------------------------------------------------------------------------
/// @brief : Runtime information for a PBR Material. This data is uploaded to the GPU.
//----------------------------------------------------------------------------------------------------
struct alignas(16) PBRMaterialInstance
{
    // Scalar values for Base Color (xyz) and Metallic (w).
    nes::Float4 m_baseColorAndMetallic      = nes::Float4(helpers::kMaxLinearColorValue, helpers::kMaxLinearColorValue, helpers::kMaxLinearColorValue, 0.f);
    
    // Scalar values for Emissive Color (xyz) and Roughness (w).
    nes::Float4 m_emissionAndRoughness      = nes::Float4(0.f, 0.f, 0.f, 0.5f);

    // Texture Maps
    uint32      m_baseColorMapIndex         = helpers::kInvalidIndex;   // Index into the Texture Array in the Device Buffer.
    uint32      m_normalMapIndex            = helpers::kInvalidIndex;
    uint32      m_roughnessMetallicMapIndex = helpers::kInvalidIndex;  
    uint32      m_emissionMapIndex          = helpers::kInvalidIndex;
    
    float       m_alpha                     = 1.f;
    // float       m_subsurface            = 0.f;
    // float       m_specular              = 0.5f;
    // float       m_specularTint          = 0.f;
    // float       m_anisotropic           = 0.f;
    // float       m_sheen                 = 0.f;
    // float       m_sheenTint             = 0.f;
    // float       m_clearCoat             = 0.f;
    // float       m_clearCoatGloss        = 0.f;
};

//----------------------------------------------------------------------------------------------------
/// @brief : Material Data for a PBR Material Asset. 
//----------------------------------------------------------------------------------------------------
struct PBRMaterialDesc
{
    // Texture Maps:
    nes::AssetID                m_baseColorMap  = nes::kInvalidAssetID;
    nes::AssetID                m_normalMap     = nes::kInvalidAssetID;
    nes::AssetID                m_roughnessMap  = nes::kInvalidAssetID;
    nes::AssetID                m_metallicMap   = nes::kInvalidAssetID;
    nes::AssetID                m_emissionMap   = nes::kInvalidAssetID;
    ETextureMapBits             m_textureMapUsage = ETextureMapBits::None;

    /// Surface color of the Material. Usually supplied by a texture map.
    nes::Float3 m_baseColor = nes::Float3(helpers::kMaxLinearColorValue);

    /// Controls diffuse shape using a subsurface approximation.
    float m_subsurface      = 0.f;
    
    /// Metallic describes if your material should behave like a metal or not.
    /// Should be thought of as a binary option, either 0 or 1, nonmetal or metal.
    /// The metallic model has no diffuse component and also has a tinted incident specular, equal to the base color.
    float m_metallic        = 0.f;
    
    /// Specular defines how strong the reflection is when the viewer is directly looking at a surface.
    /// 0 = no reflection, 1 = %8 reflection (e.g. gemstones). Most materials can use 0.5.
    /// This is in lieu of an explicit index-of-refraction.
    float m_specular        = 0.5f;

    /// A concession for artistic control that tints the incident specular towards the base color. Grazing specular
    /// is still achromatic.
    float m_specularTint    = 0.f;
    
    /// Roughness describes how smooth a surface is. 0 = rough, 1 = smooth.
    /// The Roughness should be the value that you use to determine how reflective a
    /// surface is. Not the specular value.
    float m_roughness       = 0.5f;

    /// Degree of anisotropy. This controls the aspect ratio of the specular highlight
    /// (0 = isotropic, 1 = maximally anisotropic)
    float m_anisotropic     = 0.f;

    /// An additional grazing component, primarily intended for cloth.
    float m_sheen           = 0.f;

    /// Amount to tint sheen towards the base color.
    float m_sheenTint       = 0.5f;

    /// A seconds, special-purpose specular lobe.
    float m_clearCoat       = 0.f;

    /// Controls clear coat glossiness (0 = a "satin" appearance, 1 = a "gloss" appearance).
    float m_clearCoatGloss  = 1.f;
    
    /// How 'opaque' the material is from 0 = invisible, 1 = fully opaque. This value will be ignored if the
    /// Material is not-transparent.
    float m_alpha           = 1.f;

    /// Emission represents how emissive a material is.
    float m_emission        = 0.f;

    /// Whether this Material should be considered Transparent or not. 
    bool m_isTransparent    = false;
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
    //nes::ELoadResult            LoadFromYAML(const YAML::Node& node);
    
    PBRMaterialDesc             m_desc{};
};

static_assert(nes::ValidAssetType<PBRMaterial>);