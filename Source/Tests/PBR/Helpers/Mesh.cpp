// Mesh.cpp
#include "Mesh.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "Scene.h"
#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Graphics/Texture.h"

std::array<nes::VertexAttributeDesc, 5> Vertex::GetBindingDescs()
{
    static constexpr std::array kBindings =
    {
        // Position
        nes::VertexAttributeDesc
        {
            .m_location = 0,
            .m_offset = offsetof(Vertex, m_position),
            .m_format = nes::EFormat::RGB32_SFLOAT,
            .m_streamIndex = 0
        },
        
        // Normal
        nes::VertexAttributeDesc
        {
            .m_location = 1,
            .m_offset = offsetof(Vertex, m_normal),
            .m_format = nes::EFormat::RGB32_SFLOAT,
            .m_streamIndex = 0
        },

        // UV
        nes::VertexAttributeDesc
        {
            .m_location = 2,
            .m_offset = offsetof(Vertex, m_texCoord),
            .m_format = nes::EFormat::RG32_SFLOAT,
            .m_streamIndex = 0
        },

        // Tangent
        nes::VertexAttributeDesc
        {
            .m_location = 3,
            .m_offset = offsetof(Vertex, m_tangent),
            .m_format = nes::EFormat::RGB32_SFLOAT,
            .m_streamIndex = 0
        },

        // Bitangent.
        nes::VertexAttributeDesc
        {
            .m_location = 4,
            .m_offset = offsetof(Vertex, m_bitangent),
            .m_format = nes::EFormat::RGB32_SFLOAT,
            .m_streamIndex = 0
        },
    };

    return kBindings;
}

MeshAsset::MeshAsset(const std::vector<Vertex>& vertices, const std::vector<uint32>& indices, const nes::AssetID defaultMaterialID)
    : m_vertices(vertices)
    , m_indices(indices)
    , m_defaultMaterialID(defaultMaterialID)
{
    //
}

MeshAsset::MeshAsset(Vertex* pVertices, const uint32 vertexCount, uint32* indices, const uint32 indexCount, const nes::AssetID defaultMaterialID)
    : m_vertices(pVertices, pVertices + vertexCount)
    , m_indices(indices, indices + indexCount)
    , m_defaultMaterialID(defaultMaterialID)
{
    //
}

nes::ELoadResult MeshAsset::LoadFromFile(const std::filesystem::path& path)
{
    YAML::Node file = YAML::LoadFile(path.string());
    if (!file)
    {
        NES_ERROR("Failed to load Mesh! Expecting a YAML file.");
        return nes::ELoadResult::InvalidArgument;
    }

    YAML::Node mesh = file["Mesh"];
    if (!mesh)
    {
        NES_ERROR("Failed to load Mesh! YAML file invalid: Missing 'Mesh' entry!");
        return nes::ELoadResult::InvalidArgument;
    }

    return LoadFromYAML(mesh);
}

