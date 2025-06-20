// AxisConstraintPart.inl
#pragma once

namespace nes
{
    template <EBodyMotionType Type1, EBodyMotionType Type2>
    void AxisConstraintPart::TemplatedCalculateConstraintProperties(const float invMass1, const Mat44& invI1, const Vec3 r1PlusU, const float invMass2, const Mat44& invI2, const Vec3 r2, const Vec3 worldSpaceAxis, float bias)
    {
        const float invEffectiveMass = TemplatedCalculateInverseEffectiveMass<Type1, Type2>(invMass1, invI1, r1PlusU, invMass2, invI2, r2, worldSpaceAxis);

        if (invEffectiveMass == 0.f)
        {
            Deactivate();
        }
        else
        {
            m_effectiveMass = 1.f / invEffectiveMass;
            m_springPart.CalculateSpringPropertiesWithBias(bias);
        }
    }

    inline void AxisConstraintPart::CalculateConstraintProperties(const Body& body1, const Vec3 r1PlusU, const Body& body2, const Vec3 r2, const Vec3 worldSpaceAxis, float bias)
    {
        const float invEffectiveMass = CalculateInverseEffectiveMass(body1, r1PlusU, body2, r2, worldSpaceAxis);
        if (invEffectiveMass == 0.f)
        {
            Deactivate();
        }
        else
        {
            m_effectiveMass = 1.f / invEffectiveMass;
            m_springPart.CalculateSpringPropertiesWithBias(bias);
        }
    }

    inline void AxisConstraintPart::CalculateConstraintPropertiesWithMassOverride(const Body& body1, const float invMass1, const float invInertiaScale1, const Vec3 r1PlusU, const Body& body2, const float invMass2, const float invInertiaScale2, const Vec3 r2, const Vec3 worldSpaceAxis, const float bias)
    {
        const float invEffectiveMass = CalculateInverseEffectiveMassWithMassOverride(body1, invMass1, invInertiaScale1, r1PlusU, body2, invMass2, invInertiaScale2, r2, worldSpaceAxis);
        if (invEffectiveMass == 0.f)
        {
            Deactivate();
        }
        else
        {
            m_effectiveMass = 1.f / invEffectiveMass;
            m_springPart.CalculateSpringPropertiesWithBias(bias);
        }
    }

    inline void AxisConstraintPart::CalculateConstraintPropertiesWithFrequencyAndDamping(const float deltaTime, const Body& body1, const Vec3 r1PlusU, const Body& body2, const Vec3 r2, const Vec3 worldSpaceAxis, const float bias, const float inC, const float frequency, const float damping)
    {
        const float invEffectiveMass = CalculateInverseEffectiveMass(body1, r1PlusU, body2, r2, worldSpaceAxis);
        if (invEffectiveMass == 0.f)
            Deactivate();
        else
            m_springPart.CalculateSpringPropertiesWithFrequencyAndDamping(deltaTime, invEffectiveMass, bias, inC, frequency, damping, m_effectiveMass);
    }

    inline void AxisConstraintPart::CalculateConstraintPropertiesWithStiffnessAndDamping(const float deltaTime, const Body& body1, const Vec3 r1PlusU, const Body& body2, const Vec3 r2, const Vec3 worldSpaceAxis, const float bias, const float inC, const float stiffness, const float damping)
    {
        const float invEffectiveMass = CalculateInverseEffectiveMass(body1, r1PlusU, body2, r2, worldSpaceAxis);
        if (invEffectiveMass == 0.f)
            Deactivate();
        else
            m_springPart.CalculateSpringPropertiesWithStiffnessAndDamping(deltaTime, invEffectiveMass, bias, inC, stiffness, damping, m_effectiveMass);
    }

