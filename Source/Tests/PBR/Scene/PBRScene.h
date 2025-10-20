// PBRScene.h
#pragma once
#include "PBRLights.h"
#include "PBRMaterial.h"
#include "PBRMesh.h"
#include "Nessie/World.h"

namespace pbr
{
    namespace helpers
    {
        static constexpr uint32 kInvalidSceneIndex = std::numeric_limits<uint32>::max();
    }

    enum class EDefaultMeshType : uint8
    {
        Cube,
        Plane,
        Sphere,
    };
    
    struct alignas (64) CameraUBO
    {
        nes::Mat44  m_view              = nes::Mat44::Identity();
        nes::Mat44  m_projection        = nes::Mat44::Identity();
        nes::Mat44  m_viewProjection    = nes::Mat44::Identity();   // Cached proj * view.
        nes::Float3 m_position          = nes::Float3::Zero();
        float       m_exposureFactor    = 0.000125f;
    };

    struct EntityInstance
    {
        nes::Mat44          m_model = nes::Mat44::Identity();
        nes::EntityHandle   m_entity = nes::kInvalidEntityHandle;
        uint32              m_meshIndex = helpers::kInvalidSceneIndex;
        uint32              m_materialIndex = helpers::kInvalidSceneIndex;
    };

    //----------------------------------------------------------------------------------------------------
    // [TODO]: Is this a Scene Proxy Component? I can add this to an Entity...
    /// @brief : Render information for an object in the Scene. Contains the object's model matrix, as well
    ///     as the mesh and material indices.
    //----------------------------------------------------------------------------------------------------
    struct alignas (64) InstanceUBO
    {
        nes::Mat44  m_model             = nes::Mat44::Identity();       // Converts vertex positions to world space.
        nes::Mat44  m_normal            = nes::Mat44::Identity();       // Converts vertex normals/tangents to world space.
        uint32      m_meshIndex         = helpers::kInvalidSceneIndex;  // Index into the Scene's Mesh buffer.
        uint32      m_materialIndex     = helpers::kInvalidSceneIndex;  // Index into the Scene's MaterialUBO buffer.

        InstanceUBO&  SetTransform(const nes::Vec3 translation, const nes::Quat rotation, const nes::Vec3 scale = { 1.f, 1.f, 1.f });
        InstanceUBO&  SetTransform(const nes::Mat44& transform);
        InstanceUBO&  SetMesh(const uint32 meshIndex);
        InstanceUBO&  SetMaterial(const uint32 materialIndex);
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief :  Contains the render information for a scene - all Textures, Vertices, Indices, Meshes, etc.
    //----------------------------------------------------------------------------------------------------
    struct Scene
    {
        std::vector<Vertex>                 m_vertices{};           // Array of all vertices for all meshes used in the scene. 
        std::vector<uint32>                 m_indices{};            // Array of all indices for all meshes used in the scene.
        std::vector<Mesh>                   m_meshes{};             // Array of meshes that can be used by instances.
        std::vector<EntityInstance>         m_instances{};          // Each entry is an entity with geometry that needs to be rendered.
        std::vector<MaterialUBO>            m_materials{};          // Each element contains information to render and instance.
        std::vector<PointLight>             m_pointLights{};        // Array of point lights for the scene.
        std::vector<DirectionalLight>       m_directionalLights{};  // Array of directional lights for the scene.
        std::vector<nes::Descriptor>        m_textures{};
        nes::AssetID                        m_skyboxTextureID = nes::kInvalidAssetID;
        
        using AssetIDToIndexMap = std::unordered_map<nes::AssetID, uint32, nes::UUIDHasher>;
        using EntityHandleToIndexMap = std::unordered_map<nes::EntityHandle, uint32>;
        
        EntityHandleToIndexMap              m_entityToInstanceMap{};
        AssetIDToIndexMap                   m_idToTextureIndex{};
        AssetIDToIndexMap                   m_idToMaterialIndex{};
        AssetIDToIndexMap                   m_idToMeshIndex{};
    };
}


