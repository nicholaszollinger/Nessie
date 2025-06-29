// MotionProperties.h
#pragma once
#include "BodyAccess.h"
#include "DOF.h"
#include "MotionQuality.h"
#include "MotionType.h"
#include "Math/Quat.h"
#include "Math/Mat44.h"
#include "Geometry/Sphere.h"

namespace nes
{
    struct MassProperties;

    enum class ECanSleep : uint8_t
    {
        CannotSleep = 0,    /// Object cannot go to sleep
        CanSleep = 1,       // Object can go to sleep
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Motion Properties contains all the state for Dynamic Bodies. By default, only data needed
    ///     for static bodies are present on the Body, with a pointer to an allocated MotionProperties
    ///     object if dynamic.
    //----------------------------------------------------------------------------------------------------
    class MotionProperties
    {
        friend class Body;
        friend class BodyManager;
        
    public:
        static constexpr uint32_t kInactiveIndex = std::numeric_limits<uint32_t>::max();

        MotionProperties() = default;

    private:
        // 1st Cache line.
        Vec3                m_linearVelocity        = Vec3::Zero();          /// World space linear velocity of the center of mass (m/s)
        Vec3                m_angularVelocity       = Vec3::Zero();          /// World space angular velocity (rad/s)
        Vec3                m_inverseInertiaDiagonal = Vec3::Zero();         /// Diagonal of inverse inertia matrix: (D)
        Quat                m_inertiaRotation       = Quat::Identity();         /// Rotation (R) that takes inverse inertia diagonal to local space: IBody^-1 = R * D * R^-1

        // 2nd Cache line
        Vec3                m_force                 = Vec3::Zero();          /// Accumulated world space force (N). Note loaded through intrinsics so ensure that the 4 bytes after this are readable!
        Vec3                m_torque                = Vec3::Zero();          /// Accumulated world space torgue (N m). Note loaded through intrinsics so ensure that the 4 bytes after this are readable!
        float               m_inverseMass           = 0.f;                      /// Inverse mass of the object (1/kg).
        float               m_linearDamping         = 0.f;                      /// Linear damping: dv/dt = -c * v. c must be between [0, 1] but is usually close to 0.
        float               m_angularDamping        = 0.f;                      /// Angular damping: dw/dt = -c * w. c must be between [0, 1] but is usually close to 0.
        float               m_maxLinearVelocity     = 0.f;                      /// Maximum linear velocity that this body can reach (m/s).
        float               m_maxAngularVelocity    = 0.f;                      /// Maximum angular velocity that this body can reach (rad/s).
        float               m_gravityScale          = 1.f;                       /// Factor to multiply gravity with.
        uint32_t            m_indexInActiveBodies   = kInactiveIndex;           /// If the body is active, this is the index in the active body list or kInactiveIndex if it is not active.
        uint32_t            m_islandIndex           = kInactiveIndex;           /// Index of the Island that this body is a part of. When the body has not yet been updated or is not active, this equals kInactiveIndex.

        EBodyMotionQuality   m_motionQuality        = EBodyMotionQuality::Discrete;  /// Motion quality, or how well it detects collisions at high velocity.
        bool                m_canSleep              = true;                          /// If this body can go to sleep.
        EAllowedDOFs         m_allowedDoFs          = EAllowedDOFs::All;             /// Allowed degrees of freedom for this body.
        uint8_t             m_numVelocityStepsOverride = 0;                          /// Used only when this Body is dynamic and colliding. Override for the number of solver velocity iterations to run, 0 means use the default in PhysicsSettings::mNumVelocitySteps. The number of iterations to use is the max of all contacts and constraints in the island.
        uint8_t             m_numPositionStepsOverride = 0;                          /// Used only when this Body is dynamic and colliding. Override for the number of solver position iterations to run, 0 means use the default in PhysicsSettings::mNumVelocitySteps. The number of iterations to use is the max of all contacts and constraints in the island.

