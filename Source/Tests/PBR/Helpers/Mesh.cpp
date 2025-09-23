// Mesh.cpp
#include "Mesh.h"

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

nes::ELoadResult MeshAsset::LoadFromFile(const std::filesystem::path& /*path*/)
{
    // [TODO]: 
    return nes::ELoadResult::Failure;
}
