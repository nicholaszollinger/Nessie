// PhysicsScene.cpp
#include "PhysicsScene.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Add a Constraint to the Scene. 
    //----------------------------------------------------------------------------------------------------
    void PhysicsScene::AddConstraint(Constraint* pConstraint)
    {
        m_constraintManager.Add(&pConstraint, 1);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Add an array of Constraints to the Scene. 
    //----------------------------------------------------------------------------------------------------
    void PhysicsScene::AddConstraints(Constraint** constraintsArray, const int numConstraints)
    {
        m_constraintManager.Add(constraintsArray, numConstraints);
    }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Remove a Constraint from the Scene
    //----------------------------------------------------------------------------------------------------
    void PhysicsScene::RemoveConstraint(Constraint* pConstraint)
    {
        m_constraintManager.Remove(&pConstraint, 1);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Remove an array of constraints to the Scene. 
    //----------------------------------------------------------------------------------------------------
    void PhysicsScene::RemoveConstraints(Constraint** constraintsArray, const int numConstraints)
    {
        m_constraintManager.Remove(constraintsArray, numConstraints);
    }
}
