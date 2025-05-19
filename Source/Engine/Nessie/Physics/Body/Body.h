// BodyInstance.h
#pragma once
#include "BodyCreateInfo.h"
#include "BodyID.h"
#include "MotionProperties.h"
#include "Core/Generic/GenerationalID.h"
#include "Math/Bit.h"
#include "Physics/Collision/TransformedShape.h"
#include "Physics/Collision/BroadPhase/BroadPhaseLayer.h"

namespace nes
{
    class Shape;
    
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : A Body is a simulated Object that is managed by the Physics System. Bodies contain all
    ///         the motion properties for a simulated Object.
    //----------------------------------------------------------------------------------------------------
    class Body
    {
        friend class BodyManager;
        friend class BodyWithMotionProperties;

        enum class Flags : uint8_t
        {
            IsSensor                        = math::BitVal(0), /// If this Body is a Sensor. A Sensor will receive collision callbacks, but not cause collision responses.
            CollideKinematicVsNonDynamic    = math::BitVal(1), /// If kinematic objects can generate contact points against other kinematic or static objects.
            IsInBroadPhase                  = math::BitVal(2), /// Set this bit to indicate that the body is in the broadphase
            InvalidateContactCache          = math::BitVal(3), /// Set this bit to indicate that all collision caches for this body are invalid. Will be reset on the next simulation step. 
            UseManifoldReduction            = math::BitVal(4), /// Set this bit to indicate that this body can use manifold reduction.
            ApplyGyroscopicForce            = math::BitVal(5), /// Set this bit to indicate that the gyroscopic force should be applied to this Body (AKA Dzhanibekov effect, see https://en.wikipedia.org/wiki/Tennis_racket_theorem)
            EnhancedInternalEdgeRemoval     = math::BitVal(6), /// Set this bit to indicate that enhance internal edge removal should be used for this Body
        };

        static constexpr uint32_t kInactiveIndex = MotionProperties::kInactiveIndex;

        // 16 byte aligned
        Vector3                 m_position;                                 /// World space position of center of mass (COM).
        Quat                    m_rotation;                                 /// World space rotation of center of mass (COM).
        AABox                   m_bounds;                                   /// World space bounding box of the body.

        // 8 byte aligned
        ConstStrongPtr<Shape>   m_pShape;                                   /// Shape representing the volume of the Body.
        MotionProperties*       m_pMotionProperties = nullptr;              /// If this is a keyframed or dynamic object, this object holds all information about movement.
        uint64_t                m_userData = 0;                             /// User data, can be used for anything by the Application.
        CollisionGroup          m_collisionGroup;                           /// The collision group that this body belongs to. Determines if two objects can collide.

        // 4 byte aligned
        float                   m_friction;                                 /// Friction of the body. Usually between [0, 1], where 0 = no friction and 1 = friction force equals the force that presses the two bodies together. Note that bodies can have negative friction but the combined friction should never go below zero.
        float                   m_restitution;                              /// Restitution of the body. Usually between [0, 1], where 0 = completely inelastic collision response, 1 = completely elastic collision response. Note that bodies can have negative restitution but the combined restitution should never go below zero.  
        BodyID                  m_id;                                       /// ID of the Body, equal to the index into the bodies array.

        // 2 or 4 byte aligned
        CollisionLayer          m_collisionLayer = kInvalidCollisionLayer;  /// The collision layer that this body belongs to. Determines if two objects can collide.

        // 1 byte aligned
        BroadPhaseLayer         m_broadPhaseLayer = kInvalidBroadPhaseLayer; /// The broad phase layer that this body belongs to.
        BodyMotionType          m_motionType = BodyMotionType::Static;       /// The type of motion (static, dynamic, or kinematic).
        std::atomic<uint8_t>    m_flags = 0;                                 /// See Flags definition for details.
        // BodyType           m_bodyType = BodyType::Rigid;
        
        Body() = default;       /// Private Ctor.
        explicit Body(bool);    /// Explicit constructor that initializes all members.
        
    public:
        virtual ~Body() = default;
        Body(const Body&) = delete;
        Body& operator=(const Body&) = delete;

