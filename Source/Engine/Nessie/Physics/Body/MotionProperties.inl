// MotionProperties.inl
#pragma once

namespace nes
{
    Vec3 MotionProperties::GetLinearVelocity() const
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::Read));
        
        return m_linearVelocity;
    }

    void MotionProperties::SetLinearVelocity(const Vec3& linearVelocity)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        
        NES_ASSERT(linearVelocity.Length() <= m_maxLinearVelocity);
        m_linearVelocity = LockTranslation(linearVelocity);
    }

    void MotionProperties::SetLinearVelocityClamped(const Vec3& linearVelocity)
    {
        m_linearVelocity = LockTranslation(linearVelocity);
        ClampLinearVelocity();
    }

    Vec3 MotionProperties::GetAngularVelocity() const
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::Read));
        return m_angularVelocity;
    }

    void MotionProperties::SetAngularVelocity(const Vec3& angularVelocity)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        NES_ASSERT(angularVelocity.Length() <= m_maxAngularVelocity);
        m_angularVelocity = LockAngular(angularVelocity);
    }

    void MotionProperties::SetAngularVelocityClamped(const Vec3 angularVelocity)
    {
        m_angularVelocity = LockAngular(angularVelocity);
        ClampAngularVelocity();
    }

    void MotionProperties::MoveKinematic(const Vec3& deltaPos, const Quat& deltaRot, const float deltaTime)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::Read));
        //NES_ASSERT(m_cachedBodyType == BodyType::Rigid);
        NES_ASSERT(m_cachedMotionType != EBodyMotionType::Static);

        // Calculate the required linear velocity
        m_linearVelocity = LockTranslation(deltaPos / deltaTime);

        // Calculate the required angular velocity
        Vec3 axis;
        float angle;
        deltaRot.ToAxisAngle(axis, angle);
        m_angularVelocity = LockAngular(axis * (angle / deltaTime));
    }

    void MotionProperties::SetMaxLinearVelocity(const float maxLinearVelocity)
    {
        NES_ASSERT(maxLinearVelocity >= 0.f);
        m_maxLinearVelocity = maxLinearVelocity;
    }

    void MotionProperties::SetMaxAngularVelocity(const float maxAngularVelocity)
    {
        NES_ASSERT(maxAngularVelocity >= 0.f);
        m_maxAngularVelocity = maxAngularVelocity;
    }

    void MotionProperties::ClampLinearVelocity()
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));

        const float lengthSqr = m_linearVelocity.LengthSqr();
        NES_ASSERT(!math::IsInf(lengthSqr));
        if (lengthSqr > math::Squared(m_maxLinearVelocity))
            m_linearVelocity *= (m_maxLinearVelocity / std::sqrt(lengthSqr));
    }

    void MotionProperties::ClampAngularVelocity()
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        
        const float lengthSqr = m_angularVelocity.LengthSqr();
        NES_ASSERT(!math::IsInf(lengthSqr));
        if (lengthSqr > math::Squared(m_maxAngularVelocity))
            m_angularVelocity *= (m_maxAngularVelocity / std::sqrt(lengthSqr));
    }

    void MotionProperties::SetLinearDamping(const float linearDamping)
    {
        NES_ASSERT(linearDamping >= 0.f);
        m_linearDamping = linearDamping;
    }

    void MotionProperties::SetAngularDamping(const float angularDamping)
    {
        NES_ASSERT(angularDamping >= 0.f);
        m_angularDamping = angularDamping;
    }

    float MotionProperties::GetInverseMass() const
    {
        NES_ASSERT(m_cachedMotionType == EBodyMotionType::Dynamic);
        return m_inverseMass;
    }

    Vec3 MotionProperties::GetInverseInertiaDiagonal() const
    {
        NES_ASSERT(m_cachedMotionType == EBodyMotionType::Dynamic);
        return m_inverseInertiaDiagonal;
    }

    void MotionProperties::SetInverseInertia(const Vec3& diagonal, const Quat& inertiaRotation)
    {
        m_inverseInertiaDiagonal = diagonal;
        m_inertiaRotation = inertiaRotation;
    }

    void MotionProperties::ScaleToMass(float mass)
    {
        NES_ASSERT(m_inverseMass > 0.f, "Body must have finite mass!");
        NES_ASSERT(mass > 0.f, "New mass cannot be zero!");

        float newInverseMass = 1.0f / mass;
        m_inverseInertiaDiagonal *= (newInverseMass * m_inverseMass);
        m_inverseMass = newInverseMass;
    }

    Mat44 MotionProperties::GetLocalSpaceInverseInertia() const
    {
        NES_ASSERT(m_cachedMotionType == EBodyMotionType::Dynamic);
        return GetLocalSpaceInverseInertiaUnchecked();
    }

    Mat44 MotionProperties::GetLocalSpaceInverseInertiaUnchecked() const
    {
        const Mat44 rotation = Mat44::MakeRotation(m_inertiaRotation);
        const Mat44 rotationMulScaleTransposed
        (
            m_inverseInertiaDiagonal.SplatX() * rotation[0],
            m_inverseInertiaDiagonal.SplatY() * rotation[1],
            m_inverseInertiaDiagonal.SplatZ() * rotation[2],
            Vec4(0.f, 0.f, 0.f, 1.f)
        );
        
        return rotation.Multiply3x3RightTransposed(rotationMulScaleTransposed);
    }

    Mat44 MotionProperties::GetInverseInertiaForRotation(const Mat44& rotation) const
    {
        NES_ASSERT(m_cachedMotionType == EBodyMotionType::Dynamic);

        Mat44 rot = rotation.Multiply3x3(Mat44::MakeRotation(m_inertiaRotation));
        Mat44 rotationMulScaleTransposed
        (
            m_inverseInertiaDiagonal.SplatX() * rot.GetColumn4(0),
            m_inverseInertiaDiagonal.SplatY() * rot.GetColumn4(1),
            m_inverseInertiaDiagonal.SplatZ() * rot.GetColumn4(2),
            Vec4(0.f, 0.f, 0.f, 1.f)
        );
        Mat44 inverseInertia = rotation.Multiply3x3RightTransposed(rotationMulScaleTransposed);

        // We need to mask out both rows and columns of DOFs that are not allowed.
        const Vec4Reg angularDOFsMask = GetAngularDOFsMask().ReinterpretAsFloat();
        inverseInertia.SetColumn4(0, Vec4Reg::And(inverseInertia.GetColumn4(0), Vec4Reg::And(angularDOFsMask, angularDOFsMask.SplatX())));
        inverseInertia.SetColumn4(1, Vec4Reg::And(inverseInertia.GetColumn4(1), Vec4Reg::And(angularDOFsMask, angularDOFsMask.SplatY())));
        inverseInertia.SetColumn4(2, Vec4Reg::And(inverseInertia.GetColumn4(2), Vec4Reg::And(angularDOFsMask, angularDOFsMask.SplatZ())));
        
        return inverseInertia;
    }

    Vec3 MotionProperties::MultiplyWorldSpaceInverseInertiaByVector(const Quat& bodyRotation, const Vec3& vec) const
    {
        NES_ASSERT(m_cachedMotionType == EBodyMotionType::Dynamic);

        /// Mask out columns of DOFs that are not allowed
        const Vec3 angularDOFsMask = GetAngularDOFsMask().ReinterpretAsFloat().ToVec3();
        const Vec3 v = Vec3::And(vec, angularDOFsMask);

        // Multiply vector by inverse inertia
        const Mat44 rotation = Mat44::MakeRotation(bodyRotation * m_inertiaRotation);
        const Vec3 result = rotation.Multiply3x3(m_inverseInertiaDiagonal * rotation.Multiply3x3Transposed(v));

        // Mask out rows of DOFs that are not allowed.
        return Vec3::And(result, angularDOFsMask);
    }

    Vec3 MotionProperties::GetPointVelocityCOM(const Vec3& pointRelativeToCOM) const
    {
        return m_linearVelocity + m_angularVelocity.Cross(pointRelativeToCOM);
    }

    void MotionProperties::ResetMotion()
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        m_linearVelocity = m_angularVelocity = Vec3::Zero();
        m_force = m_torque = Vec3::Zero();
    }

    UVec4Reg MotionProperties::GetLinearDOFsMask() const
    {
        const UVec4Reg mask(static_cast<uint32_t>(EAllowedDOFs::TranslationX), static_cast<uint32_t>(EAllowedDOFs::TranslationY), static_cast<uint32_t>(EAllowedDOFs::TranslationZ), 0);
        return UVec4Reg::Equals(UVec4Reg::And(UVec4Reg::Replicate(static_cast<uint32_t>(m_allowedDoFs)), mask), mask);
    }

    Vec3 MotionProperties::LockTranslation(const Vec3& vec) const
    {
        return Vec3::And(vec, GetLinearDOFsMask().ReinterpretAsFloat().ToVec3());
    }

    UVec4Reg MotionProperties::GetAngularDOFsMask() const
    {
        const UVec4Reg mask(static_cast<uint32_t>(EAllowedDOFs::RotationX), static_cast<uint32_t>(EAllowedDOFs::RotationY), static_cast<uint32_t>(EAllowedDOFs::RotationZ), 0);
        return UVec4Reg::Equals(UVec4Reg::And(UVec4Reg::Replicate(static_cast<uint32_t>(m_allowedDoFs)), mask), mask);
    }

    Vec3 MotionProperties::LockAngular(const Vec3& vec) const
    {
        return Vec3::And(vec, GetAngularDOFsMask().ReinterpretAsFloat().ToVec3());
    }

    void MotionProperties::SetNumVelocityStepsOverride(const uint32_t numSteps)
    {
        NES_ASSERT(numSteps < 256);
        m_numVelocityStepsOverride = static_cast<uint8_t>(numSteps);
    }

    void MotionProperties::SetNumPositionStepsOverride(const uint32_t numSteps)
    {
        NES_ASSERT(numSteps < 256);
        m_numPositionStepsOverride = static_cast<uint8_t>(numSteps);
    }

    void MotionProperties::Internal_AddLinearVelocityStep(const Vec3& linearVelocityChange)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        
        m_linearVelocity = LockTranslation(m_linearVelocity + linearVelocityChange);
        NES_ASSERT(!m_linearVelocity.IsNaN());
    }

    void MotionProperties::Internal_SubLinearVelocityStep(const Vec3& linearVelocityChange)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        
        m_linearVelocity = LockTranslation(m_linearVelocity - linearVelocityChange);
        NES_ASSERT(!m_linearVelocity.IsNaN());
    }

    void MotionProperties::Internal_AddAngularVelocityStep(const Vec3& angularVelocityChange)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        
        m_angularVelocity += angularVelocityChange;
        NES_ASSERT(!m_angularVelocity.IsNaN());
    }

    void MotionProperties::Internal_SubAngularVelocityStep(const Vec3& angularVelocityChange)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        
        m_angularVelocity -= angularVelocityChange;
        NES_ASSERT(!m_angularVelocity.IsNaN());
    }

    void MotionProperties::Internal_ApplyGyroscopicForce(const Quat& bodyRotation, const float deltaTime)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        // [TODO]: Check that this is rigid body, not a soft body.
        NES_ASSERT(m_cachedMotionType == EBodyMotionType::Dynamic);
        
        // Compute the local space inertia tensor (a diagonal in local space).
        UVec4Reg isZero = Vec3::Equals(m_inverseInertiaDiagonal, Vec3::Zero());
        Vec3 denominator = Vec3::Select(m_inverseInertiaDiagonal, Vec3::One(), isZero);
        Vec3 numerator = Vec3::Select(Vec3::One(), Vec3::Zero(), isZero);
        Vec3 localInertia = numerator / denominator; // Avoid dividing by zero, inertia in this axis will be zero.

        // Calculate local space angular momentum:
        Quat inertiaSpaceToWorldSpace = bodyRotation * m_inertiaRotation;
        Vec3 localAngularVelocity = inertiaSpaceToWorldSpace.Conjugate() * m_angularVelocity;
        Vec3 localMomentum = localInertia * localAngularVelocity;

        // The gyroscopic force applies a torque: T = -w x I w where w is angular velocity and I the inertia tensor
        // Calculate the new angular momentum by applying the gyroscopic force and make sure the new magnitude is the same as the old one
        // to avoid introducing energy into the system due to the Euler step
        Vec3 newLocalMomentum = localMomentum - (deltaTime * localAngularVelocity.Cross(localMomentum));
        float newLocalMomentumLengthSqr = newLocalMomentum.LengthSqr();
        newLocalMomentum = newLocalMomentumLengthSqr > 0.f? newLocalMomentum * std::sqrt(localMomentum.LengthSqr() / newLocalMomentumLengthSqr) : Vec3::Zero();
        
        // Convert back to world space angular velocity
        m_angularVelocity = inertiaSpaceToWorldSpace * (m_inverseInertiaDiagonal * newLocalMomentum);
    }

    void MotionProperties::Internal_ApplyForceTorqueAndDrag(const Quat& bodyRotation, const Vec3& gravity, const float deltaTime)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        NES_ASSERT(m_cachedMotionType == EBodyMotionType::Dynamic);

        // Update Linear Velocity
        m_linearVelocity = LockTranslation(m_linearVelocity + deltaTime * (m_gravityScale * gravity + m_inverseMass * GetAccumulatedForce()));

        // Update Angular Velocity
        m_angularVelocity += deltaTime * MultiplyWorldSpaceInverseInertiaByVector(bodyRotation, GetAccumulatedTorque());

        // Linear damping: dv/dt = -c * v
        // Solution: v(t) = v(0) * e^(-c * t) or v2 = v1 * e^(-c * dt)
        // Taylor expansion of e^(-c * dt) = 1 - c * dt + ...
        // Since dt is usually in the order of 1/60 and c is a low number to this approximation is good enough
        m_linearVelocity *= math::Max(0.0f, 1.0f - m_linearDamping * deltaTime);
        m_angularVelocity *= math::Max(0.0f, 1.0f - m_angularDamping * deltaTime);

        // Clamp velocities.
        ClampLinearVelocity();
        ClampAngularVelocity();
    }

    void MotionProperties::Internal_ResetSleepTestSpheres(const Vec3* pPoints)
    {
#ifdef NES_DOUBLE_PRECISION

#else
        for (int i = 0; i < 3; ++i)
        {
            m_sleepTestSpheres[i] = Sphere(pPoints[i], 0.f);
        }
#endif

        m_sleepTestTimer = 0.f;
    }

    EAllowedSleep MotionProperties::Internal_AccumulateSleepTime(const float deltaTime, const float timeBeforeSleep)
    {
        m_sleepTestTimer += deltaTime;
        return m_sleepTestTimer >= timeBeforeSleep? EAllowedSleep::CanSleep : EAllowedSleep::CannotSleep;
    }
}
