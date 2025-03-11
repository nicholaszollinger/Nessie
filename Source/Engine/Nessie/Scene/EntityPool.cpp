// EntityPool.cpp
#include "EntityPool.h"

namespace nes
{
    EntityPoolBase::EntityPoolBase(EntityLayer* pLayer)
        : m_pLayer(pLayer)
    {
        //
    }
    
    EntityPoolBase::EntityPoolBase(EntityLayer* pLayer, const size_t initialCapacity)
        : m_pLayer(pLayer)
    {
        m_idToHandleMap.reserve(initialCapacity);
        m_entityFreeList.reserve(initialCapacity);
        m_entitiesMarkedForDestroy.reserve(initialCapacity);
    }

    void EntityPoolBase::RegisterNewEntity(StrongPtr<Entity>& pEntity, const EntityID& entityID, const LayerHandle& handle, const StringID& name) const
    {
        pEntity->m_id = entityID;
        pEntity->m_handle = handle;
        pEntity->m_pLayer = m_pLayer;
        pEntity->m_name = name;
    }

    void EntityPoolBase::MarkForDestruction(StrongPtr<Entity>& pEntity)
    {
        if (pEntity->IsMarkedForDestruction())
            return;

        pEntity->m_isMarkedForDestruction = true;
        m_entitiesMarkedForDestroy.emplace_back(pEntity->m_handle);
    }

    void EntityPoolBase::DestroyEntity(StrongPtr<Entity>& pEntity, const bool shouldNotify)
    {
        pEntity->DestroyEntity(shouldNotify);    
        pEntity->m_handle = {};
        pEntity->m_pLayer = nullptr;
        // I might not need to overwrite the EntityID.
    }
}
