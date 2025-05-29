// MotionProperties.inl
#pragma once

namespace nes
{
    Vector3 MotionProperties::GetLinearVelocity() const
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::Read));
        
        return m_linearVelocity;
    }

    void MotionProperties::SetLinearVelocity(const Vector3& linearVelocity)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        
        NES_ASSERT(linearVelocity.Magnitude() <= m_maxLinearVelocity);
        m_linearVelocity = LockTranslation(linearVelocity);
    }

    void MotionProperties::SetLinearVelocityClamped(const Vector3& linearVelocity)
    {
        m_linearVelocity = LockTranslation(linearVelocity);
        ClampLinearVelocity();
    }

    Vector3 MotionProperties::GetAngularVelocity() const
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::Read));
        return m_angularVelocity;
    }

    void MotionProperties::SetAngularVelocity(const Vector3& angularVelocity)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        NES_ASSERT(angularVelocity.Magnitude() <= m_maxAngularVelocity);
        m_angularVelocity = LockAngular(angularVelocity);
    }

    void MotionProperties::SetAngularVelocityClamped(const Vector3 angularVelocity)
    {
        m_angularVelocity = LockAngular(angularVelocity);
        ClampAngularVelocity();
    }

    void MotionProperties::MoveKinematic(const Vector3& deltaPos, const Quat& deltaRot, const float deltaTime)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::Read));
        //NES_ASSERT(m_cachedBodyType == BodyType::Rigid);
        NES_ASSERT(m_cachedMotionType != EBodyMotionType::Static);

        // Calculate the required linear velocity
        m_linearVelocity = LockTranslation(deltaPos / deltaTime);

        // Calculate the required angular velocity
        Vector3 axis;
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

        const float lengthSqr = m_linearVelocity.SquaredMagnitude();
        NES_ASSERT(!math::IsInf(lengthSqr));
        if (lengthSqr > math::Squared(m_maxLinearVelocity))
            m_linearVelocity *= (m_maxLinearVelocity / std::sqrt(lengthSqr));
    }

    void MotionProperties::ClampAngularVelocity()
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        
        const float lengthSqr = m_angularVelocity.SquaredMagnitude();
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

    Vector3 MotionProperties::GetInverseInertiaDiagonal() const
    {
        NES_ASSERT(m_cachedMotionType == EBodyMotionType::Dynamic);
        return m_inverseInertiaDiagonal;
    }

    void MotionProperties::SetInverseInertia(const Vector3& diagonal, const Quat& inertiaRotation)
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

    Mat4 MotionProperties::GetLocalSpaceInverseInertia() const
    {
        NES_ASSERT(m_cachedMotionType == EBodyMotionType::Dynamic);
        return GetLocalSpaceInverseInertiaUnchecked();
    }

    Mat4 MotionProperties::GetLocalSpaceInverseInertiaUnchecked() const
    {
        Mat4 rotation = math::ToMat4(m_inertiaRotation);
        Mat4 rotationMulScaleTransposed
        (
            math::SplatX(m_inverseInertiaDiagonal) * rotation[0],
            math::SplatY(m_inverseInertiaDiagonal) * rotation[1],
            math::SplatZ(m_inverseInertiaDiagonal) * rotation[2],
            Vector4(0.f, 0.f, 0.f, 1.f)
        );

        return math::ToMat3(rotation) * math::ToMat3(rotationMulScaleTransposed.Transpose());
    }

    Mat4 MotionProperties::GetInverseInertiaForRotation(const Mat4& rotation) const
    {
        NES_ASSERT(m_cachedMotionType == EBodyMotionType::Dynamic);

        Mat4 rot = rotation * math::ToMat4(m_inertiaRotation);
        Mat4 rotationMulScaleTransposed
        (
            math::SplatX(m_inverseInertiaDiagonal) * rot[0],
            math::SplatY(m_inverseInertiaDiagonal) * rot[1],
            math::SplatZ(m_inverseInertiaDiagonal) * rot[2],
            Vector4(0.f, 0.f, 0.f, 1.f)
        );
        Mat4 inverseInertia = math::ToMat3(rotation) * math::ToMat3(rotationMulScaleTransposed.Transpose());

        // We need to mask out both rows and columns of DOFs that are not allowed.
        // [TODO]: Once you unify the Vector Register and Vector3/4 classes, clean this up:
        const VectorRegisterF angularDOFsMask = GetAngularDOFsMask().ReinterpretAsFloat();

        VectorRegisterF columnVal = VectorRegisterF::And(VectorRegisterF::Load(&inverseInertia[0].x), VectorRegisterF::And(angularDOFsMask, angularDOFsMask.SplatX()));
        inverseInertia[0] = Vector4(columnVal.GetX(), columnVal.GetY(), columnVal.GetZ(), columnVal.GetW());

        columnVal = VectorRegisterF::And(VectorRegisterF::Load(&inverseInertia[1].x), VectorRegisterF::And(angularDOFsMask, angularDOFsMask.SplatY()));
        inverseInertia[1] = Vector4(columnVal.GetX(), columnVal.GetY(), columnVal.GetZ(), columnVal.GetW());
        
        columnVal = VectorRegisterF::And(VectorRegisterF::Load(&inverseInertia[2].x), VectorRegisterF::And(angularDOFsMask, angularDOFsMask.SplatZ()));
        inverseInertia[2] = Vector4(columnVal.GetX(), columnVal.GetY(), columnVal.GetZ(), columnVal.GetW());

        return inverseInertia;
    }

    Vector3 MotionProperties::MultiplyWorldSpaceInverseInertiaByVector(const Quat& bodyRotation, const Vector3& vec) const
    {
        NES_ASSERT(m_cachedMotionType == EBodyMotionType::Dynamic);

        /// Mask out columns of DOFs that are not allowed
        const VectorRegisterF angularDOFsMask = GetAngularDOFsMask().ReinterpretAsFloat();
        const VectorRegisterF v = VectorRegisterF::And(VectorRegisterF(vec.x, vec.y, vec.z, 0.f), angularDOFsMask);

        // Multiply vector by inverse inertia
        const Mat4 rotation = math::ToMat4(bodyRotation * m_inertiaRotation);
        const Mat3 rotation3 = math::ToMat3(rotation);
        const Vector3 result = rotation3 * (m_inverseInertiaDiagonal * rotation3.Transposed() * Vector3(v.GetX(), v.GetY(), v.GetZ())); 
        
        // Mask out rows of DOFs that are not allowed.
        VectorRegisterF regResult = VectorRegisterF::And(VectorRegisterF(result.x, result.y, result.z, 0.f), angularDOFsMask);
        return Vector3(regResult.GetX(), regResult.GetY(), regResult.GetZ());
    }

    Vector3 MotionProperties::GetPointVelocityCOM(const Vector3& pointRelativeToCOM) const
    {
        return m_linearVelocity + m_angularVelocity.Cross(pointRelativeToCOM);
    }

    void MotionProperties::ResetMotion()
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        m_linearVelocity = m_angularVelocity = Vector3::Zero();
        m_force = m_torque = Vector3::Zero();
    }

    VectorRegisterUint MotionProperties::GetLinearDOFsMask() const
    {
        const VectorRegisterUint mask(static_cast<uint32_t>(EAllowedDOFs::TranslationX), static_cast<uint32_t>(EAllowedDOFs::TranslationY), static_cast<uint32_t>(EAllowedDOFs::TranslationZ), 0);
        return VectorRegisterUint::Equals(VectorRegisterUint::And(VectorRegisterUint::Replicate(static_cast<uint32_t>(m_allowedDoFs)), mask), mask);
    }

    Vector3 MotionProperties::LockTranslation(const Vector3& vec) const
    {
        const auto regVec = VectorRegisterF(vec.x, vec.y, vec.z, 0.f);
        const VectorRegisterF result = VectorRegisterF::And(regVec, VectorRegisterF(GetLinearDOFsMask().ReinterpretAsFloat()));
        return Vector3(result.GetX(), result.GetY(), result.GetZ());
    }

    VectorRegisterUint MotionProperties::GetAngularDOFsMask() const
    {
        const VectorRegisterUint mask(static_cast<uint32_t>(EAllowedDOFs::RotationX), static_cast<uint32_t>(EAllowedDOFs::RotationY), static_cast<uint32_t>(EAllowedDOFs::RotationZ), 0);
        return VectorRegisterUint::Equals(VectorRegisterUint::And(VectorRegisterUint::Replicate(static_cast<uint32_t>(m_allowedDoFs)), mask), mask);
    }

    Vector3 MotionProperties::LockAngular(const Vector3& vec) const
    {
        const auto regVec = VectorRegisterF(vec.x, vec.y, vec.z, 0.f);
        const VectorRegisterF result = VectorRegisterF::And(regVec, VectorRegisterF(GetAngularDOFsMask().ReinterpretAsFloat()));
        return Vector3(result.GetX(), result.GetY(), result.GetZ());
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

    void MotionProperties::Internal_AddLinearVelocityStep(const Vector3& linearVelocityChange)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        
        m_linearVelocity = LockTranslation(m_linearVelocity + linearVelocityChange);
        NES_ASSERT(!m_linearVelocity.IsNaN());
    }

    void MotionProperties::Internal_SubLinearVelocityStep(const Vector3& linearVelocityChange)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        
        m_linearVelocity = LockTranslation(m_linearVelocity - linearVelocityChange);
        NES_ASSERT(!m_linearVelocity.IsNaN());
    }

    void MotionProperties::Internal_AddAngularVelocityStep(const Vector3& angularVelocityChange)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetVelocityAccess(), BodyAccess::EAccess::ReadWrite));
        
        m_angularVelocity += angularVelocityChange;
        NES_ASSERT(!m_angularVelocity.IsNaN());
    }

    void MotionProperties::Internal_SubAngularVelocityStep(const Vector3& angularVelocityChange)
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

        VectorRegisterF diag(m_inverseInertiaDiagonal.x, m_inverseInertiaDiagonal.y, m_inverseInertiaDiagonal.z, 0.f);

        // Compute the local space inertia tensor (a diagonal in local space).
        VectorRegisterUint isZero = VectorRegisterF::Equals(diag, VectorRegisterF::Zero());
        VectorRegisterF denominator = VectorRegisterF::Select(diag, VectorRegisterF::Unit(), isZero);
        VectorRegisterF numerator = VectorRegisterF::Select(VectorRegisterF::Unit(), VectorRegisterF::Zero(), isZero);
        VectorRegisterF localInertia = numerator / denominator; // Avoid dividing by zero, inertia in this axis will be zero.

        // Calculate local space angular momentum:
        Quat inertiaSpaceToWorldSpace = bodyRotation * m_inertiaRotation;
        Vector3 localAngularVelocityVec = inertiaSpaceToWorldSpace.Conjugate().RotatedVector(m_angularVelocity);
        VectorRegisterF localAngularVelocity = VectorRegisterF(localAngularVelocityVec.x, localAngularVelocityVec.y, localAngularVelocityVec.z, 0.f);
        VectorRegisterF localMomentum = localInertia * localAngularVelocity;

        // The gyroscopic force applies a torque: T = -w x I w where w is angular velocity and I the inertia tensor
        // Calculate the new angular momentum by applying the gyroscopic force and make sure the new magnitude is the same as the old one
        // to avoid introducing energy into the system due to the Euler step
        VectorRegisterF newLocalMomentum = localMomentum - (deltaTime * localAngularVelocity.Cross(localMomentum));
        float newLocalMomentumLengthSqr = newLocalMomentum.SquaredLength();
        newLocalMomentum = newLocalMomentumLengthSqr > 0.f? newLocalMomentum * std::sqrt(localMomentum.SquaredLength() / newLocalMomentumLengthSqr) : VectorRegisterF::Zero();
        
        // Convert back to world space angular velocity
        // [TODO]: This sucks, once you combine the Vector 3 and Vector Register classes, fix this!!!
        VectorRegisterF temp = (diag * newLocalMomentum);
        const Vector3 tempVec3 = Vector3(temp.m_f32[0], temp.m_f32[1], temp.m_f32[2]);
        m_angularVelocity = inertiaSpaceToWorldSpace.RotatedVector(tempVec3);
    }

    void MotionProperties::Internal_ApplyForceTorqueAndDrag(const Quat& bodyRotation, const Vector3& gravity, const float deltaTime)
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

    void MotionProperties::Internal_ResetSleepTestSpheres(const Vector3* pPoints)
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
