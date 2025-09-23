// Mesh.h
#pragma once
#include "PBRMaterial.h"
#include "Nessie/Graphics/GraphicsCommon.h"

//----------------------------------------------------------------------------------------------------
// [Note]: Data could be compacted more.
/// @brief : Vertex data for a Mesh.
//----------------------------------------------------------------------------------------------------
struct Vertex
{
    nes::Vec3 m_position{};
    nes::Vec3 m_normal{};
    nes::Vec2 m_texCoord{};
    nes::Vec3 m_tangent{};
    nes::Vec3 m_bitangent{};
    
    static std::array<nes::VertexAttributeDesc, 5> GetBindingDescs(); 
};

//----------------------------------------------------------------------------------------------------
/// @brief : Runtime data for a Mesh asset. The firstVertex/Index are offsets into the Device Buffer
///     that contains all the Vertices/Indices in the Scene.
//----------------------------------------------------------------------------------------------------
struct Mesh
{
    uint32      m_firstVertex = 0;  // Index of the first vertex in the data buffer.
    uint32      m_firstIndex = 0;   // Index of the first index in the data buffer.
    uint32      m_vertexCount = 0;  // Number of vertices that make up this Mesh.
    uint32      m_indexCount = 0;   // Number of indices that make up this Mesh.
};

//----------------------------------------------------------------------------------------------------
/// @brief : Mesh Asset loaded from disk.
//----------------------------------------------------------------------------------------------------
class MeshAsset final : public nes::AssetBase
{
    NES_DEFINE_TYPE_INFO(MeshAsset)

public:
    const std::vector<Vertex>&  GetVertices() const  { return m_vertices; }
    const std::vector<uint32>&  GetIndices() const   { return m_indices; }

private:
    virtual nes::ELoadResult    LoadFromFile(const std::filesystem::path& path) override;
    //nes::ELoadResult            LoadFromYAML(const YAML::Node& node);

private:
    std::vector<Vertex>         m_vertices{};
    std::vector<uint32>         m_indices{};
    nes::AssetID                m_defaultMaterialID = nes::kInvalidAssetID;
};

static_assert(nes::ValidAssetType<MeshAsset>, "Mesh is not a Valid Asset Type!");