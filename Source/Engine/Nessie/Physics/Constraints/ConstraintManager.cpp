// ConstraintManager.cpp
#include "ConstraintManager.h"
#include "CalculateSolverSteps.h"
#include "Core/QuickSort.h"

namespace nes
{
    void ConstraintManager::Add(Constraint** constraintsArray, const int numConstraints)
    {
        std::unique_lock lock(m_mutex);

        m_constraints.reserve(m_constraints.size() + numConstraints);
        for (Constraint** pConstraintPtr = constraintsArray, **pEnd = constraintsArray + numConstraints; pConstraintPtr < pEnd; ++pConstraintPtr)
        {
            Constraint* pConstraint = *pConstraintPtr;

            // Assume this constraint has not been added yet.
            NES_ASSERT(pConstraint->m_constraintIndex == Constraint::kInvalidConstraintIndex);

            // Add to the list
            pConstraint->m_constraintIndex = static_cast<uint32_t>(m_constraints.size());
            m_constraints.emplace_back(pConstraint);
        }
    }

    void ConstraintManager::Remove(Constraint** constraintsArray, const int numConstraints)
    {
        std::unique_lock lock(m_mutex);

        for (Constraint** pConstraintPtr = constraintsArray, **pEnd = constraintsArray + numConstraints; pConstraintPtr < pEnd; ++pConstraintPtr)
        {
            Constraint* pConstraint = *pConstraintPtr;

            // Reset the constraint index for this constraint
            uint32_t thisConstraintIndex = pConstraint->m_constraintIndex;
            pConstraint->m_constraintIndex = Constraint::kInvalidConstraintIndex;
            NES_ASSERT(thisConstraintIndex != Constraint::kInvalidConstraintIndex);
            
            // Check if this constraint is somewhere in the middle of the constraints, in this case we need to move to the last constraint
            // in this position.
            uint32_t lastConstraintIndex = static_cast<uint32_t>(m_constraints.size() - 1);
            if (thisConstraintIndex < lastConstraintIndex)
            {
                Constraint* pLastConstraint = m_constraints[lastConstraintIndex];
                pLastConstraint->m_constraintIndex = thisConstraintIndex;
                m_constraints[thisConstraintIndex] = pLastConstraint;
            }

            // Pop the last constraint.
            m_constraints.pop_back();
        }
    }

    ConstraintManager::ConstraintsArray ConstraintManager::GetConstraints() const
    {
        std::unique_lock lock(m_mutex);
        ConstraintsArray copy = m_constraints;
        return copy;
    }

    void ConstraintManager::GetActiveConstraints(uint32_t beginIndex, uint32_t endIndex, Constraint** outActiveConstraints, uint32_t& outNumActiveConstraints) const
    {
        NES_ASSERT(endIndex <= m_constraints.size());

        uint32_t numActiveConstraints = 0;
        for (uint32_t constraintIndex = beginIndex; constraintIndex < endIndex; ++constraintIndex)
        {
            Constraint* pConstraint = m_constraints[constraintIndex];
            NES_ASSERT(pConstraint->m_constraintIndex == constraintIndex);
            if (pConstraint->Internal_IsActive())
            {
                *(outActiveConstraints++) = pConstraint;
                ++numActiveConstraints;
            }
        }
        
        outNumActiveConstraints = numActiveConstraints;
    }

    void ConstraintManager::BuildIslands(Constraint** activeConstraints, uint32_t numActiveConstraints, IslandBuilder& builder, BodyManager& bodyManager)
    {
        for (uint32_t constraintIndex = 0; constraintIndex < numActiveConstraints; ++constraintIndex)
        {
            Constraint* pConstraint = activeConstraints[constraintIndex];
            pConstraint->BuildIslands(constraintIndex, builder, bodyManager);
        }
    }

    void ConstraintManager::SortConstraints(Constraint** activeConstraints, uint32_t* pIndexBegin, uint32_t* pIndexEnd)
    {
        QuickSort(pIndexBegin, pIndexEnd, [activeConstraints](uint32_t left, uint32_t right)
        {
            const Constraint* pLeft = activeConstraints[left];
            const Constraint* pRight = activeConstraints[right];

            if (pLeft->GetConstraintPriority() != pRight->GetConstraintPriority())
                return pLeft->GetConstraintPriority() < pRight->GetConstraintPriority();

            // If the same priority, defer to index order.
            return pLeft->m_constraintIndex < pRight->m_constraintIndex;
        });
    }

    void ConstraintManager::SetupVelocityConstraints(Constraint** activeConstraints, const uint32_t numActiveConstraints, float deltaTime)
    {
        for (Constraint** pConstraintPtr = activeConstraints, **pEnd = activeConstraints + numActiveConstraints; pConstraintPtr < pEnd; ++pConstraintPtr)
        {
            (*pConstraintPtr)->Internal_SetupVelocityConstraint(deltaTime);
        }
    }

    template <typename ConstraintCallback>
    void ConstraintManager::WarmStartVelocityConstraints(Constraint** activeConstraints, const uint32_t* pIndexBegin, const uint32_t* pIndexEnd, float warmStartImpulseRatio, ConstraintCallback& callback)
    {
        for (const uint32_t* pIndex = pIndexBegin; pIndex < pIndexEnd; ++pIndex)
        {
            Constraint* pConstraint = activeConstraints[*pIndex];
            callback(pConstraint);
            pConstraint->Internal_WarmStartVelocityConstraint(warmStartImpulseRatio);
        }
    }

    // Specialize for the two constraint callback types.
    template void ConstraintManager::WarmStartVelocityConstraints<CalculateSolverSteps>(Constraint** activeConstraints, const uint32_t* pIndexBegin, const uint32_t* pIndexEnd, float warmStartImpulseRatio, CalculateSolverSteps& callback);
    template void ConstraintManager::WarmStartVelocityConstraints<DummyCalculateSolverSteps>(Constraint** activeConstraints, const uint32_t* pIndexBegin, const uint32_t* pIndexEnd, float warmStartImpulseRatio, DummyCalculateSolverSteps& callback);

    bool ConstraintManager::SolveVelocityConstraints(Constraint** activeConstraints, const uint32_t* pIndexBegin, const uint32_t* pIndexEnd, const float deltaTime)
    {
        bool anyImpulseApplied = false;

        for (const uint32_t* pIndex = pIndexBegin; pIndex < pIndexEnd; ++pIndex)
        {
            Constraint* pConstraint = activeConstraints[*pIndex];
            anyImpulseApplied |= pConstraint->Internal_SolveVelocityConstraint(deltaTime);
        }

        return anyImpulseApplied;
    }

    bool ConstraintManager::SolvePositionConstraints(Constraint** activeConstraints, const uint32_t* pIndexBegin, const uint32_t* pIndexEnd, const float deltaTime, const float baumgarte)
    {
        bool anyImpulseApplied = false;

        for (const uint32_t* pIndex = pIndexBegin; pIndex < pIndexEnd; ++pIndex)
        {
            Constraint* pConstraint = activeConstraints[*pIndex];
            anyImpulseApplied |= pConstraint->Internal_SolvePositionConstraint(deltaTime, baumgarte);
        }

        return anyImpulseApplied;
    }
}
