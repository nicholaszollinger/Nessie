// Scene.cpp
#include "Scene.h"

#include "Nessie/Graphics/Descriptor.h"
#include "Nessie/Graphics/Texture.h"
#include "Nessie/Graphics/Shader.h"

// [NOTE]: Right now, this is a scene that I am loading, but in reality, this would be the world Load. The scene is
// just the current data for the render. The world assets are all the different assets that are needed for a
// world instance, including the entities and components. But this is the quick and dirty version for now as I
// build this architecture up.

bool helpers::LoadScene(const std::filesystem::path& assetPath, nes::RenderDevice& device, Scene& outScene, std::vector<nes::AssetID>& outLoadedAssets, SceneConfig& outConfig)
{
    // Load the YAML file.
    YAML::Node file = YAML::LoadFile(assetPath.string());
    if (!file)
    {
        NES_ERROR("Failed to load Scene! Expecting a YAML file type. Path: {}", assetPath.string());
        return false;
    }
    
    auto assetsNode = file["Assets"];
    NES_ASSERT(assetsNode, "Invalid Scene format! Missing 'Assets' node!");

    std::filesystem::path path{};
    
    // Textures
    {
        auto textures = assetsNode["Textures"];
        for (auto textureNode : textures)
        {
            nes::AssetID id = textureNode["AssetID"].as<uint64>();

            path = NES_CONTENT_DIR;
            path /= textureNode["Path"].as<std::string>();
            
            if (nes::AssetManager::LoadSync<nes::Texture>(id, path) != nes::ELoadResult::Success)
            {
                NES_ERROR("Failed to load texture! Path: {}", path.string());
            }
            else
            {
                outLoadedAssets.emplace_back(id);

                auto pTexture = nes::AssetManager::GetAsset<nes::Texture>(id);
                NES_ASSERT(pTexture != nullptr);

                auto& image = pTexture->GetDeviceImage();
                const auto& desc = image.GetDesc();

                // Create the image view descriptor:
                nes::Image2DViewDesc imageViewDesc;
                imageViewDesc.m_pImage = &image;
                imageViewDesc.m_baseLayer = 0;
                imageViewDesc.m_layerCount = desc.m_layerCount;
                imageViewDesc.m_baseMipLevel = 0;
                imageViewDesc.m_mipCount = static_cast<uint16>(desc.m_mipCount);
                imageViewDesc.m_format = desc.m_format;
                imageViewDesc.m_viewType = nes::EImage2DViewType::ShaderResource2D;
                outScene.m_textures.emplace_back(device, imageViewDesc);
                outScene.m_idToTextureIndex.emplace(id, static_cast<uint32>(outScene.m_textures.size() - 1));
            }
        }
    }

    // Texture Cubes
    {
        auto textureCubes = assetsNode["TextureCubes"];
        for (auto textureCubeNode : textureCubes)
        {
            nes::AssetID id = textureCubeNode["AssetID"].as<uint64>();
            
            path = NES_CONTENT_DIR;
            path /= textureCubeNode["Path"].as<std::string>();

            if (nes::AssetManager::LoadSync<nes::TextureCube>(id, path) != nes::ELoadResult::Success)
            {
                NES_ERROR("Failed to load texture cube! Path: {}", path.string());
            }
            else
            {
                outLoadedAssets.emplace_back(id);
                auto pTexture = nes::AssetManager::GetAsset<nes::TextureCube>(id);
                NES_ASSERT(pTexture != nullptr);
                
                auto& image = pTexture->GetDeviceImage();
                const auto& desc = image.GetDesc();
                
                // Create the image view descriptor:
                nes::Image2DViewDesc imageViewDesc;
                imageViewDesc.m_pImage = &image;
                imageViewDesc.m_baseLayer = 0;
                imageViewDesc.m_layerCount = desc.m_layerCount;
                imageViewDesc.m_baseMipLevel = 0;
                imageViewDesc.m_mipCount = static_cast<uint16>(desc.m_mipCount);
                imageViewDesc.m_format = desc.m_format;
                imageViewDesc.m_viewType = nes::EImage2DViewType::ShaderResourceCube;
                outScene.m_textures.emplace_back(device, imageViewDesc);
                outScene.m_idToTextureIndex.emplace(id, static_cast<uint32>(outScene.m_textures.size() - 1));
            }
        }
    }

    // Shaders
    {
        auto shaders = assetsNode["Shaders"];
        for (auto shaderNode : shaders)
        {
            nes::AssetID id = shaderNode["AssetID"].as<uint64>();
            
            path = NES_SHADER_DIR;
            path /= shaderNode["Path"].as<std::string>();

            if (nes::AssetManager::LoadSync<nes::Shader>(id, path) != nes::ELoadResult::Success)
            {
                NES_ERROR("Failed to load Shader! Path: {}", path.string());
            }
            else
            {
                outLoadedAssets.emplace_back(id);   
            }
        }
    }
    
    outScene.m_indices.reserve(128);
    outScene.m_vertices.reserve(128);

    // Meshes
    {
        // Cube Mesh:
        // [TODO]: Create the Mesh Asset?
        Mesh defaultMesh;
        helpers::AppendCubeMeshData(outScene.m_vertices, outScene.m_indices, defaultMesh);
        outScene.m_meshes.emplace_back(defaultMesh);

        // Sphere Mesh:
        // [TODO]: Create the Mesh Asset?
        helpers::AppendSphereMeshData(outScene.m_vertices, outScene.m_indices, defaultMesh);
        outScene.m_meshes.emplace_back(defaultMesh);
        
        // [TODO]: Load Meshes in the Scene file.
    }
    
    // Materials
    {
        auto materials = assetsNode["Materials"];
        for (auto material : materials)
        {
            nes::AssetID id = material["AssetID"].as<uint64>();
            
            path = NES_CONTENT_DIR;
            path /= material["Path"].as<std::string>();

            if (nes::AssetManager::LoadSync<PBRMaterial>(id, path) != nes::ELoadResult::Success)
            {
                NES_ERROR("Failed to load PBRMaterial! Path: {}", path.string());
            }
            else
            {
                auto pMaterial = nes::AssetManager::GetAsset<PBRMaterial>(id);
                NES_ASSERT(pMaterial != nullptr);
                const auto& desc = pMaterial->GetDesc();

                // Create the Material Instance:
                MaterialUBO materialInstance{};
                materialInstance.m_baseColorScale = nes::Float3(desc.m_baseColor.x, desc.m_baseColor.y, desc.m_baseColor.z);
                materialInstance.m_metallicScale = desc.m_metallic;
                materialInstance.m_emissionScale = nes::Float3(desc.m_emission.x, desc.m_emission.y, desc.m_emission.z);
                materialInstance.m_roughnessScale = desc.m_roughness;

                // Base Color:
                if (desc.m_baseColorMap != nes::kInvalidAssetID)
                {
                    NES_ASSERT(outScene.m_idToTextureIndex.contains(desc.m_baseColorMap));
                    materialInstance.m_baseColorIndex = outScene.m_idToTextureIndex.at(desc.m_baseColorMap);
                }

                // Normal:
                if (desc.m_normalMap != nes::kInvalidAssetID)
                {
                    NES_ASSERT(outScene.m_idToTextureIndex.contains(desc.m_normalMap));
                    materialInstance.m_normalIndex = outScene.m_idToTextureIndex.at(desc.m_normalMap);
                }
                
                // Roughness Metallic:
                if (desc.m_roughnessMetallicMap != nes::kInvalidAssetID)
                {
                    NES_ASSERT(outScene.m_idToTextureIndex.contains(desc.m_roughnessMetallicMap));
                    materialInstance.m_roughnessMetallicIndex = outScene.m_idToTextureIndex.at(desc.m_roughnessMetallicMap);
                }

                // Emission:
                if (desc.m_emissionMap != nes::kInvalidAssetID)
                {
                    NES_ASSERT(outScene.m_idToTextureIndex.contains(desc.m_emissionMap));
                    materialInstance.m_emissionIndex = outScene.m_idToTextureIndex.at(desc.m_emissionMap);
                }
                
                outScene.m_materials.emplace_back(materialInstance);
                outScene.m_idToMaterialIndex.emplace(id, static_cast<uint32>(outScene.m_materials.size() - 1));
                outLoadedAssets.emplace_back(id);
            }
        }
    }

    // Directional Lights
    {
        // Sun in the Afternoon:
        DirectionalLight light{};
        light.m_color = nes::Float3(1.f, 1.f, 0.95f);
        nes::Vec3 direction = nes::Vec3(0.1f, -1.f, 0.1f).Normalized();
        light.m_direction = nes::Float3(direction.x, direction.y, direction.z);
        light.m_intensity = 120000.f;
        outScene.m_directionalLights.emplace_back(light);
    }
    
    // Point Lights
    {
        // Red Light.
        PointLight light{};
        light.m_position = nes::Float3(-1.5f, 0.f, 0.f);
        light.m_color = nes::Float3(1.0f, 0.0f, 0.0f); //nes::Float3(1.0f, 0.9f, 0.8f);
        light.m_intensity = 80000.f; // in lumens.
        light.m_radius = 10.f; // In Meters
        outScene.m_pointLights.emplace_back(light);
    }
    
    // Objects:
    // [TODO]: These instances would be created from the PBR Mesh Components being created.
    {
        ObjectUBO object = ObjectUBO()
            .SetTransform(nes::Vec3(0.f, 0.f, 0.f), nes::Quat::Identity(), nes::Vec3(1.f, 1.f, 1.f))
            .SetMesh(1)
            .SetMaterial(1);
        outScene.m_objects.emplace_back(object);

        object.SetTransform(nes::Vec3(0.f, 2.f, 0.f), nes::Quat::Identity(), nes::Vec3(1.f, 1.f, 1.f))
            .SetMesh(1)
            .SetMaterial(2);
        outScene.m_objects.emplace_back(object);
    }

    // Scene Config
    {
        auto config = assetsNode["SceneConfig"];
        outConfig.m_gridShaderID = config["GridShaderID"].as<uint64>();
        outConfig.m_skyboxShaderID = config["SkyboxShaderID"].as<uint64>();
        outConfig.m_pbrShaderID = config["PBRShaderID"].as<uint64>();
        outConfig.m_skyboxTextureID = config["SkyboxTextureID"].as<uint64>();
    }
    
    return true;
}

