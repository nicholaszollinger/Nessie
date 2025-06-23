// DualAxisConstraintPart.inl
#pragma once

namespace nes
{
    inline void DualAxisConstraintPart::CalculateConstraintProperties(const Body& body1, const Mat44& rotation1, const Vec3 r1PlusU, const Body& body2, const Mat44& rotation2, const Vec3 r2, const Vec3 n1, const Vec3 n2)
    {
        NES_ASSERT(n1.IsNormalized(1.0e-5f));
        NES_ASSERT(n2.IsNormalized(1.0e-5f));

        // Calculate properties used during constraint solving:
        m_r1PlusUxN1 = r1PlusU.Cross(n1);
        m_r1PlusUxN2 = r1PlusU.Cross(n2);
        m_r2xN1 = r2.Cross(n1);
        m_r2xN2 = r2.Cross(n2);

        // Calculate the effective mass: K^-1 = (J M^-1 J^T)^-1, eq 59
        Mat22 invEffectiveMass;
        if (body1.IsDynamic())
        {
            const MotionProperties* pMotionProps1 = body1.GetMotionProperties();
            const Mat44 invI1 = pMotionProps1->GetInverseInertiaForRotation(rotation1);
            m_invI1_R1PlusUxN1 = invI1.Multiply3x3(m_r1PlusUxN1);
            m_invI1_R1PlusUxN2 = invI1.Multiply3x3(m_r1PlusUxN2);

            invEffectiveMass[0][0] = pMotionProps1->GetInverseMass() + m_r1PlusUxN1.Dot(m_invI1_R1PlusUxN1);
            invEffectiveMass[1][0] = m_r1PlusUxN1.Dot(m_invI1_R1PlusUxN2);
            invEffectiveMass[0][1] = m_r1PlusUxN2.Dot(m_invI1_R1PlusUxN1);
            invEffectiveMass[1][1] = pMotionProps1->GetInverseMass() + m_r1PlusUxN2.Dot(m_invI1_R1PlusUxN2);
        }
        else
        {
            NES_IF_DEBUG(m_invI1_R1PlusUxN1 = Vec3::NaN());
            NES_IF_DEBUG(m_invI1_R1PlusUxN2 = Vec3::NaN());
            invEffectiveMass = Mat22::Zero();
        }

        if (body2.IsDynamic())
        {
            const MotionProperties* pMotionProps2 = body2.GetMotionProperties();
            const Mat44 invI2 = pMotionProps2->GetInverseInertiaForRotation(rotation2);
            m_invI2_R2xN1 = invI2.Multiply3x3(m_r2xN1);
            m_invI2_R2xN2 = invI2.Multiply3x3(m_r2xN2);

            invEffectiveMass[0][0] += pMotionProps2->GetInverseMass() + m_r2xN1.Dot(m_invI2_R2xN1);
            invEffectiveMass[1][0] += m_r2xN1.Dot(m_invI2_R2xN2);
            invEffectiveMass[0][1] += m_r2xN2.Dot(m_invI2_R2xN1);
            invEffectiveMass[1][1] += pMotionProps2->GetInverseMass() + m_r2xN2.Dot(m_invI2_R2xN2);
        }
        else
        {
            NES_IF_DEBUG(m_invI2_R2xN1 = Vec3::NaN());
            NES_IF_DEBUG(m_invI2_R2xN2 = Vec3::NaN());
        }

        if (!m_effectiveMass.SetInversed(invEffectiveMass))
            Deactivate();
    }

    inline void DualAxisConstraintPart::Deactivate()
    {
        m_effectiveMass.SetZero();
        m_totalLambda = Vec2::Zero();
    }

    inline void DualAxisConstraintPart::WarmStart(Body& body1, Body& body2, const Vec3 n1, const Vec3 n2, const float warmStartImpulseRatio)
    {
        m_totalLambda *= warmStartImpulseRatio;
        ApplyVelocityStep(body1, body2, n1, n2, m_totalLambda);
    }

    inline bool DualAxisConstraintPart::SolveVelocityConstraint(Body& body1, Body& body2, const Vec3 n1, const Vec3 n2)
    {
        Vec2 lambda;
        CalculateLagrangeMultiplier(body1, body2, n1, n2, lambda);

        // Store accumulated lambda.
        m_totalLambda += lambda;
        
        return ApplyVelocityStep(body1, body2, n1, n2, lambda);
    }

