// ConstraintManager.h
#pragma once
#include "Constraint.h"
#include "Nessie/Physics/PhysicsLock.h"
#include "Nessie/Core/Thread/Mutex.h"
#include "Nessie/Core/Memory/StrongPtr.h"

namespace nes
{
    class IslandBuilder;
    class BodyManager;

    using Constraints = std::vector<StrongPtr<Constraint>>;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : A ConstraintManager manages all Single Body Constraints.  
    //----------------------------------------------------------------------------------------------------
    class ConstraintManager
    {
    public:
#if NES_ASSERTS_ENABLED
        ConstraintManager(PhysicsLockContext context) : m_lockContext(context) {}
#else
        ConstraintManager() = default;
#endif
        ConstraintManager(const ConstraintManager&) = delete;
        ConstraintManager& operator=(const ConstraintManager&) = delete;
        ConstraintManager(ConstraintManager&&) noexcept = delete;
        ConstraintManager& operator=(ConstraintManager&&) noexcept = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a new constraint. This is thread-safe.
        //----------------------------------------------------------------------------------------------------
        void                Add(Constraint** constraintsArray, const int numConstraints);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove a constraint. This is thread-safe.
        //----------------------------------------------------------------------------------------------------
        void                Remove(Constraint** constraintsArray, const int numConstraints);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a list of all constraints. This returns a copy of the constraints array.
        //----------------------------------------------------------------------------------------------------
        Constraints         GetConstraints() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the total number of constraints. 
        //----------------------------------------------------------------------------------------------------
        uint32_t            GetNumConstraints() const      { return static_cast<uint32_t>(m_constraints.size()); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Determine the active constraints in a subset of the total constraints.
        //----------------------------------------------------------------------------------------------------
        void                GetActiveConstraints(uint32_t beginIndex, uint32_t endIndex, Constraint** outActiveConstraints, uint32_t& outNumActiveConstraints) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Link bodies to form islands.
        //----------------------------------------------------------------------------------------------------
        static void         BuildIslands(Constraint** activeConstraints, uint32_t numActiveConstraints, IslandBuilder& builder, BodyManager& bodyManager);

        //----------------------------------------------------------------------------------------------------
        /// @brief : In order to have a deterministic simulation, we need to sort the constraints of an island before solving them.
        ///     Sorts by Constraint Priority.
        //----------------------------------------------------------------------------------------------------
        static void         SortConstraints(Constraint** activeConstraints, uint32_t* pIndexBegin, uint32_t* pIndexEnd);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Prior to solving velocity constraints, you must call this function once to precalculate
        ///     values that are independent of velocity.
        //----------------------------------------------------------------------------------------------------
        static void         SetupVelocityConstraints(Constraint** activeConstraints, const uint32_t numActiveConstraints, float deltaTime);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Apply Last frame's impulses, must be called prior to Solve Velocity Constraints.
        //----------------------------------------------------------------------------------------------------
        template <typename ConstraintCallback>
        static void         WarmStartVelocityConstraints(Constraint** activeConstraints, const uint32_t* pIndexBegin, const uint32_t* pIndexEnd, float warmStartImpulseRatio, ConstraintCallback& callback);

        //----------------------------------------------------------------------------------------------------
        /// @brief : This function is called multiple times to iteratively come to a solution that meets all velocity constraints
        //----------------------------------------------------------------------------------------------------
        static bool			SolveVelocityConstraints(Constraint** activeConstraints, const uint32_t* pIndexBegin, const uint32_t* pIndexEnd, float deltaTime);

        //----------------------------------------------------------------------------------------------------
        /// @brief : This function is called multiple times to iteratively come to a solution that meets all position constraints
        //----------------------------------------------------------------------------------------------------
        static bool         SolvePositionConstraints(Constraint** activeConstraints, const uint32_t* pIndexBegin, const uint32_t* pIndexEnd, const float deltaTime, const float baumgarte);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Lock all constraints. This should only be done during PhysicsSystem::Update(). 
        //----------------------------------------------------------------------------------------------------
        void                Internal_LockAllConstraints()           { PhysicsLock::Lock(m_mutex NES_IF_ASSERTS_ENABLED(, m_lockContext, EPhysicsLockTypes::ConstraintsArray)); }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Unlock all constraints. This should only be done during PhysicsSystem::Update().
        //----------------------------------------------------------------------------------------------------
        void                Internal_UnlockAllConstraints()         { PhysicsLock::Unlock(m_mutex NES_IF_ASSERTS_ENABLED(, m_lockContext, EPhysicsLockTypes::ConstraintsArray)); }

    private:
    #if NES_ASSERTS_ENABLED
        PhysicsLockContext  m_lockContext;
    #endif
        Constraints         m_constraints{};
        mutable Mutex       m_mutex{};
    };
}