    inline void AxisConstraintPart::CalculateConstraintPropertiesWithSettings(const float deltaTime, const Body& body1, const Vec3 r1PlusU, const Body& body2, const Vec3 r2, const Vec3 worldSpaceAxis, const float bias, const float inC, const SpringSettings& springSettings)
    {
        const float invEffectiveMass = CalculateInverseEffectiveMass(body1, r1PlusU, body2, r2, worldSpaceAxis);
        if (invEffectiveMass == 0.f)
            Deactivate();
        else if (springSettings.m_springMode == ESpringMode::FrequencyAndDamping)
            m_springPart.CalculateSpringPropertiesWithFrequencyAndDamping(deltaTime, invEffectiveMass, bias, inC, springSettings.m_frequency, springSettings.m_damping, m_effectiveMass);
        else
            m_springPart.CalculateSpringPropertiesWithStiffnessAndDamping(deltaTime, invEffectiveMass, bias, inC, springSettings.m_stiffness, springSettings.m_damping, m_effectiveMass);
    }

    inline void AxisConstraintPart::Deactivate()
    {
        m_effectiveMass = 0.f;
        m_totalLambda = 0.f;
    }

    template <EBodyMotionType Type1, EBodyMotionType Type2>
    void AxisConstraintPart::TemplatedWarmStart(MotionProperties* pMotionProps1, const float invMass1, MotionProperties* pMotionProps2, const float invMass2, const Vec3 worldSpaceAxis, const float warmStartImpulseRatio)
    {
        m_totalLambda *= warmStartImpulseRatio;
        ApplyVelocityStep<Type1, Type2>(pMotionProps1, invMass1, pMotionProps2, invMass2, worldSpaceAxis, m_totalLambda);
    }

    inline void AxisConstraintPart::WarmStart(Body& body1, Body& body2, const Vec3 worldSpaceAxis, const float warmStartImpulseRatio)
    {
        const EBodyMotionType motionType1 = body1.GetMotionType();
        MotionProperties* pMotionProps1 = body1.GetMotionProperties();

        const EBodyMotionType motionType2 = body2.GetMotionType();
        MotionProperties* pMotionProps2 = body2.GetMotionProperties();

        // Dispatch to the correct templated form:
        // Note: Warm starting doesn't differentiate between kinematic/static bodies, so we handle both as static bodies.
        if (motionType1 == EBodyMotionType::Dynamic)
        {
            if (motionType2 == EBodyMotionType::Dynamic)
                TemplatedWarmStart<EBodyMotionType::Dynamic, EBodyMotionType::Dynamic>(pMotionProps1, pMotionProps1->GetInverseMass(), pMotionProps2, pMotionProps2->GetInverseMass(), worldSpaceAxis, warmStartImpulseRatio);
            else
                TemplatedWarmStart<EBodyMotionType::Dynamic, EBodyMotionType::Static>(pMotionProps1, pMotionProps1->GetInverseMass(), pMotionProps2, 0.f /* Unused */, worldSpaceAxis, warmStartImpulseRatio);
                
        }
        else
        {
            NES_ASSERT(motionType2 == EBodyMotionType::Dynamic); // There must be a dynamic body.
            TemplatedWarmStart<EBodyMotionType::Static, EBodyMotionType::Dynamic>(pMotionProps1, 0.f /* Unused */, pMotionProps2, pMotionProps2->GetInverseMass(), worldSpaceAxis, warmStartImpulseRatio);
        }
    }

