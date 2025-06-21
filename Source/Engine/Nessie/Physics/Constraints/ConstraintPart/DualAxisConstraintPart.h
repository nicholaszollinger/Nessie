// DualAxisConstraintPart.h
#pragma once
#include "Math/Math.h"
#include "Physics/Body/Body.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Constrains movement on 2 axes.
    /// @see "Constraints Derivation for Rigid Body Simulation in 3D" - Daniel Chappuis, section 2.3.1
    //
    // Constraint Equation (eq 51)
    // C = [ (p2 - p1) * n1, (p2 - p1) * n2 ]
    //
    // Jacobian (transposed) (eq 55)
    // J^T =    ___                                  ___
    //         | -n1                   -n2             |
    //         | -(r1 + u) x n1        -(r1 + u) x n2  |
    //         | n1                    n2              |
    //         | r2 x n1               r2 x n2         |
    //         |--                                   --|
    //
    // Used Terms (here an in comments below, everything is in world space)
    // n1, n2  = Constraint Axes (normalized).
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
    class DualAxisConstraintPart
    {
    public:
        DualAxisConstraintPart() = default;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate properties used during the functions below. All input vectors should be in
        ///     world space.
        //----------------------------------------------------------------------------------------------------
        inline void         CalculateConstraintProperties(const Body& body1, const Mat44& rotation1, const Vec3 r1PlusU, const Body& body2, const Mat44& rotation2, const Vec3 r2, const Vec3 n1, const Vec3 n2);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Deactivate this constraint. 
        //----------------------------------------------------------------------------------------------------
        inline void         Deactivate();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if this constraint is active. 
        //----------------------------------------------------------------------------------------------------
        inline bool         IsActive() const { return !m_effectiveMass.IsZero(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Must be called from the WarmStartVelocityConstraint call to apply the previous frame's
        ///     impulses. All input vectors must be in world space.
        //----------------------------------------------------------------------------------------------------
        inline void         WarmStart(Body& body1, Body& body2, const Vec3 n1, const Vec3 n2, const float warmStartImpulseRatio);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Iteratively update the velocity constraint. Makes sure d/dt C(...) = 0, where C is the
        ///     constraint equation. All input vectors must be in world space.
        //----------------------------------------------------------------------------------------------------
        inline bool         SolveVelocityConstraint(Body& body1, Body& body2, const Vec3 n1, const Vec3 n2);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Iteratively update the position constraint. Makes sure C(...) = 0.
        ///     All input vectors must be in world space.
        //----------------------------------------------------------------------------------------------------
        inline bool         SolvePositionConstraint(Body& body1, Body& body2, const Vec3 inU, const Vec3 n1, const Vec3 n2, const float baumgarte) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Override total lagrange multiplier. Can be used to set the initial value for warm starting.
        //----------------------------------------------------------------------------------------------------
        inline void         SetTotalLambda(const Vec2 lambda) { m_totalLambda = lambda; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the langrange multiplier. 
        //----------------------------------------------------------------------------------------------------
        inline const Vec2&  GetTotalLambda() const { return m_totalLambda; }

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Internal helper function to update velocities of bodies after Lagrange multiplier is calculated.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool     ApplyVelocityStep(Body& body1, Body& body2, const Vec3 n1, const Vec3 n2, const Vec2 lambda) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Internal helper funciton to calculate the lagrange multiplier.
        //----------------------------------------------------------------------------------------------------
        inline void         CalculateLagrangeMultiplier(const Body& body1, const Body& body2, const Vec3 n1, const Vec3 n2, Vec2& outLambda) const;
        
    private:
        Vec3                m_r1PlusUxN1; 
        Vec3                m_r1PlusUxN2; 
        Vec3                m_r2xN1; 
        Vec3                m_r2xN2;
        Vec3                m_invI1_R1PlusUxN1;
        Vec3                m_invI1_R1PlusUxN2;
        Vec3                m_invI2_R2xN1;
        Vec3                m_invI2_R2xN2;
        Mat22               m_effectiveMass;
        Vec2                m_totalLambda = Vec2::Zero();
    };
}

#include "DualAxisConstraintPart.inl"
