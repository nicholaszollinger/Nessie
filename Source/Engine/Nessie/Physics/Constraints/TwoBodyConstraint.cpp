// TwoBodyConstraint.cpp
#include "TwoBodyConstraint.h"

namespace nes
{
    TwoBodyConstraint::TwoBodyConstraint(Body& bodyA, Body& bodyB, const TwoBodyConstraintSettings& settings)
        : Constraint(settings)
        , m_pBodyA(&bodyA)
        , m_pBodyB(&bodyB)
    {
        //
    }
    
    bool TwoBodyConstraint::Internal_IsActive() const
    {
        return Constraint::Internal_IsActive()
            && (m_pBodyA->IsActive() || m_pBodyB->IsActive())
            && (m_pBodyA->IsDynamic() || m_pBodyB->IsDynamic());
    }

    void TwoBodyConstraint::BuildIslands([[maybe_unused]] const uint32_t constraintIndex, [[maybe_unused]] IslandBuilder& builder, [[maybe_unused]] BodyManager& bodyManager)
    {
        // [TODO]: 
    }

    unsigned TwoBodyConstraint::BuildIslandSplits([[maybe_unused]] LargeIslandSplitter& splitter) const
    {
        // [TODO]:
        return 0;
    }
}
