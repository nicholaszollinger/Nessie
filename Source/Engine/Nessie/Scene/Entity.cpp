// Entity.cpp

#include "Entity.h"

#include "BleachNew.h"
#include "EntityLayer.h"
#include "Scene.h"

namespace nes
{
    bool Entity::Init()
    {
        for (size_t i = 0; i < m_components.size(); ++i)
        {
            if (!m_components[i]->Init())
            {
                NES_ERRORV("Entity", "Entity failed to initialize Components!");
                return false;
            }
        }
        
        m_isInitialized = true;

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Destroy this Entity. The Entity will not immediately be destroyed, it will be queued
    ///         for destruction and destroyed by the World when ready to.
    //----------------------------------------------------------------------------------------------------
    void Entity::Destroy()
    {
        if (m_isMarkedForDestruction)
            return;

        NES_ASSERT(m_pLayer != nullptr);
        
        // Defer the Destruction until next available cleanup.ls
        // This will set the m_isMarkedForDestruction flag on the Entity.
        m_pLayer->DestroyEntity(m_handle);

        NotifyComponentsOnDestroy();
        
        // Disable immediately without Notify?
        m_isEnabled = false;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Destroy this Entity and all of its children, recursively. This destroys the child Entities
    ///             first before this one.
    //----------------------------------------------------------------------------------------------------
    void Entity::DestroyAndAllChildren()
    {
        for (auto* pChild : m_children)
        {
            pChild->DestroyAndAllChildren();
        }

        Destroy();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Set this Entity's enabled state. When disabling an Entity, all child Entities are considered
    ///             disabled, however, their individual enabled state is not affected. So when re-enabling a parent
    ///             Entity, the child Entities that were enabled will return to their enabled state automatically.
    ///		@param isEnabled : Whether to enable or disable this Entity.
    //----------------------------------------------------------------------------------------------------
    void Entity::SetEnabled(bool isEnabled)
    {
        // If the Entity is marked for Destruction, you cannot reenable it.
        // Also catch the case where the state does not change.
        if (m_isMarkedForDestruction || m_isEnabled == isEnabled)
            return;

        m_isEnabled = isEnabled;

        // We are now enabled:
        if (m_isEnabled)
        {
            NotifyComponentsOnEnabled();

            // Propagate the Enabled state to the children.
            for (auto* pChild : m_children)
            {
                // If the child is being re-enabled call OnEnabled()
                if (pChild->m_isEnabled)
                    pChild->NotifyComponentsOnEnabled();
            }
        }

        // We are now disabled:
        else
        {
            NotifyComponentsOnDisabled();

            // Propagate the disabled state to the children.
            for (auto* pChild : m_children)
            {
                if (pChild->m_isEnabled)
                    pChild->NotifyComponentsOnDisabled();
            }
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set this Entity's Parent. 
    //----------------------------------------------------------------------------------------------------
    void Entity::SetParent(Entity* pParent)
    {
        // If we have a parent, remove us as a child.
        if (m_pParent != nullptr)
        {
            m_pParent->RemoveChild(this);
        }

        m_pParent = pParent;

        if (m_pParent != nullptr)
        {
            m_pParent->m_children.push_back(this);
            m_pParent->NotifyComponentsOnChildAdded(this);
        }

        else
        {
            // Set the Parent to the Root Entity of the Layer that this Entity exists in. 
        }
        
        NotifyComponentsOnParentSet();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Add the Entity as a child of this one. 
    ///		@param pChild : 
    //----------------------------------------------------------------------------------------------------
    void Entity::AddChild(Entity* pChild)
    {
        NES_ASSERT(pChild != nullptr);

        // Ensure that we don't have this child already:
        for (const auto* pMyChild : m_children)
        {
            if (pMyChild == pChild)
            {
                NES_WARNV("Entity", "Attempted to re-add a child Entity. Parent: ", GetName().CStr(), ", Child: ", pChild->GetName().CStr());
                return;
            }
        }

        // Set this as a parent of the child.
        pChild->SetParent(this);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Remove the Child Entity from this Entity, if it exists. This sets the Entity's parent
    ///         to nullptr.
    //----------------------------------------------------------------------------------------------------
    void Entity::RemoveChild(Entity* pChild)
    {
        NES_ASSERT(pChild != nullptr);

        for (auto it = m_children.begin(); it != m_children.end(); ++it)
        {
            if ((*it) == pChild)
            {
                pChild->SetParent(nullptr); // What about Layer Root?
                m_children.erase(it);
                return;
            }
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns all Components that this Entity owns. 
    //----------------------------------------------------------------------------------------------------
    const std::vector<Component*>& Entity::GetAllComponents() const
    {
        return m_components;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the unique identifier for this Entity. 
    //----------------------------------------------------------------------------------------------------
    const EntityID& Entity::GetID() const
    {
        return m_id;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return the Entity's World Handle.
    //----------------------------------------------------------------------------------------------------
    const LayerHandle& Entity::GetHandle() const
    {
        return m_handle;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Scene that this Entity has been placed in. If null, then this Entity is not
    ///         currently in a Scene.
    //----------------------------------------------------------------------------------------------------
    Scene* Entity::GetScene() const
    {
        return m_pLayer->GetScene();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Layer that this Entity is a part of.  
    //----------------------------------------------------------------------------------------------------
    EntityLayer* Entity::GetLayer() const
    {
        return m_pLayer;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Domain that this Entity is a part of. By default, as is the same with Components,
    ///         an Entity is a part of the Abstract domain. They exist, but not physically. Subclasses can
    ///         choose to define a different domain using the macro NES_DEFINE_ENTITY_DOMAIN() within the
    ///         subclass body.
    //----------------------------------------------------------------------------------------------------
    EntityDomain Entity::GetDomain() const
    {
        return EntityDomain::Abstract;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Check whether this Entity is enabled or not. An Entity's parents must also be enabled to
    ///         consider the Entity enabled.
    //----------------------------------------------------------------------------------------------------
    bool Entity::IsEnabled() const
    {
        if (m_pParent)
        {
            return m_isEnabled && m_pParent->IsEnabled();
        }

        return m_isEnabled;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns whether this Entity is queued to be destroyed. This Entity and its Components
    ///             should be considered deleted.
    //----------------------------------------------------------------------------------------------------
    bool Entity::IsMarkedForDestruction() const
    {
        return m_isMarkedForDestruction;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns whether the Entity's Init function has been called.
    //----------------------------------------------------------------------------------------------------
    bool Entity::IsInitialized() const
    {
        return m_isInitialized;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : An Entity is invalid if it no longer belongs to an EntityLayer in a Scene.
    //----------------------------------------------------------------------------------------------------
    bool Entity::IsValid() const
    {
        // This handle will be invalid
        return m_pLayer != nullptr && m_handle.IsValid();
    }

    void Entity::NotifyComponentsOnDestroy() const
    {
        for (auto& pComponent : m_components)
        {
            pComponent->OnDestroy();
        }
    }

    void Entity::NotifyComponentsOnEnabled() const
    {
        for (const auto& component : m_components)
        {
            if (component->IsEnabled())
                component->OnEnabled();
        }
    }

    void Entity::NotifyComponentsOnDisabled() const
    {
        for (const auto& component : m_components)
        {
            if (component->IsEnabled())
                component->OnDisabled();
        }
    }

    void Entity::NotifyComponentsOnParentSet() const
    {
        for (auto* pComponent : m_components)
        {
            pComponent->OnEntityParentSet(m_pParent);
        }
    }

    void Entity::NotifyComponentsOnChildAdded(Entity* pChild)
    {
        for (auto* pComponent : m_components)
        {
            pComponent->OnEntityChildAdded(pChild);
        }
    }

    void Entity::NotifyComponentsOnChildRemoved(Entity* pChild)
    {
        for (auto* pComponent : m_components)
        {
            pComponent->OnEntityChildRemoved(pChild);
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///     @brief : Actually destroy the Entity: Destroys all components, removes from the hierarchy, and
    ///         invalidates its Handle.
    ///     @param shouldNotify : Whether parents and children should be notified of this entity being destroyed.
    //----------------------------------------------------------------------------------------------------
    void Entity::DestroyEntity(bool shouldNotify)
    {
        if (shouldNotify)
        {
            NES_ASSERT(m_isMarkedForDestruction);
            RemoveFromHierarchy();
        }

        // Remove from hierarchy without calling SetParent().
        else
        {
            m_pParent = nullptr;
            m_children.clear();
        }
        
        // Destroy all Components:
        for (auto* pComponent : m_components)
        {
            BLEACH_DELETE(pComponent);
            pComponent = nullptr;
        }

        // Invalidate Handle and Layer.
        m_handle = {};
        m_pLayer = nullptr;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Removes this Entity from the current Hierarchy. This re-parents child Entities to this
    ///             Entity's parent.
    //----------------------------------------------------------------------------------------------------
    void Entity::RemoveFromHierarchy()
    {
        for (auto* pChild : m_children)
        {
            pChild->SetParent(m_pParent);
        }

        SetParent(nullptr);
    }
}
