// EntityPool.h
#pragma once
#include <vector>
#include "Entity.h"
#include "Core/Memory/StrongPtr.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Base class for Entity pools. Notably, this class has full write access to the Entity base
    ///         class, which allows it to manage the Entity's Handle into the pool.  
    //----------------------------------------------------------------------------------------------------
    class EntityPoolBase
    {
    protected:
        std::unordered_map<EntityID, LayerHandle> m_idToHandleMap{};
        std::vector<LayerHandle>   m_entitiesMarkedForDestroy{};
        std::vector<LayerHandle>   m_entityFreeList{};
        EntityLayer* m_pLayer = nullptr;

        explicit EntityPoolBase(EntityLayer* pLayer);
        explicit EntityPoolBase(EntityLayer* pLayer, const size_t initialCapacity);
        
    public:
        EntityPoolBase(const EntityPoolBase&) = delete;
        EntityPoolBase& operator=(const EntityPoolBase&) = delete;
        EntityPoolBase(EntityPoolBase&&) noexcept = default;
        EntityPoolBase& operator=(EntityPoolBase&&) noexcept = default;
        ~EntityPoolBase() = default;
        
        [[nodiscard]] size_t GetEntityCount() const { return m_idToHandleMap.size(); }
    
    protected:
        void RegisterNewEntity(StrongPtr<Entity>& pEntity, const EntityID& entityID, const LayerHandle& handle, const StringID& name) const;
        void MarkForDestruction(StrongPtr<Entity>& pEntity);
        void DestroyEntity(StrongPtr<Entity>& pEntity, const bool shouldNotify);
    };
    
    template <EntityType Type>
    class EntityPool : public EntityPoolBase
    {
    public:
        class Iterator
        {
            friend class EntityPool;
            StrongPtr<Type>* m_pPtr = nullptr;
            StrongPtr<Type>* m_pBegin = nullptr;
            StrongPtr<Type>* m_pEnd = nullptr;
            
        private:
            Iterator(std::vector<StrongPtr<Type>>& entitiesArray, size_t startIndex);

        public:
            Iterator() = default;
            Iterator& operator++();
            Iterator& operator--();
            Type& operator*() const;
            StrongPtr<Type> operator->() const;

            bool operator==(const Iterator& other) const;
            bool operator!=(const Iterator& other) const { return !(*this == other); }
        };

    private:
        std::vector<StrongPtr<Type>> m_entities{};
        
    public:
        explicit EntityPool(EntityLayer* pLayer);
        explicit EntityPool(EntityLayer* pLayer, const size_t size);
        EntityPool(const EntityPool&) = delete;
        EntityPool& operator=(const EntityPool&) = delete;
        EntityPool(EntityPool&&) noexcept = default;
        EntityPool& operator=(EntityPool&&) noexcept = default;
        ~EntityPool() = default; // Consider checking that Clear Pool had been called.
        
        [[nodiscard]] StrongPtr<Type> CreateEntity(const EntityID& id, const StringID& name);
        void QueueDestroyEntity(const LayerHandle& handle);
        void ProcessDestroyedEntities();
        void ClearPool();

        [[nodiscard]] Iterator begin();
        [[nodiscard]] Iterator end();
        
        [[nodiscard]] StrongPtr<Type> GetEntity(const LayerHandle& handle) const;
        [[nodiscard]] StrongPtr<Type> GetEntity(const EntityID& id) const;
        [[nodiscard]] bool            IsValidEntity(const LayerHandle& handle) const;
        [[nodiscard]] bool            IsValidEntity(const EntityID& id) const;
    
    private:
        LayerHandle GetNextFreeHandle();
    };

    template <EntityType Type>
    EntityPool<Type>::EntityPool(EntityLayer* pLayer)
        : EntityPoolBase(pLayer)
    {
        //
    }

    template <EntityType Type>
    EntityPool<Type>::EntityPool(EntityLayer* pLayer, const size_t size)
        : EntityPoolBase(pLayer, size)
    {
        m_entities.reserve(size);
    }

    template <EntityType Type>
    StrongPtr<Type> EntityPool<Type>::CreateEntity(const EntityID& id, const StringID& name)
    {
        // Must be a unique identifier:
        NES_ASSERT(!m_idToHandleMap.contains(id));
        
        LayerHandle handle = GetNextFreeHandle();
        NES_ASSERT(handle.ID() < m_entities.size());
        auto& entity = m_entities[handle.ID()];

        auto asEntityPtr = entity.template Cast<Entity>();
        RegisterNewEntity(asEntityPtr, id, handle, name);
        
        m_idToHandleMap.emplace(id, handle);
        return entity;
    }

    template <EntityType Type>
    void EntityPool<Type>::QueueDestroyEntity(const LayerHandle& handle)
    {
        if (!IsValidEntity(handle))
            return;

        auto asEntityPtr = m_entities[handle.ID()].template Cast<Entity>();
        MarkForDestruction(asEntityPtr);
    }

    template <EntityType Type>
    void EntityPool<Type>::ProcessDestroyedEntities()
    {
        for (const auto& handle : m_entitiesMarkedForDestroy)
        {
            // The Handle should be valid until this point.
            NES_ASSERT(IsValidEntity(handle));

            auto& entity = m_entities[handle.ID()];
            const EntityID id = entity->GetID();
            
            // Destroy the Entity, notifying the hierarchy of the change.
            auto asEntityPtr = entity.template Cast<Entity>();
            DestroyEntity(asEntityPtr, true);
            entity.Reset();
            
            m_entityFreeList.emplace_back(handle);
            m_idToHandleMap.erase(id);
        }

        m_entitiesMarkedForDestroy.clear();
    }

    template <EntityType Type>
    StrongPtr<Type> EntityPool<Type>::GetEntity(const LayerHandle& handle) const
    {
        if (!IsValidEntity(handle))
            return nullptr;

        return m_entities[handle.ID()];
    }

    template <EntityType Type>
    StrongPtr<Type> EntityPool<Type>::GetEntity(const EntityID& id) const
    {
        if (!m_idToHandleMap.contains(id))
            return nullptr;

        return GetEntity(m_idToHandleMap.at(id));
    }

    template <EntityType Type>
    bool EntityPool<Type>::IsValidEntity(const LayerHandle& handle) const
    {
        if (!handle.IsValid())
            return false;

        const size_t index = handle.ID();

        // Case where the Handle is invalid or points to an Entity that has been destroyed.
        if (index >= m_entities.size() || m_entities[index] == nullptr || m_entities[index]->GetHandle() != handle)
        {
            return false;
        }

        return true;
    }

    template <EntityType Type>
    bool EntityPool<Type>::IsValidEntity(const EntityID& id) const
    {
        if (!m_idToHandleMap.contains(id))
            return false;
        
        return IsValidEntity(m_idToHandleMap.at(id));
    }

    template <EntityType Type>
    typename EntityPool<Type>::Iterator EntityPool<Type>::begin()
    {
        return Iterator(m_entities, 0);
    }

    template <EntityType Type>
    typename EntityPool<Type>::Iterator EntityPool<Type>::end()
    {
        return Iterator(m_entities, m_entities.size());
    }

    template <EntityType Type>
    void EntityPool<Type>::ClearPool()
    {
        // Destroy all Entities.
        for (size_t i = 0; i < m_entities.size(); ++i)
        {
            if (IsValidEntity(m_entities[i]->GetHandle()))
            {
                auto asEntityPtr = m_entities[i].template Cast<Entity>();
                DestroyEntity(asEntityPtr, false);
            }
        }

        m_entities.clear();
        m_idToHandleMap.clear();
        m_entitiesMarkedForDestroy.clear();
        m_entityFreeList.clear();
    }

    template <EntityType Type>
    LayerHandle EntityPool<Type>::GetNextFreeHandle()
    {
        LayerHandle handle;
        if (!m_entityFreeList.empty())
        {
            handle = m_entityFreeList.back();
            m_entityFreeList.pop_back();
            
            handle.IncrementGeneration();
        }

        else
        {
            handle = LayerHandle(m_entities.size());
            m_entities.emplace_back(MakeStrong<Type>()); // Create a new, invalid, default Entity instance.
        }

        return handle;
    }

    //--------------------------------------------------------------------------------------
    // ITERATOR
    //--------------------------------------------------------------------------------------

    template <EntityType Type>
    EntityPool<Type>::Iterator::Iterator(std::vector<StrongPtr<Type>>& entitiesArray, size_t startIndex)
        : m_pPtr(entitiesArray.data() + startIndex)
        , m_pBegin(entitiesArray.data())
        , m_pEnd(entitiesArray.data() + entitiesArray.size())
    {
        // The starting index may be invalid. I need to increment until I have a valid entity.
        while (m_pPtr != m_pEnd && !m_pPtr->IsValid())
        {
            ++m_pPtr;
        }
    }

    template <EntityType Type>
    typename EntityPool<Type>::Iterator& EntityPool<Type>::Iterator::operator++()
    {
        NES_ASSERT(m_pPtr != nullptr);
        NES_ASSERTV(m_pPtr != m_pEnd, "Cannot increment past the end of the container");

        ++m_pPtr;

        // If the Ptr is valid but the Entity is not, then
        // continue incrementing until we find a valid Entity or the end.
        while (m_pPtr != m_pEnd && !m_pPtr->IsValid())
        {
            ++m_pPtr;
        }

        return *this;
    }

    template <EntityType Type>
    typename EntityPool<Type>::Iterator& EntityPool<Type>::Iterator::operator--()
    {
        NES_ASSERT(m_pPtr != nullptr);
        NES_ASSERTV(m_pPtr != m_pBegin, "Cannot decrement past the beginning of the container");

        --m_pPtr;

        // If the Ptr is valid but the Entity is not, then
        // continue decrementing until we find a valid Entity or the beginning.
        while (m_pPtr != m_pBegin && !m_pPtr->IsValid())
        {
            --m_pPtr;
        }

        return *this;
        
    }

    template <EntityType Type>
    Type& EntityPool<Type>::Iterator::operator*() const
    {
        NES_ASSERT(m_pPtr != nullptr && m_pPtr->IsValid());
        return *(*m_pPtr);
    }

    template <EntityType Type>
    StrongPtr<Type> EntityPool<Type>::Iterator::operator->() const
    {
        NES_ASSERT(m_pPtr != nullptr && m_pPtr->IsValid());
        return *m_pPtr;
    }

    template <EntityType Type>
    bool EntityPool<Type>::Iterator::operator==(const Iterator& other) const
    {
        return m_pPtr == other.m_pPtr;
    }
}
