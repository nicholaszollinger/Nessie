// TwoBodyConstraint.cpp
#include "TwoBodyConstraint.h"

namespace nes
{
    TwoBodyConstraint::TwoBodyConstraint(Body* pBodyA, Body* pBodyB)
        : m_pBodyA(pBodyA)
        , m_pBodyB(pBodyB)
    {
        //
    }

    bool TwoBodyConstraint::IsActive() const
    {
        return Constraint::IsActive()
            && (m_pBodyA->IsActive() || m_pBodyB->IsActive())
            && (m_pBodyA->IsDynamic() || m_pBodyB->IsDynamic());
    }
}
