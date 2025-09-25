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
    
    outScene.m_indices.reserve(6400);
    outScene.m_vertices.reserve(6400);

    // Meshes
    {
        Mesh sceneMesh;
        nes::AssetID defaultMaterialID = helpers::GetDefaultMaterialID();
        
        // Cube:
        {
            helpers::AppendCubeMeshData(outScene.m_vertices, outScene.m_indices, sceneMesh);
            outScene.m_meshes.emplace_back(sceneMesh);

            nes::AssetID assetID = helpers::GetDefaultMeshID(EDefaultMeshType::Cube);
            MeshAsset asset(&outScene.m_vertices[sceneMesh.m_firstVertex], sceneMesh.m_vertexCount, &outScene.m_indices[sceneMesh.m_firstIndex], sceneMesh.m_indexCount, defaultMaterialID);
            nes::AssetManager::AddMemoryAsset<MeshAsset>(assetID, std::move(asset));
            outScene.m_idToMeshIndex.emplace(assetID, static_cast<uint32>(outScene.m_meshes.size() - 1));
        }

        // Sphere: 
        {
            helpers::AppendSphereMeshData(outScene.m_vertices, outScene.m_indices, sceneMesh);
            outScene.m_meshes.emplace_back(sceneMesh);

            nes::AssetID assetID = helpers::GetDefaultMeshID(EDefaultMeshType::Sphere);
            MeshAsset asset(&outScene.m_vertices[sceneMesh.m_firstVertex], sceneMesh.m_vertexCount, &outScene.m_indices[sceneMesh.m_firstIndex], sceneMesh.m_indexCount, defaultMaterialID);
            nes::AssetManager::AddMemoryAsset<MeshAsset>(assetID, std::move(asset));
            outScene.m_idToMeshIndex.emplace(assetID, static_cast<uint32>(outScene.m_meshes.size() - 1));
        }

        // Plane: 
        {
            helpers::AppendPlaneData(outScene.m_vertices, outScene.m_indices, sceneMesh);
            outScene.m_meshes.emplace_back(sceneMesh);

            nes::AssetID assetID = helpers::GetDefaultMeshID(EDefaultMeshType::Plane);
            MeshAsset asset(&outScene.m_vertices[sceneMesh.m_firstVertex], sceneMesh.m_vertexCount, &outScene.m_indices[sceneMesh.m_firstIndex], sceneMesh.m_indexCount, defaultMaterialID);
            nes::AssetManager::AddMemoryAsset<MeshAsset>(assetID, std::move(asset));
            outScene.m_idToMeshIndex.emplace(assetID, static_cast<uint32>(outScene.m_meshes.size() - 1));
        }
        
        // Load Meshes in the Scene file.
        auto meshes = assetsNode["Meshes"];
        for (auto meshNode : meshes)
        {
            nes::AssetID id = meshNode["AssetID"].as<uint64>();
            
            path = NES_CONTENT_DIR;
            path /= meshNode["Path"].as<std::string>();

            if (nes::AssetManager::LoadSync<MeshAsset>(id, path) != nes::ELoadResult::Success)
            {
                NES_ERROR("Failed to load PBRMaterial! Path: {}", path.string());
            }
            else
            {
                // Add the Mesh Asset to the Scene:
                auto pMesh = nes::AssetManager::GetAsset<MeshAsset>(id);
                NES_ASSERT(pMesh != nullptr);

                const auto& meshVertices = pMesh->GetVertices();
                const auto& meshIndices = pMesh->GetIndices();

                // Set the Index/Vertex information.
                sceneMesh.m_firstVertex = static_cast<uint32>(outScene.m_vertices.size());
                sceneMesh.m_firstIndex = static_cast<uint32>(outScene.m_indices.size());
                sceneMesh.m_vertexCount = static_cast<uint32>(meshVertices.size());
                sceneMesh.m_indexCount = static_cast<uint32>(meshIndices.size());

                // Insert the data:
                outScene.m_vertices.insert(outScene.m_vertices.end(), meshVertices.begin(), meshVertices.end());
                outScene.m_indices.insert(outScene.m_indices.end(), meshIndices.begin(), meshIndices.end());
                outScene.m_meshes.emplace_back(sceneMesh);
                outScene.m_idToMeshIndex.emplace(id, static_cast<uint32>(outScene.m_meshes.size() - 1));
                outLoadedAssets.emplace_back(id);

                // Add the Material Data:
                const auto materialID = pMesh->GetDefaultMaterialID();
                auto pMaterial = nes::AssetManager::GetAsset<PBRMaterial>(materialID);
                NES_ASSERT(pMaterial != nullptr);
                const auto& materialDesc = pMaterial->GetDesc();
                
                // Base Color Map:
                if (!outScene.m_idToTextureIndex.contains(materialDesc.m_baseColorMap))
                {
                    auto pTexture = nes::AssetManager::GetAsset<nes::Texture>(materialDesc.m_baseColorMap);
                    NES_ASSERT(pTexture != nullptr);

                    auto& image = pTexture->GetDeviceImage();
                    const auto& imageDesc = image.GetDesc();

                    // Create the image view descriptor:
                    nes::Image2DViewDesc imageViewDesc;
                    imageViewDesc.m_pImage = &image;
                    imageViewDesc.m_baseLayer = 0;
                    imageViewDesc.m_layerCount = imageDesc.m_layerCount;
                    imageViewDesc.m_baseMipLevel = 0;
                    imageViewDesc.m_mipCount = static_cast<uint16>(imageDesc.m_mipCount);
                    imageViewDesc.m_format = imageDesc.m_format;
                    imageViewDesc.m_viewType = nes::EImage2DViewType::ShaderResource2D;
                    outScene.m_textures.emplace_back(device, imageViewDesc);
                    outScene.m_idToTextureIndex.emplace(materialDesc.m_baseColorMap, static_cast<uint32>(outScene.m_textures.size() - 1));
                }

                // Normal Map
                if (!outScene.m_idToTextureIndex.contains(materialDesc.m_normalMap))
                {
                    auto pTexture = nes::AssetManager::GetAsset<nes::Texture>(materialDesc.m_normalMap);
                    NES_ASSERT(pTexture != nullptr);

                    auto& image = pTexture->GetDeviceImage();
                    const auto& imageDesc = image.GetDesc();

                    // Create the image view descriptor:
                    nes::Image2DViewDesc imageViewDesc;
                    imageViewDesc.m_pImage = &image;
                    imageViewDesc.m_baseLayer = 0;
                    imageViewDesc.m_layerCount = imageDesc.m_layerCount;
                    imageViewDesc.m_baseMipLevel = 0;
                    imageViewDesc.m_mipCount = static_cast<uint16>(imageDesc.m_mipCount);
                    imageViewDesc.m_format = imageDesc.m_format;
                    imageViewDesc.m_viewType = nes::EImage2DViewType::ShaderResource2D;
                    outScene.m_textures.emplace_back(device, imageViewDesc);
                    outScene.m_idToTextureIndex.emplace(materialDesc.m_normalMap, static_cast<uint32>(outScene.m_textures.size() - 1));
                }

                // Material/Roughness Map
                if (!outScene.m_idToTextureIndex.contains(materialDesc.m_roughnessMetallicMap))
                {
                    auto pTexture = nes::AssetManager::GetAsset<nes::Texture>(materialDesc.m_roughnessMetallicMap);
                    NES_ASSERT(pTexture != nullptr);

                    auto& image = pTexture->GetDeviceImage();
                    const auto& imageDesc = image.GetDesc();

                    // Create the image view descriptor:
                    nes::Image2DViewDesc imageViewDesc;
                    imageViewDesc.m_pImage = &image;
                    imageViewDesc.m_baseLayer = 0;
                    imageViewDesc.m_layerCount = imageDesc.m_layerCount;
                    imageViewDesc.m_baseMipLevel = 0;
                    imageViewDesc.m_mipCount = static_cast<uint16>(imageDesc.m_mipCount);
                    imageViewDesc.m_format = imageDesc.m_format;
                    imageViewDesc.m_viewType = nes::EImage2DViewType::ShaderResource2D;
                    outScene.m_textures.emplace_back(device, imageViewDesc);
                    outScene.m_idToTextureIndex.emplace(materialDesc.m_roughnessMetallicMap, static_cast<uint32>(outScene.m_textures.size() - 1));
                }

                // Emissive Map
                if (!outScene.m_idToTextureIndex.contains(materialDesc.m_emissionMap))
                {
                    auto pTexture = nes::AssetManager::GetAsset<nes::Texture>(materialDesc.m_emissionMap);
                    NES_ASSERT(pTexture != nullptr);

                    auto& image = pTexture->GetDeviceImage();
                    const auto& imageDesc = image.GetDesc();

                    // Create the image view descriptor:
                    nes::Image2DViewDesc imageViewDesc;
                    imageViewDesc.m_pImage = &image;
                    imageViewDesc.m_baseLayer = 0;
                    imageViewDesc.m_layerCount = imageDesc.m_layerCount;
                    imageViewDesc.m_baseMipLevel = 0;
                    imageViewDesc.m_mipCount = static_cast<uint16>(imageDesc.m_mipCount);
                    imageViewDesc.m_format = imageDesc.m_format;
                    imageViewDesc.m_viewType = nes::EImage2DViewType::ShaderResource2D;
                    outScene.m_textures.emplace_back(device, imageViewDesc);
                    outScene.m_idToTextureIndex.emplace(materialDesc.m_emissionMap, static_cast<uint32>(outScene.m_textures.size() - 1));
                }

                // Create the Material Instance:
                MaterialUBO materialInstance{};
                materialInstance.m_baseColorScale = nes::Float3(materialDesc.m_baseColor.x, materialDesc.m_baseColor.y, materialDesc.m_baseColor.z);
                materialInstance.m_metallicScale = materialDesc.m_metallic;
                materialInstance.m_emissionScale = nes::Float3(materialDesc.m_emission.x, materialDesc.m_emission.y, materialDesc.m_emission.z);
                materialInstance.m_roughnessScale = materialDesc.m_roughness;

                // Base Color:
                if (materialDesc.m_baseColorMap != nes::kInvalidAssetID)
                {
                    NES_ASSERT(outScene.m_idToTextureIndex.contains(materialDesc.m_baseColorMap));
                    materialInstance.m_baseColorIndex = outScene.m_idToTextureIndex.at(materialDesc.m_baseColorMap);
                }

                // Normal:
                if (materialDesc.m_normalMap != nes::kInvalidAssetID)
                {
                    NES_ASSERT(outScene.m_idToTextureIndex.contains(materialDesc.m_normalMap));
                    materialInstance.m_normalIndex = outScene.m_idToTextureIndex.at(materialDesc.m_normalMap);
                }
                
                // Roughness Metallic:
                if (materialDesc.m_roughnessMetallicMap != nes::kInvalidAssetID)
                {
                    NES_ASSERT(outScene.m_idToTextureIndex.contains(materialDesc.m_roughnessMetallicMap));
                    materialInstance.m_roughnessMetallicIndex = outScene.m_idToTextureIndex.at(materialDesc.m_roughnessMetallicMap);
                }

                // Emission:
                if (materialDesc.m_emissionMap != nes::kInvalidAssetID)
                {
                    NES_ASSERT(outScene.m_idToTextureIndex.contains(materialDesc.m_emissionMap));
                    materialInstance.m_emissionIndex = outScene.m_idToTextureIndex.at(materialDesc.m_emissionMap);
                }
                
                outScene.m_materials.emplace_back(materialInstance);
                outScene.m_idToMaterialIndex.emplace(materialID, static_cast<uint32>(outScene.m_materials.size() - 1));
                outLoadedAssets.emplace_back(materialID);
            }
        }
    }

    // Directional Lights
    {
        auto directionalLights = assetsNode["DirectionalLights"];
        for (auto lightNode : directionalLights)
        {
            nes::Vec3 direction{};
            auto directionNode = lightNode["Direction"];
            direction.x = directionNode[0].as<float>(1.f);
            direction.y = directionNode[1].as<float>(-1.f);
            direction.z = directionNode[2].as<float>(1.f);
            direction.Normalize();

            DirectionalLight light;
            light.m_direction = nes::Float3(direction.x, direction.y, direction.z);
            light.m_intensity = lightNode["Intensity"].as<float>(100000.f); // 100K lux by default.

            auto colorNode = lightNode["Color"];
            light.m_color.x = colorNode[0].as<float>(1.f);
            light.m_color.y = colorNode[1].as<float>(1.f);
            light.m_color.z = colorNode[2].as<float>(1.f);

            outScene.m_directionalLights.emplace_back(light);
        }
    }
    
    // Point Lights
    {
        auto pointLights = assetsNode["PointLights"];
        for (auto lightNode : pointLights)
        {
            PointLight light;
            auto positionNode = lightNode["Position"];
            light.m_position.x = positionNode[0].as<float>(0.f);
            light.m_position.y = positionNode[1].as<float>(0.f);
            light.m_position.z = positionNode[2].as<float>(0.f);
            
            light.m_intensity = lightNode["Intensity"].as<float>(600.f); // 60K lumens by default.
            light.m_radius = lightNode["Radius"].as<float>(30.f);

            auto colorNode = lightNode["Color"];
            light.m_color.x = colorNode[0].as<float>(1.f);
            light.m_color.y = colorNode[1].as<float>(1.f);
            light.m_color.z = colorNode[2].as<float>(1.f);

            outScene.m_pointLights.emplace_back(light);
        }
    }

    // Objects:
    {
        // Default to Cube with Default Material.
        constexpr uint64 kDefaultMeshIDValue = helpers::GetDefaultMeshID(EDefaultMeshType::Cube).GetValue();
        constexpr uint64 kDefaultMaterialIDValue = helpers::GetDefaultMaterialID().GetValue();
        
        auto objects = assetsNode["Objects"];
        for (auto objectNode : objects)
        {
            auto transform = objectNode["Transform"];

            // Position
            nes::Vec3 position{};
            auto positionNode = transform["Position"];
            position.x = positionNode[0].as<float>(0.f);
            position.y = positionNode[1].as<float>(0.f);
            position.z = positionNode[2].as<float>(0.f);

            // Rotation
            nes::Rotation rotation;
            auto rotationNode = transform["Rotation"];
            rotation.m_pitch = rotationNode[0].as<float>(0.f);
            rotation.m_yaw = rotationNode[1].as<float>(0.f);
            rotation.m_roll = rotationNode[2].as<float>(0.f);

            // Scale
            nes::Vec3 scale{};
            auto scaleNode = transform["Scale"];
            scale.x = scaleNode[0].as<float>(1.f);
            scale.y = scaleNode[1].as<float>(1.f);
            scale.z = scaleNode[2].as<float>(1.f);

            // This would be a Mesh Component:
            nes::AssetID mesh = objectNode["Mesh"].as<uint64>(kDefaultMeshIDValue);
            nes::AssetID material = objectNode["Material"].as<uint64>(kDefaultMaterialIDValue);

            // If the material is an invalid asset, use the Mesh's default.
            if (material == nes::kInvalidAssetID)
            {
                auto pMesh = nes::AssetManager::GetAsset<MeshAsset>(mesh);
                NES_ASSERT(pMesh);
                
                material = pMesh->GetDefaultMaterialID();
                NES_ASSERT(material != nes::kInvalidAssetID);
            }

            // Convert to Scene Indices:
            const uint32 meshIndex = outScene.m_idToMeshIndex.at(mesh);
            const uint32 materialIndex = outScene.m_idToMaterialIndex.at(material);

            ObjectUBO object = ObjectUBO()
                .SetTransform(position, rotation.ToQuat(), scale)
                .SetMesh(meshIndex)
                .SetMaterial(materialIndex);
            outScene.m_objects.emplace_back(object);
        }
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
    return *this;
}

ObjectUBO& ObjectUBO::SetTransform(const nes::Mat44& transform)
{
    m_model = transform;
    m_normal = m_model.Inversed3x3().Transposed3x3();
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
