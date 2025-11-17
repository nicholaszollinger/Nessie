// ComponentInspector.cpp
#include "ComponentInspector.h"
#include "Nessie/Editor/PropertyTable.h"

namespace nes
{
    void IDComponentInspector::DrawImpl(IDComponent* pComponent, const InspectorContext&)
    {
        editor::Property("Name", pComponent->GetName(), "Name of the Entity");
        editor::Property("EntityID", pComponent->GetID(), "Unique Identifier for the Entity");
    }

    void NodeComponentInspector::DrawImpl(NodeComponent* pComponent, const InspectorContext& context)
    {
        if (auto* pRegistry = context.m_pWorld->GetEntityRegistry())
        {
            const EntityID parentID = pComponent->m_parentID;
            editor::PropertyEntityID("Parent", parentID, *pRegistry);

            const auto& childrenIDs = pComponent->m_childrenIDs;
            editor::PropertyArray("Children", childrenIDs, m_currentSelectedChild);    
        }
    }
}