    public:
        /// A Dummy body that can be used by constraints to attach a constraint to the world instead of another body.
        static Body             s_fixedToWorld;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the ID of the Body.
        //----------------------------------------------------------------------------------------------------
        inline const BodyID&    GetID() const                                           { return m_id; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get this body's current world position. 
        //----------------------------------------------------------------------------------------------------
        Vector3                 GetPosition() const                                     { return m_position; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get this body's current world rotation. 
        //----------------------------------------------------------------------------------------------------
        Quat                    GetRotation() const                                     { return m_rotation; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the bounding box for this Body. 
        //----------------------------------------------------------------------------------------------------
        const AABox&            GetWorldSpaceBounds() const                             { return m_bounds; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the degrees of freedom that this body can move in.
        //----------------------------------------------------------------------------------------------------
        inline AllowedDOFs      GetAllowedDOFs() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Collision Layer that this body belongs to - this determines if which bodies it can collide with.
        //----------------------------------------------------------------------------------------------------
        inline CollisionLayer   GetCollisionLayer() const                               { return m_collisionLayer; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the broad phase layer that this body is in - this determines what subtree the object is placed. 
        //----------------------------------------------------------------------------------------------------
        inline BroadPhaseLayer  GetBroadPhaseLayer() const                              { return m_broadPhaseLayer; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the collision group and sub-group ID, determines which other objects it collides with. 
        //----------------------------------------------------------------------------------------------------
        const CollisionGroup&   GetCollisionGroup() const                               { return m_collisionGroup; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the collision group and sub-group ID, determines which other objects it collides with. 
        //----------------------------------------------------------------------------------------------------
        CollisionGroup&         GetCollisionGroup()                                     { return m_collisionGroup; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the collision group and sub-group ID, determines which other objects it collides with. 
        //----------------------------------------------------------------------------------------------------
        void                    SetCollisionGroup(const CollisionGroup& collisionGroup) { m_collisionGroup = collisionGroup; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns whether this Body is Static (not movable).
        //----------------------------------------------------------------------------------------------------
        bool                    IsStatic() const                                        { return m_motionType == BodyMotionType::Static; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns whether this body is Kinematic (or keyframed), which means that it will move
        ///         according to its current velocity, but forces don't affect it.
        //----------------------------------------------------------------------------------------------------
        bool                    IsKinematic() const                                     { return m_motionType == BodyMotionType::Kinematic; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns whether this body is Dynamic, which means it moves and forces act on it. 
        //----------------------------------------------------------------------------------------------------
        bool                    IsDynamic() const                                       { return m_motionType == BodyMotionType::Dynamic; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if this is a RigidBody or not. 
        //----------------------------------------------------------------------------------------------------
        inline bool             IsRigidBody() const                                     { /*Ignoring Soft Body Collision for now.*/ return true; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if this is a Soft Body or not. 
        //----------------------------------------------------------------------------------------------------
        inline bool             IsSoftBody() const                                      { /*Ignoring Soft Body Collision for now.*/ return false; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check whether this body is currently simulating (true) or sleeping (false). 
        //----------------------------------------------------------------------------------------------------
        bool                    IsActive() const                                        { return m_pMotionProperties != nullptr && m_pMotionProperties->m_indexInActiveBodies != kInactiveIndex; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the body's motion type. Static, Kinematic or Dynamic.
        //----------------------------------------------------------------------------------------------------
        BodyMotionType          GetMotionType() const                                   { return m_motionType; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the motion type of this body. 
        //----------------------------------------------------------------------------------------------------
        void                    SetMotionType(const BodyMotionType& motionType); 
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if a body could be made kinematic or dynamic. It would have been created dynamic or
        ///     m_allowDynamicOrKinematic was set to true.
        //----------------------------------------------------------------------------------------------------
        bool                    CanBeKinematicOrDynamic() const         { return m_pMotionProperties != nullptr; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Change this body to a sensor. A sensor will receive collision callbacks, but it will not
        ///     cause any collision responses and can be used as a trigger volume.
        ///     The cheapest sensor (in terms of CPU usage) is a sensor with motion type Static. These sensors
        ///     will only detect collisions with active Dynamic or Kinematic bodies. As soo as they go to sleep,
        ///     the contact point with the sensor will be lost. If you make a sensor Dynamic or Kinematic and
        ///     activate them, the sensor will be able to detect collisions with sleeping bodies too. An active
        ///     sensor will never go to sleep automatically. When you make a Dynamcic or Kinematic sensor, make
        ///     sure that it is in a Collision Layer that does not collide with Static bodies or other sensors to
        ///     avoid extra overhead in the broadphase.
        //----------------------------------------------------------------------------------------------------
        inline void             SetIsSensor(const bool isSensor);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check whether this body is a sensor.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         IsSensor() const                                        { return Internal_GetFlag(Flags::IsSensor); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether kinematic objects can generate contact points against other kinematic or static
        ///     objects. Note that turning this on can be CPU intensive as much more collision detection will be
        ///     done without any effect on the simulation (kinematic objects are not affected by other kinematic/static
        ///     objects). This can be used to make sensors detect static objects. fnote that the sensor must be active
        ///     for it to detect static objects.
        //----------------------------------------------------------------------------------------------------
        inline void             SetCollideKinematicVsNonDynamic(const bool collideKinematicVsNonDynamic);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if kinematic objects can generate contact points against other kinematic or static
        ///     objects.
        //----------------------------------------------------------------------------------------------------
        inline bool             GetCollideKinematicVsNonDynamic() const                 { return Internal_GetFlag(Flags::CollideKinematicVsNonDynamic); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : If PhysicsSettings::m_useManifoldReduction is true, this allows turning off manifold reduction for this specific body.
        /// Manifold reduction by default will combine contacts with similar normals that come from different SubShapeIDs (e.g. different triangles in a mesh shape or different compound shapes).
        /// If the application requires tracking exactly which SubShapeIDs are in contact, you can turn off manifold reduction. Note that this comes at a performance cost.
        /// Consider using BodyInterface::SetUseManifoldReduction if the body could already be in contact with other bodies to ensure that the contact cache is invalidated, and you get the correct contact callbacks.
        //----------------------------------------------------------------------------------------------------
        inline void             SetUseManifoldReduction(bool useReduction);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if this body can use manifold reduction. 
        //----------------------------------------------------------------------------------------------------
        inline bool             GetUseManifoldReduction() const                         { return Internal_GetFlag(Flags::UseManifoldReduction); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set to indicate that gyroscopic force should be applied to this body.
        ///     (aka Dzhanibekov effect, see https://en.wikipedia.org/wiki/Tennis_racket_theorem)
        //----------------------------------------------------------------------------------------------------
        inline void             SetApplyGyroscopicForce(const bool apply);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if gyroscopic force is being applied to this body. 
        //----------------------------------------------------------------------------------------------------
        inline bool             GetApplyGyroscopicForce() const                         { return Internal_GetFlag(Flags::ApplyGyroscopicForce); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set to indicate that extra effort should be made to try to remove ghost contacts (collisions
        ///     with internal edges of a mesh). This is more expensive, but makes bodies move smoother over a
        ///     mesh with convex edges.
        //----------------------------------------------------------------------------------------------------
        inline void             SetEnhancedInternalEdgeRemoval(const bool apply);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if enhanced internal edge removal is turned on
        //----------------------------------------------------------------------------------------------------
        inline bool				GetEnhancedInternalEdgeRemoval() const                  { return Internal_GetFlag(Flags::EnhancedInternalEdgeRemoval); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the combination of this body and body2 should use enhanced internal edge removal.
        //----------------------------------------------------------------------------------------------------
        inline bool             GetEnhancedInternalEdgeRemovalWithBody(const Body& body2) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if this body can go to sleep. Note that disabling sleeping on a sleeping object will
        ///     not wake it up directly.
        //----------------------------------------------------------------------------------------------------
        inline bool             CanSleep() const                                        { return m_pMotionProperties->m_canSleep; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether this body can go to sleep. Note that disabling sleeping on a sleeping object will
        ///     not wake it up directly.
        //----------------------------------------------------------------------------------------------------
        void                    SetCanSleep(const bool canSleep);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Resets the sleep timer. This does not wake up the body if it is sleeping, but allows
        ///     resetting the system that detects when a body is sleeping.
        //----------------------------------------------------------------------------------------------------
        inline void             ResetSleepTimer();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Friction is usually between [0, 1], where 0 = no friction and 1 = friction
        ///     force equals the force that presses the two bodies together.
        ///     Note that bodies can have negative friction but the combined friction should never go below zero.
        //----------------------------------------------------------------------------------------------------
        inline float            GetFriction() const                                     { return m_friction; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Friction is usually set between [0, 1], where 0 = no friction and 1 = friction
        ///     force equals the force that presses the two bodies together.
        ///     Note that bodies can have negative friction but the combined friction should never go below zero.
        //----------------------------------------------------------------------------------------------------
        void                    SetFriction(const float friction)                       { m_friction = friction; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Restitution is usually between [0, 1], where 0 = completely non-elastic collision response
        ///     and 1 = completely elastic collision response.
        ///     Note that bodies can have negative restitution but the combined restitution should never go below zero.
        //----------------------------------------------------------------------------------------------------
        inline float            GetRestitution() const                                  { return m_restitution; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Restitution is usually between [0, 1], where 0 = completely non-elastic collision response
        ///     and 1 = completely elastic collision response.
        ///     Note that bodies can have negative restitution but the combined restitution should never go below zero.
        //----------------------------------------------------------------------------------------------------
        inline void             SetRestitution(const float restitution)                 { m_restitution = restitution; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the world space linear velocity of the center of mass (unit m/s). 
        //----------------------------------------------------------------------------------------------------
        inline Vector3          GetLinearVelocity() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the world space linear velocity of the center of mass (unit m/s). 
        //----------------------------------------------------------------------------------------------------
        inline void             SetLinearVelocity(const Vector3& linearVelocity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set world space linear velocity of the center of mass; will make sure the value is clamped
        ///     against the maximum linear velocity, (unit m/s).
        //----------------------------------------------------------------------------------------------------
        inline void             SetLinearVelocityClamped(const Vector3& linearVelocity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the world space angular velocity of the center of mass (unit rad/s).
        //----------------------------------------------------------------------------------------------------
        inline Vector3          GetAngularVelocity() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the world space angular velocity of the center of mass (unit rad/s).  
        //----------------------------------------------------------------------------------------------------
        inline void             SetAngularVelocity(const Vector3& angularVelocity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the world space angular velocity of the center of mass; will make sure the value is clamped
        ///     against the maximum angular velocity, (unit rad/s).
        //----------------------------------------------------------------------------------------------------
        inline void             SetAngularVelocityClamped(const Vector3& angularVelocity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the velocity of point (in center of mass space, e.g. on the surface of the body)
        ///         of the body (unit: m/s).
        //----------------------------------------------------------------------------------------------------
        inline Vector3          GetPointVelocityCOM(const Vector3& pointRelativeToCOM) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the velocity of point (in world space, e.g. on the surface of the body)
        ///         of the body (unit: m/s).
        //----------------------------------------------------------------------------------------------------
        inline Vector3          GetPointVelocity(const Vector3& point) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add force (unit: N) at the center of mass for the next time step. This will be reset after
        ///     the next call to PhysicsSystem::Update().
        ///     If you want to wake up the body when it is sleeping, use BodyInterface::AddForce instead.
        //----------------------------------------------------------------------------------------------------
        inline void             AddForce(const Vector3& force);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add force (unit: N) at "position" for the next time step, will be reset after the next call
        ///     to PhysicsSystem::Update().
        ///     If you want the body to wake up when it is sleeping, use BodyInterface::AddForce instead.
        //----------------------------------------------------------------------------------------------------
        inline void             AddForce(const Vector3& force, const Vector3& position);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the total amount of force applied to the center of mass this time step (through AddForce()
        ///     calls). Note that it will reset to zero after PhysicsSystem::Update.
        //----------------------------------------------------------------------------------------------------
        inline Vector3          GetAccumulatedForce() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add torque (unit: N m) for the next time step, will be reset after the next call to
        ///     PhysicsSystem::Update().
        ///     If you want the body to wake up when it is sleeping, use BodyInterface::AddTorque instead.
        //----------------------------------------------------------------------------------------------------
        inline void             AddTorque(const Vector3& torque);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the total amount of torque applied to the center of mass this time step (through AddTorque()
        ///     calls). Note that it will reset to zero after PhysicsSystem::Update.
        //----------------------------------------------------------------------------------------------------
        inline Vector3          GetAccumulatedTorque() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Add an impulse to the center of mess (unit: kg m/s).
        ///     If you want the body to wake up when it is sleeping, use BodyInterface::AddImpulse instead.
        //----------------------------------------------------------------------------------------------------
        inline void             AddImpulse(const Vector3& impulse);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add an impulse to "position" in world space (unit: kg m/s).
        ///     If you want the body to wake up when it is sleeping, use BodyInterface::AddImpulse instead.
        //----------------------------------------------------------------------------------------------------
        inline void             AddImpulse(const Vector3& impulse, const Vector3& position);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add angular impulse to this body in world space (unit: N m s).
        ///     If you want the body to wake up when it is sleeping, use BodyInterface::AddAngularImpulse instead.
        //----------------------------------------------------------------------------------------------------
        inline void             AddAngularImpulse(const Vector3& angularImpulse);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reset the total accumulated force, not that this will be done automatically after every time
        ///     step.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         ResetForce();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reset the total accumulated torque, not that this will be done automatically after every
        ///     time step.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         ResetTorque();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reset the current velocity and the accumulated force and torque.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         ResetMotion();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get inverse inertia tensor in world space.
        //----------------------------------------------------------------------------------------------------
        inline Mat4             GetInverseInertia() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the velocity of the Body such that it will be positioned at targetPosition/targetRotation
        ///     in deltaTime seconds.
        //----------------------------------------------------------------------------------------------------
        void                    MoveKinematic(const Vector3& targetPosition, const Quat& targetRotation, float deltaTime);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check to see if this Body has been added to the physics system. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         IsInBroadPhase() const                  { return Internal_GetFlag(Flags::IsInBroadPhase); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check to see if this Body has been changed in such a way that the collision cache should
        ///     be considered invalid for any Body interacting with this Body.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         IsCollisionCacheInvalid() const         { return Internal_GetFlag(Flags::InvalidateContactCache); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the shape of this body. 
        //----------------------------------------------------------------------------------------------------
        inline const Shape*     GetShape() const                        { return m_pShape; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates the world transform for this body.
        //----------------------------------------------------------------------------------------------------
        inline Mat4             GetWorldTransform() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the world space position of this body's center of mass. 
        //----------------------------------------------------------------------------------------------------
        inline Vector3          GetCenterOfMassPosition() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get Calculates the world space transform for this body's center of mass. 
        //----------------------------------------------------------------------------------------------------
        inline Mat4             GetCenterOfMassTransform() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates the inverse of the transform for this body's center of mass. 
        //----------------------------------------------------------------------------------------------------
        inline Mat4             GetInverseCenterOfMassTransform() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the motion properties of this Body. This is only valid if the Body is not Static.
        //----------------------------------------------------------------------------------------------------
        inline const MotionProperties* GetMotionProperties() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the motion properties of this Body. This is only valid if the Body is not Static.
        //----------------------------------------------------------------------------------------------------
        inline MotionProperties*       GetMotionProperties();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the motion properties of this Body. This will not make sure the Body is not Static.
        //----------------------------------------------------------------------------------------------------
        const MotionProperties* GetMotionPropertiesUnchecked() const { return m_pMotionProperties; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the motion properties of this Body. This will not make sure the Body is not Static.
        //----------------------------------------------------------------------------------------------------
        MotionProperties*       GetMotionPropertiesUnchecked()       { return m_pMotionProperties; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the user data, which can be used for anything by the Application. 
        //----------------------------------------------------------------------------------------------------
        uint64_t                GetUserData() const                  { return m_userData; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the user data, which can be used for anything by the Application. 
        //----------------------------------------------------------------------------------------------------
        void                    SetUserData(const uint64_t userData) { m_userData = userData; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the surface normal of a particular sub shape and its world space surface position on
        ///     the body.
        //----------------------------------------------------------------------------------------------------
        inline Vector3          GetWorldSpaceSurfaceNormal(const SubShapeID& subShapeID, const Vector3& position) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the transformed shape of the body, which can be used to do collision detection outside
        ///     of a body lock.
        //----------------------------------------------------------------------------------------------------
        inline TransformedShape GetTransformedShape() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Debug function to convert a body back to a body create info object to be able to save/recreate
        ///     the body later.
        //----------------------------------------------------------------------------------------------------
        BodyCreateInfo          GetBodyCreateInfo() const;

        // [TODO]: SoftBody version of above.
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper function for BroadPhase::FindCollidingPairs that returns true when two bodies can
        ///     collide. It assumes that body 1 is dynamic and active and guarantees that if body1 collides with
        ///     body2, then body2 will not collide with body1 in order to prevent finding duplicate collisions.
        //----------------------------------------------------------------------------------------------------
        static inline bool      Internal_FindCollidingPairsCanCollide(const Body& body1, const Body& body2);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add position using an Euler step (used during position integrate & constraint solving). 
        //----------------------------------------------------------------------------------------------------
        inline void             Internal_AddPositionStep(const Vector3& linearVelocityTimesDeltaTime);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Subtract position using an Euler step (used during position integrate & constraint solving).
        //----------------------------------------------------------------------------------------------------
        inline void             Internal_SubPositionStep(const Vector3& linearVelocityTimesDeltaTime);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add rotation using an Euler step (used during position integrate & constraint solving).
        //----------------------------------------------------------------------------------------------------
        inline void             Internal_AddRotationStep(const Vector3& angularVelocityTimesDeltaTime);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Subtract rotation using an Euler step (used during position integrate & constraint solving).
        //----------------------------------------------------------------------------------------------------
        inline void             Internal_SubRotationStep(const Vector3& angularVelocityTimesDeltaTime);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether this Body is in the Broadphase.
        /// @note : Should only be called by the BroadPhase!
        //----------------------------------------------------------------------------------------------------
        inline void             Internal_SetInBroadPhase(const bool isInBroadPhase);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Invalidate the contact cache (should only be called by the BodyManager), will be reset
        ///     on the next simulation step. Returns true if the contact cache was still valid.
        //----------------------------------------------------------------------------------------------------
        inline bool             Internal_InvalidateContactCache();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reset the collision cache invalid flag (should only be called by the BodyManager).
        //----------------------------------------------------------------------------------------------------
        inline void             Internal_ValidateContactCache();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Update the world space bounding box (should only be called by the physics system 
        //----------------------------------------------------------------------------------------------------
        void                    Internal_CalculateWorldSpaceBounds();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Function to update body's position (should only be called by the BodyInterface since it
        ///     also requires updating the broadphase)
        //----------------------------------------------------------------------------------------------------
        void                    Internal_SetPositionAndRotation(const Vector3& position, const Quat& rotation, bool resetSleepTimer = true);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Updates the center of mass and optionally mass properties after shifting the center of mass
        ///     or changes to the shape (should only be called by the BodyInterface since it also requires updating the broadphase)
        ///	@param previousCenterOfMass : Center of Mass of the shape before alterations.
        ///	@param updateMassProperties : When true, the mass and inertia tensor is recalculated.
        //----------------------------------------------------------------------------------------------------
        void                    Internal_UpdateCenterOfMass(const Vector3& previousCenterOfMass, bool updateMassProperties);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Function to update a body's shape (should only be called by the BodyInterface since it
        ///     also requires updating the broadphase)
        ///	@param pShape : The new shape for this body. 
        ///	@param updateMassProperties : When true, the mass and inertia tensor is recalculated.
        //----------------------------------------------------------------------------------------------------
        void                    Internal_SetShape(const Shape* pShape, bool updateMassProperties);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the index in the BodyManager::m_activeBodies array. 
        //----------------------------------------------------------------------------------------------------
        uint32_t                Internal_GetIndexInActiveBodies() const     { return m_pMotionProperties != nullptr ? m_pMotionProperties->m_indexInActiveBodies : kInactiveIndex; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Update eligibility for sleeping. 
        //----------------------------------------------------------------------------------------------------
        AllowedSleep            Internal_UpdateSleepState(const float deltaTime, float maxMovement, float timeBeforeSleep);

#ifdef NES_LOGGING_ENABLED
        inline void             Internal_ValidateCachedBounds() const;
#endif

    private:
        inline void             Internal_SetFlag(const Flags flag, bool set);
        inline bool             Internal_GetFlag(const Flags flag) const;
        inline void             GetSleepTestPoints(Vector3* outPoints) const;
    };
}

#include "Body.inl"