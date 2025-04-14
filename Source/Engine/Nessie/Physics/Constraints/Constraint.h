// Constraint.h
#pragma once
#include <cstdint>
#include "Math/Vector3.h"

namespace nes
{
    class BodyID;
    
    enum class ConstraintType
    {
        SingleBodyConstraint, // Constraint that is applied to a single body.
        TwoBodyConstraint,    // Constraint that is applied to two connected bodies.
    };

    enum class ConstraintSubType
    {
        Fixed,
        Point,
        Hinge,
        Slider,
        Distance,
        // ..
    };

    enum class ConstraintSpace
    {
        LocalToBodyCOM, /// All constraint properties are specified in local space to center of mass of the bodies that are being constrained (so e.g. 'constraint position 1' will be local to body 1 COM, 'constraint position 2' will be local to body 2 COM). Note that this means you need to subtract Shape::GetCenterOfMass() from positions!
        WorldSpace,     /// All constraint properties are specified in world space.
    };

    class Constraint
    {
        friend class ConstraintManager;

    public:
        static constexpr uint32_t kInvalidConstraintIndex = std::numeric_limits<uint32_t>::max();

    private:
        /// Index of the Constraint in the ConstraintManager's array.
        uint32_t m_constraintIndex = kInvalidConstraintIndex;

        /// Priority of the Constraint when solving. Higher values are more likely to be solved correctly.
        uint32_t m_constraintPriority = 0;
        
        uint8_t m_numVelocityStepsOverride = 0;
        uint8_t m_numPositionStepsOverride = 0;

        /// Whether this Constraint is currently Active.
        bool m_isEnabled = true;

    public:
        virtual ~Constraint() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Notify the constraint that the shape of a body has changed and that its center of mass
        ///     has moved by `deltaCOM`. Bodies don't know which constraints are connected to them so the
        ///     user is responsible for notifying the relevant constraints when a Body is updated.
        ///	@param bodyID : ID of the changed Body.
        ///	@param deltaCOM : The delta of the center of mass of the Body. (shape->GetCenterOfMass() -
        ///     shapeBeforeChanged->GetCenterOfMass());
        //----------------------------------------------------------------------------------------------------
        virtual void NotifyShapeChanges(const BodyID& bodyID, const Vector3& deltaCOM) = 0;
        virtual void SolveVelocityConstraint(const float deltaTime) = 0;
        virtual void SolvePositionConstraint(const float deltaTime) = 0;
        virtual bool IsActive() const { return m_isEnabled; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Enable or Disable this constraint. This can be used to implement a breakable constraint by
        ///     detecting that the constraint impulse went over a certain limit and then disabling the constraint
        ///     Note that although the disabled constraint will not affect the simulation in any way anymore, it
        ///     does incur some processing overhead. You can alternatively remove the constraint from the
        ///     ConstraintManager (which will be more costly if you are toggling this constraint on and off).
        //----------------------------------------------------------------------------------------------------
        void SetEnabled(bool isEnabled);
        void SetConstraintPriority(const uint32_t priority) { m_constraintPriority = priority; }

        virtual ConstraintType GetType() const { return ConstraintType::SingleBodyConstraint; }
        virtual ConstraintSubType GetSubType() const = 0;
        
        uint32_t GetConstraintPriority() const { return m_constraintPriority; }
        bool IsEnabled() const { return m_isEnabled; }
    };
}