nes::ELoadResult MeshAsset::LoadFromYAML(const YAML::Node& node)
{
    const bool invertWinding = node["InvertWinding"].as<bool>(true);
    
    Assimp::Importer importer;
    int importFlags = aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_FlipUVs;
    if (invertWinding)
        importFlags |= aiProcess_FlipWindingOrder;

    std::filesystem::path path = NES_CONTENT_DIR;
        path /= node["Path"].as<std::string>();

    const aiScene* pScene = importer.ReadFile(path.string().c_str(), importFlags);
    if (!pScene)
    {
        NES_ERROR("Failed to load assimp file! Error: {}", importer.GetErrorString());
        return nes::ELoadResult::Failure;
    }

    // Assume a single mesh for now.
    NES_ASSERT(pScene->mNumMeshes > 0);
    const aiMesh* pMesh = pScene->mMeshes[0];

    // Load Index data:
    const uint32 numIndices = pMesh->mNumFaces * 3;
    m_indices.reserve(numIndices);
    for (uint32 i = 0; i < pMesh->mNumFaces; ++i)
    {
        const aiFace& face = pMesh->mFaces[i];
        for (uint32 j = 0; j < face.mNumIndices; ++j)
        {
            m_indices.emplace_back(face.mIndices[j]);
        }
    }

    // Load Vertex Data:
    m_vertices.resize(pMesh->mNumVertices);
    for (uint32 i = 0; i < pMesh->mNumVertices; ++i)
    {
        m_vertices[i].m_position = nes::Vec3(pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z);
        m_vertices[i].m_normal = nes::Vec3(pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z);

        if (pMesh->HasTangentsAndBitangents())
        {
            m_vertices[i].m_tangent = { pMesh->mTangents[i].x, pMesh->mTangents[i].y ,pMesh->mTangents[i].z };
            m_vertices[i].m_bitangent = { pMesh->mBitangents[i].x, pMesh->mBitangents[i].y ,pMesh->mBitangents[i].z };
        }

        if (pMesh->HasTextureCoords(0))
        {
            m_vertices[i].m_texCoord = nes::Vec2(pMesh->mTextureCoords[0][i].x, pMesh->mTextureCoords[0][i].y);
        }
    }

    // Calculate Tangent space if necessary:
    if (!pMesh->HasTangentsAndBitangents())
    {
        Mesh mesh;
        mesh.m_vertexCount = pMesh->mNumVertices;
        mesh.m_indexCount = numIndices;
        mesh.m_firstVertex = 0;
        mesh.m_firstIndex = 0;
        helpers::CalculateTangentSpace(m_vertices, m_indices, mesh);
    }
    
    // Create the Default PBR Material for the Mesh:
    // [TODO]: Assuming a single material for now.
    // [TODO]: This would probably be done in an import step.
    PBRMaterialDesc materialDesc{};
    materialDesc.m_baseColorMap = nes::kInvalidAssetID;
    materialDesc.m_normalMap = nes::kInvalidAssetID;
    materialDesc.m_roughnessMetallicMap = nes::kInvalidAssetID;
    materialDesc.m_emissionMap = nes::kInvalidAssetID;
    materialDesc.m_baseColor = nes::Float4(1.f);
    materialDesc.m_emission = nes::Float3(1.f);
    materialDesc.m_metallic = 1.f;
    materialDesc.m_roughness = 1.f;
    materialDesc.m_isTransparent = false;
    
    if (pScene->HasMaterials())
    {
        const aiMaterial* pMaterial = pScene->mMaterials[0];
        
        // Base Color Factor
        aiColor4D colorFactor;
        if (pMaterial->Get(AI_MATKEY_BASE_COLOR, colorFactor) != AI_SUCCESS)
        {
            NES_ERROR("Failed to get Base Color from material! Name: {}", pMaterial->GetName().C_Str());
            return nes::ELoadResult::Failure;
        }
        materialDesc.m_baseColor = nes::Float4(colorFactor.r, colorFactor.g, colorFactor.b, colorFactor.a);

        // Emission Color Factor
        if (pMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, colorFactor) == AI_SUCCESS)
            materialDesc.m_emission = nes::Float3(colorFactor.r, colorFactor.g, colorFactor.b);
        
        // Metallic Factor
        float metallic;
        if (pMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS)
            materialDesc.m_metallic = metallic;

        // Roughness Factor
        float roughness;
        if (pMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS)
            materialDesc.m_roughness = roughness;
            
        // Opacity
        float opacity;
        if (pMaterial->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
        {
            materialDesc.m_baseColor.w = opacity;
            materialDesc.m_isTransparent = opacity < 1.f;
        }

        // Texture Maps:
        aiString texturePath;
        
        // Base Color Texture
        if (pMaterial->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &texturePath) == AI_SUCCESS)
        {
            if ([[maybe_unused]] auto pEmbeddedTexture = pScene->GetEmbeddedTexture(texturePath.C_Str()))
            {
                NES_ASSERT(false, "Embedded texture not implemented yet!");
                return nes::ELoadResult::Failure;
            }
            else
            {
                // Load texture from path:
                std::filesystem::path filePath = path.parent_path();
                filePath /= texturePath.C_Str();
                
                const auto result = nes::AssetManager::LoadSync<nes::Texture>(materialDesc.m_baseColorMap, filePath);
                if (result != nes::ELoadResult::Success)
                {
                    // Set to the Error Texture
                    NES_ERROR("Failed to load Base Color texture for Mesh! Setting to Error Texture...\n\t - Mesh Path: {}", path.string());
                    materialDesc.m_baseColorMap = helpers::GetDefaultTextureID(EDefaultTextureType::Error);
                }
            }
        }

        // Normal Texture
        if (pMaterial->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS)
        {
            if ([[maybe_unused]] auto pEmbeddedTexture = pScene->GetEmbeddedTexture(texturePath.C_Str()))
            {
                NES_ASSERT(false, "Embedded texture not implemented yet!");
                return nes::ELoadResult::Failure;
            }
            else
            {
                // Load texture from path:
                std::filesystem::path filePath = path.parent_path();
                filePath /= texturePath.C_Str();
                
                const auto result = nes::AssetManager::LoadSync<nes::Texture>(materialDesc.m_normalMap, filePath);
                if (result != nes::ELoadResult::Success)
                {
                    // Set to the FlatNormal Texture
                    NES_ERROR("Failed to load Normal texture for Mesh! Setting to FlatNormal Texture...\n\t - Mesh Path: {}", path.string());
                    materialDesc.m_normalMap = helpers::GetDefaultTextureID(EDefaultTextureType::FlatNormal);
                }
            }
        }

        // Roughness (G), Metallic (B)
        if (pMaterial->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &texturePath) == AI_SUCCESS)
        {
#if NES_DEBUG
            // Ensure that Roughness & Metallic are the same texture.
            aiString roughnessPath;
            pMaterial->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &roughnessPath);
            NES_ASSERT(roughnessPath == texturePath, "Invalid PBR Material Textures. Roughness and Metallic should in the same texture. Roughness in the G channel, and Metallic in the B channel.");
