// PhysicsUpdateContext.cpp

#include "PhysicsUpdateContext.h"

namespace nes
{
    PhysicsUpdateContext::Step::CCDBody::CCDBody(BodyID bodyID1, const Vec3& deltaPos, const float linearCastThresholdSqr, const float maxPenetration)
        : m_deltaPosition(deltaPos)
        , m_bodyID1(bodyID1)
        , m_linearCastThresholdSqr(linearCastThresholdSqr)
        , m_maxPenetration(maxPenetration)
    {
        //
    }

    PhysicsUpdateContext::PhysicsUpdateContext(StackAllocator& allocator)
        : m_pAllocator(&allocator)
        , m_steps(allocator)
    {
        //
    }

    PhysicsUpdateContext::~PhysicsUpdateContext()
    {
        NES_ASSERT(m_pBodyPairs == nullptr);
        NES_ASSERT(m_pActiveConstraints == nullptr);
    }
}