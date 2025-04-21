// PhysicsScene.cpp
#include "PhysicsScene.h"

#include "PhysicsUpdateContext.h"
#include "Collision/BroadPhase/BroadPhaseQuadTree.h"

namespace nes
{
    PhysicsScene::PhysicsScene()
    {
        //
    }

    PhysicsScene::~PhysicsScene()
    {
        NES_DELETE(m_pBroadphase);
    }

    void PhysicsScene::Init(const CreateInfo& createInfo)
    {
        const uint32_t maxBodies = math::Min(createInfo.m_maxBodies, kMaxBodiesLimit);
        NES_ASSERTV(maxBodies == createInfo.m_maxBodies, "Cannot support this many bodies!");

        m_pCollisionLayerPairFilter = createInfo.m_pCollisionLayerPairFilter;
        m_pCollisionVsBroadPhaseLayerFilter = createInfo.m_pCollisionVsBroadPhaseLayerFilter;

        // Initialize the Body Manager
        m_bodyManager.Init(maxBodies, createInfo.m_numBodyMutexes, *createInfo.m_pLayerInterface);
        
        // Create the Broadphase.
        // [TODO]: The idea is that the Broadphase class can be modified in the future, but for now,
        // I am going to force the use of the QuadTree version.
        m_pBroadphase = NES_NEW(BroadPhaseQuadTree());
        m_pBroadphase->Init(&m_bodyManager, *createInfo.m_pLayerInterface);

        // Init Contact Constraint Manager:

        // Init Islands Builder

        // Init the Body Interface...

        // [TODO]: 
        // Init the NarrowPhase Query
    }

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

    void PhysicsScene::OptimizeBroadPhase()
    {
        NES_ASSERT(m_pBroadphase != nullptr);
        m_pBroadphase->Optimize();
    }

    PhysicsUpdateErrorCode PhysicsScene::Update(const float deltaTime, int collisionSteps, StackAllocator* pAllocator/*, JobSystem* pJobSystem*/)
    {
        NES_ASSERT(m_pBroadphase != nullptr);
        NES_ASSERT(collisionSteps > 0);
        NES_ASSERT(deltaTime > 0.0f);

        // Sync point for the Broadphase. This will allow it to do clean up operations without having any mutexes locked yet.
        m_pBroadphase->FrameSync();

        // If there are no active bodies (and no step listener to wake them up) or there's no time delta
        const uint32_t numActiveRigidBodies = m_bodyManager.GetNumActiveBodies();
        if ((numActiveRigidBodies == 0 && m_stepListeners.empty()) || deltaTime <= 0.f)
        {
            m_bodyManager.LockAllBodies();

            // Update the Broadphase:
            m_pBroadphase->LockModifications();
            BroadPhase::UpdateState updateState = m_pBroadphase->UpdatePrepare();
            m_pBroadphase->UpdateFinalize(updateState);
            m_pBroadphase->UnlockModifications();

            // [TODO]: 
            // If time has passed, call contact removal callbacks from contacts that existed in the previous update
            //if (deltaTime > 0.0f)
            //  m_contactManager.FinalizeContactCache

            m_bodyManager.UnlockAllBodies();
            return PhysicsUpdateErrorCode::None;
        }

        // Calculate the ratio between the current and previous frame deltaTime to scale initial constraint forces
        const float stepDeltaTime = deltaTime / static_cast<float>(collisionSteps);
        const float warmStartImpulseRatio = m_settings.m_useConstraintWarmStart && m_previousStepDeltaTime > 0.f ? stepDeltaTime / m_previousStepDeltaTime : 0.0f;

        // Create the Context used for passing information between Jobs:
        PhysicsUpdateContext context = PhysicsUpdateContext(*pAllocator);
        // [TODO]: Job System:
        context.m_pScene = this;
        context.m_stepDeltaTime = stepDeltaTime;
        context.m_warmStartImpulseRatio = warmStartImpulseRatio;
        context.m_steps.resize(collisionSteps);

        // Allocate space for body pairs
        //NES_ASSERT(context.m_pBodyPairs == nullptr);
        //context.m_pBodyPairs = static_cast<BodyPair*>(pAllocator->Allocate(sizeof(BodyPair) * m_settings.m_maxInFlightBodyPairs));

        // Lock all bodies for write:
        m_stepListenersMutex.lock();
        m_bodyManager.LockAllBodies();
        m_pBroadphase->LockModifications();

        //const int maxConcurrency = context.GetMaxConcurrency();

        // Calculate how many step listener jobs we need to spawn:
        //const int numStepListenerJobs = m_stepListeners.empty()? 0 : math::Max(static_cast<int>(m_stepListeners.size()) / m_settings.m_stepListenersBatchSize / m_settings.m_stepListenersBatchesPerJob, maxConcurrency);

        // [TODO]: Build and Run Jobs:
        
        // Report any accumulated errors:
        const auto errors = static_cast<PhysicsUpdateErrorCode>(context.m_errors.load(std::memory_order_acquire));
        NES_ASSERTV(errors == PhysicsUpdateErrorCode::None, "An Error occurred during the physics update! Error: ", ToString(errors));
        return errors;
    }
}