#endif

            if ([[maybe_unused]] auto pEmbeddedTexture = pScene->GetEmbeddedTexture(texturePath.C_Str()))
            {
                NES_ASSERT(false, "Embedded texture not implemented yet!");
                return nes::ELoadResult::Failure;
            }
            else
            {
                // Load texture from path:
                std::filesystem::path filePath = path.parent_path();
                filePath /= texturePath.C_Str();
                
                const auto result = nes::AssetManager::LoadSync<nes::Texture>(materialDesc.m_roughnessMetallicMap, filePath);
                if (result != nes::ELoadResult::Success)
                {
                    // Set to the FlatNormal Texture
                    NES_ERROR("Failed to load Roughness/Metallic texture for Mesh! Setting to Black Texture...\n\t - Mesh Path: {}", path.string());
                    materialDesc.m_roughnessMetallicMap = helpers::GetDefaultTextureID(EDefaultTextureType::Black);
                }
            }
        }

        // Emission Map
        // [TODO]: Emissive Amount should probably be in the same texture (Alpha Channel).
        if (pMaterial->GetTexture(aiTextureType_EMISSION_COLOR, 0, &texturePath) == AI_SUCCESS)
        {
            if ([[maybe_unused]] auto pEmbeddedTexture = pScene->GetEmbeddedTexture(texturePath.C_Str()))
            {
                NES_ASSERT(false, "Embedded texture not implemented yet!");
                return nes::ELoadResult::Failure;
            }
            else
            {
                // Load texture from path:
                std::filesystem::path filePath = path.parent_path();
                filePath /= texturePath.C_Str();
                
                const auto result = nes::AssetManager::LoadSync<nes::Texture>(materialDesc.m_emissionMap, filePath);
                if (result != nes::ELoadResult::Success)
                {
                    // Set to the FlatNormal Texture
                    NES_ERROR("Failed to load Emissive texture for Mesh! Setting to Black Texture...\n\t - Mesh Path: {}", path.string());
                    materialDesc.m_emissionMap = helpers::GetDefaultTextureID(EDefaultTextureType::Black);
                }
            }
        }
    }

    // Create the default material for the Mesh:
    {
        // Ensure default values for each map type:
        if (materialDesc.m_baseColorMap == nes::kInvalidAssetID)
            materialDesc.m_baseColorMap = helpers::GetDefaultTextureID(EDefaultTextureType::White);

        if (materialDesc.m_normalMap == nes::kInvalidAssetID)
            materialDesc.m_normalMap = helpers::GetDefaultTextureID(EDefaultTextureType::FlatNormal);

        if (materialDesc.m_roughnessMetallicMap == nes::kInvalidAssetID)
            materialDesc.m_roughnessMetallicMap = helpers::GetDefaultTextureID(EDefaultTextureType::White);

        if (materialDesc.m_emissionMap == nes::kInvalidAssetID)
            materialDesc.m_emissionMap = helpers::GetDefaultTextureID(EDefaultTextureType::Black);
        
        PBRMaterial defaultMaterial(materialDesc);
        nes::AssetManager::AddMemoryAsset<PBRMaterial>(m_defaultMaterialID, std::move(defaultMaterial));
    }

    return nes::ELoadResult::Success;
}
