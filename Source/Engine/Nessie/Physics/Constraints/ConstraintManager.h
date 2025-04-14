// ConstraintManager.h
#pragma once
#include "Constraint.h"
#include "Core/Memory/StrongPtr.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //		
    /// @brief : A ConstraintManager manages all Single Body Constraints.  
    //----------------------------------------------------------------------------------------------------
    class ConstraintManager
    {
    public:
        using ConstraintsArray = std::vector<StrongPtr<Constraint>>;
        
    private:
        ConstraintsArray m_constraints{};

    public:
        ConstraintManager(const ConstraintManager&) = delete;
        ConstraintManager& operator=(const ConstraintManager&) = delete;
        ConstraintManager(ConstraintManager&&) noexcept = delete;
        ConstraintManager& operator=(ConstraintManager&&) noexcept = delete;

        void Add(Constraint** constraintsArray, const int numConstraints);
        void Remove(Constraint** constraintsArray, const int numConstraints);
        
        [[nodiscard]] const ConstraintsArray& GetConstraintsArray() const { return m_constraints; }
        uint32_t GetNumConstraints() const { return static_cast<uint32_t>(m_constraints.size()); }

        static bool SolveVelocityConstraints(Constraint** activeConstraints, const uint32_t indexBegin, const uint32_t indexEnd, const float deltaTime);
        static bool SolvePositionConstraints(Constraint** activeConstraints, const uint32_t indexBegin, const uint32_t indexEnd, const float deltaTime, const float baumgarte);
    };
}
