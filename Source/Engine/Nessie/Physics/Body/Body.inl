// Body.inl
#pragma once

namespace nes
{
    EAllowedDOFs Body::GetAllowedDOFs() const
    {
        if (m_pMotionProperties != nullptr)
            return m_pMotionProperties->GetAllowedDOFs();
        
        return EAllowedDOFs::All;
    }

    void Body::SetIsSensor(const bool isSensor)
    {
        NES_ASSERT(IsRigidBody());
        SetFlag(EFlags::IsSensor, isSensor);
    }

    void Body::SetCollideKinematicVsNonDynamic(const bool collideKinematicVsNonDynamic)
    {
        NES_ASSERT(IsRigidBody());
        SetFlag(EFlags::CollideKinematicVsNonDynamic, collideKinematicVsNonDynamic);
    }

    void Body::SetUseManifoldReduction(bool useReduction)
    {
        NES_ASSERT(IsRigidBody());
        SetFlag(EFlags::UseManifoldReduction, useReduction);
    }

    void Body::SetApplyGyroscopicForce(const bool apply)
    {
        NES_ASSERT(IsRigidBody());
        SetFlag(EFlags::ApplyGyroscopicForce, apply);
    }

    void Body::SetEnhancedInternalEdgeRemoval(const bool apply)
    {
        NES_ASSERT(IsRigidBody());
        SetFlag(EFlags::EnhancedInternalEdgeRemoval, apply);
    }
    
    bool Body::GetEnhancedInternalEdgeRemovalWithBody(const Body& body2) const
    {
        return ((m_flags.load(std::memory_order_relaxed) & body2.m_flags.load(std::memory_order_relaxed)) & static_cast<uint8_t>(EFlags::EnhancedInternalEdgeRemoval)) != 0;
    }

    void Body::ResetSleepTimer()
    {
        Vec3 points[3];
        GetSleepTestPoints(points);
        m_pMotionProperties->Internal_ResetSleepTestSpheres(points);
    }

    Vec3 Body::GetLinearVelocity() const
    {
        return !IsStatic()? m_pMotionProperties->GetLinearVelocity() : Vec3::Zero();
    }

    inline void Body::SetLinearVelocity(const Vec3& linearVelocity)
    {
        NES_ASSERT(!IsStatic());
        m_pMotionProperties->SetLinearVelocity(linearVelocity);
    }

    void Body::SetLinearVelocityClamped(const Vec3& linearVelocity)
    {
        NES_ASSERT(!IsStatic());
        m_pMotionProperties->SetLinearVelocityClamped(linearVelocity);
    }

    Vec3 Body::GetAngularVelocity() const
    {
        return !IsStatic()? m_pMotionProperties->GetAngularVelocity() : Vec3::Zero();
    }

    void Body::SetAngularVelocity(const Vec3& angularVelocity)
    {
        NES_ASSERT(!IsStatic());
        m_pMotionProperties->SetAngularVelocity(angularVelocity);
    }

    void Body::SetAngularVelocityClamped(const Vec3& angularVelocity)
    {
        NES_ASSERT(!IsStatic());
        m_pMotionProperties->SetAngularVelocityClamped(angularVelocity);
    }

    Vec3 Body::GetPointVelocityCOM(const Vec3& pointRelativeToCOM) const
    {
        return !IsStatic()? m_pMotionProperties->GetPointVelocityCOM(pointRelativeToCOM) : Vec3::Zero();
    }

