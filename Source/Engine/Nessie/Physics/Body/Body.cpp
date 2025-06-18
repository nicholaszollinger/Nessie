// Body.cpp
#include "Body.h"
#include "Physics/Collision/Shapes/EmptyShape.h"

namespace nes
{
    static const EmptyShape s_fixedToWorldShape;
    
    // Initialize the Placeholder Body using the boolean constructor.
    Body Body::s_fixedToWorld(false);
    
    Body::Body(bool)
        : m_position(Vec3::Zero())
        , m_rotation(Quat::Identity())
        , m_pShape(&s_fixedToWorldShape)
        , m_friction(0.f)
        , m_restitution(0.f)
        , m_collisionLayer(kInvalidCollisionLayer)
        , m_motionType(EBodyMotionType::Static)
    {
        s_fixedToWorldShape.SetEmbedded();
    }

    void Body::SetMotionType(const EBodyMotionType& motionType)
    {
        if (m_motionType == motionType)
            return;

        NES_ASSERT((motionType == EBodyMotionType::Static || m_pMotionProperties != nullptr), "Body needs to be created with m_allowDynamicOrKinematic set to true!");
        NES_ASSERT((motionType == EBodyMotionType::Static || !IsActive()), "Deactivate body first!");
        //NES_ASSERT((motionType == BodyMotionType::Dynamic || !IsSoftBody()), "Soft bodies can only be dynamic, you can make individual vertices kinematic by setting their inverse mass to 0.");

        m_motionType = motionType;

        if (m_pMotionProperties != nullptr)
        {
            NES_IF_LOGGING_ENABLED(m_pMotionProperties->m_cachedMotionType = motionType);

            switch (m_motionType)
            {
                case EBodyMotionType::Static:
                {
                    // Stop the object
                    m_pMotionProperties->m_linearVelocity = Vec3::Zero();
                    m_pMotionProperties->m_angularVelocity = Vec3::Zero();
                    [[fallthrough]];
                }
                
                case EBodyMotionType::Kinematic:
                {
                    // Cancel forces
                    m_pMotionProperties->ResetForce();
                    m_pMotionProperties->ResetTorque();
                    break;
                }
                
                case EBodyMotionType::Dynamic:
                    break;
            }
        }
    }

    void Body::SetCanSleep(const bool canSleep)
    {
        m_pMotionProperties->m_canSleep = canSleep;
        if (canSleep)
            ResetSleepTimer();
    }

