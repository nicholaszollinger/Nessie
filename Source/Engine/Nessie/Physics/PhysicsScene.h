// PhysicsScene.h
#pragma once
#include "PhysicsUpdateErrorCodes.h"
#include "Body/BodyManager.h"
#include "Constraints/ConstraintManager.h"
#include "Scene/TickFunction.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Settings for the Physics Simulation. 
    //----------------------------------------------------------------------------------------------------
    struct GlobalPhysicsSettings
    {
        float m_penetrationFactor = 0.02f;

        /// Time before a Body is allowed to go to sleep.
        float m_timeBeforeSleep = 0.5f;

        /// To detect if a Body is sleeping, we use 3 points:
        /// - The center of mass.
        /// - The centers of the faces of the bounding box that are furthest away from the center.
        /// The movement of these points is tracked and if the velocity of all 3 points is lower than this value,
        /// the Body is allowed to go to sleep. Must be a positive number. (unit: m/s)
        float m_pointVelocitySleepThreshold = 0.03f;
    };
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Class that runs physics simulation for all registered Bodies.   
    //----------------------------------------------------------------------------------------------------
    class PhysicsScene
    {
        friend class SceneManager; // Or just the World?
        static constexpr uint32_t kMaxBodies = BodyID::kMaxBodyIndex + 1;

        GlobalPhysicsSettings m_settings;
        BodyManager m_bodyManager;
        ConstraintManager m_constraintManager;
        Vector3 m_gravity = Vector3(0.0f, -9.81f, 0.0f);
    
    public:
        PhysicsScene(const PhysicsScene&) = delete;
        PhysicsScene& operator=(const PhysicsScene&) = delete;
        PhysicsScene(PhysicsScene&&) = delete;
        PhysicsScene& operator=(PhysicsScene&&) = delete;

        Body* CreateBody();
        //void AddBody(const BodyID& bodyID, BodyActivation)

        // Constraints
        void AddConstraint(Constraint* pConstraint);
        void AddConstraints(Constraint** constraintsArray, const int numConstraints);
        void RemoveConstraint(Constraint* pConstraint);
        void RemoveConstraints(Constraint** constraintsArray, const int numConstraints);

        // Raycast into the System:
        //bool CastRay();
        //bool CastShape();
        
    private:
        PhysicsUpdateErrorCode Update(const float deltaTime /*, int collisionSteps, TempAllocator* pTempAllocator, JobSystem* pJobSystem*/);

        // [TODO]: 
        // Physics Step Jobs:
    };
}