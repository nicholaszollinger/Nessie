// BodyCreateInfo.h
#pragma once
#include "Core/Memory/StrongPtr.h"
#include "Physics/Body/DOF.h"
#include "Physics/Body/MotionQuality.h"
#include "Physics/Body/MotionType.h"
#include "Physics/Collision/CollisionGroup.h"
#include "Physics/Collision/CollisionLayer.h"
#include "Physics/Collision/Shapes/Shape.h"

namespace nes
{
    enum class EOverrideMassProperties : uint8_t
    {
        CalculateMassAndInertia,    /// Tells the system to calculate the mass and inertia based on density.
        CalculateInertia,           /// Tells the system to take the mass form m_massPropertiesOverride and to calculate the inertia based on the density of shapes and to scale it to the provided mass.
        MassAndInertiaProvided,     /// Tells the system to take the mass and inertia from m_massPropertiesOverride.
    };
    
    //----------------------------------------------------------------------------------------------------
    // [TODO]: Serialization functions.
    /// @brief : Struct containing the initial settings used to create a Body. 
    //----------------------------------------------------------------------------------------------------
    struct BodyCreateInfo
    {
        Vec3                    m_position                      = Vec3::Zero();                      /// Position of the body (not center of mass).
        Quat                    m_rotation                      = Quat::Identity();                     /// Rotation of the body
        Vec3                    m_linearVelocity                = Vec3::Zero();                      /// World space linear velocity of the center of mass (m/s). 
        Vec3                    m_angularVelocity               = Vec3::Zero();                      /// World space angular velocity (rad/s).
        
        uint64_t                m_userData                      = 0;                                    /// User data value.
        
        CollisionLayer          m_collisionLayer                = 0;                                    /// Collision layer this body belongs to (determines if two objects can collide).
        CollisionGroup          m_collisionGroup;                                                       /// Collision group this body belongs to (determines if two objects can collide).
        
        EBodyMotionType         m_motionType                    = EBodyMotionType::Static;               /// Motion type, determines if the object is static, dynamic, or kinematic. 
        EAllowedDOFs            m_allowedDOFs                   = EAllowedDOFs::All;                     /// Which degrees of freedom this body has (can be used to limit simulation to 2D).
        EBodyMotionQuality       m_motionQuality                 = EBodyMotionQuality::Discrete;          /// Motion Quality, or how well it detects collisions when it has a high velocity.
        float                   m_gravityScale                  = 1.0f;                                 /// Value to multiply gravity with for this body
        float                   m_maxLinearVelocity             = 500.f;                                /// Maximum linear velocity that this body can reach (m/s).        
        float                   m_maxAngularVelocity            = 0.25f * math::Pi<float>() * 60.f;     /// Maximum angular velocity that this body can reach (rad/s).
        float                   m_friction                      = 0.2f;                                 /// Friction of the body (usually between [0, 1], where 0 = no friction and 1 = friction force equals force that presses the two bodies together).
        float                   m_restitution                   = 0.0f;                                 /// Restitution of the body (usually between [0, 1], where 0 = completely inelastic collision response and 1 = completely elastic collision response).
        float                   m_linearDamping                 = 0.05f;                                /// Linear damping: dv/dt = -c * v. c must be between 0 and 1 but is usually close to 0.
        float                   m_angularDamping                = 0.05f;                                /// Angular damping: dw/dt = -c * w. c must be between 0 and 1 but is usually close to 0.
        uint32_t                m_numVelocityStepsOverride      = 0;                                    /// Used only when this body is dynamic and colliding. Override for the number of solver velocity iterations to run, 0 means use the default in PhysicsSettings::mNumVelocitySteps. The number of iterations to use is the max of all contacts and constraints in the island.
        uint32_t                m_numPositionStepsOverride      = 0;                                    /// Used only when this body is dynamic and colliding. Override for the number of solver position iterations to run, 0 means use the default in PhysicsSettings::mNumPositionSteps. The number of iterations to use is the max of all contacts and constraints in the island.
        bool                    m_isSensor                      = false;                                /// If this body is a sensor. A sensor will receive collision callbacks, but will not cause any collision responses and can be used as a trigger volume. See description at Body::SetIsSensor.
        bool                    m_allowSleeping                 = true;                                 /// If this body can go to sleep or not.
        bool                    m_allowDynamicOrKinematic       = false;                                /// When this body is created as static, this setting tells the system to create a MotionProperties object so that the object can be switched to kinematic or dynamic.
        bool                    m_collideKinematicVsNonDynamic  = false;                                /// If kinematic objects can generate contact points against other kinematic or static objects. See description at Body::SetCollideKinematicVsNonDynamic.
        bool                    m_enhancedInternalEdgeRemoval   = false;                                /// Set to indicate that extra effort should be made to try to remove ghost contacts (collisions with internal edges of a mesh). This is more expensive but makes bodies move smoother over a mesh with convex edges.
        bool                    m_useManifoldReduction          = true;                                 /// If this body should use manifold reduction (see description at Body::SetUseManifoldReduction)
        bool                    m_applyGyroscopicForce          = false;                                /// Set to indicate that the gyroscopic force should be applied to this body (aka Dzhanibekov effect, see https://en.wikipedia.org/wiki/Tennis_racket_theorem)

        EOverrideMassProperties m_overrideMassProperties        = EOverrideMassProperties::CalculateMassAndInertia; /// Determines how m_massPropertiesOverride will be used
        float                   m_inertiaMultiplier             = 1.0f;                                 /// When calculating the inertia (not when it is provided) the calculated inertia will be multiplied by this value.
        MassProperties          m_massPropertiesOverride;                                               /// Contains replacement mass settings which override the automatically calculated values.

    private:
        ConstStrongPtr<ShapeSettings>   m_shapeSettings;
        ConstStrongPtr<Shape>           m_shape;

    public:
        BodyCreateInfo() = default;
        BodyCreateInfo(const ShapeSettings* pSettings, const Vec3& position, const Quat& rotation, EBodyMotionType motionType, CollisionLayer layer);
        BodyCreateInfo(const Shape* pShape, const Vec3& position, const Quat& rotation, EBodyMotionType motionType, CollisionLayer layer);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get access to the shape settings object. This contains serializable (non-runtime optimized)
        ///     information about the shape.
        //----------------------------------------------------------------------------------------------------
        const ShapeSettings*    GetShapeSettings() const                    { return m_shapeSettings.Get(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the shape settings object. This contains serializable (non-runtime optimized)
        ///     information about the shape.
        //----------------------------------------------------------------------------------------------------
        void                    SetShapeSettings(const StrongPtr<ShapeSettings>& pSettings);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get access to the runtime shape object. Will convert from ShapeSettings object if needed. 
        //----------------------------------------------------------------------------------------------------
        const Shape*            GetShape() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the runtime shape object. 
        //----------------------------------------------------------------------------------------------------
        void                    SetShape(const Shape* pShape);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Convert current ShapeSettings into a Shape. This will free the Shape Settings object and
        ///     make the object ready for runtime. Serialization is no longer possible after this.
        //----------------------------------------------------------------------------------------------------
        Shape::ShapeResult      ConvertShapeSettings();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the mass properties of this body will be calculated (Only relevant for kinematic
        ///     or dynamic objects that need a MotionProperties object).
        //----------------------------------------------------------------------------------------------------
        bool                    HasMassProperties() const                   { return m_allowDynamicOrKinematic || m_motionType != EBodyMotionType::Static; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate (or return when overriden) the mass and inertia for the body. 
        //----------------------------------------------------------------------------------------------------
        MassProperties          GetMassProperties() const;
    };
}