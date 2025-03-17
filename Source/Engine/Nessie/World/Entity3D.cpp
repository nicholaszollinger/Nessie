// Entity3D.cpp
#include "Entity3D.h"
#include "BleachNew.h"
#include "Components/Entity3DComponent.h"
#include "Math/VectorConversions.h"
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
        for (auto& pComponent : m_components)
        {
            if (!pComponent->Init())
            {
                NES_ERRORV("Entity3D", "Failed to initialize Entity! Failed to initialize component!");
                return false;
            }
        }

        m_isInitialized = true;
        
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Rotate this Entity by a delta angle around an axis. The angle must be in radians.
    //----------------------------------------------------------------------------------------------------
    void Entity3D::Rotate(const float angle, const Vector3& axis)
    {
        m_rotation = Quat::MakeFromAngleAxis(angle, axis).EulerAngles();
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Rotate this Entity by a delta rotation.
    //----------------------------------------------------------------------------------------------------
    void Entity3D::Rotate(const Rotation& rotation)
    {
        m_rotation += rotation;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Move this Entity's local location based on the translation. 
    //----------------------------------------------------------------------------------------------------
    void Entity3D::Translate(const Vector3& translation)
    {
        m_location += translation;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Increase the local scale by a delta scalar amount. This multiplies the current scale of each
    ///         axis by the scalar values of deltaScale.
    //----------------------------------------------------------------------------------------------------
    void Entity3D::Scale(const float uniformScale)
    {
        m_scale *= uniformScale;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Increase the local scale by an amount on each axis. This multiplies the current local
    ///             scale by the scale vector.
    //----------------------------------------------------------------------------------------------------
    void Entity3D::Scale(const Vector3& scale)
    {
        m_scale *= scale;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Sets this Entity's location relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    void Entity3D::SetLocalLocation(const Vector3& location)
    {
        m_location = location;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set this Entity's orientation relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    void Entity3D::SetLocalRotation(const Rotation& rotation)
    {
        m_rotation = rotation;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set this Entity's local orientation, from euler angles. The euler angles are expected
    ///             to be in degrees.
    //----------------------------------------------------------------------------------------------------
    void Entity3D::SetLocalRotation(const Vector3& eulerAngles)
    {
        m_rotation = Rotation(eulerAngles);
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set this Entity's scale relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    void Entity3D::SetLocalScale(const Vector3& scale)
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

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Entity's location, orientation and scale relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    void Entity3D::SetLocalTransform(const Vector3& location, const Rotation& rotation, const Vector3& scale)
    {
        m_location = location;
        m_rotation = rotation;
        m_scale = scale;
        UpdateWorldTransform(m_pParent, LocalTransformMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Entity's location, in world space. 
    //----------------------------------------------------------------------------------------------------
    void Entity3D::SetWorldLocation(const Vector3& location)
    {
        // Get our current parent location:
        Vector3 parentLocation = Vector3::GetZeroVector();
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

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Entity's orientation, in world space. 
    //----------------------------------------------------------------------------------------------------
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

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Entity's scale, in world space. 
    //----------------------------------------------------------------------------------------------------
    void Entity3D::SetWorldScale(const Vector3& scale)
    {
        Vector3 parentScale = Vector3::GetUnitVector();
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

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Entity3Ds transform, in world space. 
    //----------------------------------------------------------------------------------------------------
    void Entity3D::SetWorldTransform(const Mat4& transform)
    {
        m_worldTransformNeedsUpdate = true;
        
        Mat4 parentTransform = Mat4::Identity();
        if (m_pParent)
        {
            // Update the Parent, if necessary:
            if (m_pParent->WorldTransformNeedsUpdate())
            {
                m_pParent->UpdateWorldTransform(m_pParent->m_pParent, m_pParent->LocalTransformMatrix());
            }
            parentTransform = m_pParent->GetWorldTransformMatrix();
        }

        Vector3 parentTranslation;
        Vector3 parentScale;
        Rotation parentRotation;
        math::DecomposeMatrix(parentTransform, parentTranslation, parentRotation, parentScale);

        Vector3 translation;
        Vector3 scale;
        Rotation rotation;
        math::DecomposeMatrix(transform, translation, rotation, scale);
        
        // Convert to local space:
        m_location = translation - parentTranslation;
        m_rotation = rotation - parentRotation;
        m_scale = scale / parentScale;

        m_worldTransformNeedsUpdate = false;
        m_worldTransformMatrix = transform;
        m_onWorldTransformUpdated.Broadcast();
        PropagateTransformUpdateToChildren();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Entity3Ds transform, in world space. 
    //----------------------------------------------------------------------------------------------------
    void Entity3D::SetWorldTransform(const Vector3& worldLocation, const Rotation& worldRotation, const Vector3& worldScale)
    {
        m_worldTransformNeedsUpdate = true;
        
        // Calculate the Local Transform:
        Mat4 parentTransform = Mat4::Identity();
        if (m_pParent)
        {
            // Update the Parent, if necessary:
            if (m_pParent->WorldTransformNeedsUpdate())
            {
                m_pParent->UpdateWorldTransform(m_pParent->m_pParent, m_pParent->LocalTransformMatrix());
            }
            parentTransform = m_pParent->GetWorldTransformMatrix();
        }

        Vector3 parentTranslation;
        Vector3 parentScale;
        Rotation parentRotation;
        math::DecomposeMatrix(parentTransform, parentTranslation, parentRotation, parentScale);

        // Convert to local space:
        m_location = worldLocation - parentTranslation;
        m_rotation = worldRotation - parentRotation;
        m_scale = worldScale / parentScale;

        // Compose our world matrix:
        m_worldTransformMatrix = math::MakeTranslationMatrix(worldLocation) * math::ToMat4(worldRotation) * math::MakeScaleMatrix(worldScale);
        
        m_worldTransformNeedsUpdate = false;
        m_onWorldTransformUpdated.Broadcast();
        PropagateTransformUpdateToChildren();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Entity's location, in world space. 
    //----------------------------------------------------------------------------------------------------
    Vector3 Entity3D::GetLocation() const
    {
        return math::XYZ(m_worldTransformMatrix.GetColumn(3));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Entity's world orientation. 
    //----------------------------------------------------------------------------------------------------
    Rotation Entity3D::GetRotation() const
    {
        return math::ExtractRotation(m_worldTransformMatrix);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Entity's total world scale. 
    //----------------------------------------------------------------------------------------------------
    Vector3 Entity3D::GetScale() const
    {
        return m_worldTransformMatrix.GetScale();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Entity's location relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    const Vector3& Entity3D::GetLocalLocation() const
    {
        return m_location;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Entity's rotation relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    const Rotation& Entity3D::GetLocalRotation() const
    {
        return m_rotation;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Entity's scale relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    const Vector3& Entity3D::GetLocalScale() const
    {
        return m_scale;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Local Transform in its Matrix representation. 
    //----------------------------------------------------------------------------------------------------
    Mat4 Entity3D::LocalTransformMatrix() const
    {
        return math::ComposeTransformMatrix(m_location, m_rotation, m_scale);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the World Transformation Matrix of this Entity. 
    //----------------------------------------------------------------------------------------------------
    const Mat4& Entity3D::GetWorldTransformMatrix() const
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

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Sets the "WorldTransformNeedsUpdate" flag to true, which will ensure that the next
    ///             call to get or set will update the Transform appropriately, including Children.
    //----------------------------------------------------------------------------------------------------
    void Entity3D::MarkWorldTransformDirty()
    {
        m_worldTransformNeedsUpdate = true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Updates our calculated world transformation matrix.
    ///         Called when a Parent's transform has changed or the Parent itself has changed.
    //----------------------------------------------------------------------------------------------------
    void Entity3D::UpdateWorldTransform(Entity3D* pParent, const Matrix4x4& localTransform)
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
            
            m_worldTransformMatrix = pParent->GetWorldTransformMatrix() * localTransform;
        }

        // Our transform is now updated.
        m_worldTransformNeedsUpdate = false;
        m_onWorldTransformUpdated.Broadcast();
        PropagateTransformUpdateToChildren();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Walk down the tree, updating the world Transforms of all children.
    //----------------------------------------------------------------------------------------------------
    void Entity3D::PropagateTransformUpdateToChildren()
    {
        for (auto* pChild : m_children)
        {
            pChild->UpdateWorldTransform(this, pChild->LocalTransformMatrix());
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : If true, then the current World Transform is out of date. This can be caused when the
    ///         Entity hierarchy is changed or this Entity or a parent has moved.
    //----------------------------------------------------------------------------------------------------
    bool Entity3D::WorldTransformNeedsUpdate() const
    {
        return m_worldTransformNeedsUpdate;
    }
    
    World* Entity3D::GetWorld() const
    {
        return checked_cast<World*>(GetLayer());
    }
}