    template <EBodyMotionType Type1, EBodyMotionType Type2>
    float AxisConstraintPart::TemplatedSolveVelocityConstraintGetTotalLambda(const MotionProperties* pMotionProps1, const MotionProperties* pMotionProps2, const Vec3 worldSpaceAxis) const
    {
        // Calculate jacobian multiplied by linear velocity
        float jv{};
        if constexpr (Type1 != EBodyMotionType::Static && Type2 != EBodyMotionType::Static)
            jv = worldSpaceAxis.Dot(pMotionProps1->GetLinearVelocity() - pMotionProps2->GetLinearVelocity());
        else if constexpr (Type1 != EBodyMotionType::Static)
            jv = worldSpaceAxis.Dot(pMotionProps1->GetLinearVelocity());
        else if constexpr (Type2 != EBodyMotionType::Static)
            jv = worldSpaceAxis.Dot(-pMotionProps2->GetLinearVelocity());
        else
            NES_ASSERT(false); // Static vs. Static is nonsensical!

        // Calculate jacobian multiplied by angular velocity
        if constexpr (Type1 != EBodyMotionType::Static)
            jv += Vec3::LoadFloat3Unsafe(m_R1PlusUxAxis).Dot(pMotionProps1->GetAngularVelocity());
        if constexpr (Type2 != EBodyMotionType::Static)
            jv -= Vec3::LoadFloat3Unsafe(m_R2xAxis).Dot(pMotionProps2->GetAngularVelocity());

        // Lagrange multiplier is:
        //  Lambda = -K^-1 (J v + b)
        const float lambda = m_effectiveMass * (jv - m_springPart.GetBias(m_totalLambda));

        // Return the total accumulated lambda
        return m_totalLambda + lambda;
    }

    template <EBodyMotionType Type1, EBodyMotionType Type2>
    bool AxisConstraintPart::TemplatedSolveVelocityConstraintApplyLambda(MotionProperties* pMotionProps1, const float invMass1, MotionProperties* pMotionProps2, const float invMass2, const Vec3 worldSpaceAxis, const float totalLambda)
    {
        const float deltaLambda = totalLambda - m_totalLambda; // Calculate change in lambda
        m_totalLambda += totalLambda;

        return ApplyVelocityStep<Type1, Type2>(pMotionProps1, invMass1, pMotionProps2, invMass2, worldSpaceAxis, deltaLambda);
    }

    template <EBodyMotionType Type1, EBodyMotionType Type2>
    bool AxisConstraintPart::TemplatedSolveVelocityConstraint(MotionProperties* pMotionProps1, const float invMass1, MotionProperties* pMotionProps2, const float invMass2, const Vec3 worldSpaceAxis, const float minLambda, const float maxLambda)
    {
        float totalLambda = TemplatedSolveVelocityConstraintGetTotalLambda<Type1, Type2>(pMotionProps1, pMotionProps2, worldSpaceAxis);

        // Clamp impulse to specified range
        totalLambda = math::Clamp(totalLambda, minLambda, maxLambda);

        return TemplatedSolveVelocityConstraintApplyLambda<Type1, Type2>(pMotionProps1, invMass1, pMotionProps2, invMass2, worldSpaceAxis, totalLambda);
    }

    inline bool AxisConstraintPart::SolveVelocityConstraint(Body& body1, Body& body2, const Vec3 worldSpaceAxis, const float minLambda, const float maxLambda)
    {
        const EBodyMotionType motionType1 = body1.GetMotionType();
        MotionProperties* pMotionProps1 = body1.GetMotionPropertiesUnchecked();

        const EBodyMotionType motionType2 = body2.GetMotionType();
        MotionProperties* pMotionProps2 = body2.GetMotionPropertiesUnchecked();

        // Dispatch to the correct templated form
        switch (motionType1)
        {
            case EBodyMotionType::Dynamic:
            {
                switch (motionType2)
                {
                    case EBodyMotionType::Dynamic:
                        return TemplatedSolveVelocityConstraint<EBodyMotionType::Dynamic, EBodyMotionType::Dynamic>(pMotionProps1, pMotionProps1->GetInverseMass(), pMotionProps2, pMotionProps2->GetInverseMass(), worldSpaceAxis, minLambda, maxLambda);

                    case EBodyMotionType::Kinematic:
                        return TemplatedSolveVelocityConstraint<EBodyMotionType::Dynamic, EBodyMotionType::Kinematic>(pMotionProps1, pMotionProps1->GetInverseMass(), pMotionProps2, 0.0f /* Unused */, worldSpaceAxis, minLambda, maxLambda);
                    
                    case EBodyMotionType::Static:
                        return TemplatedSolveVelocityConstraint<EBodyMotionType::Dynamic, EBodyMotionType::Static>(pMotionProps1, pMotionProps1->GetInverseMass(), pMotionProps2, 0.0f /* Unused */, worldSpaceAxis, minLambda, maxLambda);
                        
                    default:
                    {
                        NES_ASSERT(false);
                        break;
                    }
                }
                break;
            }

            case EBodyMotionType::Kinematic:
            {
                NES_ASSERT(motionType2 == EBodyMotionType::Dynamic);
                return TemplatedSolveVelocityConstraint<EBodyMotionType::Kinematic, EBodyMotionType::Dynamic>(pMotionProps1, 0.f /* Unused */, pMotionProps2, pMotionProps2->GetInverseMass(), worldSpaceAxis, minLambda, maxLambda);
            }

            case EBodyMotionType::Static:
            {
                NES_ASSERT(motionType2 == EBodyMotionType::Dynamic);
                return TemplatedSolveVelocityConstraint<EBodyMotionType::Static, EBodyMotionType::Dynamic>(pMotionProps1, 0.f /* Unused */, pMotionProps2, pMotionProps2->GetInverseMass(), worldSpaceAxis, minLambda, maxLambda);
            }

            default:
            {
                NES_ASSERT(false);
                break;
            }
        }

        return false;
    }

