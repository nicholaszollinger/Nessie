// TwoBodyConstraint.h
#pragma once
#include "Constraint.h"
#include "Physics/Body/Body.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for all Constraints that involve two Bodies. Generally, Body A is considered the Parent,
    ///     and Body B is considered the Child. "Body B is constrained to Body A".
    //----------------------------------------------------------------------------------------------------
    class TwoBodyConstraint : public Constraint
    {
    protected:
        Body* m_pBodyA = nullptr;
        Body* m_pBodyB = nullptr;

    public:
        explicit TwoBodyConstraint(Body* pBodyA, Body* pBodyB);
        
        virtual ConstraintType GetType() const override { return ConstraintType::TwoBodyConstraint; }
        virtual bool IsActive() const override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access to Body A of the Constraint. Generally, Generally, Body A is considered the Parent,
        ///     and Body B is considered the Child.
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] Body* GetBodyA() const { return m_pBodyA; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access to Body B of the Constraint. Generally, Generally, Body A is considered the Parent,
        ///     and Body B is considered the Child.
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] Body* GetBodyB() const { return m_pBodyB; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates the transform that transforms from constraint space to BodyA space. The first
        ///     column of the matrix is the primary constraint axis (e.g. the hinge axis / slider direction),
        ///     second column the secondary etc.
        //----------------------------------------------------------------------------------------------------
        virtual Mat4 GetConstraintToBodyAMatrix() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates the transform that transforms from constraint space to BodyB space. The first
        ///     column of the matrix is the primary constraint axis (e.g. the hinge axis / slider direction),
        ///     second column the secondary etc.
        //----------------------------------------------------------------------------------------------------
        virtual Mat4 GetConstraintToBodyBMatrix() const = 0;
    };
}
