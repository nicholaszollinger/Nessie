// Scene.h
#pragma once
#include "Primitives.h"
#include "LightTypes.h"

//----------------------------------------------------------------------------------------------------
/// @brief : Parameters for a given Material.
//----------------------------------------------------------------------------------------------------
struct alignas(64) MaterialUBO
{
    nes::Float3                 m_baseColorScale            = nes::Float3(1.f);
    float                       m_metallicScale             = 1.f;
    nes::Float3                 m_emissionScale             = nes::Float3(1.f);
    float                       m_roughnessScale            = 1.f; 
    float                       m_normalScale               = 1.f;
    
    uint32                      m_baseColorIndex            = static_cast<uint32>(EDefaultTextures::White);
    uint32                      m_normalIndex               = static_cast<uint32>(EDefaultTextures::FlatNormal);
    uint32                      m_roughnessMetallicIndex    = static_cast<uint32>(EDefaultTextures::White);
    uint32                      m_emissionIndex             = static_cast<uint32>(EDefaultTextures::White);
};

//----------------------------------------------------------------------------------------------------
/// @brief : Render information for an object in the Scene. Contains the object's model matrix, as well
///     as the mesh and material indices.
//----------------------------------------------------------------------------------------------------
struct alignas (64) ObjectUBO
{
    nes::Mat44              m_model             = nes::Mat44::Identity();   // Converts vertex positions to world space.
    nes::Mat44              m_normal            = nes::Mat44::Identity();   // Converts vertex normals/tangents to world space.
    uint32                  m_meshIndex         = helpers::kInvalidIndex;   // Index into the Scene's Mesh buffer.
    uint32                  m_materialIndex     = helpers::kInvalidIndex;   // Index into the Scene's MaterialUBO buffer.

    ObjectUBO&              SetTransform(const nes::Vec3 translation, const nes::Quat rotation, const nes::Vec3 scale = { 1.f, 1.f, 1.f });
    ObjectUBO&              SetTransform(const nes::Mat44& transform);
    ObjectUBO&              SetMesh(const uint32 meshIndex);
    ObjectUBO&              SetMaterial(const uint32 materialIndex);
};

//----------------------------------------------------------------------------------------------------
//	NOTES:
// [TODO]: Built when loading the world; A Scene is pure render data built during runtime. A world is
//  all the assets, entities and components that exist in the space.
//		
/// @brief : Contains the render information for a scene - all Textures, Vertices, Indices, Meshes,
/// etc.
//----------------------------------------------------------------------------------------------------
struct Scene
{
    std::vector<Vertex>                 m_vertices{};           // Array of all vertices for all meshes used in the scene. 
    std::vector<uint32>                 m_indices{};            // Array of all indices for all meshes used in the scene.
    std::vector<Mesh>                   m_meshes{};             // Array of meshes that can be used by instances.
    std::vector<ObjectUBO>              m_objects{};            // Each entry is an object that is rendered in the scene.
    std::vector<MaterialUBO>            m_materials{};          // Each element contains information to render and instance.
    std::vector<PointLight>             m_pointLights{};        // Array of point light info for the scene.
    std::vector<DirectionalLight>       m_directionalLights{};  // Array of directional light info for the scene.
    std::vector<nes::Descriptor>        m_textures{};       
    
    using AssetIDToIndexMap = std::unordered_map<nes::AssetID, uint32, nes::UUIDHasher>;
    AssetIDToIndexMap                   m_idToTextureIndex{};
    AssetIDToIndexMap                   m_idToMaterialIndex{};
};

namespace helpers
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : HACK. I need to know specific AssetIDs when loading the scene. 
    //----------------------------------------------------------------------------------------------------
    struct SceneConfig
    {
        nes::AssetID m_gridShaderID = nes::kInvalidAssetID;
        nes::AssetID m_skyboxShaderID = nes::kInvalidAssetID;
        nes::AssetID m_skyboxTextureID = nes::kInvalidAssetID;
        nes::AssetID m_pbrShaderID = nes::kInvalidAssetID;
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : HACK. A Scene will not be loaded from data, the world will be. But for now, I am going to load the scene.
    //----------------------------------------------------------------------------------------------------
    bool LoadScene(const std::filesystem::path& assetPath, nes::RenderDevice& device, Scene& outScene, std::vector<nes::AssetID>& outLoadedAssets, SceneConfig& outConfig);
}