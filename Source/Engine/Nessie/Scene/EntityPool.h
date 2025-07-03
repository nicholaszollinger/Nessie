// SceneNodePool.h
#pragma once
#include "Entity.h"
#include "Nessie/Core/Memory/StrongPtr.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Manages the lifetime of a type of Entities, within the context of an EntityLayer.   
    ///		@tparam Type : Type of Entity that this pool creates. 
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    class TEntityPool
    {
        static_assert(TypeIsDerivedFrom<Type, TEntity<Type>>, "TEntityPool Type must inherit from TEntity!");
        using EntityBaseType = typename Type::BaseType;
        
    public:
        class Iterator
        {
            friend class TEntityPool;
            StrongPtr<Type>* m_pPtr = nullptr;
            StrongPtr<Type>* m_pBegin = nullptr;
            StrongPtr<Type>* m_pEnd = nullptr;
            
        private:
            Iterator(std::vector<StrongPtr<Type>>& entityArray, size_t startIndex);

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
        std::unordered_map<EntityID, LayerHandle>   m_idToHandleMap{};
        std::vector<LayerHandle>                    m_entitiesMarkedForDestroy{};
        std::vector<LayerHandle>                    m_handleFreeList{};
        std::vector<StrongPtr<Type>>                m_entities{}; // Ideally this is not a set of Pointers, but the types themselves.
        EntityLayer*                                 m_pLayer = nullptr;

    public:
        explicit TEntityPool(EntityLayer* pLayer);
        explicit TEntityPool(EntityLayer* pLayer, const size_t initialCapacity);
        TEntityPool(const TEntityPool&) = delete;
        TEntityPool& operator=(const TEntityPool&) = delete;
        TEntityPool(TEntityPool&&) noexcept = default;
        TEntityPool& operator=(TEntityPool&&) noexcept = default;
        ~TEntityPool() = default;

        [[nodiscard]] StrongPtr<Type> CreateEntity(const EntityID& id, const StringID& name);
        void QueueDestroyEntity(const LayerHandle& handle);
        void ProcessDestroyedEntities();
        void ClearPool();

        [[nodiscard]] Iterator begin();
        [[nodiscard]] Iterator end();
        
        [[nodiscard]] StrongPtr<Type> GetEntity(const LayerHandle& handle) const;
        [[nodiscard]] StrongPtr<Type> GetEntity(const EntityID& id) const;
        [[nodiscard]] size_t          GetEntityCount() const { return m_idToHandleMap.size(); }
        [[nodiscard]] bool            IsValidEntity(const LayerHandle& handle) const;
        [[nodiscard]] bool            IsValidEntity(const EntityID& id) const;

    private:
        LayerHandle GetNextFreeHandle();
    };

    template <typename Type>
    TEntityPool<Type>::TEntityPool(EntityLayer* pLayer)
        : m_pLayer(pLayer) 
    {
        //
    }

    template <typename Type>
    TEntityPool<Type>::TEntityPool(EntityLayer* pLayer, const size_t initialCapacity)
        : m_pLayer(pLayer)
    {
        m_entities.reserve(initialCapacity);
        m_handleFreeList.reserve(initialCapacity);
        m_entitiesMarkedForDestroy.reserve(initialCapacity);
        m_idToHandleMap.reserve(initialCapacity);
    }

    template <typename Type>
    StrongPtr<Type> TEntityPool<Type>::CreateEntity(const EntityID& id, const StringID& name)
    {
        // Must be unique identifier.
        NES_ASSERT(!m_idToHandleMap.contains(id));

        LayerHandle handle = GetNextFreeHandle();
        NES_ASSERT(handle.GetValue() < m_entities.size());
        auto& pEntity = m_entities[handle.GetValue()];

        pEntity->m_id = id;
        pEntity->m_name = name;
        pEntity->m_pLayer = m_pLayer;
        pEntity->m_layerHandle = handle;

        m_idToHandleMap.emplace(id, handle);
        
        return pEntity;
    }

    template <typename Type>
    void TEntityPool<Type>::QueueDestroyEntity(const LayerHandle& handle)
    {
        if (!IsValidEntity(handle))
            return;
        
        auto& pEntity = m_entities[handle.GetValue()];
        if (pEntity->IsMarkedForDestruction())
            return;

        pEntity->m_isMarkedForDestruction = true;
        m_entitiesMarkedForDestroy.push_back(handle);
    }

    template <typename Type>
    void TEntityPool<Type>::ProcessDestroyedEntities()
    {
        for (const auto& handle : m_entitiesMarkedForDestroy)
        {
            // The Handle should be valid until this point.
            NES_ASSERT(IsValidEntity(handle));

            auto& pEntity = m_entities[handle.GetValue()];
            const EntityID id = pEntity->GetID();

            // Complete destroying the Node:
            {
                EntityBaseType* pEntityBase = checked_cast<EntityBaseType*>(pEntity.Get());
                pEntityBase->OnFinishDestroy();
            }
            pEntity.Reset();
            
            m_handleFreeList.emplace_back(handle);
            m_idToHandleMap.erase(id);
        }

        m_entitiesMarkedForDestroy.clear();
    }

    template <typename Type>
    void TEntityPool<Type>::ClearPool()
    {
        // Process any currently queued entities to Destroy:
        ProcessDestroyedEntities();
        
        size_t numEntitiesLeft = m_idToHandleMap.size();
        
        // Destroy all remaining Entities.
        for (size_t i = 0; i < m_entities.size(); ++i)
        {
            // Early out if complete.
            if (numEntitiesLeft == 0)
                break;
            
            if (IsValidEntity(m_entities[i]->GetLayerHandle()))
            {
                EntityBaseType* pEntityBase = checked_cast<EntityBaseType*>(m_entities[i].Get());
                pEntityBase->m_isMarkedForDestruction = true;
                pEntityBase->OnQueueDestroy();
                pEntityBase->OnFinishDestroy();
                m_entities[i].Reset();

                --numEntitiesLeft;
            }
        }

        m_entities.clear();
        m_idToHandleMap.clear();
        m_entitiesMarkedForDestroy.clear();
        m_handleFreeList.clear();
    }

    template <typename Type>
    typename TEntityPool<Type>::Iterator TEntityPool<Type>::begin()
    {
        return Iterator(m_entities, 0);
    }

    template <typename Type>
    typename TEntityPool<Type>::Iterator TEntityPool<Type>::end()
    {
        return Iterator(m_entities, m_entities.size());
    }

    template <typename Type>
    StrongPtr<Type> TEntityPool<Type>::GetEntity(const LayerHandle& handle) const
    {
        if (!IsValidEntity(handle))
            return nullptr;

        return m_entities[handle.GetValue()];
    }

    template <typename Type>
    StrongPtr<Type> TEntityPool<Type>::GetEntity(const EntityID& id) const
    {
        if (!m_idToHandleMap.contains(id))
            return nullptr;

        return GetEntity(m_idToHandleMap.at(id));
    }

    template <typename Type>
    bool TEntityPool<Type>::IsValidEntity(const LayerHandle& handle) const
    {
        if (!handle.IsValid())
            return false;

        const size_t index = handle.GetValue();

        // Case where the Handle is invalid or points to an Entity that has been destroyed.
        if (index >= m_entities.size() || m_entities[index] == nullptr || m_entities[index]->GetLayerHandle() != handle)
        {
            return false;
        }

        return true;
    }

    template <typename Type>
    bool TEntityPool<Type>::IsValidEntity(const EntityID& id) const
    {
        if (!m_idToHandleMap.contains(id))
            return false;
        
        return IsValidEntity(m_idToHandleMap.at(id));
    }

    template <typename Type>
    LayerHandle TEntityPool<Type>::GetNextFreeHandle()
    {
        LayerHandle handle;
        if (!m_handleFreeList.empty())
        {
            handle = m_handleFreeList.back();
            m_handleFreeList.pop_back();
            
            handle.IncrementGeneration();
        }

        else
        {
            handle = LayerHandle(m_entities.size());
            m_entities.emplace_back(Create<Type>()); // Create a new, invalid, default Entity instance.
        }

        return handle;
    }

    //--------------------------------------------------------------------------------------
    // ITERATOR
    //--------------------------------------------------------------------------------------

    template <typename Type>
    TEntityPool<Type>::Iterator::Iterator(std::vector<StrongPtr<Type>>& entityArray, size_t startIndex)
        : m_pPtr(entityArray.data() + startIndex)
        , m_pBegin(entityArray.data())
        , m_pEnd(entityArray.data() + entityArray.size())
    {
        // The starting index may be invalid. I need to increment until I have a valid entity.
        while (m_pPtr != m_pEnd && m_pPtr->Get() == nullptr)
        {
            ++m_pPtr;
        }
    }

    template <typename Type>
    typename TEntityPool<Type>::Iterator& TEntityPool<Type>::Iterator::operator++()
    {
        NES_ASSERT(m_pPtr != nullptr);
        NES_ASSERT(m_pPtr != m_pEnd, "Cannot increment past the end of the container");

        ++m_pPtr;

        // If the Ptr is valid but the Entity is not, then
        // continue incrementing until we find a valid Entity or the end.
        while (m_pPtr != m_pEnd && m_pPtr->Get() == nullptr)
        {
            ++m_pPtr;
        }

        return *this;
    }

    template <typename Type>
    typename TEntityPool<Type>::Iterator& TEntityPool<Type>::Iterator::operator--()
    {
        NES_ASSERT(m_pPtr != nullptr);
        NES_ASSERT(m_pPtr != m_pBegin, "Cannot decrement past the beginning of the container");

        --m_pPtr;

        // If the Ptr is valid but the Entity is not, then
        // continue decrementing until we find a valid Entity or the beginning.
        while (m_pPtr != m_pBegin && m_pPtr->Get() == nullptr)
        {
            --m_pPtr;
        }

        return *this;
    }

    template <typename Type>
    Type& TEntityPool<Type>::Iterator::operator*() const
    {
        NES_ASSERT(m_pPtr != nullptr && m_pPtr->Get() != nullptr);
        return *(*m_pPtr);
    }

    template <typename Type>
    StrongPtr<Type> TEntityPool<Type>::Iterator::operator->() const
    {
        NES_ASSERT(m_pPtr != nullptr && m_pPtr->Get() != nullptr);
        return *m_pPtr;
    }

    template <typename Type>
    bool TEntityPool<Type>::Iterator::operator==(const Iterator& other) const
    {
        return m_pPtr == other.m_pPtr;
    }
}
