// Scene.h
#pragma once
#include "Primitives.h"

//----------------------------------------------------------------------------------------------------
/// @brief : Render information for an object in the Scene. Contains the object's model matrix, as well
///     as the mesh and material indices.
//----------------------------------------------------------------------------------------------------
struct Instance
{
    // Each represents a row in the Object matrix.
    // The last row is omitted, since it will always be [0, 0, 0, 1].
    nes::Vec4               m_matRow0 = nes::Vec4(0.0f);
    nes::Vec4               m_matRow1 = nes::Vec4(0.0f);
    nes::Vec4               m_matRow2 = nes::Vec4(0.0f);

    uint32                  m_meshIndex = helpers::kInvalidIndex;
    uint32                  m_materialIndex = helpers::kInvalidIndex;
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
    std::vector<Vertex>                 m_vertices{};       // Array of all vertices for all meshes used in the scene. 
    std::vector<uint32>                 m_indices{};        // Array of all indices for all meshes used in the scene.
    std::vector<Mesh>                   m_meshes{};         // Array of meshes that can be used by instances.
    std::vector<Instance>               m_instances{};      // Each entry is an object that is rendered in the scene.
    std::vector<PBRMaterialInstance>    m_materials{};      // Array of materials that can be used by instances. 
};