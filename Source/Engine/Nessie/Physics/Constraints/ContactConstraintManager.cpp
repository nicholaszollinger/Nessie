// ContactConstraintManager.cpp
#include "ContactConstraintManager.h"

#include "Physics/Constraints/CalculateSolverSteps.h"
#include "Physics/Body/Body.h"
#include "Physics/PhysicsUpdateContext.h"
#include "Physics/PhysicsScene.h"
#include "Physics/IslandBuilder.h"
#include "Core/Memory/StackAllocator.h"

namespace nes
{
    ContactConstraintManager::ContactConstraintManager(const PhysicsSettings& settings)
        : m_physicsSettings(settings)
    {
        // [TODO]: 
    }

    ContactConstraintManager::~ContactConstraintManager()
    {
        // [TODO]: 
    }

    void ContactConstraintManager::Init([[maybe_unused]] uint32_t maxBodyPairs, [[maybe_unused]] uint32_t maxContactConstraints)
    {
        NES_ASSERT(false);
    }
}
