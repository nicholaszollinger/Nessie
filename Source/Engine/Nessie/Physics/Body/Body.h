// BodyInstance.h
#pragma once
#include "BodyID.h"
#include "DOF.h"
#include "Core/Generic/GenerationalID.h"
#include "Math/AABox.h"
#include "Math/Matrix.h"
#include "Physics/Collision/CollisionLayer.h"
#include "Physics/Collision/BroadPhase/BroadPhaseLayer.h"

namespace nes
{
    enum class BodyMotionType
    {
        Static,         /// Non-movable.
        Kinematic,      /// Movable using velocities only - does not respond to any forces.
        Dynamic         /// Responds to forces.
    };

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      This is the object that should be saved to disk.
    //		
    ///		@brief : Struct containing the initial settings used to create a Body. 
    //----------------------------------------------------------------------------------------------------
    struct BodyCreateInfo
    {
        Vector3 m_position          = Vector3::Zero();
        Rotation m_rotation         = Rotation::Identity();
        Vector3 m_linearVelocity    = Vector3::Zero();
        Vector3 m_angularVelocity   = Vector3::Zero();

        CollisionLayer m_collisionLayer     = 0;
        BroadPhaseLayer m_broadPhaseLayer   = BroadPhaseLayer(0);
        BodyMotionType m_motionType         = BodyMotionType::Static;
        AllowedDOFs    m_allowedDOFs        = AllowedDOFs::All;
        float          m_gravityScale       = 1.0f;
        float          m_maxLinearVelocity  = 500.f;
        float          m_maxAngularVelocity = 0.25f * math::Pi<float>() * 60.f;
        float          m_friction           = 0.2f;
        float          m_restitution        = 0.0f;
        float          m_linearDamping      = 0.05f;
        float          m_angularDamping     = 0.05f;
        bool           m_isSensor           = false;
        bool           m_allowSleeping      = true;
    };

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : A Body is a simulated Object that is managed by the Physics System. Bodies contain all
    ///         the motion properties for a simulated Object.
    //----------------------------------------------------------------------------------------------------
    class Body
    {
        friend class BodyManager;

        AABox m_bounds{}; // World space bounding box of the Body.
        BodyID m_id;
        CollisionLayer m_collisionLayer = kInvalidCollisionLayer;
        BroadPhaseLayer m_broadPhaseLayer = kInvalidBroadPhaseLayer;
        BodyMotionType m_motionType = BodyMotionType::Static;
        
    public:
        Body() = default; 
        Body(const Body&) = delete;
        Body& operator=(const Body&) = delete;

        Mat4            GetWorldTransform() const;
        Mat4            GetCenterOfMassTransform() const;
        Vector3         GetPosition();
        Quat            GetRotation();
        Vector3         GetCenterOfMass() const;
        Vector3         GetAccumulatedForce() const;
        Vector3         GetAccumulatedTorque() const;
        Vector3         GetLinearVelocity() const;
        Vector3         GetAngularVelocity() const;
        const AABox&    GetWorldSpaceBounds() const;
        BodyID          GetID() const;
        AllowedDOFs     GetAllowedDOFs() const;
        BodyMotionType  GetMotionType() const;
        CollisionLayer  GetCollisionLayer() const;
        BroadPhaseLayer GetBroadPhaseLayer() const;
        
        bool            IsStatic() const;
        bool            IsKinematic() const;
        bool            IsDynamic() const;
        bool            CanSleep() const;
        bool            IsSleeping() const;
        bool            IsSensor() const;
        bool            IsActive() const;

#ifdef NES_LOGGING_ENABLED
        void ValidateCachedBounds() const;
#endif
        
        // Force functions for Dynamic Bodies.
        void AddForce(const Vector3& force);
        void AddForce(const Vector3& force, const Vector3& position);
        void AddTorque(const Vector3& torque);
        void AddImpulse(const Vector3& impulse);
        void AddImpulse(const Vector3& impulse, const Vector3& position);
        void AddAngularImpulse(const Vector3& angularImpulse);
        void ResetForce();
        void ResetTorque();
    };
}