    inline bool AxisConstraintPart::SolveVelocityConstraintWithMassOverride(Body& body1, const float invMass1, Body& body2, const float invMass2, const Vec3 worldSpaceAxis, const float minLambda, const float maxLambda)
    {
        const EBodyMotionType motionType1 = body1.GetMotionType();
        MotionProperties* pMotionProps1 = body1.GetMotionPropertiesUnchecked();

        const EBodyMotionType motionType2 = body2.GetMotionType();
        MotionProperties* pMotionProps2 = body2.GetMotionPropertiesUnchecked();

        // Dispatch to the correct templated form
        switch (motionType1)
        {
            case EBodyMotionType::Dynamic:
            {
                switch (motionType2)
                {
                    case EBodyMotionType::Dynamic:
                        return TemplatedSolveVelocityConstraint<EBodyMotionType::Dynamic, EBodyMotionType::Dynamic>(pMotionProps1, invMass1, pMotionProps2, invMass2, worldSpaceAxis, minLambda, maxLambda);

                    case EBodyMotionType::Kinematic:
                        return TemplatedSolveVelocityConstraint<EBodyMotionType::Dynamic, EBodyMotionType::Kinematic>(pMotionProps1, invMass1, pMotionProps2, 0.0f /* Unused */, worldSpaceAxis, minLambda, maxLambda);
                    
                    case EBodyMotionType::Static:
                        return TemplatedSolveVelocityConstraint<EBodyMotionType::Dynamic, EBodyMotionType::Static>(pMotionProps1, invMass1, pMotionProps2, 0.0f /* Unused */, worldSpaceAxis, minLambda, maxLambda);
                        
                    default:
                    {
                        NES_ASSERT(false);
                        break;
                    }
                }
                break;
            }

            case EBodyMotionType::Kinematic:
            {
                NES_ASSERT(motionType2 == EBodyMotionType::Dynamic);
                return TemplatedSolveVelocityConstraint<EBodyMotionType::Kinematic, EBodyMotionType::Dynamic>(pMotionProps1, 0.f /* Unused */, pMotionProps2, invMass2, worldSpaceAxis, minLambda, maxLambda);
            }

            case EBodyMotionType::Static:
            {
                NES_ASSERT(motionType2 == EBodyMotionType::Dynamic);
                return TemplatedSolveVelocityConstraint<EBodyMotionType::Static, EBodyMotionType::Dynamic>(pMotionProps1, 0.f /* Unused */, pMotionProps2, invMass2, worldSpaceAxis, minLambda, maxLambda);
            }

            default:
            {
                NES_ASSERT(false);
                break;
            }
        }

        return false;
    }

