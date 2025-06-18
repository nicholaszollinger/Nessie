// Entity3D.cpp
#include "Entity3D.h"
#include "Core/Memory/Memory.h"
#include "Components/Entity3DComponent.h"
#include "Physics/Components/ShapeComponent.h"
#include "Scene/EntityLayer.h"
#include "World/World.h"

namespace nes
{
    Scene* Entity3D::GetScene() const
    {
        return GetLayer()->GetScene();
    }

    bool Entity3D::Init()
    {
        std::vector<StrongPtr<ShapeComponent>> physicsShapes{};
        physicsShapes.reserve(m_components.size());
        
        for (auto& pComponent : m_components)
        {
            if (!pComponent->Init())
            {
                NES_ERROR(kEntityLogTag, "Failed to initialize Entity! Failed to initialize component!");
                return false;
            }
        }

        if (!physicsShapes.empty())
        {
            // If there is more than 1 Physics Shape, then the Body needs to build a CompoundShape.
            if (physicsShapes.size() > 1)
            {
                // [TODO]: 
            }
        }
        
        m_isInitialized = true;
        
        return true;
    }
    
    void Entity3D::Rotate(const float angle, const Vec3& axis)
    {
        m_rotation = Quat::FromAxisAngle(axis, angle).ToEulerAngles();
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }
    
    void Entity3D::Rotate(const Rotation& rotation)
    {
        m_rotation += rotation;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }
    
    void Entity3D::Translate(const Vec3& translation)
    {
        m_location += translation;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }
    
    void Entity3D::Scale(const float uniformScale)
    {
        m_scale *= uniformScale;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }
    
    void Entity3D::Scale(const Vec3& scale)
    {
        m_scale *= scale;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }
    
    void Entity3D::SetLocalLocation(const Vec3& location)
    {
        m_location = location;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }
    
    void Entity3D::SetLocalRotation(const Rotation& rotation)
    {
        m_rotation = rotation;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }
    
    void Entity3D::SetLocalRotation(const Vec3& eulerAngles)
    {
        m_rotation = Rotation(eulerAngles);
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }
    
    void Entity3D::SetLocalScale(const Vec3& scale)
    {
        m_scale = scale;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }

    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Set this Entity's Transform relative to its parent.
    // //----------------------------------------------------------------------------------------------------
    // void Entity3D::SetLocalTransform(const Transform& transform)
    // {
    //     // [TODO]: 
    //     //m_localTransform = transform;
    //     UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    // }

    void Entity3D::SetLocalTransform(const Vec3& location, const Rotation& rotation, const Vec3& scale)
    {
        m_location = location;
        m_rotation = rotation;
        m_scale = scale;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }
    
    void Entity3D::SetWorldLocation(const Vec3& location)
    {
        // Get our current parent location:
        Vec3 parentLocation = Vec3::Zero();
        if (m_pParent)
        {
            // Update the Parent, if necessary:
            if (m_pParent->WorldTransformNeedsUpdate())
            {
                m_pParent->UpdateWorldTransform(m_pParent->m_pParent, m_pParent->LocalTransformMatrix());
            }
            parentLocation = m_pParent->GetLocation();
        }

        // Update our local position based on the new location.
        m_location = location - parentLocation;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }
    
    void Entity3D::SetWorldRotation(const Rotation& rotation)
    {
        // Get our current parent orientation:
        Rotation parentRotation{}; // Zero.
        if (m_pParent)
        {
            // Update the Parent, if necessary:
            if (m_pParent->WorldTransformNeedsUpdate())
            {
                m_pParent->UpdateWorldTransform(m_pParent->m_pParent, m_pParent->LocalTransformMatrix());
            }
            parentRotation = m_pParent->GetRotation();
        }
        
        m_rotation = rotation - parentRotation;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }
    
    void Entity3D::SetWorldScale(const Vec3& scale)
    {
        Vec3 parentScale = Vec3::One();
        if (m_pParent)
        {
            // Update the Parent, if necessary:
            if (m_pParent->WorldTransformNeedsUpdate())
            {
                m_pParent->UpdateWorldTransform(m_pParent->m_pParent, m_pParent->LocalTransformMatrix());
            }
            parentScale = m_pParent->GetScale();
        }

        m_scale = scale / parentScale;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }
    
    void Entity3D::SetWorldTransform(const Mat44& transform)
    {
        m_worldTransformNeedsUpdate = true;
        
        Mat44 parentTransform = Mat44::Identity();
        if (m_pParent)
        {
            // Update the Parent, if necessary:
            if (m_pParent->WorldTransformNeedsUpdate())
            {
                m_pParent->UpdateWorldTransform(m_pParent->m_pParent, m_pParent->LocalTransformMatrix());
            }
            parentTransform = m_pParent->GetWorldTransformMatrix();
        }

        Vec3 parentTranslation;
        Vec3 parentScale;
        Rotation parentRotation;
        parentTransform.Decompose(parentTranslation, parentRotation, parentScale);

        Vec3 translation;
        Vec3 scale;
        Rotation rotation;
        transform.Decompose(translation, rotation, scale);
        
        // Convert to local space:
        m_location = translation - parentTranslation;
        m_rotation = rotation - parentRotation;
        m_scale = scale / parentScale;

        m_worldTransformNeedsUpdate = false;
        m_worldTransformMatrix = transform;
        m_onWorldTransformUpdated.Broadcast();
        PropagateTransformUpdateToChildren();
    }
    
