// SceneNode.h
#pragma once
#include <vector>
#include "Component.h"
#include "Core/Generic/GenerationalID.h"
#include "Core/String/StringID.h"

namespace nes
{
    template <typename Type>
    class TSceneNodePool;

    using NodeID = uint64_t;
    using LayerHandle = nes::GenerationalID<uint64_t>;

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : A SceneNode is an object that exists in the Scene. Different SceneNodes can be thought of
    ///         as existing in a separate domain (3D space, Screen space, etc.). This is why the class is templated,
    ///         so that there are distinct hierarchies between the Nodes. You can't parent a Node that exists in
    ///         3D space to a Node that exists in Screen space, for example.
    ///		@tparam DerivedType : This is the Type of the derived class. Ex: Actor : TSceneNode<Actor>.
    //----------------------------------------------------------------------------------------------------
    template <typename DerivedType>
    class TSceneNode
    {
        friend class Scene;
        friend class SceneLayer;        
        friend class TSceneNodePool<DerivedType>;
        friend DerivedType;
        
        NodeID m_id{};
        LayerHandle m_layerHandle{};
        SceneLayer* m_pLayer = nullptr;
        
    protected:
        using Base = TSceneNode<DerivedType>;
        
        StringID m_name{};      // Could be an Editor Only construct
        bool m_isEnabled                = true;
        bool m_isMarkedForDestruction   = false;
        bool m_isInitialized            = false;

    private:
        TSceneNode() = default;
        TSceneNode(TSceneNode&&) noexcept = default;
        TSceneNode& operator=(TSceneNode&&) noexcept = default;
        TSceneNode(const TSceneNode&) = default;
        TSceneNode& operator=(const TSceneNode&) = default;
        virtual ~TSceneNode() = default;
        
    public:
        virtual bool Init() = 0;
        void Destroy();
        void DestroyAndAllChildren();
        void SetEnabled(bool isEnabled);
        void SetName(const StringID& name) { m_name = name; }
        virtual void SetParent(DerivedType* pParent) = 0;
        
        [[nodiscard]] virtual DerivedType*               GetParent() const = 0;
        [[nodiscard]] virtual std::vector<DerivedType*>  GetChildren() const = 0;
        [[nodiscard]] virtual bool                       IsValid() const;
        [[nodiscard]] SceneLayer*                         GetLayer() const                { return m_pLayer; } 
        [[nodiscard]] LayerHandle                        GetLayerHandle() const          { return m_layerHandle; }
        [[nodiscard]] StringID                           GetName() const                 { return m_name; }
        [[nodiscard]] bool                               IsEnabled() const;
        [[nodiscard]] bool                               IsMarkedForDestruction() const  { return m_isMarkedForDestruction; }
        [[nodiscard]] bool                               IsInitialized() const           { return m_isInitialized; }
        
    protected:
        virtual void OnParentSet([[maybe_unused]] DerivedType* pParent) {}
        virtual void OnChildAdded([[maybe_unused]] DerivedType* pChild) {}
        virtual void OnBeginDestroy() {}
        virtual void OnFinishDestroy() {}
        virtual void OnEnabled() {}
        virtual void OnDisabled() {}
        void RemoveFromHierarchy();
        
    };

    template <typename DerivedType>
    void TSceneNode<DerivedType>::Destroy()
    {
        if (m_isMarkedForDestruction)
            return;

        NES_ASSERT(m_pLayer != nullptr);
        
        GetLayer()->DestroyNode(GetLayerHandle());
        m_isEnabled = false;
        OnBeginDestroy();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Destroy this Node and all of its children, recursively. This destroys the child Nodes
    ///             first before this one.
    //----------------------------------------------------------------------------------------------------
    template <typename DerivedType>
    void TSceneNode<DerivedType>::DestroyAndAllChildren()
    {
        for (auto& pChild : GetChildren())
        {
            NES_ASSERT(pChild != nullptr);
            pChild->DestroyAndAllChildren();
        }

        Destroy();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Set this Node's enabled state. When disabling a Node, all child Nodes are considered
    ///             disabled. However, their individual enabled state is not affected. So when re-enabling a parent
    ///             Node, the child Nodes that were enabled will return to their enabled state automatically.
    ///		@param isEnabled : Whether to enable or disable this Entity.
    //----------------------------------------------------------------------------------------------------
    template <typename DerivedType>
    void TSceneNode<DerivedType>::SetEnabled(bool isEnabled)
    {
        // If the Node is marked for Destruction, you cannot reenable it.
        // Also catch the case where the state does not change.
        if (m_isMarkedForDestruction || m_isEnabled == isEnabled)
            return;

        m_isEnabled = isEnabled;
        
        if (m_isEnabled)
        {
            OnEnabled();

            auto children = GetChildren();
            for (auto& pChild : children)
            {
                // The Child is being "re-enabled" because its parent (this node)
                // is now enabled
                if (pChild->m_isEnabled)
                    pChild->OnEnabled();
            }
        }

        else
        {
            OnDisabled();

            auto children = GetChildren();
            for (auto& pChild : children)
            {
                // The Child is being disabled because its parent (this node)
                // is now disabled
                if (pChild->m_isEnabled)
                    pChild->OnDisabled();
            }
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Check whether this Node is enabled or not. An Node's parent must also be enabled to
    ///         consider the Node enabled.
    //----------------------------------------------------------------------------------------------------
    template <typename DerivedType>
    bool TSceneNode<DerivedType>::IsEnabled() const
    {
        if (auto* pParent = GetParent())
        {
            return m_isEnabled && pParent->IsEnabled();
        }

        return m_isEnabled;
    }

    template <typename DerivedType>
    void TSceneNode<DerivedType>::RemoveFromHierarchy()
    {
        auto children = GetChildren();
        for (auto* pChild : children)
        {
            pChild->SetParent(GetParent());            
        }

        SetParent(nullptr);
    }

    template <typename DerivedType>
    bool TSceneNode<DerivedType>::IsValid() const
    {
        return m_pLayer != nullptr && m_layerHandle.IsValid();
    }

    template <typename Type>
    concept ValidNodeType = TypeIsDerivedFrom<Type, TSceneNode<Type>>;
}