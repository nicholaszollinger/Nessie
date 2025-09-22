// PBRMaterial.cpp
#include "PBRMaterial.h"

PBRMaterial::PBRMaterial(const PBRMaterialDesc& desc)
    : m_desc(desc)
{
    //
}

nes::ELoadResult PBRMaterial::LoadFromFile(const std::filesystem::path& path)
{
    YAML::Node file = YAML::LoadFile(path.string());
    if (!file)
    {
        NES_ERROR("Failed to load PBRMaterial. Expecting a YAML file!");
        return nes::ELoadResult::InvalidArgument;
    }

    YAML::Node shader = file["PBRMaterial"];
    if (!shader)
    {
        NES_ERROR("Failed to load PBRMaterial. YAML file invalid! Missing 'PBRMaterial' entry!");
        return nes::ELoadResult::InvalidArgument;
    }

    return LoadFromYAML(shader);
}

nes::ELoadResult PBRMaterial::LoadFromYAML(const YAML::Node& node)
{
    // Base Color
    const auto baseColorNode = node["BaseColor"]; 
    m_desc.m_baseColor.x = baseColorNode[0].as<float>(helpers::kMaxLinearColorValue);
    m_desc.m_baseColor.y = baseColorNode[1].as<float>(helpers::kMaxLinearColorValue);
    m_desc.m_baseColor.z = baseColorNode[2].as<float>(helpers::kMaxLinearColorValue);
    m_desc.m_baseColor.w = baseColorNode[3].as<float>(1.f);

    // Emission
    const auto emissionNode = node["Emission"]; 
    m_desc.m_emission.x = emissionNode[0].as<float>(0.f);
    m_desc.m_emission.y = emissionNode[1].as<float>(0.f);
    m_desc.m_emission.z = emissionNode[2].as<float>(0.f);

    // Other Params:
    m_desc.m_roughness  = node["Roughness"].as<float>(0.5f);
    m_desc.m_metallic   = node["Metallic"].as<float>(0.f);

    // Texture Maps:
    const auto mapsNode = node["Maps"];
    m_desc.m_baseColorMap           = mapsNode["BaseColor"].as<uint64>(nes::kInvalidAssetID.GetValue()); 
    m_desc.m_normalMap              = mapsNode["Normal"].as<uint64>(nes::kInvalidAssetID.GetValue()); 
    m_desc.m_roughnessMetallicMap   = mapsNode["RoughnessMetallic"].as<uint64>(nes::kInvalidAssetID.GetValue());
    m_desc.m_emissionMap            = mapsNode["Emission"].as<uint64>(nes::kInvalidAssetID.GetValue());

    return nes::ELoadResult::Success;
}
