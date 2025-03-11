// WorldComponent.cpp

#include "WorldComponent.h"
#include "Math/VectorConversions.h"
#include "Scene/Entity.h"
#include "World/World.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Parent of this WorldComponent. 
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::SetParent(WorldComponent* pParent)
    {
        if (m_pParent == pParent)
            return;
        
        if (m_pParent != nullptr)
        {
            m_pParent->RemoveChild(this);
        }

        m_pParent = pParent;

        // If we now have a parent, add us as a child.
        if (m_pParent != nullptr)
        {
            m_pParent->m_children.emplace_back(this);
            m_pParent->OnChildAdded(this);
        }
        
        OnParentChanged(m_pParent);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      If successful, this sets the Child's Parent to this, and calls
    //      this->OnParentChanged() and pChild->OnChildAdded(pChild).
    //		
    ///		@brief : Attach a WorldComponent to this.
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::AddChild(WorldComponent* pChild)
    {
        // Don't re-add a child
        for (auto* pCurrentChild : m_children)
        {
            if (pCurrentChild == pChild)
                return;
        }

        pChild->m_pParent = this;
        m_children.emplace_back(pChild);
        
        pChild->OnParentChanged(this);
        OnChildAdded(pChild);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      If successful, this sets the Child's Parent to this, and calls
    //      pChild->OnParentSet() and this->OnChildRemoved(pChild).
    
    ///		@brief : Remove a Child from this Component. 
    ///		@param pChild : 
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::RemoveChild(WorldComponent* pChild)
    {
        for (size_t i = 0; i < m_children.size(); i++)
        {
            if (m_children[i] == pChild)
            {
                std::swap(m_children[i], m_children.back());
                m_children.pop_back();
                pChild->m_pParent = nullptr;

                pChild->OnParentChanged(nullptr);
                OnChildRemoved(pChild);
                return;
            }
        }
    }

    WorldComponent* WorldComponent::GetParent() const
    {
        return m_pParent;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Component Children owned by the same Entity. 
    //----------------------------------------------------------------------------------------------------
    std::vector<WorldComponent*> WorldComponent::GetChildren() const
    {
        std::vector<WorldComponent*> sameOwnerChildren;
        sameOwnerChildren.reserve(m_children.size());

        auto* pOwner = GetOwner();
        
        for (auto* pChild : m_children)
        {
            if (pChild->GetOwner() == pOwner)
            {
                sameOwnerChildren.emplace_back(pChild);
            }
        }

        return sameOwnerChildren;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns all child Components of this Component, regardless if owned by other entities that
    ///             have been attached to the Component's Owner.
    //----------------------------------------------------------------------------------------------------
    const std::vector<WorldComponent*>& WorldComponent::GetAllChildren() const
    {
        return m_children;
    }

    void WorldComponent::SetEnabled(const bool enabled)
    {
        if (m_isEnabled == enabled)
            return;

        m_isEnabled = enabled;
        
        if (m_isEnabled)
        {
            OnEnabled();

            for (auto& child : m_children)
            {
                if (child->m_isEnabled)
                {
                    child->OnEnabled();
                }
            }
        }

        else
        {
            OnDisabled();

            for (auto& child : m_children)
            {
                if (child->m_isEnabled)
                {
                    child->OnDisabled();
                }
            }
        }
    }

    bool WorldComponent::IsEnabled() const
    {
        if (!GetOwner()->IsEnabled())
        {
            return false;
        }

        if (m_pParent)
        {
            return m_pParent->IsEnabled() && m_isEnabled;
        }

        return m_isEnabled;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Rotate this Component by a delta angle around an axis. The angle must be in radians.
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::Rotate(const float angle, const Vector3& axis)
    {
        m_localTransform.m_orientation = Quat::MakeFromAngleAxis(angle, axis);
        UpdateWorldTransform(m_pParent, m_localTransform.ToMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Rotate this Component by a delta rotation.
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::Rotate(const Quat& rotation)
    {
        m_localTransform.m_orientation = rotation * m_localTransform.m_orientation;
        UpdateWorldTransform(m_pParent, m_localTransform.ToMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Move this Component's local location based on the translation. 
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::Translate(const Vector3& translation)
    {
        m_localTransform.Translate(translation);
        UpdateWorldTransform(m_pParent, m_localTransform.ToMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Increase the local scale by a delta scalar amount. This multiplies the current scale of each
    ///         axis by the scalar values of deltaScale.
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::Scale(const float uniformScale)
    {
        m_localTransform.Scale(uniformScale);
        UpdateWorldTransform(m_pParent, m_localTransform.ToMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Increase the local scale by a uniform amount on each axis. This multiplies the current local
    ///             scale by the deltaUniformScale.
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::Scale(const Vector3& scale)
    {
        m_localTransform.Scale(scale);
        UpdateWorldTransform(m_pParent, m_localTransform.ToMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Sets this Component's location relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::SetLocalLocation(const Vector3& location)
    {
        m_localTransform.m_location = location;
        UpdateWorldTransform(m_pParent, m_localTransform.ToMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set this Component's orientation relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::SetLocalOrientation(const Quat& orientation)
    {
        m_localTransform.m_orientation = orientation;
        UpdateWorldTransform(m_pParent, m_localTransform.ToMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set this Component's local orientation, from euler angles. The euler angles are expected
    ///             to be in degrees.
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::SetLocalOrientation(const Vector3& eulerAngles)
    {
        m_localTransform.m_orientation = Quat::MakeFromEuler(eulerAngles);
        UpdateWorldTransform(m_pParent, m_localTransform.ToMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set this Component's scale relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::SetLocalScale(const Vector3& scale)
    {
        m_localTransform.m_scale = scale;
        UpdateWorldTransform(m_pParent, m_localTransform.ToMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set this Component's Transform relative to its parent.
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::SetLocalTransform(const Transform& transform)
    {
        m_localTransform = transform;
        UpdateWorldTransform(m_pParent, m_localTransform.ToMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Component's location, orientation and scale relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::SetLocalTransform(const Vector3& location, const Quat& orientation, const Vector3& scale)
    {
        m_localTransform = Transform(location, orientation, scale);
        UpdateWorldTransform(m_pParent, m_localTransform.ToMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Component's location, in world space. 
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::SetWorldLocation(const Vector3& location)
    {
        // Get our current parent location:
        Vector3 parentLocation = Vector3::GetZeroVector();
        if (m_pParent)
        {
            // Update the Parent, if necessary:
            if (m_pParent->WorldTransformNeedsUpdate())
            {
                m_pParent->UpdateWorldTransform(m_pParent->m_pParent, m_pParent->GetLocalTransformMatrix());
            }
            parentLocation = m_pParent->GetLocation();
        }

        // Update our local position based on the new location.
        m_localTransform.m_location = location - parentLocation;
        UpdateWorldTransform(m_pParent, m_localTransform.ToMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Component's orientation, in world space. 
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::SetWorldOrientation(const Quat& orientation)
    {
        // Get our current parent orientation:
        Quat parentOrientation = Quat::Identity();
        if (m_pParent)
        {
            // Update the Parent, if necessary:
            if (m_pParent->WorldTransformNeedsUpdate())
            {
                m_pParent->UpdateWorldTransform(m_pParent->m_pParent, m_pParent->GetLocalTransformMatrix());
            }
            parentOrientation = m_pParent->GetOrientation();
        }

        m_localTransform.m_orientation = orientation - parentOrientation;
        UpdateWorldTransform(m_pParent, m_localTransform.ToMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Set the Component's scale, in world space. 
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::SetWorldScale(const Vector3& scale)
    {
        Vector3 parentScale = Vector3::GetUnitVector();
        if (m_pParent)
        {
            // Update the Parent, if necessary:
            if (m_pParent->WorldTransformNeedsUpdate())
            {
                m_pParent->UpdateWorldTransform(m_pParent->m_pParent, m_pParent->GetLocalTransformMatrix());
            }
            parentScale = m_pParent->GetScale();
        }

        m_localTransform.m_scale = scale / parentScale;
        UpdateWorldTransform(m_pParent, m_localTransform.ToMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Sets the Location, Orientation and Scale of the Component, in World Space. 
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::SetWorldTransform(const Mat4& transform)
    {
        m_worldTransformNeedsUpdate = true;
        
        Mat4 parentTransform = Mat4::Identity();
        if (m_pParent)
        {
            // Update the Parent, if necessary:
            if (m_pParent->WorldTransformNeedsUpdate())
            {
                m_pParent->UpdateWorldTransform(m_pParent->m_pParent, m_pParent->GetLocalTransformMatrix());
            }
            parentTransform = m_pParent->GetWorldTransformMatrix();
        }

        Vector3 parentTranslation;
        Vector3 parentScale;
        Quat parentOrientation;
        math::DecomposeMatrix(parentTransform, parentTranslation, parentOrientation, parentScale);

        Vector3 translation;
        Vector3 scale;
        Quat orientation;
        math::DecomposeMatrix(transform, translation, orientation, scale);
        
        // Convert to local space:
        m_localTransform.m_location = translation - parentTranslation;
        m_localTransform.m_orientation = orientation - parentOrientation;
        m_localTransform.m_scale = scale / parentScale;

        m_worldTransformNeedsUpdate = false;
        m_worldTransformMatrix = transform;
        OnWorldTransformUpdated();
        PropagateTransformUpdateToChildren();
    }

    void WorldComponent::SetWorldTransform(const Vector3& worldLocation, const Quat& worldOrientation,
        const Vector3& worldScale)
    {
        m_worldTransformNeedsUpdate = true;
        
        // Calculate the Local Transform:
        Mat4 parentTransform = Mat4::Identity();
        if (m_pParent)
        {
            // Update the Parent, if necessary:
            if (m_pParent->WorldTransformNeedsUpdate())
            {
                m_pParent->UpdateWorldTransform(m_pParent->m_pParent, m_pParent->GetLocalTransformMatrix());
            }
            parentTransform = m_pParent->GetWorldTransformMatrix();
        }

        Vector3 parentTranslation;
        Vector3 parentScale;
        Quat parentOrientation;
        math::DecomposeMatrix(parentTransform, parentTranslation, parentOrientation, parentScale);

        // Convert to local space:
        m_localTransform.m_location = worldLocation - parentTranslation;
        m_localTransform.m_orientation = worldOrientation - parentOrientation;
        m_localTransform.m_scale = worldScale / parentScale;

        // Compose our world matrix:
        m_worldTransformMatrix = math::MakeTranslationMatrix(worldLocation) * math::ToMat4(worldOrientation) * math::MakeScaleMatrix(worldScale);
        
        m_worldTransformNeedsUpdate = false;
        OnWorldTransformUpdated();
        PropagateTransformUpdateToChildren();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Component's location, in world space. 
    //----------------------------------------------------------------------------------------------------
    Vector3 WorldComponent::GetLocation() const
    {
        return math::XYZ(m_worldTransformMatrix.GetColumn(3));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Component's world orientation. 
    //----------------------------------------------------------------------------------------------------
    Quat WorldComponent::GetOrientation() const
    {
        return math::ExtractOrientation(m_worldTransformMatrix);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Component's total world scale. 
    //----------------------------------------------------------------------------------------------------
    Vector3 WorldComponent::GetScale() const
    {
        // To extract the scale from a transformation matrix, you need to calculate the magnitude of
        // each of the x, y and z axes, which are the first 3 columns in the transformation matrix.
        // Each column's magnitude represents the scale on that axis.
        return m_worldTransformMatrix.GetScale();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Component's location relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    const Vector3& WorldComponent::GetLocalLocation() const
    {
        return m_localTransform.m_location;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Component's orientation relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    const Quat& WorldComponent::GetLocalOrientation() const
    {
        return m_localTransform.m_orientation;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Component's scale relative to its parent. 
    //----------------------------------------------------------------------------------------------------
    const Vector3& WorldComponent::GetLocalScale() const
    {
        return m_localTransform.m_scale;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Local Transform in its Matrix representation. 
    //----------------------------------------------------------------------------------------------------
    Mat4 WorldComponent::GetLocalTransformMatrix() const
    {
        return m_localTransform.ToMatrix();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the World Transformation Matrix of this Component. 
    //----------------------------------------------------------------------------------------------------
    const Mat4& WorldComponent::GetWorldTransformMatrix() const
    {
        return m_worldTransformMatrix;
    }
    
    EntityDomain WorldComponent::GetDomain() const
    {
        return EntityDomain::Physical3D;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Called when this Component has a new parent set.
    ///		@param pParent : The new parent.
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::OnParentChanged(WorldComponent* pParent)
    {
        auto* pOwner = GetOwner();
        NES_ASSERT(pOwner != nullptr);

        // Don't bother updating transforms if the Entity is being destroyed.
        if (pOwner->IsMarkedForDestruction())
            return;

        MarkWorldTransformDirty();
        UpdateWorldTransform(pParent, GetLocalTransformMatrix());
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Sets the "WorldTransformNeedsUpdate" flag to true, which will ensure that the next
    ///             call to our set or get the transform will be updated appropriately.
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::MarkWorldTransformDirty()
    {
        m_worldTransformNeedsUpdate = true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Updates our calculated world transformation matrix.
    ///         Called when a Parent's transform has changed or the Parent itself has changed.
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::UpdateWorldTransform(WorldComponent* pParent, const Matrix4x4& localTransform)
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
                pParent->UpdateWorldTransform(pParent->m_pParent, pParent->GetLocalTransformMatrix());
            }
            
            m_worldTransformMatrix = pParent->GetWorldTransformMatrix() * localTransform;
        }

        // Our transform is now updated.
        m_worldTransformNeedsUpdate = false;
        OnWorldTransformUpdated();
        PropagateTransformUpdateToChildren();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Walk down the tree, updating the world Transforms of all children.
    //----------------------------------------------------------------------------------------------------
    void WorldComponent::PropagateTransformUpdateToChildren()
    {
        for (auto* pChild : m_children)
        {
            pChild->UpdateWorldTransform(this, pChild->GetLocalTransformMatrix());
            pChild->PropagateTransformUpdateToChildren();
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : If true, then the current World Transform is out of date. This can be caused when the
    ///         Component hierarchy is changed or this Component or a parent has moved.
    //----------------------------------------------------------------------------------------------------
    bool WorldComponent::WorldTransformNeedsUpdate() const
    {
        // Should I return false if the Entity is marked for destroy? Or should I just defer that check to
        // OnParentChanged()?
        return m_worldTransformNeedsUpdate;
    }
}
