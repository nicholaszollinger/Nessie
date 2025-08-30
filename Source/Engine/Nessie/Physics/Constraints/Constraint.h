// Constraint.h
#pragma once
#include <cstdint>
#include "Nessie/Core/Result.h"
#include "Nessie/Core/Memory/StrongPtr.h"
#include "Nessie/Math/Math.h"

namespace nes
{
    class BodyID;
    class IslandBuilder;
    class LargeIslandSplitter;
    class BodyManager;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Enum to identify constraint type. 
    //----------------------------------------------------------------------------------------------------
    enum class EConstraintType
    {
        Constraint, // Constraint that is applied to a single body.
        TwoBodyConstraint,    // Constraint that is applied to two connected bodies.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Enum to identity constraint subtype. 
    //----------------------------------------------------------------------------------------------------
    enum class EConstraintSubType
    {
        Fixed,
        Point,
        Hinge,
        Slider,
        Distance,
        // [TODO]: Others...
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Certain constraints support setting them up in local or world space. This governs what is used. 
    //----------------------------------------------------------------------------------------------------
    enum class EConstraintSpace
    {
        LocalToBodyCOM, /// All constraint properties are specified in local space to center of mass of the bodies that are being constrained (so e.g. 'constraint position 1' will be local to body 1 COM, 'constraint position 2' will be local to body 2 COM). Note that this means you need to subtract Shape::GetCenterOfMass() from positions!
        WorldSpace,     /// All constraint properties are specified in world space.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Class used to store the configuration of a constraint. Allows run-time creation of Constraints. 
    //----------------------------------------------------------------------------------------------------
    class ConstraintSettings : public RefTarget<ConstraintSettings>
    {
    public:
        using ConstraintResult = Result<StrongPtr<ConstraintSettings>>;

        /// User data value (can be used by the application).
        uint64_t    m_userData = 0;
        
        /// Priority of the constraint when solving. Higher number are more likely to be solved correctly.
        /// Note that if you want a deterministic simulation, and you cannot guarantee the order in which constraints are added/removed, then you
        /// can make the priority for all constraints unique to get a deterministic ordering.
        uint32_t    m_constraintPriority = 0;

        /// Use only when the constraint is active. Override for the number of solver iterations to run.
        /// 0 means use the default in PhysicsSettings::m_numVelocitySteps. The number of iterations to use is the max of all contacts and constraints in the island.
        unsigned    m_numVelocityStepsOverride = 0;

        /// Use only when the constraint is active. Override for the number of solver iterations to run.
        /// 0 means use the default in PhysicsSettings::m_numPositionSteps. The number of iterations to use is the max of all contacts and constraints in the island.
        unsigned    m_numPositionStepsOverride = 0;

        /// [TODO]: Size of the constraint when drawing it through the debug renderer 
        float       m_drawConstraintSize = 1.0f;

        /// If this constraint is enabled initially. Use Constraint::SetEnabled() to toggle after creation.
        bool        m_isEnabled = true;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for all physics constraints. A constraint removes one or more degrees of
    ///     freedom from a rigid body.
    //----------------------------------------------------------------------------------------------------
    class Constraint : public RefTarget<Constraint>
    {
        friend class ConstraintManager;

    public:
        static constexpr uint32_t kInvalidConstraintIndex = std::numeric_limits<uint32_t>::max();

