// PBRMesh.cpp
#include "PBRMesh.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "PBRScene.h"
#include "ComponentSystems/PBRSceneRenderer.h"
#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Graphics/Texture.h"
#include "Nessie/FileIO/YAML/Serializers/YamlCoreSerializers.h"

namespace pbr
{
    void helpers::CalculateTangentSpace(std::vector<Vertex>& outVertices, const std::vector<uint32>& indices, const MeshInstance& mesh)
    {
        std::vector<nes::Vec3> tangents(mesh.m_vertexCount, nes::Vec3::Zero());
        std::vector<nes::Vec3> bitangents(mesh.m_vertexCount, nes::Vec3::Zero());
        
        for (uint32 i = 0; i < mesh.m_vertexCount; ++i)
        {
            const uint32 vertexIndex = i + mesh.m_firstVertex;
            outVertices[vertexIndex].m_tangent = nes::Vec3::Zero();
            outVertices[vertexIndex].m_bitangent = nes::Vec3::Zero();
        }
        
        for (size_t j = 0; j < mesh.m_indexCount; j += 3)
        {
            const size_t indexOffset = mesh.m_firstIndex + j;
            
            const uint32 i0 = indices[indexOffset];
            const uint32 i1 = indices[indexOffset + 1];
            const uint32 i2 = indices[indexOffset + 2];
        
            Vertex& v0 = outVertices[mesh.m_firstVertex + i0];
            Vertex& v1 = outVertices[mesh.m_firstVertex + i1];
            Vertex& v2 = outVertices[mesh.m_firstVertex + i2];
        
            // Normals
            //nes::Vec3 n0(v0.m_normal);
            nes::Vec3 n1(v1.m_normal);
            //nes::Vec3 n2(v2.m_normal);
        
            // Calculate triangle edges in position and UV space
            const nes::Vec3 edge1 = v1.m_position - v0.m_position;
            const nes::Vec3 edge2 = v2.m_position - v0.m_position;
        
            const nes::Vec2 deltaUV10 = v1.m_texCoord - v0.m_texCoord;
            const nes::Vec2 deltaUV20 = v2.m_texCoord - v0.m_texCoord;
        
            const float r = 1.0f / (deltaUV10.x * deltaUV20.y - deltaUV10.y * deltaUV20.x);
            nes::Vec3 tangent;
            nes::Vec3 bitangent;
            if (nes::math::Abs(r) < 1e-9f)
            {
                n1.z += 1e-6f;
                tangent = n1.NormalizedPerpendicular();
                bitangent = nes::Vec3::Cross(n1, tangent);
            }
            else
            {
                const float invR = 1.f / r;
                nes::Vec3 a = edge1 * invR;
                nes::Vec3 b = edge2 * invR;
                tangent = a * deltaUV20.y - b * deltaUV10.y;
                bitangent = b * deltaUV10.x - a * deltaUV20.x;
            }
            
            tangents[i0] += tangent;
            tangents[i1] += tangent;
            tangents[i2] += tangent;
            
            bitangents[i0] += bitangent;
            bitangents[i1] += bitangent;
            bitangents[i2] += bitangent;
        }
        
        // Normalize and orthogonalize
        for (uint32 j = 0; j < mesh.m_vertexCount; ++j)
        {
            const uint32 vertexIndex = j + mesh.m_firstVertex;
            auto& vertex = outVertices[vertexIndex];
            
            nes::Vec3 normal = vertex.m_normal;
            
            nes::Vec3 tangent = tangents[j];
            if (tangent.Length() < 1e-9f)
                tangent = nes::Vec3::Cross(bitangents[j], vertex.m_normal);
            else
                tangent -= normal * nes::Vec3::Dot(normal, tangent);
            tangent.Normalize();
            
            vertex.m_tangent = tangent;
            vertex.m_bitangent = nes::Vec3::Cross(vertex.m_tangent, vertex.m_normal);
            
            // Assert Left-Handed:
            NES_ASSERT(nes::Vec3::IsLeftHanded(vertex.m_tangent, vertex.m_bitangent, vertex.m_normal));
        }
    }

