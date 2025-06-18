// Entity.h
#pragma once
#include <vector>
#include "Component.h"
#include "Core/Generic/GenerationalID.h"
#include "Core/Memory/StrongPtr.h"
#include "Core/String/StringID.h"

namespace nes
{
    template <typename Type>
    class TEntityPool;

    using EntityID = uint64_t;
    using LayerHandle = nes::GenerationalID<uint64_t>;

    NES_DEFINE_LOG_TAG(kEntityLogTag, "Entity", Warn);

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : An Entity is an object that exists in the Scene. Different Entity types can be thought of
    ///         as existing in a separate domain (3D space, Screen space, etc.). This is why the class is templated,
    ///         so that there are distinct hierarchies between the Entities. You can't parent an Entity that exists in
    ///         3D space to an Entity that exists in Screen space, for example.
    ///		@tparam DerivedType : This is the Type of the derived class. Ex: Actor : TEntity<Actor>.
    //----------------------------------------------------------------------------------------------------
    template <typename DerivedType>
    class TEntity
    {
        friend class Scene;
        friend class EntityLayer;
        friend class TEntityPool<DerivedType>;
        friend DerivedType;

    public:
        using EntityDerivedType = DerivedType;
        using BaseComponentType = TComponent<DerivedType>;
        
    private:
        std::vector<StrongPtr<BaseComponentType>>   m_components{};
        std::vector<DerivedType*>                   m_children{};
        DerivedType*                                m_pParent = nullptr;
        EntityID                                    m_id{};
        LayerHandle                                 m_layerHandle{};
        EntityLayer*                                m_pLayer = nullptr;
        
    public:
        using BaseType = TEntity<DerivedType>;

    protected:
        StringID m_name{};      // Could be an Editor Only construct
        bool m_isEnabled                = true;
        bool m_isMarkedForDestruction   = false;
        bool m_isInitialized            = false;

    private:
        TEntity() = default;
        TEntity(TEntity&&) noexcept = default;
        TEntity& operator=(TEntity&&) noexcept = default;
        TEntity(const TEntity&) = default;
        TEntity& operator=(const TEntity&) = default;
        virtual ~TEntity() = default;
        
    public:
        void Destroy();
        void DestroyAndAllChildren();
        void SetEnabled(bool isEnabled);
        void SetName(const StringID& name) { m_name = name; }
        void SetParent(DerivedType* pParent);
        void AddChild(DerivedType* pChild);
        void RemoveChild(DerivedType* pChild);

        template <typename Type> requires TypeIsDerivedFrom<Type, TComponent<DerivedType>>
        StrongPtr<Type> AddComponent(const StringID& name);

        template <typename Type> requires TypeIsDerivedFrom<Type, TComponent<DerivedType>>
        StrongPtr<Type> GetComponent();

        [[nodiscard]] const std::vector<StrongPtr<BaseComponentType>>&  GetComponents() const { return m_components; }
        [[nodiscard]] std::vector<StrongPtr<BaseComponentType>>&  GetComponents()       { return m_components; }
        [[nodiscard]] DerivedType*                      GetParent() const               { return m_pParent; }
        [[nodiscard]] const std::vector<DerivedType*>&  GetChildren() const             { return m_children; }
        [[nodiscard]] EntityLayer*                      GetLayer() const                { return m_pLayer; }
        [[nodiscard]] LayerHandle                       GetLayerHandle() const          { return m_layerHandle; }
        [[nodiscard]] StringID                          GetName() const                 { return m_name; }
        [[nodiscard]] EntityID                          GetID() const                   { return m_id; }
        [[nodiscard]] bool                              IsValid() const;
        [[nodiscard]] bool                              IsEnabled() const;
        [[nodiscard]] bool                              IsMarkedForDestruction() const  { return m_isMarkedForDestruction; }
        [[nodiscard]] bool                              IsInitialized() const           { return m_isInitialized; }
        
    protected:
        virtual bool Init() = 0;
        virtual void OnParentSet([[maybe_unused]] DerivedType* pParent) {}
        virtual void OnChildAdded([[maybe_unused]] DerivedType* pChild) {}
        virtual void OnChildRemoved([[maybe_unused]] DerivedType* pChild) {}
        virtual void OnQueueDestroy();
        virtual void OnFinishDestroy();
        virtual void OnEnabled() {}
        virtual void OnDisabled() {}
        void RemoveFromHierarchy();

        void NotifyComponentsOnDestroy();
        void NotifyComponentsOnEnabled();
        void NotifyComponentsOnDisabled();
        void NotifyComponentsOnParentSet(DerivedType* pParent);
        void NotifyComponentsOnChildAdded(DerivedType* pChild);
        void NotifyComponentsOnChildRemoved(DerivedType* pChild);
    };

