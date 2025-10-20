// PBRMesh.cpp
#include "PBRMesh.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "PBRScene.h"
#include "ComponentSystems/PBRSceneRenderer.h"
#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Graphics/Texture.h"

namespace pbr
{
    void helpers::CalculateTangentSpace(std::vector<Vertex>& outVertices, const std::vector<uint32>& indices, const Mesh& mesh)
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
            nes::Vec3 n0(v0.m_normal);
            nes::Vec3 n1(v1.m_normal);
            nes::Vec3 n2(v2.m_normal);
        
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

    void helpers::AppendCubeMeshData(std::vector<Vertex>& outVertices, std::vector<uint32>& outIndices, Mesh& outMesh)
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

    void helpers::AppendSphereMeshData(const SphereGenDesc& sphereDesc, std::vector<Vertex>& outVertices, std::vector<uint32>& outIndices, Mesh& outMesh)
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

    void helpers::AppendPlaneData(const PlaneGenDesc& planeDesc, std::vector<Vertex>& outVertices, std::vector<uint32>& outIndices, Mesh& outMesh)
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
                        materialDesc.m_baseColorMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::Error);
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
                        materialDesc.m_normalMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::FlatNormal);
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
                        materialDesc.m_roughnessMetallicMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::Black);
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
                        materialDesc.m_emissionMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::Black);
                    }
                }
            }
        }

        // Create the default material for the Mesh:
        {
            // Ensure default values for each map type:
            if (materialDesc.m_baseColorMap == nes::kInvalidAssetID)
                materialDesc.m_baseColorMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::White);

            if (materialDesc.m_normalMap == nes::kInvalidAssetID)
                materialDesc.m_normalMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::FlatNormal);

            if (materialDesc.m_roughnessMetallicMap == nes::kInvalidAssetID)
                materialDesc.m_roughnessMetallicMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::White);

            if (materialDesc.m_emissionMap == nes::kInvalidAssetID)
                materialDesc.m_emissionMap = PBRSceneRenderer::GetDefaultTextureID(EDefaultTextureType::Black);
            
            PBRMaterial defaultMaterial(materialDesc);
            nes::AssetManager::AddMemoryAsset<PBRMaterial>(m_defaultMaterialID, std::move(defaultMaterial));
        }

        return nes::ELoadResult::Success;
    }
}
