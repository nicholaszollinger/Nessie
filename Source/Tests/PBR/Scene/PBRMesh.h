// PBRMesh.h
#pragma once
#include "PBRMaterial.h"
#include "Nessie/Graphics/GraphicsCommon.h"

namespace pbr
{
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

    namespace helpers
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Parameters for generating Sphere geometry. 
        //----------------------------------------------------------------------------------------------------
        struct SphereGenDesc
        {
            float m_radius = 0.5f;          // Radius of the
            float m_latitudeBands = 30.f;
            float m_longitudeBands = 30.f;
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Parameters for generating Plane geometry. 
        //----------------------------------------------------------------------------------------------------
        struct PlaneGenDesc
        {
            uint32  m_subdivisionsX = 10;   // The number of subdivisions in the horizontal plane.
            uint32  m_subdivisionsZ = 10;   // The number of subdivisions in the forward plane.
            float   m_width = 10.f;         // Width of the plane, in meters.
            float   m_height = 10.f;        // Height of the plane, in meters.
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates the tangents and bitangents for a set of vertices.
        //----------------------------------------------------------------------------------------------------
        void CalculateTangentSpace(std::vector<Vertex>& outVertices, const std::vector<uint32>& indices, const Mesh& mesh);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Generate a cube's vertices, indices, and associated mesh data for the scene.
        //----------------------------------------------------------------------------------------------------
        void AppendCubeMeshData(std::vector<Vertex>& outVertices, std::vector<uint32>& outIndices, Mesh& outMesh);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Generate a sphere's vertices, indices, and associated mesh data for the scene.
        //----------------------------------------------------------------------------------------------------
        void AppendSphereMeshData(const SphereGenDesc& sphereDesc, std::vector<Vertex>& outVertices, std::vector<uint32>& outIndices, Mesh& outMesh);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Generate a plane's vertices, indices, and associated mesh data for the scene.
        //----------------------------------------------------------------------------------------------------
        void AppendPlaneData(const PlaneGenDesc& planeDesc, std::vector<Vertex>& outVertices, std::vector<uint32>& outIndices, Mesh& outMesh);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Mesh Asset loaded from disk.
    //----------------------------------------------------------------------------------------------------
    class MeshAsset final : public nes::AssetBase
    {
        NES_DEFINE_TYPE_INFO(MeshAsset)

    public:
        MeshAsset() = default;
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a Mesh Asset from data. Must be manually added to Asset Manager. 
        //----------------------------------------------------------------------------------------------------
        MeshAsset(const std::vector<Vertex>& vertices, const std::vector<uint32>& indices, const nes::AssetID defaultMaterialID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a Mesh Asset from data. Must be manually added to Asset Manager. 
        //----------------------------------------------------------------------------------------------------
        MeshAsset(Vertex* pVertices, const uint32 vertexCount, uint32* indices, const uint32 indexCount, const nes::AssetID defaultMaterialID);
    
        const std::vector<Vertex>&  GetVertices() const  { return m_vertices; }
        const std::vector<uint32>&  GetIndices() const   { return m_indices; }
        nes::AssetID                GetDefaultMaterialID() const  { return m_defaultMaterialID; }

    private:
        virtual nes::ELoadResult    LoadFromFile(const std::filesystem::path& path) override;
        nes::ELoadResult            LoadFromYAML(const nes::YamlNode& node);

    private:
        std::vector<Vertex>         m_vertices{};
        std::vector<uint32>         m_indices{};
        nes::AssetID                m_defaultMaterialID = nes::kInvalidAssetID;
    };

    static_assert(nes::ValidAssetType<MeshAsset>, "Mesh is not a Valid Asset Type!");
}