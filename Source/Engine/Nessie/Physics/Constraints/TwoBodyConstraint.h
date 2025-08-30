// TwoBodyConstraint.h
#pragma once
#include "Constraint.h"
#include "Nessie/Physics/Body/Body.h"

namespace nes
{
    class TwoBodyConstraint;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for settings for all constraints that involve 2 bodies. 
    //----------------------------------------------------------------------------------------------------
    class TwoBodyConstraintSettings : public ConstraintSettings
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create an instance of this constraint.
        ///     You can use Body::s_FixedToWorld for body1 if you want to attach body2 to the world.
        //----------------------------------------------------------------------------------------------------
        virtual TwoBodyConstraint* Create(Body& body1, Body& body2) const = 0; 
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for all Constraints that involve two Bodies. Generally, Body A is considered the Parent,
    ///     and Body B is considered the Child. "Body B is constrained to Body A".
    //----------------------------------------------------------------------------------------------------
    class TwoBodyConstraint : public Constraint
    {
    public:
        explicit TwoBodyConstraint(Body& bodyA, Body& bodyB, const TwoBodyConstraintSettings& settings);
        
        virtual EConstraintType GetType() const override { return EConstraintType::TwoBodyConstraint; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access to Body A of the Constraint. Generally, Generally, Body A is considered the Parent,
        ///     and Body B is considered the Child.
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] Body*     GetBodyA() const { return m_pBodyA; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access to Body B of the Constraint. Generally, Generally, Body A is considered the Parent,
        ///     and Body B is considered the Child.
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] Body*     GetBodyB() const { return m_pBodyB; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates the matrix that transforms from constraint space to BodyA space. The first
        ///     column of the matrix is the primary constraint axis (e.g. the hinge axis / slider direction),
        ///     second column the secondary etc.
        //----------------------------------------------------------------------------------------------------
        virtual Mat44           ConstraintToBodyAMatrix() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates the matrix that transforms from constraint space to BodyB space. The first
        ///     column of the matrix is the primary constraint axis (e.g. the hinge axis / slider direction),
        ///     second column the secondary etc.
        //----------------------------------------------------------------------------------------------------
        virtual Mat44           ConstraintToBodyBMatrix() const = 0;
        
        virtual bool            Internal_IsActive() const override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Link bodies that are connected by this constraint in the island builder
        //----------------------------------------------------------------------------------------------------
        virtual void            BuildIslands(const uint32_t constraintIndex, IslandBuilder& builder, BodyManager& bodyManager) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Link bodies that are connected by this constraint in the same split. Returns the split index.
        //----------------------------------------------------------------------------------------------------
        virtual unsigned        BuildIslandSplits(LargeIslandSplitter& splitter) const override;

    protected:
        Body* m_pBodyA = nullptr;
        Body* m_pBodyB = nullptr;
    };
}
