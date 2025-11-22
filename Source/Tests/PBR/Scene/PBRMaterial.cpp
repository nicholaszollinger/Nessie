// PBRMaterial.cpp
#include "PBRMaterial.h"
#include "Nessie/FileIO/YAML/Serializers/YamlCoreSerializers.h"
#include "Nessie/FileIO/YAML/Serializers/YamlMathSerializers.h"

namespace pbr
{
    bool PBRMaterialDesc::operator==(const PBRMaterialDesc& other) const
    {
        return m_isTransparent == other.m_isTransparent
            && m_emissionMap == other.m_emissionMap
            && m_baseColorMap == other.m_baseColorMap
            && m_normalMap == other.m_normalMap
            && m_roughnessMetallicMap == other.m_roughnessMetallicMap
            && m_metallic == other.m_metallic
            && m_roughness == other.m_roughness
            && m_baseColor ==  other.m_baseColor
            && m_emission ==  other.m_emission;
    }

    PBRMaterial::PBRMaterial(const PBRMaterialDesc& desc)
        : m_desc(desc)
    {
        //
    }

    nes::ELoadResult PBRMaterial::LoadFromFile(const std::filesystem::path& path)
    {
        nes::YamlInStream reader(path);
        if (!reader.IsOpen())
        {
            NES_ERROR("Failed to load PBRMaterial. Expecting a YAML file!");
            return nes::ELoadResult::InvalidArgument;
        }

        auto root = reader.GetRoot()["PBRMaterial"];
        if (!root)
        {
            NES_ERROR("Failed to load PBRMaterial. YAML file invalid! Missing 'PBRMaterial' entry!");
            return nes::ELoadResult::InvalidArgument;
        }

        return LoadFromYAML(root);
    }

    nes::ELoadResult PBRMaterial::LoadFromYAML(const nes::YamlNode& node)
    {
        node["BaseColor"].Read(m_desc.m_baseColor, nes::Float4(helpers::kMaxLinearColorValue, helpers::kMaxLinearColorValue, helpers::kMaxLinearColorValue, 1.f));
        node["Emission"].Read(m_desc.m_emission, nes::Float3(0.f));
        node["Roughness"].Read(m_desc.m_roughness, 0.5f);
        node["Metallic"].Read(m_desc.m_metallic, 0.f);

        // Texture Maps:
        const auto mapsNode = node["Maps"];
        mapsNode["BaseColor"].Read(m_desc.m_baseColorMap, nes::kInvalidAssetID);
        mapsNode["Normal"].Read(m_desc.m_normalMap, nes::kInvalidAssetID);
        mapsNode["RoughnessMetallic"].Read(m_desc.m_roughnessMetallicMap, nes::kInvalidAssetID);
        mapsNode["Emission"].Read(m_desc.m_emissionMap, nes::kInvalidAssetID);

        return nes::ELoadResult::Success;
    }
}
