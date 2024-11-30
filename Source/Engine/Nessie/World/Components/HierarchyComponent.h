#pragma once
// HierarchyComponent.h
#include <vector>
#include "IDComponent.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : This component is used to store the hierarchy information of an entity.
    //----------------------------------------------------------------------------------------------------
    class HierarchyComponent
    {
        EntityID m_parentID = IDComponent::kInvalidID;
        std::vector<EntityID> m_childrenIDs;

    public:
        void SetParent(const EntityID parent);
        void RemoveParent();
        void AddChild(const EntityID child);
        void RemoveChild(const EntityID child);

        [[nodiscard]] EntityID GetParent() const { return m_parentID; }
        [[nodiscard]] const std::vector<EntityID>& GetChildren() const { return m_childrenIDs; }
    };

    // [TODO]: Serializer:
}