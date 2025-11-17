// TransformComponentInspector.cpp
#include "TransformComponentInspector.h"

#include "Nessie/World/WorldBase.h"
#include "Nessie/Editor/PropertyTable.h"
#include "Nessie/Graphics/ImGui/ImGuiUtils.h"

namespace nes
{
    void TransformComponentInspector::DrawImpl(TransformComponent* pTarget, const InspectorContext& context)
    {
        Vec3 position = pTarget->GetLocalPosition();
        Rotation rotation = pTarget->GetLocalRotation();
        Vec3 scale = pTarget->GetLocalScale();
        
        bool modified = editor::Property("Position", position);
        modified |= editor::Property("Rotation", rotation);
        modified |= editor::Property("Scale", scale);

        auto* pRegistry = context.m_pWorld->GetEntityRegistry();
        if (modified && pRegistry)
        {
            auto pTransformSystem = context.m_pWorld->GetSystem<TransformSystem>();
            auto entity = pRegistry->GetEntity(context.m_selectionIDs.front());
            
            NES_ASSERT(pTransformSystem, "No Transform System in World!");
            pTransformSystem->SetLocalTransform(entity, position, rotation, scale);
        }
    }
}