ObjectUBO& ObjectUBO::SetTransform(const nes::Vec3 translation, const nes::Quat rotation, const nes::Vec3 scale)
{
    m_model = nes::Mat44::ComposeTransform(translation, rotation, scale);
    m_normal = m_model.Inversed3x3().Transposed3x3();
    
    //nes::Mat44 normal44 = m_model.Inversed3x3().Transposed3x3();
    //m_normal = nes::Mat33(normal44.GetColumn3(0), normal44.GetColumn3(1), normal44.GetColumn3(2));
    return *this;
}

ObjectUBO& ObjectUBO::SetTransform(const nes::Mat44& transform)
{
    m_model = transform;
    m_normal = m_model.Inversed3x3().Transposed3x3();
    
    //nes::Mat44 normal44 = m_model.Inversed3x3().Transposed3x3();
    //m_normal = nes::Mat33(normal44.GetColumn3(0), normal44.GetColumn3(1), normal44.GetColumn3(2));
    return *this;
}

ObjectUBO& ObjectUBO::SetMesh(const uint32 meshIndex)
{
    m_meshIndex = meshIndex;
    return *this;
}

ObjectUBO& ObjectUBO::SetMaterial(const uint32 materialIndex)
{
    m_materialIndex = materialIndex;
    return *this;
}
