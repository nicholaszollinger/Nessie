// ConstraintManager.cpp
#include "ConstraintManager.h"

namespace nes
{
    void ConstraintManager::Add(Constraint** constraintsArray, const int numConstraints)
    {
        // [TODO]: 
    }

    void ConstraintManager::Remove(Constraint** constraintsArray, const int numConstraints)
    {
        // [TODO]: 
    }

    bool ConstraintManager::SolveVelocityConstraints(Constraint** activeConstraints, const uint32_t indexBegin,
        const uint32_t indexEnd, const float deltaTime)
    {
        // [TODO]: 
        return false;
    }

    bool ConstraintManager::SolvePositionConstraints(Constraint** activeConstraints, const uint32_t indexBegin,
        const uint32_t indexEnd, const float deltaTime, const float baumgarte)
    {
        // [TODO]: 
        return false;
    }
}