    inline bool AxisConstraintPart::SolvePositionConstraint(Body& body1, Body& body2, const Vec3 worldSpaceAxis, const float inC, const float baumgarte) const
    {
        // Only apply position constraint when the constraint is hard, otherwise the velocity bias will fix the constraint
        if (inC != 0.f && !m_springPart.IsActive())
        {
            // Calculate lagrange multiplier (lambda) for Baumgarte stabilization:
            //
            // lambda = -K^-1 * beta / dt * C
            //
            // We should divide by deltaTime, but we should multiply by deltaTime in the Euler step below, so they cancel out.
            const float lambda = -m_effectiveMass * baumgarte * inC;

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
            if (body1.IsDynamic())
            {
                body1.Internal_SubPositionStep((lambda * body1.GetMotionProperties()->GetInverseMass()) * worldSpaceAxis);
                body1.Internal_SubRotationStep(lambda * Vec3::LoadFloat3Unsafe(m_InvI1_R1PlusUxAxis));
            }
            if (body2.IsDynamic())
            {
                body2.Internal_AddPositionStep((lambda * body2.GetMotionProperties()->GetInverseMass()) * worldSpaceAxis);
                body2.Internal_AddRotationStep(lambda * Vec3::LoadFloat3Unsafe(m_InvI2_R2xAxis));
            }
            return true;
        }

        return false;
    }

    inline bool AxisConstraintPart::SolvePositionContraintWithMassOverride(Body& body1, const float invMass1, Body& body2, const float invMass2, const Vec3 worldSpaceAxis, const float inC, const float baumgarte) const
    {
        // Only apply position constraint when the constraint is hard, otherwise the velocity bias will fix the constraint
        if (inC != 0.f && !m_springPart.IsActive())
        {
            // Calculate lagrange multiplier (lambda) for Baumgarte stabilization:
            //
            // lambda = -K^-1 * beta / dt * C
            //
            // We should divide by deltaTime, but we should multiply by deltaTime in the Euler step below, so they cancel out.
            const float lambda = -m_effectiveMass * baumgarte * inC;

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
            if (body1.IsDynamic())
            {
                body1.Internal_SubPositionStep((lambda * invMass1) * worldSpaceAxis);
                body1.Internal_SubRotationStep(lambda * Vec3::LoadFloat3Unsafe(m_InvI1_R1PlusUxAxis));
            }
            if (body2.IsDynamic())
            {
                body2.Internal_AddPositionStep((lambda * invMass2) * worldSpaceAxis);
                body2.Internal_AddRotationStep(lambda * Vec3::LoadFloat3Unsafe(m_InvI2_R2xAxis));
            }
            return true;
        }

        return false;
    }

    template <EBodyMotionType Type1, EBodyMotionType Type2>
    bool AxisConstraintPart::ApplyVelocityStep(MotionProperties* pMotionProps1, const float invMass1, MotionProperties* pMotionProps2, const float invMass2, const Vec3 worldSpaceAxis, const float lambda) const
    {
        // Apply impulse if delta is not zero:
        if (lambda != 0.f)
        {
            // Calculate velocity change due to constraint
            //
            // Impulse:
            // P = J^T lambda
            //
            // Euler velocity integration:
            // v' = v + M^-1 P
            if constexpr (Type1 == EBodyMotionType::Dynamic)
            {
                pMotionProps1->Internal_SubLinearVelocityStep((lambda * invMass1) * worldSpaceAxis);
                pMotionProps1->Internal_SubLinearVelocityStep(lambda * Vec3::LoadFloat3Unsafe(m_InvI1_R1PlusUxAxis));
            }
            if constexpr (Type2 == EBodyMotionType::Dynamic)
            {
                pMotionProps2->Internal_AddLinearVelocityStep((lambda * invMass2) * worldSpaceAxis);
                pMotionProps2->Internal_AddLinearVelocityStep(lambda * Vec3::LoadFloat3Unsafe(m_InvI2_R2xAxis));
            }
            return true;
        }
        return false;
    }

