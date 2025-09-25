// Primitives.h
#pragma once
#include "Mesh.h"

namespace helpers
{
    inline void CalculateTangentSpace(std::vector<Vertex>& outVertices, const std::vector<uint32>& indices, const Mesh& mesh)
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
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the vertices and indices for a 3D cube to the two output arrays. 
    //----------------------------------------------------------------------------------------------------
    static inline void AppendCubeMeshData(std::vector<Vertex>& outVertices, std::vector<uint32>& outIndices, Mesh& outMesh)
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

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the vertices and indices for a Sphere to the two output arrays. 
    //----------------------------------------------------------------------------------------------------
    static inline void AppendSphereMeshData(std::vector<Vertex>& outVertices, std::vector<uint32>& outIndices, Mesh& outMesh)
    {
        outMesh.m_firstVertex = static_cast<uint32>(outVertices.size());
        outMesh.m_firstIndex = static_cast<uint32>(outIndices.size());

        static constexpr float kRadius = 0.5f;
        static constexpr float kLatitudeBands = 30.f;
        static constexpr float kLongitudeBands = 30.f;

        for (float latitude = 0.f; latitude <= kLatitudeBands; ++latitude)
        {
            const float theta = latitude * nes::math::Pi<float>() / kLatitudeBands;
            const float sinTheta = nes::math::Sin(theta);
            const float cosTheta = nes::math::Cos(theta);

            for (float longitude = 0.f; longitude <= kLongitudeBands; ++longitude)
            {
                const float phi = longitude * 2.f * nes::math::Pi<float>() / kLongitudeBands;
                const float sinPhi = nes::math::Sin(phi);
                const float cosPhi = nes::math::Cos(phi);

                Vertex vertex{};
                vertex.m_normal.x = cosPhi * sinTheta;
                vertex.m_normal.y = cosTheta;
                vertex.m_normal.z = sinPhi * sinTheta;
                vertex.m_position = vertex.m_normal * kRadius;
                vertex.m_texCoord.x = 1.f - (longitude / kLongitudeBands); 
                vertex.m_texCoord.y = 1.f - (latitude / kLatitudeBands);
                outVertices.emplace_back(vertex);
            }
        }

        for (uint32_t latitude = 0; latitude < static_cast<uint32_t>(kLatitudeBands); ++latitude)
        {
            for (uint32_t longitude = 0; longitude < static_cast<uint32_t>(kLongitudeBands); ++longitude)
            {
                const uint32_t first = (latitude * (static_cast<uint32_t>(kLongitudeBands) + 1)) + longitude;
                const uint32_t second = first + static_cast<uint32_t>(kLongitudeBands) + 1;

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

    static inline void AppendPlaneData(std::vector<Vertex>& outVertices, std::vector<uint32>& outIndices, Mesh& outMesh)
    {
        outMesh.m_firstVertex = static_cast<uint32>(outVertices.size());
        outMesh.m_firstIndex = static_cast<uint32>(outIndices.size());
        
        static constexpr uint32 kSubdivisionsX = 10;
        static constexpr uint32 kSubdivisionsY = 10;
        static constexpr float kWidth = 10.f;
        static constexpr float kHeight = 10.f;

        for (uint32 y = 0; y <= kSubdivisionsY; ++y)
        {
            const float yTexCoord = static_cast<float>(y) / static_cast<float>(kSubdivisionsY);
            const float yVertPos = static_cast<float>(y) / static_cast<float>(kSubdivisionsY) * kHeight - (kHeight * 0.5f);
            
            for (uint32 x = 0; x <= kSubdivisionsX; ++x)
            {
                Vertex vertex{};
                vertex.m_position = nes::Vec3(
                    static_cast<float>(x) / static_cast<float>(kSubdivisionsX) * kWidth - (kWidth * 0.5f),
                    0.f,
                    yVertPos);

                vertex.m_normal = nes::Vec3::AxisY();
                vertex.m_tangent = nes::Vec3::AxisX();
                vertex.m_bitangent = nes::Vec3::AxisZ();
                NES_ASSERT(nes::Vec3::IsLeftHanded(vertex.m_tangent, vertex.m_bitangent, vertex.m_normal));
                vertex.m_texCoord = nes::Vec2(static_cast<float>(x) / static_cast<float>(kSubdivisionsX), yTexCoord);
                outVertices.emplace_back(vertex);

                // Add indices for two triangles of the quad.
                if (y < kSubdivisionsY && x < kSubdivisionsX)
                {
                    const uint32 topLeft = y * (kSubdivisionsX + 1) + x;            // 0
                    const uint32 topRight = topLeft + 1;                            // 1
                    const uint32 bottomLeft = (y + 1) * (kSubdivisionsX + 1) + x;   // 2
                    const uint32 bottomRight = bottomLeft + 1;                      // 3

                    outIndices.insert(outIndices.end(), {
                        topLeft, topRight, bottomLeft,                  // 0, 1, 2
                        topRight, bottomRight, bottomLeft});            // 1, 3, 2
                }
            }
        }

        outMesh.m_indexCount = static_cast<uint32>(outIndices.size()) - outMesh.m_firstIndex;
        outMesh.m_vertexCount = static_cast<uint32>(outVertices.size()) - outMesh.m_firstVertex;
        
        //CalculateTangentSpace(outVertices, outIndices, outMesh);
    }
}