    Vec3 Body::GetPointVelocity(const Vec3& point) const
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::Read));
        return GetPointVelocityCOM(point - m_position);
    }

    void Body::AddForce(const Vec3& force)
    {
        NES_ASSERT(IsDynamic());
        // [TODO]: Register operations
        m_pMotionProperties->m_force += force;
    }

    void Body::AddForce(const Vec3& force, const Vec3& position)
    {
        AddForce(force);
        AddTorque(Vec3(position - m_position).Cross(force));
    }

    Vec3 Body::GetAccumulatedForce() const
    {
        NES_ASSERT(IsDynamic());
        return m_pMotionProperties->GetAccumulatedForce();
    }

    void Body::AddTorque(const Vec3& torque)
    {
        NES_ASSERT(IsDynamic());
        // [TODO]: Register operations
        m_pMotionProperties->m_torque += torque;
    }

    Vec3 Body::GetAccumulatedTorque() const
    {
        NES_ASSERT(IsDynamic());
        return m_pMotionProperties->GetAccumulatedTorque();
    }

    void Body::AddImpulse(const Vec3& impulse)
    {
        NES_ASSERT(IsDynamic());
        SetLinearVelocityClamped(m_pMotionProperties->GetLinearVelocity() + impulse * m_pMotionProperties->GetInverseMass());
    }

    void Body::AddImpulse(const Vec3& impulse, const Vec3& position)
    {
        NES_ASSERT(IsDynamic());
        SetLinearVelocityClamped(m_pMotionProperties->GetLinearVelocity() + impulse * m_pMotionProperties->GetInverseMass());
        SetAngularVelocityClamped(m_pMotionProperties->GetAngularVelocity() + m_pMotionProperties->MultiplyWorldSpaceInverseInertiaByVector(m_rotation, Vec3(position - m_position).Cross(impulse)));
    }

    void Body::AddAngularImpulse(const Vec3& angularImpulse)
    {
        NES_ASSERT(IsDynamic());
        SetAngularVelocityClamped(m_pMotionProperties->GetAngularVelocity() + m_pMotionProperties->MultiplyWorldSpaceInverseInertiaByVector(m_rotation, angularImpulse));
    }

    void Body::ResetForce()
    {
        NES_ASSERT(IsDynamic());
        m_pMotionProperties->ResetForce();
    }

    void Body::ResetTorque()
    {
        NES_ASSERT(IsDynamic());
        m_pMotionProperties->ResetTorque();
    }

    void Body::ResetMotion()
    {
        NES_ASSERT(IsDynamic());
        m_pMotionProperties->ResetMotion();
    }

    Mat44 Body::GetInverseInertia() const
    {
        NES_ASSERT(IsDynamic());
        return GetMotionProperties()->GetInverseInertiaForRotation(Mat44::MakeRotation(m_rotation));
    }

    Mat44 Body::GetWorldTransform() const
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::Read));
        return Mat44::MakeRotationTranslation(m_rotation, m_position).PreTranslated(-m_pShape->GetCenterOfMass());
    }

    Vec3 Body::GetCenterOfMassPosition() const
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::Read));
        return m_position;
    }

    Mat44 Body::GetCenterOfMassTransform() const
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::Read));
        return Mat44::MakeRotationTranslation(m_rotation, m_position);
    }

    Mat44 Body::GetInverseCenterOfMassTransform() const
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::Read));
        return Mat44::MakeInverseRotationTranslation(m_rotation, m_position);
    }

    const MotionProperties* Body::GetMotionProperties() const
    {
        NES_ASSERT(!IsStatic());
        return m_pMotionProperties;
    }

    MotionProperties* Body::GetMotionProperties()
    {
        NES_ASSERT(!IsStatic());
        return m_pMotionProperties;
    }

    Vec3 Body::GetWorldSpaceSurfaceNormal(const SubShapeID& subShapeID, const Vec3& position) const
    {
        const Mat44 inverseCOM = GetInverseCenterOfMassTransform();
        return inverseCOM.Multiply3x3Transposed(m_pShape->GetSurfaceNormal(subShapeID, Vec3(inverseCOM.TransformPoint(position)))).Normalized();
    }

    TransformedShape Body::GetTransformedShape() const
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::Read));
        return TransformedShape(m_position, m_rotation, m_pShape, m_id);
    }

    bool Body::Internal_FindCollidingPairsCanCollide(const Body& body1, const Body& body2)
    {
        // First body should never be a soft body.
        NES_ASSERT(!body1.IsSoftBody());

        // One of these conditions must be true:
        // - We always allow detecting collisions between kinematic and non-dynamic bodies
        // - One of the bodies must be dynamic to collide.
        // - A kinematic object can collide with a sensor.
        if (!body1.GetCollideKinematicVsNonDynamic()
            && !body2.GetCollideKinematicVsNonDynamic()
            && (!body1.IsDynamic() && !body2.IsDynamic())
            && !(body1.IsKinematic() && body2.IsSensor())
            && !(body2.IsKinematic() && body1.IsSensor()))
            return false;

        const uint32_t body1IndexInActiveBodies = body1.Internal_GetIndexInActiveBodies();
        NES_ASSERT(!body1.IsStatic() && body1IndexInActiveBodies != Body::kInactiveIndex, "This function assumes that Body 1 is active.");

        // If the pair A, B collides we need to ensure that the pair B, A does not collide or else we will handle the collision twice..
        // If A is the same body as B we don't want to collide (1)
        // If A is dynamic / kinematic and B is static we should collide (2)
        // If A is dynamic / kinematic and B is dynamic / kinematic we should only collide if
        //	- A is active and B is not active (3)
        //	- A is active and B will become active during this simulation step (4)
        //	- A is active and B is active, we require a condition that makes A, B collide and B, A not (5)
        //
        // In order to implement this we use the index in the active body list and make use of the fact that
        // a body not in the active list has Body.Index = 0xffffffff which is the highest possible value for an uint32.
        //
        // Because we know that A is active we know that A.Index != 0xffffffff:
        // (1) Because A.Index != 0xffffffff, if A.Index = B.Index then A = B, so to collide A.Index != B.Index
        // (2) A.Index != 0xffffffff, B.Index = 0xffffffff (because it's static and cannot be in the active list), so to collide A.Index != B.Index
        // (3) A.Index != 0xffffffff, B.Index = 0xffffffff (because it's not yet active), so to collide A.Index != B.Index
        // (4) A.Index != 0xffffffff, B.Index = 0xffffffff currently. But it can activate during the Broad/NarrowPhase step at which point it
        //     will be added to the end of the active list which will make B.Index > A.Index (this holds true only when we don't deactivate
        //     bodies during the Broad/NarrowPhase step), so to collide A.Index < B.Index.
        // (5) As a tie-breaker we can use the same condition A.Index < B.Index to collide, this means that if A, B collides then B, A won't
        static_assert(Body::kInactiveIndex == 0xffffffff, "The algorithm below uses this value");

        if (!body2.IsSoftBody() && body1IndexInActiveBodies >= body2.Internal_GetIndexInActiveBodies())
            return false;

        NES_ASSERT(body1.GetID() != body2.GetID(), "Read the comment above, A and B are the same body which should not be possible!");

        // Check collision group filter
        if (!body1.GetCollisionGroup().CanCollide(body2.GetCollisionGroup()))
            return false;
        
        return true;
    }

    void Body::Internal_AddPositionStep(const Vec3& linearVelocityTimesDeltaTime)
    {
        NES_ASSERT(IsRigidBody());
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::ReadWrite));
        
        m_position += m_pMotionProperties->LockTranslation(linearVelocityTimesDeltaTime);
        
        NES_ASSERT(!m_position.IsNaN());
    }

    void Body::Internal_SubPositionStep(const Vec3& linearVelocityTimesDeltaTime)
    {
        NES_ASSERT(IsRigidBody());
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::ReadWrite));
        
        m_position -= m_pMotionProperties->LockTranslation(linearVelocityTimesDeltaTime);
        
        NES_ASSERT(!m_position.IsNaN());
    }

    void Body::Internal_AddRotationStep(const Vec3& angularVelocityTimesDeltaTime)
    {
        NES_ASSERT(IsRigidBody());
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::ReadWrite));

        // From Jolt:
        // This used to use the equation: d/dt R(t) = 1/2 * w(t) * R(t) so that R(t + dt) = R(t) + 1/2 * w(t) * R(t) * dt
        // See: Appendix B of An Introduction to Physically Based Modeling: Rigid Body Simulation II-Nonpenetration Constraints
        // URL: https://www.cs.cmu.edu/~baraff/sigcourse/notesd2.pdf
        // But this is a first order approximation and does not work well for kinematic ragdolls that are driven to a new
        // pose if the poses differ enough. So now we split w(t) * dt into an axis and angle part and create a quaternion with it.
        // Note that the resulting quaternion is normalized since otherwise numerical drift will eventually make the rotation non-normalized.
        const float length = angularVelocityTimesDeltaTime.Length();
        if (length > 1.0e-6f)
        {
            m_rotation = (Quat::FromAxisAngle(angularVelocityTimesDeltaTime / length, length) * m_rotation).Normalized();
            NES_ASSERT(!m_rotation.IsNaN());
        }
    }

    void Body::Internal_SubRotationStep(const Vec3& angularVelocityTimesDeltaTime)
    {
        NES_ASSERT(IsRigidBody());
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::ReadWrite));
        
        // See comment in Internal_AddRotationStep().
        const float length = angularVelocityTimesDeltaTime.Length();
        if (length > 1.0e-6f)
        {
            m_rotation = (Quat::FromAxisAngle(angularVelocityTimesDeltaTime / length, -length) * m_rotation).Normalized();
            NES_ASSERT(!m_rotation.IsNaN());
        }
    }
    
    void Body::Internal_SetInBroadPhase(const bool isInBroadPhase)
    {
        if (isInBroadPhase)
            m_flags.fetch_or(static_cast<uint8_t>(EFlags::IsInBroadPhase), std::memory_order::relaxed);
        else
            m_flags.fetch_and(static_cast<uint8_t>(~static_cast<uint8_t>(EFlags::IsInBroadPhase)), std::memory_order::relaxed);
    }

    bool Body::Internal_InvalidateContactCache()
    {
        return (m_flags.fetch_or(static_cast<uint8_t>(EFlags::InvalidateContactCache), std::memory_order::relaxed) & static_cast<uint8_t>(EFlags::InvalidateContactCache)) == 0;
    }

    void Body::Internal_ValidateContactCache()
    {
        NES_IF_LOGGING_ENABLED(const uint8_t oldValue = )
        m_flags.fetch_and(static_cast<uint8_t>(~static_cast<uint8_t>(EFlags::InvalidateContactCache)), std::memory_order_relaxed);
        
        NES_ASSERT((oldValue & static_cast<uint8_t>(EFlags::InvalidateContactCache)) != 0);
    }

    void Body::Internal_ValidateCachedBounds() const
    {
        [[maybe_unused]] const AABox actualBodyBounds = m_pShape->GetWorldBounds(GetCenterOfMassTransform(), Vec3::One());
        NES_ASSERT(actualBodyBounds == m_bounds, "Mismatch between cached bounding box and actual bounding box!");
    }

    void Body::SetFlag(const EFlags flag, const bool set)
    {
        if (set)
            m_flags.fetch_or(static_cast<uint8_t>(flag), std::memory_order::relaxed);
        else
            m_flags.fetch_and(~static_cast<uint8_t>(flag), std::memory_order::relaxed);
    }

    bool Body::GetFlag(const EFlags flag) const
    {
        return (m_flags.load(std::memory_order_relaxed) & static_cast<uint8_t>(flag)) != 0;
    }

    void Body::GetSleepTestPoints(Vec3* outPoints) const
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::Read));

        // Center of mass is the first position
        outPoints[0] = m_position;

        // The second and third positions are on the largest axis of the bounding box
        Vec3 extent = m_pShape->GetLocalBounds().Extent();
        const int lowestComponent = extent.MinComponentIndex();
        const Mat44 rotation = Mat44::MakeRotation(m_rotation);

        switch (lowestComponent)
        {
            case 0:
            {
                outPoints[1] = m_position + extent.y * rotation.GetColumn3(1);
                outPoints[2] = m_position + extent.z * rotation.GetColumn3(2);
                break;
            }

            case 1:
            {
                outPoints[1] = m_position + extent.x * rotation.GetColumn3(0);
                outPoints[2] = m_position + extent.z * rotation.GetColumn3(2);
                break;
            }

            case 2:
            {
                outPoints[1] = m_position + extent.x * rotation.GetColumn3(0);
                outPoints[2] = m_position + extent.y * rotation.GetColumn3(1);
                break;
            }

            default:
                NES_ASSERT(false);
                break;
        }
    }
}
