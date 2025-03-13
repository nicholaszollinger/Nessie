// SceneNodePool.h
#pragma once
#include "SceneNode.h"
#include "Core/Memory/StrongPtr.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Manages the lifetime of a type of SceneNode.   
    ///		@tparam Type : Type of SceneNode that 
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    class TSceneNodePool
    {
        static_assert(TypeIsDerivedFrom<Type, TSceneNode<Type>>, "SceneNodePool Type must inherit from SceneNode!");
        
    public:
        class Iterator
        {
            friend class TSceneNodePool;
            StrongPtr<Type>* m_pPtr = nullptr;
            StrongPtr<Type>* m_pBegin = nullptr;
            StrongPtr<Type>* m_pEnd = nullptr;
            
        private:
            Iterator(std::vector<StrongPtr<Type>>& nodeArray, size_t startIndex);

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
        std::unordered_map<NodeID, LayerHandle> m_idToHandleMap{};
        std::vector<LayerHandle>   m_nodesMarkedForDestroy{};
        std::vector<LayerHandle>   m_nodeFreeList{};
        std::vector<StrongPtr<Type>> m_nodes{}; // Ideally this is not a set of Pointers, but the types themselves.
        SceneLayer* m_pLayer = nullptr;

    public:
        explicit TSceneNodePool(SceneLayer* pLayer);
        explicit TSceneNodePool(SceneLayer* pLayer, const size_t initialCapacity);
        TSceneNodePool(const TSceneNodePool&) = delete;
        TSceneNodePool& operator=(const TSceneNodePool&) = delete;
        TSceneNodePool(TSceneNodePool&&) noexcept = default;
        TSceneNodePool& operator=(TSceneNodePool&&) noexcept = default;
        ~TSceneNodePool() = default;

        [[nodiscard]] StrongPtr<Type> CreateNode(const NodeID& id, const StringID& name);
        void QueueDestroyNode(const LayerHandle& handle);
        void ProcessDestroyedNodes();
        void ClearPool();

        [[nodiscard]] Iterator begin();
        [[nodiscard]] Iterator end();
        
        [[nodiscard]] StrongPtr<Type> GetNode(const LayerHandle& handle) const;
        [[nodiscard]] StrongPtr<Type> GetNode(const NodeID& id) const;
        [[nodiscard]] size_t          GetNodeCount() const { return m_idToHandleMap.size(); }
        [[nodiscard]] bool            IsValidNode(const LayerHandle& handle) const;
        [[nodiscard]] bool            IsValidNode(const NodeID& id) const;

    private:
        LayerHandle GetNextFreeHandle();
    };

    template <typename Type>
    TSceneNodePool<Type>::TSceneNodePool(SceneLayer* pLayer)
        : m_pLayer(pLayer) 
    {
        //
    }

    template <typename Type>
    TSceneNodePool<Type>::TSceneNodePool(SceneLayer* pLayer, const size_t initialCapacity)
        : m_pLayer(pLayer)
    {
        m_nodes.reserve(initialCapacity);
        m_nodeFreeList.reserve(initialCapacity);
        m_nodesMarkedForDestroy.reserve(initialCapacity);
        m_idToHandleMap.reserve(initialCapacity);
    }

    template <typename Type>
    StrongPtr<Type> TSceneNodePool<Type>::CreateNode(const NodeID& id, const StringID& name)
    {
        // Must be unique identifier.
        NES_ASSERT(!m_idToHandleMap.contains(id));

        LayerHandle handle = GetNextFreeHandle();
        NES_ASSERT(handle.ID() < m_nodes.size());
        auto& pNode = m_nodes[handle.ID()];

        pNode->m_id = id;
        pNode->m_name = name;
        pNode->m_pLayer = m_pLayer;
        pNode->m_layerHandle = handle;

        m_idToHandleMap.emplace(id, handle);
        
        return pNode;
    }

    template <typename Type>
    void TSceneNodePool<Type>::QueueDestroyNode(const LayerHandle& handle)
    {
        if (!IsValidNode(handle))
            return;
        
        auto& pNode = m_nodes[handle.ID()];
        if (pNode->IsMarkedForDestruction())
            return;

        pNode->m_isMarkedForDestruction = true;
        m_nodesMarkedForDestroy.push_back(handle);
    }

    template <typename Type>
    void TSceneNodePool<Type>::ProcessDestroyedNodes()
    {
        for (const auto& handle : m_nodesMarkedForDestroy)
        {
            // The Handle should be valid until this point.
            NES_ASSERT(IsValidNode(handle));

            auto& pNode = m_nodes[handle.ID()];
            const NodeID id = pNode->GetID();

            // Complete destroying the Node:
            {
                TSceneNode<Type>* pNodeBase = checked_cast<TSceneNode<Type>*>(pNode.Get());
                pNodeBase->OnFinishDestroy();
            }
            pNode.Reset();
            
            m_nodeFreeList.emplace_back(handle);
            m_idToHandleMap.erase(id);
        }

        m_nodesMarkedForDestroy.clear();
    }

    template <typename Type>
    void TSceneNodePool<Type>::ClearPool()
    {
        size_t numNodesLeft = m_idToHandleMap.size();
        
        // Destroy all Entities.
        for (size_t i = 0; i < m_nodes.size(); ++i)
        {
            // Early out if complete.
            if (numNodesLeft == 0)
                break;
            
            if (IsValidNode(m_nodes[i]->GetLayerHandle()))
            {
                TSceneNode<Type>* pNodeBase = checked_cast<TSceneNode<Type>*>(m_nodes[i].Get());
                pNodeBase->m_isMarkedForDestruction = true;
                pNodeBase->OnBeginDestroy();
                pNodeBase->OnFinishDestroy();
                m_nodes[i].Reset();

                --numNodesLeft;
            }
        }

        m_nodes.clear();
        m_idToHandleMap.clear();
        m_nodesMarkedForDestroy.clear();
        m_nodeFreeList.clear();
    }

    template <typename Type>
    typename TSceneNodePool<Type>::Iterator TSceneNodePool<Type>::begin()
    {
        return Iterator(m_nodes, 0);
    }

    template <typename Type>
    typename TSceneNodePool<Type>::Iterator TSceneNodePool<Type>::end()
    {
        return Iterator(m_nodes, m_nodes.size());
    }

    template <typename Type>
    StrongPtr<Type> TSceneNodePool<Type>::GetNode(const LayerHandle& handle) const
    {
        if (!IsValidNode(handle))
            return nullptr;

        return m_nodes[handle.ID()];
    }

    template <typename Type>
    StrongPtr<Type> TSceneNodePool<Type>::GetNode(const NodeID& id) const
    {
        if (!m_idToHandleMap.contains(id))
            return nullptr;

        return GetNode(m_idToHandleMap.at(id));
    }

    template <typename Type>
    bool TSceneNodePool<Type>::IsValidNode(const LayerHandle& handle) const
    {
        if (!handle.IsValid())
            return false;

        const size_t index = handle.ID();

        // Case where the Handle is invalid or points to an Entity that has been destroyed.
        if (index >= m_nodes.size() || m_nodes[index] == nullptr || m_nodes[index]->GetLayerHandle() != handle)
        {
            return false;
        }

        return true;
    }

    template <typename Type>
    bool TSceneNodePool<Type>::IsValidNode(const NodeID& id) const
    {
        if (!m_idToHandleMap.contains(id))
            return false;
        
        return IsValidNode(m_idToHandleMap.at(id));
    }

    template <typename Type>
    LayerHandle TSceneNodePool<Type>::GetNextFreeHandle()
    {
        LayerHandle handle;
        if (!m_nodeFreeList.empty())
        {
            handle = m_nodeFreeList.back();
            m_nodeFreeList.pop_back();
            
            handle.IncrementGeneration();
        }

        else
        {
            handle = LayerHandle(m_nodes.size());
            m_nodes.emplace_back(MakeStrong<Type>()); // Create a new, invalid, default Entity instance.
        }

        return handle;
    }

    //--------------------------------------------------------------------------------------
    // ITERATOR
    //--------------------------------------------------------------------------------------

    template <typename Type>
    TSceneNodePool<Type>::Iterator::Iterator(std::vector<StrongPtr<Type>>& nodeArray, size_t startIndex)
        : m_pPtr(nodeArray.data() + startIndex)
        , m_pBegin(nodeArray.data())
        , m_pEnd(nodeArray.data() + nodeArray.size())
    {
        // The starting index may be invalid. I need to increment until I have a valid entity.
        while (m_pPtr != m_pEnd && !m_pPtr->IsValid())
        {
            ++m_pPtr;
        }
    }

    template <typename Type>
    typename TSceneNodePool<Type>::Iterator& TSceneNodePool<Type>::Iterator::operator++()
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

    template <typename Type>
    typename TSceneNodePool<Type>::Iterator& TSceneNodePool<Type>::Iterator::operator--()
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

    template <typename Type>
    Type& TSceneNodePool<Type>::Iterator::operator*() const
    {
        NES_ASSERT(m_pPtr != nullptr && m_pPtr->IsValid());
        return *(*m_pPtr);
    }

    template <typename Type>
    StrongPtr<Type> TSceneNodePool<Type>::Iterator::operator->() const
    {
        NES_ASSERT(m_pPtr != nullptr && m_pPtr->IsValid());
        return *m_pPtr;
    }

    template <typename Type>
    bool TSceneNodePool<Type>::Iterator::operator==(const Iterator& other) const
    {
        return m_pPtr == other.m_pPtr;
    }
}
