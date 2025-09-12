// Primitives.h
#pragma once

struct Vertex
{
    nes::Vec3   m_position{};
    nes::Vec3   m_normal{};
    nes::Vec2   m_uv{};
};

struct Mesh
{
    uint32      m_firstVertex = 0;
    uint32      m_firstIndex = 0;
    uint32      m_vertexCount = 0;
    uint32      m_indexCount = 0;
};

namespace helpers
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the vertices and indices for a 3D cube to the two output arrays. 
    //----------------------------------------------------------------------------------------------------
    static inline void AppendCubeMeshData(std::vector<Vertex>& outVertices, std::vector<uint32>& outIndices, Mesh& outMesh)
    {
        //
        //   4----5
        //  /|   /|
        // 0----1 |
        // | 7--|-6    y z
        // |/   |/     |/
        // 3----2      +--x
        //

        outMesh.m_firstVertex = static_cast<uint32>(outVertices.size());
        outMesh.m_firstIndex = static_cast<uint32>(outIndices.size());
        outMesh.m_vertexCount = 8;
        outMesh.m_indexCount = 36; // 3 vertex per triangle * 2 triangles per face * 6 faces = 36 vertices.
        
        outVertices.insert(outVertices.end(), 
        {
            { { -0.5f,  0.5f, -0.5f }, {}, { } },
            { {  0.5f,  0.5f, -0.5f }, {}, { } },
            { {  0.5f, -0.5f, -0.5f }, {}, { } },
            { { -0.5f, -0.5f, -0.5f }, {}, { } },
            { { -0.5f,  0.5f,  0.5f }, {}, { } },
            { {  0.5f,  0.5f,  0.5f }, {}, { } },
            { {  0.5f, -0.5f,  0.5f }, {}, { } },
            { { -0.5f, -0.5f,  0.5f }, {}, { } },
        });

        // Generate normals and UVs from position.
        for (size_t i = outMesh.m_firstVertex; i < outVertices.size(); ++i)
        {
            auto& vertex = outVertices[i];
            vertex.m_normal = vertex.m_position.Normalized();
            vertex.m_uv.x = vertex.m_position.x + 0.5f;
            vertex.m_uv.y = vertex.m_position.y + 0.5f;
        }

        outIndices.insert(outIndices.end(),
    {
            0, 3, 2, // Front
            0, 2, 1,
            4, 5, 7, // Rear
            5, 6, 7,
            1, 2, 6, // Right
            5, 1, 6,
            0, 4, 7, // Left
            0, 7, 3,
            5, 4, 0, // Top
            5, 0, 1,
            7, 6, 2, // Bottom
            7, 2, 3,
        });
    }
}
