// Actor.cpp
#include "Actor.h"
#include "BleachNew.h"
#include "Scene/SceneLayer.h"

namespace nes
{
    Scene* Actor::GetScene() const
    {
        return GetLayer()->GetScene();
    }

    bool Actor::Init()
    {
        for (auto& pComponent : m_components)
        {
            if (!pComponent->Init())
            {
                NES_ERRORV("Actor", "Failed to initialize Actor! Failed to initialize component!");
                return false;
            }
        }

        m_isInitialized = true;
        
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Set an Actor as a Parent  
    ///		@param pParent : 
    //----------------------------------------------------------------------------------------------------
    void Actor::SetParent(Actor* pParent)
    {
        NES_ASSERT(m_pRootComponent != nullptr);

        StrongPtr<WorldComponent> pOtherRoot = nullptr;
        if (pParent != nullptr)
        {
            pOtherRoot = pParent->GetRootComponent();
            NES_ASSERT(pOtherRoot != nullptr);
        }

        m_pRootComponent->SetParent(pOtherRoot.Get());
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Get this Actor's immediate children. This checks all attached components to our
    ///             RootComponent, and 
    //----------------------------------------------------------------------------------------------------
    std::vector<Actor*> Actor::GetChildren() const
    {
        std::vector<Actor*> children;
        NES_ASSERT(m_pRootComponent != nullptr);

        const auto& componentChildren = m_pRootComponent->GetChildren();
        children.reserve(m_pRootComponent->GetChildren().size());
        
        // [TODO]: This is only check our first layer of Components for Child Actors.
        // Really, this should be searching through the components until there are no children
        // or the Actor is not the same.
        for (const auto& pComponent : componentChildren)
        {
            Actor* pChildActor = pComponent->GetOwner();
            if (pChildActor != this)
            {
                children.emplace_back(pChildActor);
            }
        }

        return children;
    }

    bool Actor::IsValid() const
    {
        return Base::IsValid() && m_pRootComponent != nullptr;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Translate this Actor's world Location by a delta amount. 
    //----------------------------------------------------------------------------------------------------
    void Actor::AddTranslation(const Vector3& deltaLocation)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->Translate(deltaLocation);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Rotate this Actor's world orientation by a delta amount. 
    //----------------------------------------------------------------------------------------------------
    void Actor::AddRotation(const Quat& deltaRotation)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->Rotate(deltaRotation);
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Scale each of this Actor's world scale vector by a delta amount. 
    //----------------------------------------------------------------------------------------------------
    void Actor::AddScale(const Vector3& deltaScale)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->Scale(deltaScale);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Scale this Actor's world scale vector by a uniform amount. 
    //----------------------------------------------------------------------------------------------------
    void Actor::AddScale(const float deltaUniformScale)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->Scale(deltaUniformScale);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Actor's world Transform.  
    //----------------------------------------------------------------------------------------------------
    void Actor::SetTransform(const Mat4& transform)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetWorldTransform(transform);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Actor's world location.  
    //----------------------------------------------------------------------------------------------------
    void Actor::SetLocation(const Vector3& location)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetWorldLocation(location);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Actor's world orientation.  
    //----------------------------------------------------------------------------------------------------
    void Actor::SetOrientation(const Quat& orientation)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetWorldOrientation(orientation);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Actor's world scale.  
    //----------------------------------------------------------------------------------------------------
    void Actor::SetScale(const Vector3& scale)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetWorldScale(scale);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Actor's transform, relative to its parent.  
    //----------------------------------------------------------------------------------------------------
    void Actor::SetLocalTransform(const Transform& localTransform)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetLocalTransform(localTransform);
    }

    void Actor::SetLocalTransform(const Vector3& location, const Quat& orientation, const Vector3& scale)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetLocalTransform(location, orientation, scale);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Actor's location, relative to its parent.  
    //----------------------------------------------------------------------------------------------------
    void Actor::SetLocalLocation(const Vector3& localLocation)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetLocalLocation(localLocation);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Actor's orientation, relative to its parent.  
    //----------------------------------------------------------------------------------------------------
    void Actor::SetLocalOrientation(const Quat& localOrientation)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetLocalOrientation(localOrientation);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Actor's scale, relative to its parent.  
    //----------------------------------------------------------------------------------------------------
    void Actor::SetLocalScale(const Vector3& localScale)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetLocalScale(localScale);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's world transformation matrix.  
    //----------------------------------------------------------------------------------------------------
    Mat4 Actor::GetTransformMatrix() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetWorldTransformMatrix();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's transformation matrix, relative to its parent.  
    //----------------------------------------------------------------------------------------------------
    Mat4 Actor::GetLocalTransformMatrix() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetLocalTransformMatrix();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's location, in world space. 
    //----------------------------------------------------------------------------------------------------
    Vector3 Actor::GetLocation() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetLocation();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's scale, in world space. 
    //----------------------------------------------------------------------------------------------------
    Vector3 Actor::GetScale() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetScale();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's orientation, in world space. 
    //----------------------------------------------------------------------------------------------------
    Quat Actor::GetOrientation() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetOrientation();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's location, relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    Vector3 Actor::GetLocalLocation() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetLocalLocation();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's scale, relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    Vector3 Actor::GetLocalScale() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetLocalScale();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's orientation, relative to is parent. 
    //----------------------------------------------------------------------------------------------------
    Quat Actor::GetLocalOrientation() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetLocalOrientation();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Root Component of this Actor. The Actor's transform will be equal to this
    ///         Component. 
    //----------------------------------------------------------------------------------------------------
    void Actor::SetRootComponent(StrongPtr<WorldComponent>& pRoot)
    {
        NES_ASSERT(pRoot);
        
        if (m_pRootComponent == pRoot)
            return;

        if (m_pRootComponent->GetOwner() != this)
        {
            NES_WARNV("Actor", "Attempted to set RootComponent of Actor to a Component that is not owned by that Actor!");
            return;
        }

        // If we have a Root Component already, re-parent our root.
        if (m_pRootComponent != nullptr)
            m_pRootComponent->SetParent(pRoot.Get());
        
        m_pRootComponent = pRoot;
    }

    Actor* Actor::GetParent() const
    {
        Actor* pParent = nullptr;
        NES_ASSERT(m_pRootComponent != nullptr);
        
        if (auto* pParentComponent = m_pRootComponent->GetParent())
        {
            pParent = pParentComponent->GetOwner();
        }

        return pParent;
    }

    void Actor::OnParentSet(Actor* pParent)
    {
        NotifyComponentsOnParentSet(pParent);
    }

    void Actor::OnChildAdded(Actor* pChild)
    {
        NotifyComponentsOnChildAdded(pChild);
    }

    void Actor::OnEnabled()
    {
        NotifyComponentsOnEnabled();
    }

    void Actor::OnDisabled()
    {
        NotifyComponentsOnDisabled();
    }

    void Actor::OnBeginDestroy()
    {
        NotifyComponentsOnDestroy();
    }

    void Actor::OnFinishDestroy()
    {
        NES_ASSERT(m_isMarkedForDestruction);

        if (!GetLayer()->IsBeingDestroyed())
        {
            RemoveFromHierarchy();
        }
        
        for (auto& pComponent : m_components)
        {
            // Remove the ownership, making the Component invalid.
            pComponent->m_pOwner = nullptr;
            pComponent.Reset();
        }
    }

    void Actor::NotifyComponentsOnDestroy()
    {
        for (auto& pComponent : m_components)
        {
            pComponent->OnDestroy();
        }
    }

    void Actor::NotifyComponentsOnEnabled()
    {
        for (auto& pComponent : m_components)
        {
            pComponent->OnEnabled();
        }
    }

    void Actor::NotifyComponentsOnDisabled()
    {
        for (auto& pComponent : m_components)
        {
            pComponent->OnDisabled();
        }
    }

    void Actor::NotifyComponentsOnParentSet(Actor* pParent)
    {
        for (auto& pComponent : m_components)
        {
            pComponent->OnEntityParentSet(pParent);
        }
    }

    void Actor::NotifyComponentsOnChildAdded(Actor* pChild)
    {
        for (auto& pComponent : m_components)
        {
            pComponent->OnEntityChildAdded(pChild);
        }
    }

    void Actor::NotifyComponentsOnChildRemoved(Actor* pChild)
    {
        for (auto& pComponent : m_components)
        {
            pComponent->OnEntityChildRemoved(pChild);
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Finalize adding the Component, ensuring to initialize if necessary and add to the Component
    ///         set. If initialization fails, the component will be deleted.
    //----------------------------------------------------------------------------------------------------
    bool Actor::FinishAddComponent(StrongPtr<ActorComponent>&& pComponent)
    {
        // If the Actor has already been initialized, run through the initialization
        // of the Component.
        if (IsInitialized())
        {
            if (!pComponent->Init())
            {
                NES_ERRORV("Actor", "Failed to Add Component! Type: ", pComponent->GetTypename());
                pComponent.Reset();
                return false;
            }
        }

        // Add to our component set:
        m_components.emplace_back(std::move(pComponent));
        return true;
    }

}