    void Body::MoveKinematic(const Vec3& targetPosition, const Quat& targetRotation, const float deltaTime)
    {
        NES_ASSERT(IsRigidBody());
        NES_ASSERT(!IsStatic());
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::Read));

        // Calculate the center of mass at the end situation
        const Vec3 newCOM = targetPosition + (targetRotation * m_pShape->GetCenterOfMass());
        
        // Calculate the delta position and rotation
        const Vec3 deltaPos = newCOM - m_position;
        const Quat deltaRot = targetRotation * m_rotation.Conjugate();

        // Move the Body
        m_pMotionProperties->MoveKinematic(deltaPos, deltaRot, deltaTime);
    }

    BodyCreateInfo Body::GetBodyCreateInfo() const
    {
        NES_ASSERT(IsRigidBody());

        BodyCreateInfo result;

        result.m_position = GetPosition();
        result.m_rotation = GetRotation();
        result.m_linearVelocity = m_pMotionProperties != nullptr ? m_pMotionProperties->m_linearVelocity : Vec3::Zero();
        result.m_angularVelocity = m_pMotionProperties != nullptr ? m_pMotionProperties->m_angularVelocity : Vec3::Zero();
        result.m_collisionLayer = GetCollisionLayer();
        result.m_userData = m_userData;
        result.m_collisionGroup = GetCollisionGroup();
        result.m_motionType = GetMotionType();
        result.m_allowedDOFs = m_pMotionProperties != nullptr ? m_pMotionProperties->GetAllowedDOFs() : EAllowedDOFs::All;
        result.m_allowDynamicOrKinematic = m_pMotionProperties != nullptr;
        result.m_isSensor = IsSensor();
        result.m_collideKinematicVsNonDynamic = GetCollideKinematicVsNonDynamic();
        result.m_useManifoldReduction = GetUseManifoldReduction();
        result.m_applyGyroscopicForce = GetApplyGyroscopicForce();
        result.m_motionQuality = m_pMotionProperties != nullptr ? m_pMotionProperties->m_motionQuality : EBodyMotionQuality::Discrete;
        result.m_enhancedInternalEdgeRemoval = GetEnhancedInternalEdgeRemoval();
        result.m_allowSleeping = m_pMotionProperties != nullptr? m_pMotionProperties->GetCanSleep() : true;
        result.m_friction = GetFriction();
        result.m_restitution = GetRestitution();
        result.m_linearDamping = m_pMotionProperties != nullptr ? m_pMotionProperties->GetLinearDamping() : 0.0f;
        result.m_angularDamping = m_pMotionProperties != nullptr ? m_pMotionProperties->GetAngularDamping() : 0.0f;
        result.m_maxLinearVelocity = m_pMotionProperties != nullptr ? m_pMotionProperties->GetMaxLinearVelocity() : 0.0f;
        result.m_maxAngularVelocity = m_pMotionProperties != nullptr ? m_pMotionProperties->GetMaxAngularVelocity() : 0.0f;
        result.m_gravityScale = m_pMotionProperties != nullptr ? m_pMotionProperties->GetGravityScale() : 1.0f;
        result.m_numVelocityStepsOverride = m_pMotionProperties != nullptr ? m_pMotionProperties->GetNumVelocityStepsOverride() : 0;
        result.m_numPositionStepsOverride = m_pMotionProperties != nullptr ? m_pMotionProperties->GetNumPositionStepsOverride() : 0;
        result.m_overrideMassProperties = EOverrideMassProperties::MassAndInertiaProvided;

        // Invert inertia and mass
        if (m_pMotionProperties != nullptr)
        {
            const float inverseMass = m_pMotionProperties->GetInverseMass();
            const Mat44 inverseInertia = m_pMotionProperties->GetLocalSpaceInverseInertiaUnchecked();

            // Set Mass
            result.m_massPropertiesOverride.m_mass = inverseMass != 0.f? 1.f / inverseMass : FLT_MAX;

            // Set Inertia
            //Mat3 inverseInertia3 = math::ToMat3(inverseInertia);
            //if (inertia.SetInversed3x3(inverseInertia))
            if (inverseInertia.Determinant3x3() != 0.f)
            {
                // Inertia was invertible, we can use it.
                result.m_massPropertiesOverride.m_inertia = inverseInertia;
            }

            else
            {
                const Vec3 diagonal = Vec3::Max(inverseInertia.GetDiagonal3(), Vec3::Replicate(FLT_MAX));
                result.m_massPropertiesOverride.m_inertia = Mat44::MakeScale(diagonal.Reciprocal());
            }
        }

        else
        {
            result.m_massPropertiesOverride.m_mass = FLT_MAX;
            result.m_massPropertiesOverride.m_inertia = Mat44::MakeScale(Vec3::Replicate(FLT_MAX));
        }

        result.SetShape(GetShape());
        
        return result;
    }

    void Body::Internal_CalculateWorldSpaceBounds()
    {
        m_bounds = m_pShape->GetWorldBounds(GetCenterOfMassTransform(), Vec3::One());
    }

    void Body::Internal_SetPositionAndRotation(const Vec3& position, const Quat& rotation, bool resetSleepTimer)
    {
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::ReadWrite));

        m_position = position + rotation * m_pShape->GetCenterOfMass();
        m_rotation = rotation;

        // Initialize the bounding box
        Internal_CalculateWorldSpaceBounds();

        // Reset sleeping test
        if (resetSleepTimer && m_pMotionProperties != nullptr)
            ResetSleepTimer();
    }

    void Body::Internal_UpdateCenterOfMass(const Vec3& previousCenterOfMass, bool updateMassProperties)
    {
        // Update center of mass position so the world position for this body stays the same.
        m_position += m_rotation * (m_pShape->GetCenterOfMass() - previousCenterOfMass);

        // Recalculate mass and inertia if requested
        if (updateMassProperties && m_pMotionProperties != nullptr)
            m_pMotionProperties->SetMassProperties(m_pMotionProperties->GetAllowedDOFs(), m_pShape->GetMassProperties());
    }

    void Body::Internal_SetShape(const Shape* pShape, bool updateMassProperties)
    {
        NES_ASSERT(IsRigidBody());
        NES_ASSERT(BodyAccess::CheckRights(BodyAccess::GetPositionAccess(), BodyAccess::EAccess::ReadWrite));

        // Get the old center of mass
        const Vec3 oldCOM = m_pShape->GetCenterOfMass();

        // Update the Shape
        m_pShape = pShape;

        // Update the center of mass
        Internal_UpdateCenterOfMass(oldCOM, updateMassProperties);

        // Recalculate the bounding box
        Internal_CalculateWorldSpaceBounds();
    }

    EAllowedSleep Body::Internal_UpdateSleepState(const float deltaTime, float maxMovement, float timeBeforeSleep)
    {
        // Check override & sensors will never go to sleep (they would stop detecting collisions with sleeping bodies).
        if (!m_pMotionProperties->m_canSleep || IsSensor())
            return EAllowedSleep::CannotSleep;

        // Get the points to test
        Vec3 points[3];
        GetSleepTestPoints(points);

        // [TODO]: If double precision, Get the bass offset for spheres  

        for (int i = 0; i < 3; ++i)
        {
            Sphere& sphere = m_pMotionProperties->m_sleepTestSpheres[i];

            // Make point relative to base offset
            // [TODO]: If double precision, Subtract bass offset 
            Vec3 point = points[i];

            // Encapsulate point in a sphere
            sphere.Encapsulate(point);

            // Test if it exceeded the max movement
            if (sphere.GetRadius() > maxMovement)
            {
                m_pMotionProperties->Internal_ResetSleepTestSpheres(points);
                return EAllowedSleep::CannotSleep;
            }
        }

        return m_pMotionProperties->Internal_AccumulateSleepTime(deltaTime, timeBeforeSleep);
    }
}