        // 3rd Cache line - Not used often.
        Sphere              m_sleepTestSpheres[3]{};                             /// Measure motion for 3 points on the body to see if it is resting: COM, COM + largest bounding box axis, COM + second largest bounding box axis.
        float               m_sleepTestTimer = 0.f;                              /// How long this body has been within the movement tolerance.

#if NES_LOGGING_ENABLED
        EBodyMotionType  m_cachedMotionType = EBodyMotionType::Static;
        // [TODO]:  RigidBody vs SoftBody, I am going to assume rigidbody for now.
        //BodyType        m_cachedBodyType;
#endif

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Motion quality, or how well it detects collisions at high velocity.
        //----------------------------------------------------------------------------------------------------
        inline EBodyMotionQuality   GetMotionQuality() const                                { return m_motionQuality; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allowed degrees of freedom for this body. This can be changed by calling SetMassProperties
        //----------------------------------------------------------------------------------------------------
        inline EAllowedDOFs         GetAllowedDOFs() const                                  { return m_allowedDoFs; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : If this body can go to sleep.
        //----------------------------------------------------------------------------------------------------
        inline bool                 GetCanSleep() const                                     { return m_canSleep; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get world space linear velocity of the center of mass (m/s)  
        //----------------------------------------------------------------------------------------------------
        inline Vec3                 GetLinearVelocity() const;                             

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set world space linear velocity of the center of mass (m/s)
        //----------------------------------------------------------------------------------------------------
        inline void                 SetLinearVelocity(const Vec3& linearVelocity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set world space linear velocity of the center of mass (m/s), and makes sure that the value
        ///     is clamped to the max linear velocity.
        //----------------------------------------------------------------------------------------------------
        inline void                 SetLinearVelocityClamped(const Vec3& linearVelocity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the world space angular velocity of the center of mass.
        //----------------------------------------------------------------------------------------------------
        inline Vec3                 GetAngularVelocity() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the world space angular velocity of the center of mass.
        //----------------------------------------------------------------------------------------------------
        inline void                 SetAngularVelocity(const Vec3& angularVelocity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the world space angular velocity of the center of mass, and makes sure that the value
        ///     is clamped against the maximum angular velocity.
        //----------------------------------------------------------------------------------------------------
        inline void					SetAngularVelocityClamped(const Vec3 angularVelocity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set velocity of the body such that it will be rotated/translated by deltaRot/deltaPos in
        ///         deltaTime seconds.
        //----------------------------------------------------------------------------------------------------
        inline void                 MoveKinematic(const Vec3& deltaPos, const Quat& deltaRot, const float deltaTime);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the maximum linear velocity that a body can achieve. Used to prevent the system from exploding. 
        //----------------------------------------------------------------------------------------------------
        inline float                GetMaxLinearVelocity() const                        { return m_maxLinearVelocity; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the maximum linear velocity that a body can achieve. Used to prevent the system from exploding.
        //----------------------------------------------------------------------------------------------------
        inline void                 SetMaxLinearVelocity(const float maxLinearVelocity);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the maximum angular velocity that a body can achieve. Used to prevent the system from exploding. 
        //----------------------------------------------------------------------------------------------------
        inline float                GetMaxAngularVelocity() const                        { return m_maxAngularVelocity; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the maximum angular velocity that a body can achieve. Used to prevent the system from exploding.
        //----------------------------------------------------------------------------------------------------
        inline void                 SetMaxAngularVelocity(const float maxAngularVelocity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Clamp linear velocity according to its max. 
        //----------------------------------------------------------------------------------------------------
        inline void                 ClampLinearVelocity();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Clamp angular velocity according to its max. 
        //----------------------------------------------------------------------------------------------------
        inline void                 ClampAngularVelocity();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get linear damping: dv/dt = -c * v. Value must be between [0, 1] but is usually close to 0. 
        //----------------------------------------------------------------------------------------------------
        inline float                GetLinearDamping() const                            { return m_linearDamping; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set linear damping: dv/dt = -c * v. Value must be between [0, 1] but is usually close to 0. 
        //----------------------------------------------------------------------------------------------------
        inline void                 SetLinearDamping(const float linearDamping);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get angular damping: dw/dt = -c * w. Value must be between [0, 1] but is usually close to 0.
        //----------------------------------------------------------------------------------------------------
        inline float                GetAngularDamping() const                            { return m_angularDamping; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set angular damping: dv/dt = -c * w. Value must be between [0, 1] but is usually close to 0. 
        //----------------------------------------------------------------------------------------------------
        inline void                 SetAngularDamping(const float angularDamping);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the value to scale gravity by. (1 = normal gravity, 0 = no gravity).  
        //----------------------------------------------------------------------------------------------------
        inline float                GetGravityScale() const                             { return m_gravityScale; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the value to scale gravity by. (1 = normal gravity, 0 = no gravity).
        //----------------------------------------------------------------------------------------------------
        inline void                 SetGravityScale(const float scale)                  { m_gravityScale = scale; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the mass and inertia tensor.
        //----------------------------------------------------------------------------------------------------
        void                        SetMassProperties(const EAllowedDOFs allowedDoFs, const MassProperties& massProperties);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get inverse mass (1 / mass). Should only be called on a dynamic object (static or kinematic
        ///     bodies have infinite mass so should be treated as 1 / mass = 0). Asserts that is body is dynamic.
        //----------------------------------------------------------------------------------------------------
        inline float                GetInverseMass() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get inverse mass (1 / mass). Should only be called on a dynamic object (static or kinematic
        ///     bodies have infinite mass so should be treated as 1 / mass = 0).
        //----------------------------------------------------------------------------------------------------
        inline float                GetInverseMassUnchecked() const                     { return m_inverseMass; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the inverse mass (1 / mass).
        /// @note : Mass and inertia are linearly related. If you change mass, inertia will probably need to
        ///     change as well. You can use ScaleToMass() to update mass and inertia at the same time. If
        ///     all your translation degrees of freedom are restricted, make sure this is zero (see AllowedDOFs).
        //----------------------------------------------------------------------------------------------------
        void                        SetInverseMass(const float inverseMass)             { m_inverseMass = inverseMass; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the diagonal of the inertia matrix: "D".
        /// Should only be called on a dynamic object (static or kinematic bodies have infinite mass so should be treated as D = 0)
        //----------------------------------------------------------------------------------------------------
        inline Vec3                 GetInverseInertiaDiagonal() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Rotation (R) that takes inverse inertia diagonal to local space.
        //----------------------------------------------------------------------------------------------------
        inline Quat                 GetInertiaRotation() const                          { return m_inertiaRotation; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the inverse inertia tensor in local space by setting the diagonal and the rotation.
        /// @note : Mass and inertia are linearly related. If you change mass, inertia will probably need to
        ///     change as well. You can use ScaleToMass() to update mass and inertia at the same time. If
        ///     all your translation degrees of freedom are restricted, make sure this is zero (see AllowedDOFs).
        //----------------------------------------------------------------------------------------------------
        inline void                 SetInverseInertia(const Vec3& diagonal, const Quat& inertiaRotation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sets this body's mass to "mass" and scales the inertia tensor based on the ratio between
        ///     the old and new mass.
        ///	@note : This only works when the current mass is finite (i.e. the body is dynamic and translational
        ///     degrees of freedom are not restricted).
        //----------------------------------------------------------------------------------------------------
        inline void                 ScaleToMass(float mass);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the inverse inertia matrix. This will be a matrix of zeros for a static/kinematic object. 
        //----------------------------------------------------------------------------------------------------
        inline Mat44                GetLocalSpaceInverseInertia() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Same as GetLocalSpaceInverseInertia() but does not assert that the body is dynamic.
        //----------------------------------------------------------------------------------------------------
        inline Mat44                GetLocalSpaceInverseInertiaUnchecked() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the inverse inertia matrix for a given object rotation (translation will be ignored).
        ///     Zero if the object is static/kinematic.
        //----------------------------------------------------------------------------------------------------
        inline Mat44			    GetInverseInertiaForRotation(const Mat44& rotation) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Multiply a vector with the inverse world space inertia tensor. Zero if the object is
        ///     static/kinematic.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3             MultiplyWorldSpaceInverseInertiaByVector(const Quat& bodyRotation, const Vec3& vec) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the velocity of "pointRelativeToCOM" (in center of mass space, e.g. on the surface of the
        ///     body) of the body (unit m/s).  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3             GetPointVelocityCOM(const Vec3& pointRelativeToCOM) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the total amount of force applied to the center of mass this time step.
        /// @note : This wil be reset to zero after PhysicsSystem::Update().
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3             GetAccumulatedForce() const                             { return m_force; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the total amount of torque applied to the center of mass this time step.
        /// @note : This wil be reset to zero after PhysicsSystem::Update().
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3             GetAccumulatedTorque() const                            { return m_torque; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reset the total accumulated force. This will be done automatically after every time step.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             ResetForce()                                            { m_force = Vec3::Zero(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reset the total accumulated torque. This will be done automatically after every time step.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             ResetTorque()                                           { m_torque = Vec3::Zero(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reset the current velocity and accumulated force and torque. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void             ResetMotion();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a vector where the linear components that are not allowed by m_allowedDOFs are set
        ///     to zero and the rest to 0xffffffff.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE UVec4Reg         GetLinearDOFsMask() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Takes a translation vector "vec" and returns the same vector but with the components that are not
        ///     allowed to change by m_allowedDOFs are set to 0.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3             LockTranslation(const Vec3& vec) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a vector where the angular components that are not allowed by m_allowedDOFs are set
        ///     to zero and the rest to 0xffffffff.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE UVec4Reg         GetAngularDOFsMask() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Takes an angular velocity/torque vector "vec" and returns the same vector but with the
        ///     components that are not allowed to change by m_allowedDOFs are set to 0.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3             LockAngular(const Vec3& vec) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Used only when this body is dynamic and colliding. Override for the number of solver velocity
        ///     iterations to run, 0 means use the default in PhysicsSettings::m_numVelocitySteps. The number
        ///     of iterations to use is the max of all contacts and constraints in the island.
        //----------------------------------------------------------------------------------------------------
        inline uint32_t             GetNumVelocityStepsOverride() const                     { return m_numVelocityStepsOverride; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Used only when this body is dynamic and colliding. Override for the number of solver velocity
        ///     iterations to run, 0 means use the default in PhysicsSettings::m_numVelocitySteps. The number
        ///     of iterations to use is the max of all contacts and constraints in the island.
        //----------------------------------------------------------------------------------------------------
        inline void                 SetNumVelocityStepsOverride(const uint32_t numSteps);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Used only when this body is dynamic and colliding. Override for the number of solver position
        ///     iterations to run, 0 means use the default in PhysicsSettings::m_numPositionSteps. The number
        ///     of iterations to use is the max of all contacts and constraints in the island.
        //----------------------------------------------------------------------------------------------------
        inline uint32_t             GetNumPositionStepsOverride() const                     { return m_numPositionStepsOverride; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Used only when this body is dynamic and colliding. Override for the number of solver position
        ///     iterations to run, 0 means use the default in PhysicsSettings::m_numPositionSteps. The number
        ///     of iterations to use is the max of all contacts and constraints in the island.
        //----------------------------------------------------------------------------------------------------
        inline void                 SetNumPositionStepsOverride(const uint32_t numSteps);

        //----------------------------------------------------------------------------------------------------
        // FUNCTIONS BELOW ARE FOR INTERNAL USE ONLY.
        //----------------------------------------------------------------------------------------------------

        inline void                 Internal_AddLinearVelocityStep(const Vec3& linearVelocityChange);
        inline void                 Internal_SubLinearVelocityStep(const Vec3& linearVelocityChange);
        inline void                 Internal_AddAngularVelocityStep(const Vec3& angularVelocityChange);
        inline void                 Internal_SubAngularVelocityStep(const Vec3& angularVelocityChange);
        inline void                 Internal_ApplyGyroscopicForce(const Quat& bodyRotation, const float deltaTime);
        inline void                 Internal_ApplyForceTorqueAndDrag(const Quat& bodyRotation, const Vec3& gravity, const float deltaTime);

        inline uint32_t             Internal_GetIslandIndex() const                         { return m_islandIndex; }
        inline void                 Internal_SetIslandIndex(const uint32_t islandIndex)     { m_islandIndex = islandIndex; }
        inline uint32_t             Internal_GetIndexInActiveBodies() const                 { return m_indexInActiveBodies; }
        
        inline void                 Internal_ResetSleepTestSpheres(const Vec3* pPoints);
        inline void                 Internal_ResetSleepTestTimer()                          { m_sleepTestTimer = 0.f; }
        inline ECanSleep            Internal_AccumulateSleepTime(float deltaTime, float timeBeforeSleep);
        
    };
}

#include "MotionProperties.inl"