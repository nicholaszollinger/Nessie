// HierarchyComponent.cpp

#include "HierarchyComponent.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the ParentID of the Entity.
    //----------------------------------------------------------------------------------------------------
    void HierarchyComponent::SetParent(const EntityID parent)
    {
        m_parentID = parent;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Reset the ParentID of the Entity.
    //----------------------------------------------------------------------------------------------------
    void HierarchyComponent::RemoveParent()
    {
        m_parentID = IDComponent::kInvalidID;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Add a Child to the Entity.
    //----------------------------------------------------------------------------------------------------
    void HierarchyComponent::AddChild(const EntityID child)
    {
        m_childrenIDs.push_back(child);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Remove a child from the Entity, if it exists.
    ///		@param child : ID of the Child to remove.
    //----------------------------------------------------------------------------------------------------
    void HierarchyComponent::RemoveChild(const EntityID child)
    {
        std::erase(m_childrenIDs, child);
    }
}