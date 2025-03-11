// Actor.cpp
#include "Actor.h"

namespace nes
{
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
    void Actor::SetActorTransform(const Mat4& transform)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetWorldTransform(transform);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Actor's world location.  
    //----------------------------------------------------------------------------------------------------
    void Actor::SetActorLocation(const Vector3& location)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetWorldLocation(location);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Actor's world orientation.  
    //----------------------------------------------------------------------------------------------------
    void Actor::SetActorOrientation(const Quat& orientation)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetWorldOrientation(orientation);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Actor's world scale.  
    //----------------------------------------------------------------------------------------------------
    void Actor::SetActorScale(const Vector3& scale)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetWorldScale(scale);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Actor's transform, relative to its parent.  
    //----------------------------------------------------------------------------------------------------
    void Actor::SetActorLocalTransform(const Transform& localTransform)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetLocalTransform(localTransform);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Actor's location, relative to its parent.  
    //----------------------------------------------------------------------------------------------------
    void Actor::SetActorLocalLocation(const Vector3& localLocation)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetLocalLocation(localLocation);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Actor's orientation, relative to its parent.  
    //----------------------------------------------------------------------------------------------------
    void Actor::SetActorLocalOrientation(const Quat& localOrientation)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetLocalOrientation(localOrientation);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Actor's scale, relative to its parent.  
    //----------------------------------------------------------------------------------------------------
    void Actor::SetActorLocalScale(const Vector3& localScale)
    {
        NES_ASSERT(m_pRootComponent);
        m_pRootComponent->SetLocalScale(localScale);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's world transformation matrix.  
    //----------------------------------------------------------------------------------------------------
    Mat4 Actor::GetActorTransformMatrix() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetWorldTransformMatrix();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's transformation matrix, relative to its parent.  
    //----------------------------------------------------------------------------------------------------
    Mat4 Actor::GetActorLocalTransformMatrix() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetLocalTransformMatrix();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's location, in world space. 
    //----------------------------------------------------------------------------------------------------
    Vector3 Actor::GetActorLocation() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetLocation();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's scale, in world space. 
    //----------------------------------------------------------------------------------------------------
    Vector3 Actor::GetActorScale() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetScale();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's orientation, in world space. 
    //----------------------------------------------------------------------------------------------------
    Quat Actor::GetActorOrientation() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetOrientation();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's location, relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    Vector3 Actor::GetActorLocalLocation() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetLocalLocation();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's scale, relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    Vector3 Actor::GetActorLocalScale() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetLocalScale();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Actor's orientation, relative to is parent. 
    //----------------------------------------------------------------------------------------------------
    Quat Actor::GetActorLocalOrientation() const
    {
        NES_ASSERT(m_pRootComponent);
        return m_pRootComponent->GetLocalOrientation();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Root Component of this Actor. The Actor's transform will be equal to this
    ///         Component. 
    //----------------------------------------------------------------------------------------------------
    void Actor::SetRootComponent(WorldComponent* pRoot)
    {
        NES_ASSERT(pRoot);
        
        if (m_pRootComponent == pRoot)
            return;

        // If we have a Root Component already, re-parent our root.
        if (m_pRootComponent != nullptr)
            m_pRootComponent->SetParent(pRoot);
        
        m_pRootComponent = pRoot;
    }
}