    void helpers::AppendCubeMeshData(std::vector<Vertex>& outVertices, std::vector<uint32>& outIndices, MeshInstance& outMesh)
    {
        outMesh.m_firstVertex = static_cast<uint32>(outVertices.size());
        outMesh.m_firstIndex = static_cast<uint32>(outIndices.size());
        outMesh.m_vertexCount = 24; // 24 vertices - 4 per face for proper normals and UVs.
        outMesh.m_indexCount = 36; // 3 vertex per triangle * 2 triangles per face * 6 faces = 36 vertices.

        
        outVertices.insert(outVertices.end(),
    {
            // Front face (-Z in left-handed, closest to viewer)
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        
            // Back face (+Z in left-handed, farthest from viewer)
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},

            // Left face (-X)
            {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        
            // Right face (+X)
            {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},

            // Top face (+Y)
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},

            // Bottom face (-Y)
            {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},
        });

        outIndices.insert(outIndices.end(),
    {
            // Front face
            0, 1, 2, 2, 3, 0,
            // Back face  
            4, 5, 6, 6, 7, 4,
            // Left face
            8, 9, 10, 10, 11, 8,
            // Right face
            12, 13, 14, 14, 15, 12,
            // Top face
            16, 17, 18, 18, 19, 16,
            // Bottom face
            20, 21, 22, 22, 23, 20
        });