    inline bool DualAxisConstraintPart::SolvePositionConstraint(Body& body1, Body& body2, const Vec3 inU, const Vec3 n1, const Vec3 n2, const float baumgarte) const
    {
        Vec2 c;
        c[0] = inU.Dot(n1);
        c[1] = inU.Dot(n2);
        if (!c.IsNearZero(0.f))
        {
            // Calculate lagrange multiplier (lambda) for Baumgarte stabilization:
            //
            // lambda = -K^-1 * beta / dt * C
            //
            // We should divide by inDeltaTime, but we should multiply by inDeltaTime in the Euler step below, so they cancel out.
            Vec2 lambda = -baumgarte * (m_effectiveMass * c);

            // Directly integrate velocity change for one time step
            //
            // Euler velocity integration:
            // dv = M^-1 P
            //
            // Impulse:
            // P = J^T lambda
            //
            // Euler position integration:
            // x' = x + dv * dt
            //
            // Note we don't accumulate velocities for the stabilization. This is using the approach described in 'Modeling and
            // Solving Constraints' by Erin Catto presented at GDC 2007. On slide 78, it is suggested to split up the Baumgarte
            // stabilization for positional drift so that it does not add to the momentum. We combine an Euler velocity
            // integrate + a position integrate and then discard the velocity change.
            const Vec3 impulse = n1 * lambda[0] + n2 * lambda[1];
            if (body1.IsDynamic())
            {
                body1.Internal_SubPositionStep(body1.GetMotionPropertiesUnchecked()->GetInverseMass() * impulse);
                body1.Internal_SubRotationStep(m_invI1_R1PlusUxN1 * lambda[0] + m_invI1_R1PlusUxN2 * lambda[1]);
            }
            
            if (body2.IsDynamic())
            {
                body2.Internal_AddPositionStep(body2.GetMotionPropertiesUnchecked()->GetInverseMass() * impulse);
                body2.Internal_AddRotationStep(m_invI2_R2xN1 * lambda[0] + m_invI2_R2xN2 * lambda[1]);
            }
            return true;
        }

        return false;
    }

    inline bool DualAxisConstraintPart::ApplyVelocityStep(Body& body1, Body& body2, const Vec3 n1, const Vec3 n2, const Vec2 lambda) const
    {
        // Apply impulse if delta is not zero
        if (!lambda.IsNearZero(0.f))
        {
            // Calculate velocity change due to constraint.
            //
            // Impulse:
            // P = J^T lambda
            //
            // Euler velocity integration:
            // v' = v + M^-1 P
            const Vec3 impulse = n1 * lambda[0] + n2 * lambda[1];
            if (body1.IsDynamic())
            {
                MotionProperties* pMotionProps1 = body1.GetMotionProperties();
                pMotionProps1->Internal_SubLinearVelocityStep(pMotionProps1->GetInverseMass() * impulse);
                pMotionProps1->Internal_SubAngularVelocityStep(m_invI1_R1PlusUxN1 * lambda[0] + m_invI1_R1PlusUxN2 * lambda[1]);
            }
            if (body2.IsDynamic())
            {
                MotionProperties* pMotionProps2 = body2.GetMotionProperties();
                pMotionProps2->Internal_AddLinearVelocityStep(pMotionProps2->GetInverseMass() * impulse);
                pMotionProps2->Internal_AddAngularVelocityStep(m_invI2_R2xN1 * lambda[0] + m_invI2_R2xN2 * lambda[1]);
            }
            return true;
        }

        return false;
    }

    inline void DualAxisConstraintPart::CalculateLagrangeMultiplier(const Body& body1, const Body& body2, const Vec3 n1, const Vec3 n2, Vec2& outLambda) const
    {
        // Calculate lagrange multiplier:
        //
        // lambda = -K^-1 (J v + b)
        const Vec3 deltaLinear = body1.GetLinearVelocity() - body2.GetLinearVelocity();
        Vec2 jv;
        jv[0] = n1.Dot(deltaLinear) + m_r1PlusUxN1.Dot(body1.GetAngularVelocity()) - m_r2xN1.Dot(body2.GetAngularVelocity());
        jv[1] = n2.Dot(deltaLinear) + m_r1PlusUxN2.Dot(body1.GetAngularVelocity()) - m_r2xN2.Dot(body2.GetAngularVelocity());
        outLambda = m_effectiveMass * jv;
    }
}