    template <EBodyMotionType Type1, EBodyMotionType Type2>
    float AxisConstraintPart::TemplatedCalculateInverseEffectiveMass(const float invMass1, const Mat44& invI1, const Vec3 r1PlusU, const float invMass2, const Mat44& invI2, const Vec3 r2, const Vec3 worldSpaceAxis)
    {
        NES_ASSERT(worldSpaceAxis.IsNormalized(1.0e-5f));

        // Calculate properties used below:
        Vec3 r1PlusUxAxis;
        if constexpr (Type1 != EBodyMotionType::Static)
        {
            r1PlusUxAxis = r1PlusU.Cross(worldSpaceAxis);
            r1PlusUxAxis.StoreFloat3(&m_R1PlusUxAxis);
        }
        else
        {
        #ifdef NES_DEBUG
            Vec3::NaN().StoreFloat3(&m_R1PlusUxAxis);
        #endif
        }

        Vec3 r2xAxis;
        if constexpr (Type2 != EBodyMotionType::Static)
        {
            r2xAxis = r2.Cross(worldSpaceAxis);
            r2xAxis.StoreFloat3(&m_R2xAxis);
        }
        else
        {
        #ifdef NES_DEBUG
            Vec3::NaN().StoreFloat3(&m_R2xAxis);
        #endif
        }

        // Calculate inverse effective mass: K = J M^-1 J^T
        float invEffectiveMass;

        if constexpr (Type1 == EBodyMotionType::Dynamic)
        {
            const Vec3 invI1R1PlusUxAxis = invI1.Multiply3x3(r1PlusUxAxis);
            invI1R1PlusUxAxis.StoreFloat3(&m_InvI1_R1PlusUxAxis);
            invEffectiveMass = invMass1 + invI1R1PlusUxAxis.Dot(r1PlusUxAxis);
        }
        
        else
        {
            (void)r1PlusUxAxis; // Fix compiler warning: Not using this (it's not calculated either).
            NES_IF_DEBUG(Vec3::NaN().StoreFloat3(&m_InvI1_R1PlusUxAxis));
            invEffectiveMass = 0.f;
        }

        if constexpr (Type2 == EBodyMotionType::Dynamic)
        {
            const Vec3 invI2R2xAxis = invI2.Multiply3x3(r2xAxis);
            invI2R2xAxis.StoreFloat3(&m_InvI2_R2xAxis);
            invEffectiveMass += invMass2 + invI2R2xAxis.Dot(r2xAxis);
        }

        else
        {
            (void)r2xAxis; // Fix compiler warning: Not using this (it's not calculated either).
            NES_IF_DEBUG(Vec3::NaN().StoreFloat3(&m_InvI2_R2xAxis));
        }
        
        return invEffectiveMass;
    }

