// MeshComponentInspector.cpp
#include "MeshComponentInspector.h"
#include "Nessie/Editor/PropertyTable.h"
#include "Scene/PBRMesh.h"

namespace pbr
{
    void MeshComponentInspector::DrawImpl(MeshComponent* pTarget, const nes::InspectorContext& context)
    {
        bool modified = nes::editor::PropertyAssetID<MeshAsset>("Mesh", pTarget->m_meshID);
        modified |= nes::editor::PropertyAssetID<PBRMaterial>("Material", pTarget->m_materialID);

        auto* pRegistry = context.m_pWorld->GetEntityRegistry();
        if (modified && pRegistry)
        {
            const nes::EntityID id = context.m_selectionIDs[0];
            pRegistry->TriggerUpdate<MeshComponent>(pRegistry->GetEntity(id), pTarget->m_meshID, pTarget->m_materialID);
        }
    }
}