        // Check pre-calculated results:
        //CalculateTangentSpace(outVertices, outIndices, outMesh);
    }

    void helpers::AppendSphereMeshData(const SphereGenDesc& sphereDesc, std::vector<Vertex>& outVertices, std::vector<uint32>& outIndices, MeshInstance& outMesh)
    {
        outMesh.m_firstVertex = static_cast<uint32>(outVertices.size());
        outMesh.m_firstIndex = static_cast<uint32>(outIndices.size());

        for (float latitude = 0.f; latitude <= sphereDesc.m_latitudeBands; ++latitude)
        {
            const float theta = latitude * nes::math::Pi<float>() / sphereDesc.m_latitudeBands;
            const float sinTheta = nes::math::Sin(theta);
            const float cosTheta = nes::math::Cos(theta);

            for (float longitude = 0.f; longitude <= sphereDesc.m_longitudeBands; ++longitude)
            {
                const float phi = longitude * 2.f * nes::math::Pi<float>() / sphereDesc.m_longitudeBands;
                const float sinPhi = nes::math::Sin(phi);
                const float cosPhi = nes::math::Cos(phi);

                Vertex vertex{};
                vertex.m_normal.x = cosPhi * sinTheta;
                vertex.m_normal.y = cosTheta;
                vertex.m_normal.z = sinPhi * sinTheta;
                vertex.m_position = vertex.m_normal * sphereDesc.m_radius;
                vertex.m_texCoord.x = 1.f - (longitude / sphereDesc.m_longitudeBands); 
                vertex.m_texCoord.y = 1.f - (latitude / sphereDesc.m_latitudeBands);
                outVertices.emplace_back(vertex);
            }
        }

        for (uint32_t latitude = 0; latitude < static_cast<uint32_t>(sphereDesc.m_latitudeBands); ++latitude)
        {
            for (uint32_t longitude = 0; longitude < static_cast<uint32_t>(sphereDesc.m_longitudeBands); ++longitude)
            {
                const uint32_t first = (latitude * (static_cast<uint32_t>(sphereDesc.m_longitudeBands) + 1)) + longitude;
                const uint32_t second = first + static_cast<uint32_t>(sphereDesc.m_longitudeBands) + 1;

                outIndices.emplace_back(first);
                outIndices.emplace_back(second);
                outIndices.emplace_back(first + 1);
                outIndices.emplace_back(second);
                outIndices.emplace_back(second + 1);
                outIndices.emplace_back(first + 1);
            }
        }

        outMesh.m_indexCount = static_cast<uint32>(outIndices.size()) - outMesh.m_firstIndex;
        outMesh.m_vertexCount = static_cast<uint32>(outVertices.size()) - outMesh.m_firstVertex;

        CalculateTangentSpace(outVertices, outIndices, outMesh);
    }

    void helpers::AppendPlaneData(const PlaneGenDesc& planeDesc, std::vector<Vertex>& outVertices, std::vector<uint32>& outIndices, MeshInstance& outMesh)
    {
        outMesh.m_firstVertex = static_cast<uint32>(outVertices.size());
        outMesh.m_firstIndex = static_cast<uint32>(outIndices.size());

        for (uint32 y = 0; y <= planeDesc.m_subdivisionsZ; ++y)
        {
            const float yTexCoord = static_cast<float>(y) / static_cast<float>(planeDesc.m_subdivisionsZ);
            const float yVertPos = static_cast<float>(y) / static_cast<float>(planeDesc.m_subdivisionsZ) * planeDesc.m_height - (planeDesc.m_height * 0.5f);
            
            for (uint32 x = 0; x <= planeDesc.m_subdivisionsX; ++x)
            {
                Vertex vertex{};
                vertex.m_position = nes::Vec3(
                    static_cast<float>(x) / static_cast<float>(planeDesc.m_subdivisionsX) * planeDesc.m_width - (planeDesc.m_width * 0.5f),
                    0.f,
                    yVertPos);

                vertex.m_normal = nes::Vec3::AxisY();
                vertex.m_tangent = nes::Vec3::AxisX();
                vertex.m_bitangent = nes::Vec3::AxisZ();
                NES_ASSERT(nes::Vec3::IsLeftHanded(vertex.m_tangent, vertex.m_bitangent, vertex.m_normal));
                vertex.m_texCoord = nes::Vec2(static_cast<float>(x) / static_cast<float>(planeDesc.m_subdivisionsX), yTexCoord);
                outVertices.emplace_back(vertex);

                // Add indices for two triangles of the quad.
                if (y < planeDesc.m_subdivisionsZ && x < planeDesc.m_subdivisionsX)
                {
                    const uint32 topLeft = y * (planeDesc.m_subdivisionsX + 1) + x;            // 0
                    const uint32 topRight = topLeft + 1;                            // 1
                    const uint32 bottomLeft = (y + 1) * (planeDesc.m_subdivisionsX + 1) + x;   // 2
                    const uint32 bottomRight = bottomLeft + 1;                      // 3

                    outIndices.insert(outIndices.end(), {
                        topLeft, topRight, bottomLeft,                  // 0, 1, 2
                        topRight, bottomRight, bottomLeft});            // 1, 3, 2
                }
            }
        }

        outMesh.m_indexCount = static_cast<uint32>(outIndices.size()) - outMesh.m_firstIndex;
        outMesh.m_vertexCount = static_cast<uint32>(outVertices.size()) - outMesh.m_firstVertex;

        // Check calculated results:
        //CalculateTangentSpace(outVertices, outIndices, outMesh);
    }

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

    static bool LoadMaterialDataForSubMesh(const uint32, const aiScene* pScene, const aiMaterial* pMaterial, const std::filesystem::path& path, PBRMaterialDesc& outMaterialDesc)
    {
        // Create the Default PBR Material for the Mesh:
        outMaterialDesc.m_baseColorMap = nes::kInvalidAssetID;
        outMaterialDesc.m_normalMap = nes::kInvalidAssetID;
        outMaterialDesc.m_roughnessMetallicMap = nes::kInvalidAssetID;
        outMaterialDesc.m_emissionMap = nes::kInvalidAssetID;
        outMaterialDesc.m_baseColor = nes::Float4(1.f);
        outMaterialDesc.m_emission = nes::Float3(1.f);
        outMaterialDesc.m_metallic = 1.f;
        outMaterialDesc.m_roughness = 1.f;
        outMaterialDesc.m_isTransparent = false;
        
        // Base Color Factor
        aiColor4D colorFactor;
        bool hasBaseColor = pMaterial->Get(AI_MATKEY_BASE_COLOR, colorFactor) == AI_SUCCESS;
        if (!hasBaseColor)
        {
            hasBaseColor = pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, colorFactor) == AI_SUCCESS;
        }
        if (!hasBaseColor)
        {
            NES_ERROR("Failed to get Base Color from material! Name: {}", pMaterial->GetName().C_Str());
            return false;
        }
        outMaterialDesc.m_baseColor = nes::Float4(colorFactor.r, colorFactor.g, colorFactor.b, colorFactor.a);

        // Emission Color Factor
        if (pMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, colorFactor) == AI_SUCCESS)
            outMaterialDesc.m_emission = nes::Float3(colorFactor.r, colorFactor.g, colorFactor.b);
        
        // Metallic Factor
        float metallic;
        if (pMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS)
            outMaterialDesc.m_metallic = metallic;

        // Roughness Factor
        float roughness;
        if (pMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS)
            outMaterialDesc.m_roughness = roughness;
            
        // Opacity
        float opacity;
        if (pMaterial->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
        {
            outMaterialDesc.m_baseColor.w = opacity;
            outMaterialDesc.m_isTransparent = opacity < 1.f;
        }

        // Texture Maps:
        aiString texturePath;
        
        // Base Color Texture
        if (pMaterial->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &texturePath) == AI_SUCCESS)
        {
            if ([[maybe_unused]] auto pEmbeddedTexture = pScene->GetEmbeddedTexture(texturePath.C_Str()))
            {
                NES_ASSERT(false, "Embedded texture not implemented yet!");
                return false;
            }
            else
            {
                // Load texture from path:
                std::filesystem::path filePath = path.parent_path();
                filePath /= texturePath.C_Str();
                
                const auto result = nes::AssetManager::LoadSync<nes::Texture>(outMaterialDesc.m_baseColorMap, filePath);
                if (result != nes::ELoadResult::Success)
                {
                    // Set to the Error Texture
                    NES_ERROR("Failed to load Base Color texture for Mesh! Setting to Error Texture...\n\t - Mesh Path: {}", path.string());
                    outMaterialDesc.m_baseColorMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::Error);
                }
            }
        }

        // Normal Texture
        if (pMaterial->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS)
        {
            if ([[maybe_unused]] auto pEmbeddedTexture = pScene->GetEmbeddedTexture(texturePath.C_Str()))
            {
                NES_ASSERT(false, "Embedded texture not implemented yet!");
                return false;
            }
            else
            {
                // Load texture from path:
                std::filesystem::path filePath = path.parent_path();
                filePath /= texturePath.C_Str();
                
                const auto result = nes::AssetManager::LoadSync<nes::Texture>(outMaterialDesc.m_normalMap, filePath);
                if (result != nes::ELoadResult::Success)
                {
                    // Set to the FlatNormal Texture
                    NES_ERROR("Failed to load Normal texture for Mesh! Setting to FlatNormal Texture...\n\t - Mesh Path: {}", path.string());
                    outMaterialDesc.m_normalMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::FlatNormal);
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
                return false;
            }
            else
            {
                // Load texture from path:
                std::filesystem::path filePath = path.parent_path();
                filePath /= texturePath.C_Str();
                
                const auto result = nes::AssetManager::LoadSync<nes::Texture>(outMaterialDesc.m_roughnessMetallicMap, filePath);
                if (result != nes::ELoadResult::Success)
                {
                    // Set to the FlatNormal Texture
                    NES_ERROR("Failed to load Roughness/Metallic texture for Mesh! Setting to Black Texture...\n\t - Mesh Path: {}", path.string());
                    outMaterialDesc.m_roughnessMetallicMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::Black);
                }
            }
        }

        // Emission Map
        // [TODO]: Emission Factor should probably be in the same texture (Alpha Channel).
        if (pMaterial->GetTexture(aiTextureType_EMISSION_COLOR, 0, &texturePath) == AI_SUCCESS)
        {
            if ([[maybe_unused]] auto pEmbeddedTexture = pScene->GetEmbeddedTexture(texturePath.C_Str()))
            {
                NES_ASSERT(false, "Embedded texture not implemented yet!");
                return false;
            }
            else
            {
                // Load texture from path:
                std::filesystem::path filePath = path.parent_path();
                filePath /= texturePath.C_Str();
                
                const auto result = nes::AssetManager::LoadSync<nes::Texture>(outMaterialDesc.m_emissionMap, filePath);
                if (result != nes::ELoadResult::Success)
                {
                    // Set to the FlatNormal Texture
                    NES_ERROR("Failed to load Emissive texture for Mesh! Setting to Black Texture...\n\t - Mesh Path: {}", path.string());
                    outMaterialDesc.m_emissionMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::Black);
                }
            }
        }

        // Create the default material for the Mesh.
        // Ensure default values for each map type:
        if (outMaterialDesc.m_baseColorMap == nes::kInvalidAssetID)
            outMaterialDesc.m_baseColorMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::White);

        if (outMaterialDesc.m_normalMap == nes::kInvalidAssetID)
            outMaterialDesc.m_normalMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::FlatNormal);

        if (outMaterialDesc.m_roughnessMetallicMap == nes::kInvalidAssetID)
            outMaterialDesc.m_roughnessMetallicMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::White);

        if (outMaterialDesc.m_emissionMap == nes::kInvalidAssetID)
            outMaterialDesc.m_emissionMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::Black);
        
        return true;
    }

    MeshAsset::MeshAsset(const std::vector<Vertex>& vertices, const std::vector<uint32>& indices, const nes::AssetID defaultMaterialID)
        : m_vertices(vertices)
        , m_indices(indices)
    {
        m_materialIDs.emplace_back(defaultMaterialID);
        m_subMeshes.emplace_back(0u, static_cast<uint32>(indices.size()), static_cast<uint32>(vertices.size()),0u);
    }

    MeshAsset::MeshAsset(Vertex* pVertices, const uint32 vertexCount, uint32* indices, const uint32 indexCount, const nes::AssetID defaultMaterialID)
        : m_vertices(pVertices, pVertices + vertexCount)
        , m_indices(indices, indices + indexCount)
    {
        m_materialIDs.emplace_back(defaultMaterialID);
        m_subMeshes.emplace_back(0u, indexCount, vertexCount, 0u);
    }

    nes::ELoadResult MeshAsset::LoadFromFile(const std::filesystem::path& path)
    {
        nes::YamlInStream file(path);

        if (!file.IsOpen())
        {
            NES_ERROR("Failed to load Mesh! Expecting a YAML file.");
            return nes::ELoadResult::InvalidArgument;
        }

        nes::YamlNode mesh = file.GetRoot()["Mesh"];
        if (!mesh)
        {
            NES_ERROR("Failed to load Mesh! YAML file invalid: Missing 'Mesh' entry!");
            return nes::ELoadResult::InvalidArgument;
        }

        return LoadFromYAML(mesh, path.stem().string());
    }

    nes::ELoadResult MeshAsset::LoadFromYAML(const nes::YamlNode& node, const std::string& /*yamlFileName*/)
    {
        bool invertWinding;
        node["InvertWinding"].Read(invertWinding, true);
    
        Assimp::Importer importer;
        int importFlags = aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_FlipUVs;
        if (invertWinding)
            importFlags |= aiProcess_FlipWindingOrder;
        
        std::filesystem::path path = NES_CONTENT_DIR;
        std::string relativePath;
        node["Path"].Read(relativePath);
        path /= relativePath;
        
        const aiScene* pScene = importer.ReadFile(path.string().c_str(), importFlags);
        if (!pScene)
        {
            NES_ERROR("Failed to load assimp file! Error: {}", importer.GetErrorString());
            return nes::ELoadResult::Failure;
        }
        
        NES_ASSERT(pScene->mNumMeshes > 0, "No Meshes in file! {}", path.string());

        std::unordered_map<uint32, uint32> sourceMaterialIndexToAssetIndex{};
        std::vector<PBRMaterialDesc> uniqueMaterialDescs{};
        uniqueMaterialDescs.reserve(pScene->mNumMeshes);
        
        for (uint32 meshIndex = 0; meshIndex < pScene->mNumMeshes; ++meshIndex)
        {
            const aiMesh* pMesh = pScene->mMeshes[meshIndex];

            const uint32 firstVertex = static_cast<uint32>(m_vertices.size());
            const uint32 firstIndex = static_cast<uint32>(m_indices.size());
        
            // Load Index data:
            const uint32 numIndices = pMesh->mNumFaces * 3;
            m_indices.reserve(firstIndex + numIndices);
            for (uint32 i = 0; i < pMesh->mNumFaces; ++i)
            {
                const aiFace& face = pMesh->mFaces[i];
                for (uint32 j = 0; j < face.mNumIndices; ++j)
                {
                    m_indices.emplace_back(face.mIndices[j]);
                }
            }

            // Load Vertex Data:
            m_vertices.reserve(firstVertex + pMesh->mNumVertices);
            for (uint32 i = 0; i < pMesh->mNumVertices; ++i)
            {
                auto& vertex = m_vertices.emplace_back();
                vertex.m_position = nes::Vec3(pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z);
                vertex.m_normal = nes::Vec3(pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z);

                if (pMesh->HasTangentsAndBitangents())
                {
                    vertex.m_tangent = { pMesh->mTangents[i].x, pMesh->mTangents[i].y ,pMesh->mTangents[i].z };
                    vertex.m_bitangent = { pMesh->mBitangents[i].x, pMesh->mBitangents[i].y ,pMesh->mBitangents[i].z };
                }

                if (pMesh->HasTextureCoords(0))
                {
                    vertex.m_texCoord = nes::Vec2(pMesh->mTextureCoords[0][i].x, pMesh->mTextureCoords[0][i].y);
                }
            }
            
            // Calculate Tangent space if necessary:
            if (!pMesh->HasTangentsAndBitangents())
            {
                MeshInstance mesh;
                mesh.m_firstVertex = firstVertex;
                mesh.m_vertexCount = pMesh->mNumVertices;
                mesh.m_firstIndex = firstIndex;
                mesh.m_indexCount = numIndices;
                helpers::CalculateTangentSpace(m_vertices, m_indices, mesh);
            }
            
            nes::AssetID materialID = nes::kInvalidAssetID;
            uint32 materialIndex = 0;
            if (pScene->HasMaterials() && pMesh->mMaterialIndex < pScene->mNumMaterials)
            {
                if (!sourceMaterialIndexToAssetIndex.contains(pMesh->mMaterialIndex))
                {
                    // We haven't loaded this material yet, load it:
                    PBRMaterialDesc materialDesc;
                    const aiMaterial* pMaterial = pScene->mMaterials[pMesh->mMaterialIndex];
                    if (!LoadMaterialDataForSubMesh(meshIndex, pScene, pMaterial, path, materialDesc))
                    {
                        NES_ERROR("Failed to load Material for Submesh '{}'! MeshAsset: '{}'", meshIndex, path.string());
                        return nes::ELoadResult::Failure;
                    }

                    // Compare against other materials in the submesh, so that
                    // we don't create duplicates of the same material asset value.
                    materialIndex = std::numeric_limits<uint32>::max();
                    for (size_t i = 0; i < uniqueMaterialDescs.size(); ++i)
                    {
                        if (uniqueMaterialDescs[i] == materialDesc)
                        {
                            // We already created an asset with this value:
                            materialIndex = static_cast<uint32>(i);
                            materialID = m_materialIDs[materialIndex];
                            m_materialIDs.emplace_back(materialID);
                            break;
                        }
                    }

                    // This is a unique material desc, create the asset.
                    if (materialIndex == std::numeric_limits<uint32>::max())
                    {
                        PBRMaterial defaultMaterial(materialDesc);
                        nes::AssetManager::AddMemoryAsset<PBRMaterial>(materialID, std::move(defaultMaterial), std::format("M_{}_{}", path.stem().string(), meshIndex));
                        
                        m_materialIDs.emplace_back(materialID);
                        materialIndex = static_cast<uint32>(m_materialIDs.size() - 1);
                        uniqueMaterialDescs.emplace_back(materialDesc);
                    }
                    
                    sourceMaterialIndexToAssetIndex.emplace(pMesh->mMaterialIndex, materialIndex);
                }
                else
                {
                    // This material is already loaded, reuse it for this submesh.
                    // Set the existing material index from the loaded result.
                    materialIndex = sourceMaterialIndexToAssetIndex.at(pMesh->mMaterialIndex);
                    m_materialIDs.emplace_back(m_materialIDs[materialIndex]);
                }
            }

            // Create the submesh:
            m_subMeshes.emplace_back(firstIndex, numIndices, pMesh->mNumVertices, materialIndex);
        }
        
        return nes::ELoadResult::Success;
    }
}