    inline float AxisConstraintPart::CalculateInverseEffectiveMass(const Body& body1, const Vec3 r1PlusU, const Body& body2, const Vec3 r2, const Vec3 worldSpaceAxis)
    {
        // Dispatch to the correct templated form.
        switch (body1.GetMotionType())
        {
            case EBodyMotionType::Dynamic:
            {
                const MotionProperties* pMotionProps1 = body1.GetMotionPropertiesUnchecked();
                const float invMass1 = pMotionProps1->GetInverseMass();
                const Mat44 invI1 = body1.GetInverseInertia();
                switch (body2.GetMotionType())
                {
                    case EBodyMotionType::Dynamic:
                        return TemplatedCalculateInverseEffectiveMass<EBodyMotionType::Dynamic, EBodyMotionType::Dynamic>(invMass1, invI1, r1PlusU, body2.GetMotionPropertiesUnchecked()->GetInverseMass(), body2.GetInverseInertia(), r2, worldSpaceAxis);

                    case EBodyMotionType::Kinematic:
                        return TemplatedCalculateInverseEffectiveMass<EBodyMotionType::Dynamic, EBodyMotionType::Kinematic>(invMass1, invI1, r1PlusU, 0.f /* Will not be used */, Mat44() /* Will not be used */, r2, worldSpaceAxis);
                        
                    case EBodyMotionType::Static:
                        return TemplatedCalculateInverseEffectiveMass<EBodyMotionType::Dynamic, EBodyMotionType::Static>(invMass1, invI1, r1PlusU, 0.f /* Will not be used */, Mat44() /* Will not be used */, r2, worldSpaceAxis);

                    default: break;
                }
                
                break;
            }

            case EBodyMotionType::Kinematic:
            {
                NES_ASSERT(body2.IsDynamic());
                return TemplatedCalculateInverseEffectiveMass<EBodyMotionType::Kinematic, EBodyMotionType::Dynamic>(0.f /*Will not be used*/, Mat44() /*Will not be used*/, r1PlusU, body2.GetMotionPropertiesUnchecked()->GetInverseMass(), body2.GetInverseInertia(), r2, worldSpaceAxis);
            }

            case EBodyMotionType::Static:
            {
                NES_ASSERT(body2.IsDynamic());
                return TemplatedCalculateInverseEffectiveMass<EBodyMotionType::Static, EBodyMotionType::Dynamic>(0.f /*Will not be used*/, Mat44() /*Will not be used*/, r1PlusU, body2.GetMotionPropertiesUnchecked()->GetInverseMass(), body2.GetInverseInertia(), r2, worldSpaceAxis);
            }

            default: break;
        }

        NES_ASSERT(false);
        return 0.f;
    }

    inline float AxisConstraintPart::CalculateInverseEffectiveMassWithMassOverride(const Body& body1, const float invMass1, const float invInertiaScale1, const Vec3 r1PlusU, const Body& body2, const float invMass2, const float invInertiaScale2, const Vec3 r2, const Vec3 worldSpaceAxis)
    {
        // Dispatch to the correct templated form.
        switch (body1.GetMotionType())
        {
            case EBodyMotionType::Dynamic:
            {
                const Mat44 invI1 = invInertiaScale1 * body1.GetInverseInertia();
                switch (body2.GetMotionType())
                {
                    case EBodyMotionType::Dynamic:
                        return TemplatedCalculateInverseEffectiveMass<EBodyMotionType::Dynamic, EBodyMotionType::Dynamic>(invMass1, invI1, r1PlusU, invMass2,  invInertiaScale1 * body2.GetInverseInertia(), r2, worldSpaceAxis);

                    case EBodyMotionType::Kinematic:
                        return TemplatedCalculateInverseEffectiveMass<EBodyMotionType::Dynamic, EBodyMotionType::Kinematic>(invMass1, invI1, r1PlusU, 0.f /* Will not be used */, Mat44() /* Will not be used */, r2, worldSpaceAxis);
                        
                    case EBodyMotionType::Static:
                        return TemplatedCalculateInverseEffectiveMass<EBodyMotionType::Dynamic, EBodyMotionType::Static>(invMass1, invI1, r1PlusU, 0.f /* Will not be used */, Mat44() /* Will not be used */, r2, worldSpaceAxis);

                    default: break;
                }
                
                break;
            }

            case EBodyMotionType::Kinematic:
            {
                NES_ASSERT(body2.IsDynamic());
                return TemplatedCalculateInverseEffectiveMass<EBodyMotionType::Kinematic, EBodyMotionType::Dynamic>(0.f /*Will not be used*/, Mat44() /*Will not be used*/, r1PlusU, invMass2,  invInertiaScale2 * body2.GetInverseInertia(), r2, worldSpaceAxis);
            }

            case EBodyMotionType::Static:
            {
                NES_ASSERT(body2.IsDynamic());
                return TemplatedCalculateInverseEffectiveMass<EBodyMotionType::Static, EBodyMotionType::Dynamic>(0.f /*Will not be used*/, Mat44() /*Will not be used*/, r1PlusU, invMass2,  invInertiaScale2 * body2.GetInverseInertia(), r2, worldSpaceAxis);
            }

            default: break;
        }

        NES_ASSERT(false);
        return 0.f;
    }
}