    template <typename Type>
    concept ValidEntityType = TypeIsDerivedFrom<Type, TEntity<Type>>;
    
    template <typename DerivedType>
    void TEntity<DerivedType>::Destroy()
    {
        if (m_isMarkedForDestruction)
            return;

        NES_ASSERT(m_pLayer != nullptr);
        
        GetLayer()->QueueDestroyEntity(m_layerHandle);
        m_isEnabled = false;
        OnQueueDestroy();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Destroy this Entity and all of its children, recursively. This destroys the child Entities
    ///             first before this one.
    //----------------------------------------------------------------------------------------------------
    template <typename DerivedType>
    void TEntity<DerivedType>::DestroyAndAllChildren()
    {
        for (auto& pChild : m_children)
        {
            NES_ASSERT(pChild != nullptr);
            pChild->DestroyAndAllChildren();
        }

        Destroy();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Set this Entity's enabled state. When disabling a Entity, all child Entities are considered
    ///             disabled. However, their individual enabled state is not affected. So when re-enabling a parent
    ///             Entity, the child Entities that were enabled will return to their enabled state automatically.
    ///		@param isEnabled : Whether to enable or disable this Entity.
    //----------------------------------------------------------------------------------------------------
    template <typename DerivedType>
    void TEntity<DerivedType>::SetEnabled(bool isEnabled)
    {
        // If the Entity is marked for Destruction, you cannot reenable it.
        // Also catch the case where the state does not change.
        if (m_isMarkedForDestruction || m_isEnabled == isEnabled)
            return;

        m_isEnabled = isEnabled;
        
        if (m_isEnabled)
        {
            OnEnabled();
            
            for (auto& pChild : m_children)
            {
                // The Child is being "re-enabled" because its parent (this entity)
                // is now enabled
                if (pChild->m_isEnabled)
                    pChild->OnEnabled();
            }
        }

        else
        {
            OnDisabled();

            auto children = GetChildren();
            for (auto& pChild : m_children)
            {
                // The Child is being disabled because its parent (this entity)
                // is now disabled
                if (pChild->m_isEnabled)
                    pChild->OnDisabled();
            }
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set this Entity's parent.
    //----------------------------------------------------------------------------------------------------
    template <typename DerivedType>
    void TEntity<DerivedType>::SetParent(DerivedType* pParent)
    {
        if (m_pParent == pParent)
            return;

        DerivedType* pThis = static_cast<DerivedType*>(this);
        
        if (m_pParent != nullptr)
        {
            m_pParent->RemoveChild(pThis);
        }

        m_pParent = pParent;

        // If we now have a parent, add us as a child.
        if (m_pParent != nullptr)
        {
            m_pParent->m_children.emplace_back(pThis);
            m_pParent->OnChildAdded(pThis);
        }

        OnParentSet(m_pParent);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      If successful, this sets the Child's Parent to this, and calls
    //      this->OnParentChanged() and pChild->OnChildAdded(pChild).
    //		
    ///		@brief : Attach a Entity to this.
    //----------------------------------------------------------------------------------------------------
    template <typename DerivedType>
    void TEntity<DerivedType>::AddChild(DerivedType* pChild)
    {
        NES_ASSERT(pChild != nullptr);
        
        // Don't re-add a child
        for (auto* pCurrentChild : m_children)
        {
            if (pCurrentChild == pChild)
            {
                NES_WARN(kEntityLogTag, "Attempted to re-add Child that already exists!");
                return;
            }
        }

        DerivedType* pThis = static_cast<DerivedType*>(this);

        pChild->m_pParent = pThis;
        m_children.emplace_back(pChild);

        BaseType* pChildAsEntity = static_cast<BaseType*>(pChild);
        pChildAsEntity->OnParentSet(pThis);
        OnChildAdded(pChild);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      If successful, this sets the Child's Parent to this, and calls
    //      pChild->OnParentSet() and this->OnChildRemoved(pChild).
    //         
    ///		@brief : Remove a Child from this Entity. 
    //----------------------------------------------------------------------------------------------------
    template <typename DerivedType>
    void TEntity<DerivedType>::RemoveChild(DerivedType* pChild)
    {
        NES_ASSERT(pChild != nullptr);
        
        for (size_t i = 0; i < m_children.size(); i++)
        {
            if (m_children[i] == pChild)
            {
                std::swap(m_children[i], m_children.back());
                m_children.pop_back();
                pChild->m_pParent = nullptr;

                BaseType* pChildAsEntity = static_cast<BaseType*>(pChild);
                pChildAsEntity->OnParentSet(nullptr);
                OnChildRemoved(pChild);
                return;
            }
        }

        NES_WARN(kEntityLogTag, "Attempted to remove Child that did not exist!");
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Add a Component to this Entity.
    ///		@tparam Type : Type of Component to Add.
    ///     @param name : User-defined name of the Component to identify it.
    ///		@returns : Pointer to the created Component.
    //----------------------------------------------------------------------------------------------------
    template <typename DerivedType>
    template <typename Type> requires TypeIsDerivedFrom<Type, TComponent<DerivedType>>
    StrongPtr<Type> TEntity<DerivedType>::AddComponent(const StringID& name)
    {
        StrongPtr<Type> pComponent = Create<Type>();

        BaseComponentType* pBaseComponent = static_cast<BaseComponentType*>(pComponent.Get());  
        pBaseComponent->m_pOwner = static_cast<DerivedType*>(this);
        pBaseComponent->m_name = name;

        // If the Entity has already been initialized, then run through the component's initialization.
        if (m_isInitialized)
        {
            if (!pBaseComponent->Init())
            {
                pComponent.Reset();
                return nullptr;
            }
        }

        m_components.emplace_back(pComponent);
        return pComponent;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the first component of the given type. If none are found, returns nullptr. 
    ///		@tparam Type : Type of Component you are looking for.
    //----------------------------------------------------------------------------------------------------
    template <typename DerivedType>
    template <typename Type> requires TypeIsDerivedFrom<Type, TComponent<DerivedType>>
    StrongPtr<Type> TEntity<DerivedType>::GetComponent()
    {
        for (auto& pComponent : m_components)
        {
            if (pComponent->GetTypeID() == Type::GetStaticTypeID())
            {
                return pComponent;
            }
        }
        
        return nullptr;
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A Entity is valid if the Entity contains a valid Layer, LayerHandle, and RootComponent. 
    //----------------------------------------------------------------------------------------------------
    template <typename DerivedType>
    bool TEntity<DerivedType>::IsValid() const
    {
        // [TODO]: This is a bit questionable. I should be checking that we are valid in the layer, not
        // just that the handle is valid.
        return m_pLayer != nullptr && m_layerHandle.IsValid();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Check whether this Entity is enabled or not. A Entity's parent must also be enabled to
    ///         consider the Entity enabled.
    //----------------------------------------------------------------------------------------------------
    template <typename DerivedType>
    bool TEntity<DerivedType>::IsEnabled() const
    {
        if (m_pParent)
        {
            return m_isEnabled && m_pParent->IsEnabled();
        }

        return m_isEnabled;
    }

    template <typename DerivedType>
    void TEntity<DerivedType>::OnQueueDestroy()
    {
        // Disable all Components, and notify that we are going to be destroyed:
        for (auto& pComponent : m_components)
        {
            pComponent->SetEnabled(false);
            pComponent->OnDestroy();
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Removes this Entity from the current hierarchy. This will leave the Entity in an
    ///         invalid state. Child nodes will be re-parented to this Entity's parent. 
    //----------------------------------------------------------------------------------------------------
    template <typename DerivedType>
    void TEntity<DerivedType>::RemoveFromHierarchy()
    {
        for (auto* pChild : m_children)
        {
            pChild->SetParent(m_pParent);            
        }

        SetParent(nullptr);
    }

    template <typename DerivedType>
    void TEntity<DerivedType>::NotifyComponentsOnDestroy()
    {
        for (auto& pComponent : m_components)
        {
            pComponent->OnDestroy();
        }
    }

    template <typename DerivedType>
    void TEntity<DerivedType>::NotifyComponentsOnEnabled()
    {
        for (auto& pComponent : m_components)
        {
            pComponent->OnEnabled();
        }
    }

    template <typename DerivedType>
    void TEntity<DerivedType>::NotifyComponentsOnDisabled()
    {
        for (auto& pComponent : m_components)
        {
            pComponent->OnDisabled();
        }
    }

    template <typename DerivedType>
    void TEntity<DerivedType>::NotifyComponentsOnParentSet(DerivedType* pParent)
    {
        for (auto& pComponent : m_components)
        {
            pComponent->OnOwnerParentSet(pParent);
        }
    }

    template <typename DerivedType>
    void TEntity<DerivedType>::NotifyComponentsOnChildAdded(DerivedType* pChild)
    {
        for (auto& pComponent : m_components)
        {
            pComponent->OnOwnerChildAdded(pChild);
        }
    }

    template <typename DerivedType>
    void TEntity<DerivedType>::NotifyComponentsOnChildRemoved(DerivedType* pChild)
    {
        for (auto& pComponent : m_components)
        {
            pComponent->OnOwnerChildRemoved(pChild);
        }
    }

    
}