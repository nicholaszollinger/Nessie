// MeshComponentInspector.cpp
#include "MeshComponentInspector.h"
#include "Nessie/Editor/PropertyTable.h"
#include "Scene/PBRMesh.h"

namespace pbr
{
    void MeshComponentInspector::DrawImpl(MeshComponent* pTarget, const nes::InspectorContext& context)
    {
        bool modified = nes::editor::PropertyAssetID<MeshAsset>("Mesh", pTarget->m_sourceMeshID);
        if (modified)
        {
            // The Mesh Asset has changed, so I have to resize the Material IDs array to fit the number of submeshes.
            auto pMeshAsset = nes::AssetManager::GetAsset<MeshAsset>(pTarget->m_sourceMeshID);
            
            // Initialize to the default materials for the mesh. 
            pTarget->m_materials = pMeshAsset->GetMaterials();
        }

        // The Mesh Asset has changed, so I have to resize the Material IDs array to fit the number of submeshes.
        auto pMeshAsset = nes::AssetManager::GetAsset<MeshAsset>(pTarget->m_sourceMeshID);
        const std::vector<nes::AssetID>& defaultMaterialIDs = pMeshAsset? pMeshAsset->GetMaterials() : std::vector<nes::AssetID>{};

        if (pTarget->m_materials.size() == 1)
        {
            modified |= nes::editor::PropertyAssetID<PBRMaterial>("Material", pTarget->m_materials[0], "Default", defaultMaterialIDs[0]);
        }
        else
        {
            modified |= nes::editor::PropertyAssetIDArray<PBRMaterial>("Materials", pTarget->m_materials, "Default", defaultMaterialIDs);   
        }

        auto* pRegistry = context.m_pWorld->GetEntityRegistry();
        if (modified && pRegistry)
        {
            const nes::EntityID id = context.m_selectionIDs[0];
            pRegistry->TriggerUpdate<MeshComponent>(pRegistry->GetEntity(id), pTarget->m_sourceMeshID, pTarget->m_materials);
        }
    }
}