    public:
        Constraint(const ConstraintSettings& settings);
        virtual ~Constraint() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Type of constraint.
        //----------------------------------------------------------------------------------------------------
        virtual EConstraintType     GetType() const                                     { return EConstraintType::Constraint; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the subtype of a constraint. 
        //----------------------------------------------------------------------------------------------------
        virtual EConstraintSubType  GetSubType() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Priority of the Constraint when solving. Higher values are more likely to be solved correctly.
        /// @note : If you want a deterministic simulation, and you cannot guarantee the order in which constraints are added/removed, then you
        ///     can make the priority for all constraints unique to get a deterministic ordering.
        //----------------------------------------------------------------------------------------------------
        uint32_t                    GetConstraintPriority() const                       { return m_constraintPriority; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the priority of the Constraint when solving. Higher values are more likely to be solved correctly.
        /// @note : If you want a deterministic simulation, and you cannot guarantee the order in which constraints are added/removed, then you
        ///     can make the priority for all constraints unique to get a deterministic ordering.
        //----------------------------------------------------------------------------------------------------
        void                        SetConstraintPriority(const uint32_t priority)      { m_constraintPriority = priority; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Used only when the constraint is active. Override for the number of solver velocity iterations to run.
        /// 0 means use the default in PhysicsSettings::m_numVelocitySteps. The number of iterations to use is the max of all contacts and constraints in the island.
        //----------------------------------------------------------------------------------------------------
        inline void                 SetNumVelocityStepsOverride(const uint32_t num);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Used only when the constraint is active.
        /// 0 means use the default in PhysicsSettings::m_numVelocitySteps. The number of iterations to use is the max of all contacts and constraints in the island.
        //----------------------------------------------------------------------------------------------------
        unsigned int                GetNumVelocityStepsOverride() const                 { return m_numVelocityStepsOverride; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Used only when the constraint is active. Override for the number of solver velocity iterations to run.
        /// 0 means use the default in PhysicsSettings::m_numPositionSteps. The number of iterations to use is the max of all contacts and constraints in the island.
        //----------------------------------------------------------------------------------------------------
        inline void                 SetNumPositionStepsOverride(const uint32_t num);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Used only when the constraint is active.
        /// 0 means use the default in PhysicsSettings::m_numPositionSteps. The number of iterations to use is the max of all contacts and constraints in the island.
        //----------------------------------------------------------------------------------------------------
        unsigned int                GetNumPositionStepsOverride() const                 { return m_numPositionStepsOverride; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Enable or Disable this constraint. This can be used to implement a breakable constraint by
        ///     detecting that the constraint impulse went over a certain limit and then disabling the constraint
        ///     Note that although the disabled constraint will not affect the simulation in any way anymore, it
        ///     does incur some processing overhead. You can alternatively remove the constraint from the
        ///     ConstraintManager (which will be more costly if you are toggling this constraint on and off).
        //----------------------------------------------------------------------------------------------------
        void                        SetEnabled(const bool isEnabled)                     { m_isEnabled = isEnabled; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if a Constraint is enabled. 
        //----------------------------------------------------------------------------------------------------
        bool                        IsEnabled() const                                    { return m_isEnabled; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the user data for this Constraint. Can be used for anything by the application.
        //----------------------------------------------------------------------------------------------------
        uint64_t                    GetUserData() const                                  { return m_userData; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the user data for this Constraint. Can be used for anything by the application.
        //----------------------------------------------------------------------------------------------------
        void                        SetUserData(const uint64_t userData)                 { m_userData = userData;}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Notify the constraint that the shape of a body has changed and that its center of mass
        ///     has moved by `deltaCOM`. Bodies don't know which constraints are connected to them so the
        ///     user is responsible for notifying the relevant constraints when a Body is updated.
        ///	@param bodyID : ID of the changed Body.
        ///	@param deltaCOM : The delta of the center of mass of the Body. (shape->GetCenterOfMass() -
        ///     shapeBeforeChanged->GetCenterOfMass());
        //----------------------------------------------------------------------------------------------------
        virtual void                NotifyShapeChanged(const BodyID& bodyID, const Vec3& deltaCOM) = 0;

        /// Solver interface
        virtual bool                Internal_IsActive() const                             { return m_isEnabled; }
        virtual void                Internal_SetupVelocityConstraint(const float deltaTime) = 0;
        virtual void                Internal_WarmStartVelocityConstraint(const float warmStartImpulseRatio) = 0;
        virtual bool                Internal_SolveVelocityConstraint(const float deltaTime) = 0;
        virtual bool                Internal_SolvePositionConstraint(const float deltaTime, const float baumgarte) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Link bodies that are connected by this constraint in the island builder.
        //----------------------------------------------------------------------------------------------------
        virtual void                BuildIslands(const uint32_t constraintIndex, IslandBuilder& builder, BodyManager& bodyManager) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Link bodies that are connected by this constraint in the same split. Returns the split index. 
        //----------------------------------------------------------------------------------------------------
        virtual unsigned            BuildIslandSplits(LargeIslandSplitter& splitter) const = 0;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Debug function to convert a constraint to its settings, not that this will not save to which bodies
        ///     the constraint is connected to.
        //----------------------------------------------------------------------------------------------------
        virtual StrongPtr<ConstraintSettings> GetConstraintSettings() const = 0;

    protected:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper function to copy settings beack to constraint settings for this base class.
        //----------------------------------------------------------------------------------------------------
        void                        ToConstraintSettings(ConstraintSettings& outSettings) const;

    private:
        /// Index of the Constraint in the ConstraintManager's array.
        uint32_t    m_constraintIndex = kInvalidConstraintIndex;

        /// Priority of the Constraint when solving. Higher values are more likely to be solved correctly.
        uint32_t    m_constraintPriority = 0;

        /// Use only when the constraint is active. Override for the number of solver iterations to run.
        /// 0 means use the default in PhysicsSettings::m_numVelocitySteps. The number of iterations to use is the max of all contacts and constraints in the island.
        uint8_t     m_numVelocityStepsOverride = 0;

        /// Use only when the constraint is active. Override for the number of solver iterations to run.
        /// 0 means use the default in PhysicsSettings::m_numPositionSteps. The number of iterations to use is the max of all contacts and constraints in the island.
        uint8_t     m_numPositionStepsOverride = 0;

        /// Whether this Constraint is currently Active.
        bool        m_isEnabled = true;

        /// User data value (can be used by the application).
        uint64_t    m_userData = 0;
    };
}
