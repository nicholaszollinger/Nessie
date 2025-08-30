// AxisConstraintPart.h
#pragma once
#include "Nessie/Physics/Body/Body.h"
#include "Nessie/Physics/Constraints/ConstraintPart/SpringPart.h"
#include "Nessie/Physics/Constraints/SpringSettings.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Constraint that constrains motion along 1 axis.
    ///
    /// @see : "Constraints Derivation for Rigid Body Simulation in 3D" - Daniel Chappuis, section 2.1.1
    /// (we're not using the approximation of eq 27 but instead add the U term as in eq 55)
    //
    // Constraint Equation (eq 51)
    // C = (p2 - p1) * n
    //
    // Jacobian (transposed) (eq 55)
    // J^T =    ___           ___
    //         |        -n      |
    //         | -(r1 + u) x n  |
    //         |        n       |
    //         |     r2 x n     |
    //         |--            --|
    //
    // Used Terms (here an in comments below, everything is in world space)
    // n       = Constraint Axis (normalized).
    // p1, p2  = Constraint Points.
    // r1      = p1 - x1 
    // r2      = p2 - x2
    // u       = (x2 + r2 - x1 - r1) = (p2 - p1)
    // x1, x2  = Center of Mass for the bodies.
    // v1, v2  = Linear velocity of body1 and body2. 
    // w1, w2  = Angular velocity of body1 and body2.
    // M       = Mass Matrix - a diagonal matrix of the mass and inertia with diagonal = [m1, I1, m2, I2]
    // K^-1    = (J M-1 J^T) = effective mass.
    // b       = Velocity bias.
    // B       = Baumgarte constant.
    //----------------------------------------------------------------------------------------------------
    class AxisConstraintPart
    {
    public:
        AxisConstraintPart() = default;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Templated form of CalculateConstraintProperties with the motion types baked in.
        //----------------------------------------------------------------------------------------------------
        template <EBodyMotionType Type1, EBodyMotionType Type2>
        NES_INLINE void     TemplatedCalculateConstraintProperties(const float invMass1, const Mat44& invI1, const Vec3 r1PlusU, const float invMass2, const Mat44& invI2, const Vec3 r2, const Vec3 worldSpaceAxis, float bias = 0.f);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate properties used during the functions below. 
        ///	@param body1 : The first body that this constraint is attached to.
        ///	@param r1PlusU : See equations above (r1 + u)
        ///	@param body2 : The second body that this constraint is attached to.
        ///	@param r2 : See equations above (r2)
        ///	@param worldSpaceAxis : Axis along which the constraint acts (normalized, pointing from body 1 to 2)
        ///	@param bias : Bias term (b) for the constraint impulse: lambda = J v + b
        //----------------------------------------------------------------------------------------------------
        inline void         CalculateConstraintProperties(const Body& body1, const Vec3 r1PlusU, const Body& body2, const Vec3 r2, const Vec3 worldSpaceAxis, float bias = 0.f);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate properties used during the functions below, version that supports mass scaling
        ///	@param body1 : The first body that this constraint is attached to.
        ///	@param invMass1 : The inverse mass of body 1 (only used when body 1 is dynamic)
        ///	@param invInertiaScale1 : Scale factor for the inverse inertia of body 1.
        ///	@param r1PlusU : See equations above (r1 + u)
        ///	@param body2 : The second body that this constraint is attached to.
        ///	@param invMass2 : The inverse mass of body 2 (only used when body 2 is dynamic).
        ///	@param invInertiaScale2 : Scale factor for the inverse inertia of body 2.
        ///	@param r2 : See equations above (r2)
        ///	@param worldSpaceAxis : Axis along which the constraint acts (normalized, pointing from body 1 to 2)
        ///	@param bias : Bias term (b) for the constraint impulse: lambda = J v + b 
        //----------------------------------------------------------------------------------------------------
        inline void         CalculateConstraintPropertiesWithMassOverride(const Body& body1, const float invMass1, const float invInertiaScale1, const Vec3 r1PlusU, const Body& body2, const float invMass2, const float invInertiaScale2, const Vec3 r2, const Vec3 worldSpaceAxis, const float bias = 0.f);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate properties used during the functions below
        ///	@param deltaTime : Time step.
        ///	@param body1 : The first body that this constraint is attached to.
        ///	@param r1PlusU : See equations above (r1 + u).
        ///	@param body2 : The second body that this constraint is attached to.
        ///	@param r2 : See equations above (r2)
        ///	@param worldSpaceAxis : Axis along which the constraint acts (normalized, pointing from body 1 to 2).
        ///	@param bias : Bias term (b) for the constraint impulse: lambda = J v + b.
        ///	@param inC : Value of the constraint equation (C).
        ///	@param frequency : Oscillation frequency (Hz).
        ///	@param damping : Damping factor (0 = no damping, 1 = critical damping).
        //----------------------------------------------------------------------------------------------------
        inline void         CalculateConstraintPropertiesWithFrequencyAndDamping(const float deltaTime, const Body& body1, const Vec3 r1PlusU, const Body& body2, const Vec3 r2, const Vec3 worldSpaceAxis, const float bias, const float inC, const float frequency, const float damping);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate properties used during the functions below
        ///	@param deltaTime : Time step.
        ///	@param body1 : The first body that this constraint is attached to.
        ///	@param r1PlusU : See equations above (r1 + u).
        ///	@param body2 : The second body that this constraint is attached to.
        ///	@param r2 : See equations above (r2)
        ///	@param worldSpaceAxis : Axis along which the constraint acts (normalized, pointing from body 1 to 2).
        ///	@param bias : Bias term (b) for the constraint impulse: lambda = J v + b.
        ///	@param inC : Value of the constraint equation (C).
        ///	@param stiffness : Spring stiffness k.
        ///	@param damping : Damping factor (0 = no damping, 1 = critical damping).
        //----------------------------------------------------------------------------------------------------
        inline void         CalculateConstraintPropertiesWithStiffnessAndDamping(const float deltaTime, const Body& body1, const Vec3 r1PlusU, const Body& body2, const Vec3 r2, const Vec3 worldSpaceAxis, const float bias, const float inC, const float stiffness, const float damping);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate properties used during the functions below
        ///	@param deltaTime : Time step.
        ///	@param body1 : The first body that this constraint is attached to.
        ///	@param r1PlusU : See equations above (r1 + u).
        ///	@param body2 : The second body that this constraint is attached to.
        ///	@param r2 : See equations above (r2)
        ///	@param worldSpaceAxis : Axis along which the constraint acts (normalized, pointing from body 1 to 2).
        ///	@param bias : Bias term (b) for the constraint impulse: lambda = J v + b.
        ///	@param inC : Value of the constraint equation (C).
        ///	@param springSettings : Spring settings to use.
        //----------------------------------------------------------------------------------------------------
        inline void         CalculateConstraintPropertiesWithSettings(const float deltaTime, const Body& body1, const Vec3 r1PlusU, const Body& body2, const Vec3 r2, const Vec3 worldSpaceAxis, const float bias, const float inC, const SpringSettings& springSettings);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Deactivate this constraint. 
        //----------------------------------------------------------------------------------------------------
        inline void         Deactivate();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if this constraint is active. 
        //----------------------------------------------------------------------------------------------------
        inline bool         IsActive() const { return m_effectiveMass != 0.f;}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Templated form of WarmStart with the motion types baked in.
        //----------------------------------------------------------------------------------------------------
        template <EBodyMotionType Type1, EBodyMotionType Type2>
        inline void         TemplatedWarmStart(MotionProperties* pMotionProps1, const float invMass1, MotionProperties* pMotionProps2, const float invMass2, const Vec3 worldSpaceAxis, const float warmStartImpulseRatio);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Must be called from the WarmStartVelocityConstraint call to apply the previous frame's impulses.
        ///	@param body1 : The first body that this constraint is attached to.
        ///	@param body2 : The second body that this constraint is attached to.
        ///	@param worldSpaceAxis : Axis along which the constraint acts (normalized).
        ///	@param warmStartImpulseRatio : Ratio of the new time-step to the old time-step (dtNew / dtOld) for scaling the lagrange multiplier from the previous frame.
        //----------------------------------------------------------------------------------------------------
        inline void         WarmStart(Body& body1, Body& body2, const Vec3 worldSpaceAxis, const float warmStartImpulseRatio);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Templated form of SolveVelocityConstraint with the motion types baked in, part 1: get the total lambda
        //----------------------------------------------------------------------------------------------------
        template <EBodyMotionType Type1, EBodyMotionType Type2>
        NES_INLINE float    TemplatedSolveVelocityConstraintGetTotalLambda(const MotionProperties* pMotionProps1, const MotionProperties* pMotionProps2, const Vec3 worldSpaceAxis) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Templated form of SolveVelocityConstraint with the motion types baked in, part 2: apply new lambda 
        //----------------------------------------------------------------------------------------------------
        template <EBodyMotionType Type1, EBodyMotionType Type2>
        NES_INLINE bool     TemplatedSolveVelocityConstraintApplyLambda(MotionProperties* pMotionProps1, const float invMass1, MotionProperties* pMotionProps2, const float invMass2, const Vec3 worldSpaceAxis, const float totalLambda);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Templated form of SolveVelocityConstraint with the motion types baked in
        //----------------------------------------------------------------------------------------------------
        template <EBodyMotionType Type1, EBodyMotionType Type2>
        inline bool         TemplatedSolveVelocityConstraint(MotionProperties* pMotionProps1, const float invMass1, MotionProperties* pMotionProps2, const float invMass2, const Vec3 worldSpaceAxis, const float minLambda, const float maxLambda);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Iteratively update the velocity constraint. Makes sure d/dt C(...) = 0, where C is the constraint equation.
        ///	@param body1 : The first body that this constraint is attached to.
        ///	@param body2 : The second body that this constraint is attached to.
        ///	@param worldSpaceAxis : Axis along which the constraint acts (normalized)
        ///	@param minLambda : Minimum value of constraint impulse to apply (N s).
        ///	@param maxLambda : Maximum value of constraint impulse to apply (N s).
        ///	@returns : 
        //----------------------------------------------------------------------------------------------------
        inline bool         SolveVelocityConstraint(Body& body1, Body& body2, const Vec3 worldSpaceAxis, const float minLambda, const float maxLambda);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Iteratively update the velocity constraint. Makes sure d/dt C(...) = 0, where C is the constraint equation.
        ///	@param body1 : The first body that this constraint is attached to.
        ///	@param body2 : The second body that this constraint is attached to.
        ///	@param invMass1 : The inverse mass of body 1 (only when body 1 is dynamic). 
        ///	@param invMass2 : The inverse mass of body 2 (only when body 2 is dynamic). 
        ///	@param worldSpaceAxis : Axis along which the constraint acts (normalized)
        ///	@param minLambda : Minimum value of constraint impulse to apply (N s).
        ///	@param maxLambda : Maximum value of constraint impulse to apply (N s).
        ///	@returns : 
        //----------------------------------------------------------------------------------------------------
        inline bool         SolveVelocityConstraintWithMassOverride(Body& body1, const float invMass1, Body& body2, const float invMass2, const Vec3 worldSpaceAxis, const float minLambda, const float maxLambda);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Iteratively update the position constraint. Makes sure C(...) = 0. 
        ///	@param body1 : The first body that this constraint is attached to.
        ///	@param body2 : The second body that this constraint is attached to.
        ///	@param worldSpaceAxis : Axis along which the constraint acts (normalized).
        ///	@param inC : Value fo the constraint equation (C).
        ///	@param baumgarte : Baumgarte constant (fraction of the error to correct).
        /// @returns : Returns true if the position was applied; this only applies the position constraint when the
        ///     spring constraint is hard, otherwise, the velocity bias will fix the constraint.
        //----------------------------------------------------------------------------------------------------
        inline bool         SolvePositionConstraint(Body& body1, Body& body2, const Vec3 worldSpaceAxis, const float inC, const float baumgarte) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Iteratively update the position constraint. Makes sure C(...) = 0. 
        ///	@param body1 : The first body that this constraint is attached to.
        ///	@param body2 : The second body that this constraint is attached to.
        ///	@param invMass1 : The inverse mass of body 1 (only when body 1 is dynamic). 
        ///	@param invMass2 : The inverse mass of body 2 (only when body 2 is dynamic). 
        ///	@param worldSpaceAxis : Axis along which the constraint acts (normalized).
        ///	@param inC : Value fo the constraint equation (C).
        ///	@param baumgarte : Baumgarte constant (fraction of the error to correct).
        /// @returns : Returns true if the position was applied; this only applies the position constraint when the
        ///     spring constraint is hard, otherwise, the velocity bias will fix the constraint.
        //----------------------------------------------------------------------------------------------------
        inline bool         SolvePositionContraintWithMassOverride(Body& body1, const float invMass1, Body& body2, const float invMass2, const Vec3 worldSpaceAxis, const float inC, const float baumgarte) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Override total lagrange multiplier. Can be used to set the initial value for warm starting. 
        //----------------------------------------------------------------------------------------------------
        inline void         SetTotalLambda(const float lambda)  { m_totalLambda = lambda; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the lagrange multiplier. 
        //----------------------------------------------------------------------------------------------------
        inline float        GetTotalLambda() const { return m_totalLambda; }

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Internal helper function to update velocities after Lagrange multiplier is calculated. 
        //----------------------------------------------------------------------------------------------------
        template <EBodyMotionType Type1, EBodyMotionType Type2>
        NES_INLINE bool     ApplyVelocityStep(MotionProperties* pMotionProps1, const float invMass1, MotionProperties* pMotionProps2, const float invMass2, const Vec3 worldSpaceAxis, const float lambda) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Internal helper function to calcualte the inverse effective mass. 
        //----------------------------------------------------------------------------------------------------
        template <EBodyMotionType Type1, EBodyMotionType Type2>
        NES_INLINE float    TemplatedCalculateInverseEffectiveMass(const float invMass1, const Mat44& invI1, const Vec3 r1PlusU, const float invMass2, const Mat44& invI2, const Vec3 r2, const Vec3 worldSpaceAxis);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Internal helper function to calculate the inverse effective mass
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float    CalculateInverseEffectiveMass(const Body& body1, const Vec3 r1PlusU, const Body& body2, const Vec3 r2, const Vec3 worldSpaceAxis);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Internal helper function to calculate the inverse effective mass, version that supports mass scaling
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float    CalculateInverseEffectiveMassWithMassOverride(const Body& body1, const float invMass1, const float invInertiaScale1, const Vec3 r1PlusU, const Body& body2, const float invMass2, const float invInertiaScale2, const Vec3 r2, const Vec3 worldSpaceAxis);
    
    private:
        Float3              m_R1PlusUxAxis;
        Float3              m_R2xAxis;
        Float3              m_InvI1_R1PlusUxAxis;
        Float3              m_InvI2_R2xAxis;
        float               m_effectiveMass = 0.f;
        SpringPart          m_springPart;
        float               m_totalLambda = 0.f;
    };
}

#include "AxisConstraintPart.inl"