    void Entity3D::SetWorldTransform(const Vec3& worldLocation, const Rotation& worldRotation, const Vec3& worldScale)
    {
        m_worldTransformNeedsUpdate = true;
        
        // Calculate the Local Transform:
        Mat44 parentTransform = Mat44::Identity();
        if (m_pParent)
        {
            // Update the Parent, if necessary:
            if (m_pParent->WorldTransformNeedsUpdate())
            {
                m_pParent->UpdateWorldTransform(m_pParent->m_pParent, m_pParent->LocalTransformMatrix());
            }
            parentTransform = m_pParent->GetWorldTransformMatrix();
        }

        Vec3 parentTranslation;
        Vec3 parentScale;
        Rotation parentRotation;
        parentTransform.Decompose(parentTranslation, parentRotation, parentScale);
        
        // Convert to local space:
        m_location = worldLocation - parentTranslation;
        m_rotation = (worldRotation - parentRotation).Normalized();
        m_scale = worldScale / parentScale;

        // Compose our world matrix:
        m_worldTransformMatrix = Mat44::ComposeTransform(worldLocation, worldRotation, worldScale);
        
        m_worldTransformNeedsUpdate = false;
        m_onWorldTransformUpdated.Broadcast();
        PropagateTransformUpdateToChildren();
    }
    
    Vec3 Entity3D::GetLocation() const
    {
        return m_worldTransformMatrix.GetTranslation();
    }
    
    Rotation Entity3D::GetRotation() const
    {
        return Rotation(m_worldTransformMatrix.GetRotation().ToQuaternion().ToEulerAngles() * math::RadiansToDegrees());
    }
    
    Vec3 Entity3D::GetScale() const
    {
        return m_worldTransformMatrix.GetScale();
    }
    
    const Vec3& Entity3D::GetLocalLocation() const
    {
        return m_location;
    }
    
    const Rotation& Entity3D::GetLocalRotation() const
    {
        return m_rotation;
    }
    
    const Vec3& Entity3D::GetLocalScale() const
    {
        return m_scale;
    }
    
    Mat44 Entity3D::LocalTransformMatrix() const
    {
        return Mat44::ComposeTransform(m_location, m_rotation, m_scale);
    }
    
    const Mat44& Entity3D::GetWorldTransformMatrix() const
    {
        return m_worldTransformMatrix;
    }

    void Entity3D::OnParentSet(Entity3D* pParent)
    {
        // Don't bother updating transforms if the Entity is being destroyed.
        if (IsMarkedForDestruction())
            return;

        MarkWorldTransformDirty();
        UpdateWorldTransform(pParent, LocalTransformMatrix());
    }
    
    void Entity3D::MarkWorldTransformDirty()
    {
        m_worldTransformNeedsUpdate = true;
    }
    
    void Entity3D::UpdateWorldTransform(Entity3D* pParent, const Mat44& localTransform)
    {
        // If we no longer have a parent, then our local transform translates this
        // component's local transformation to world space.
        if (pParent == nullptr)
        {
            m_worldTransformMatrix = localTransform;
        }

        else
        {
            // If our parent is not updated, we need to walk up the chain before updating the worldTransforms
            // before computing ours.
            if (pParent->WorldTransformNeedsUpdate())
            {
                pParent->UpdateWorldTransform(pParent->m_pParent, pParent->LocalTransformMatrix());
            }
            
            m_worldTransformMatrix = localTransform * pParent->GetWorldTransformMatrix();
        }

        // Our transform is now updated.
        m_worldTransformNeedsUpdate = false;
        m_onWorldTransformUpdated.Broadcast();
        PropagateTransformUpdateToChildren();
    }
    
    void Entity3D::PropagateTransformUpdateToChildren()
    {
        for (auto* pChild : m_children)
        {
            pChild->UpdateWorldTransform(this, pChild->LocalTransformMatrix());
        }
    }
    
    bool Entity3D::WorldTransformNeedsUpdate() const
    {
        return m_worldTransformNeedsUpdate;
    }
    
    World* Entity3D::GetWorld() const
    {
        return checked_cast<World*>(GetLayer());
    }

    void Entity3D::RebuildPhysicsBody()
    {
        // World* pWorld = GetWorld();
        // //PhysicsSystem* pPhysicsSystem = pWorld->GetPhysicsSystem();
        //
        // // Collect Physics Shapes attached to this Body.
        // std::vector<StrongPtr<ShapeComponent>> physicsShapes{};
        // physicsShapes.reserve(m_components.size());
        //
        // for (auto& pComponent : m_components)
        // {
        //     if (auto pShape = Cast<ShapeComponent>(pComponent))
        //     {
        //         physicsShapes.push_back(pShape);
        //     }
        // }
        //
        // if (!physicsShapes.empty())
        // {
        //     // If there is more than 1 Physics Shape, then the Body needs to build a CompoundShape.
        //     if (physicsShapes.size() > 1)
        //     {
        //         
        //     }
        // }
        
    }

}
