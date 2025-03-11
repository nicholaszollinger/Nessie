// EntityLayer.cpp
#include "EntityLayer.h"

namespace nes
{
    EntityLayer::EntityLayer(Scene* pScene)
        : m_pScene(pScene)
    {
        //
    }
    
    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Returns whether the Entity with the Handle exists on this Layer.
    // //----------------------------------------------------------------------------------------------------
    // bool EntityLayer::IsValidEntity(const LayerHandle& handle) const
    // {
    //     if (!handle.IsValid())
    //         return false;
    //
    //     const size_t index = handle.ID();
    //     if (index >= m_entities.size() || m_entities[index] == nullptr || m_entities[index]->m_handle != handle)
    //     {
    //         return false;
    //     }
    //
    //     return true;
    // }

    // //----------------------------------------------------------------------------------------------------
    // //		NOTES:
    // //		
    // ///		@brief : Destroy an Entity on this Layer. Note: This will not immediately delete the Entity
    // ///             , it will defer destruction to the next update.
    // ///		@param handle : EntityHandle of the Entity to destroy.
    // //----------------------------------------------------------------------------------------------------
    // void EntityLayer::DestroyEntity(const LayerHandle& handle)
    // {
    //     // If the ID is invalid, return.
    //     if (!IsValidEntity(handle))
    //         return;
    //
    //     // The ID will still be valid until the Entity is actually destroyed.
    //     auto pEntity = m_entities[handle.ID()];
    //
    //     // If this Entity is already marked for Destruction, return.
    //     if (pEntity->m_isMarkedForDestruction)
    //         return;
    //     
    //     pEntity->m_isMarkedForDestruction = true;
    //     m_entitiesMarkedForDestroy.emplace_back(handle);
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Register a newly created Entity to this Layer. This sets the Entity's ID, Name,
    // ///         Handle and Layer reference.
    // //----------------------------------------------------------------------------------------------------
    // void EntityLayer::RegisterEntityToLayer(StrongPtr<Entity>& entity, const EntityID id, const StringID& name)
    // {
    //     // This must be a new Entity:
    //     NES_ASSERT(!m_idToHandleMap.contains(id));
    //     
    //     const auto handle = GetNextFreeHandle();
    //     NES_ASSERT(handle.ID() < m_entities.size());
    //     m_entities[handle.ID()] = entity;
    //     m_idToHandleMap.emplace(id, handle);
    //     
    //     entity->m_handle = handle;
    //     entity->m_id = id;
    //     entity->m_name = name;
    //     entity->m_pLayer = this;
    // }
    //
    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Destroy all Entities that have been queued for destruction. 
    // //----------------------------------------------------------------------------------------------------
    // void EntityLayer::ProcessDestroyedEntities()
    // {
    //     for (const auto& handle : m_entitiesMarkedForDestroy)
    //     {
    //         // [TODO]: Should I double-check that the Entity is marked for destroy?
    //         
    //         // The ID should remain valid until this point.
    //         NES_ASSERT(IsValidEntity(handle));
    //         
    //         // Destroy the Entity:
    //         const size_t index = handle.ID();
    //         m_entities[index]->DestroyEntity(true);
    //         m_entities[index].Reset();
    //
    //         // Add to the free list.
    //         m_entityFreeList.emplace_back(handle);
    //     }
    //
    //     m_entitiesMarkedForDestroy.clear();
    // }

    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Returns the next available Handle for a new Entity. This may cause the Entity array to
    // ///         grow.
    // //----------------------------------------------------------------------------------------------------
    // LayerHandle EntityLayer::GetNextFreeHandle()
    // {
    //     LayerHandle handle{};
    //
    //     if (!m_entityFreeList.empty())
    //     {
    //         handle = m_entityFreeList.back();
    //         m_entityFreeList.pop_back();
    //
    //         handle.IncrementGeneration();
    //     }
    //
    //     else
    //     {
    //         handle = LayerHandle(m_entities.size());
    //         m_entities.emplace_back();
    //     }
    //     
    //     return handle;
    // }
}
