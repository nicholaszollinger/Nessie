// PhysicsScene.cpp
#include "PhysicsScene.h"
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
        NES_ASSERT(maxBodies == createInfo.m_maxBodies, "Cannot support this many bodies!");

        m_pCollisionLayerPairFilter = createInfo.m_pCollisionLayerPairFilter;
        m_pCollisionVsBroadPhaseLayerFilter = createInfo.m_pCollisionVsBroadPhaseLayerFilter;

        // Initialize the Body Manager
        m_bodyManager.Init(maxBodies, createInfo.m_numBodyMutexes, *createInfo.m_pLayerInterface);
        
        // Create the Broadphase.
        // [LATER]: The idea is that the Broadphase class can be modified in the future, but for now,
        // I am going to force the use of the QuadTree version.
        m_pBroadphase = NES_NEW(BroadPhaseQuadTree());
        m_pBroadphase->Init(&m_bodyManager, *createInfo.m_pLayerInterface);

        // Init Contact Constraint Manager:

        // Init Islands Builder

        // Init the Body Interface
        m_bodyInterfaceLocking.Internal_Init(m_bodyLockInterfaceLocking, m_bodyManager, *m_pBroadphase);
        m_bodyInterfaceNoLock.Internal_Init(m_bodyLockInterfaceNoLock, m_bodyManager, *m_pBroadphase);

        // [TODO]: 
        // Init the NarrowPhase Query
    }
    
    void PhysicsScene::AddConstraint(Constraint* pConstraint)
    {
        m_constraintManager.Add(&pConstraint, 1);
    }

    void PhysicsScene::AddConstraints(Constraint** constraintsArray, const int numConstraints)
    {
        m_constraintManager.Add(constraintsArray, numConstraints);
    }
    
    void PhysicsScene::RemoveConstraint(Constraint* pConstraint)
    {
        m_constraintManager.Remove(&pConstraint, 1);
    }

    void PhysicsScene::RemoveConstraints(Constraint** constraintsArray, const int numConstraints)
    {
        m_constraintManager.Remove(constraintsArray, numConstraints);
    }

    void PhysicsScene::OptimizeBroadPhase()
    {
        NES_ASSERT(m_pBroadphase != nullptr);
        m_pBroadphase->Optimize();
    }

    EPhysicsUpdateErrorCode PhysicsScene::Update([[maybe_unused]] const float deltaTime, [[maybe_unused]] const int collisionSteps, [[maybe_unused]] StackAllocator* pAllocator, JobSystem* pJobSystem)
    {
        NES_ASSERT(m_pBroadphase != nullptr);
        NES_ASSERT(collisionSteps > 0);
        NES_ASSERT(deltaTime > 0.0f);

        // [TEMP]: This is a test case:  
        int value = 0;
        auto testJob = pJobSystem->CreateJob("Test Job", [&value]()
        {
            value += 5;
        });
        auto pBarrier = pJobSystem->CreateBarrier();
        pBarrier->AddJob(testJob);
        pJobSystem->WaitForJobs(pBarrier);
        pJobSystem->DestroyBarrier(pBarrier);

        // // Sync point for the Broadphase. This will allow it to do clean up operations without having any mutexes locked yet.
        // m_pBroadphase->FrameSync();
        //
        // // If there are no active bodies (and no step listener to wake them up) or there's no time delta
        // const uint32_t numActiveRigidBodies = m_bodyManager.GetNumActiveBodies();
        // if ((numActiveRigidBodies == 0 && m_stepListeners.empty()) || deltaTime <= 0.f)
        // {
        //     m_bodyManager.LockAllBodies();
        //
        //     // Update the Broadphase:
        //     m_pBroadphase->LockModifications();
        //     BroadPhase::UpdateState updateState = m_pBroadphase->UpdatePrepare();
        //     m_pBroadphase->UpdateFinalize(updateState);
        //     m_pBroadphase->UnlockModifications();
        //
        //     // [TODO]: 
        //     // If time has passed, call contact removal callbacks from contacts that existed in the previous update
        //     //if (deltaTime > 0.0f)
        //     //  m_contactManager.FinalizeContactCache
        //
        //     m_bodyManager.UnlockAllBodies();
        //     return PhysicsUpdateErrorCode::None;
        // }
        //
        // // Calculate the ratio between the current and previous frame deltaTime to scale initial constraint forces
        // const float stepDeltaTime = deltaTime / static_cast<float>(collisionSteps);
        // const float warmStartImpulseRatio = m_settings.m_useConstraintWarmStart && m_previousStepDeltaTime > 0.f ? stepDeltaTime / m_previousStepDeltaTime : 0.0f;
        //
        // // Create the Context used for passing information between Jobs:
        // PhysicsUpdateContext context = PhysicsUpdateContext(*pAllocator);
        // context.m_pScene = this;
        // context.m_pJobSystem = pJobSystem;
        // context.m_pBarrier = pJobSystem->CreateBarrier();
        // //context.m_pIslandBuilder = &m_islandBuilder;
        // context.m_stepDeltaTime = stepDeltaTime;
        // context.m_warmStartImpulseRatio = warmStartImpulseRatio;
        // context.m_steps.resize(collisionSteps);
        //
        // // Allocate space for body pairs
        // NES_ASSERT(context.m_pBodyPairs == nullptr);
        // //context.m_pBodyPairs = static_cast<BodyPair*>(pAllocator->Allocate(sizeof(BodyPair) * m_settings.m_maxInFlightBodyPairs));
        //
        // // Lock all bodies for write:
        // m_stepListenersMutex.lock();
        // m_bodyManager.LockAllBodies();
        // m_pBroadphase->LockModifications();
        //
        // const int maxConcurrency = context.GetMaxConcurrency();
        //
        // // [TODO]: 
        // // Calculate how many step listener jobs we need to spawn:
        // [[maybe_unused]] const int numStepListenerJobs = m_stepListeners.empty()? 0 : math::Max(static_cast<int>(m_stepListeners.size()) / m_settings.m_stepListenersBatchSize / m_settings.m_stepListenersBatchesPerJob, maxConcurrency);
        //
        // // [TODO]: Calculate more Job counts...
        // // [TODO]: 
        // const int numFindCollisionsJobs = 0;
        //
        // // [TODO]: Build and Run Jobs:
        // {
        //     // [TODO]: Scoped Profile: "Build Jobs"
        //
        //     for (int stepIndex = 0; stepIndex < collisionSteps; ++stepIndex)
        //     {
        //         const bool isFirstStep = stepIndex == 0;
        //         const bool isLastStep = stepIndex == collisionSteps - 1;
        //
        //         PhysicsUpdateContext::Step& step = context.m_steps[stepIndex];
        //         step.m_pContext = &context;
        //         step.m_isFirst = isFirstStep;
        //         step.m_isLast = isLastStep;
        //
        //         // Create Job to do the broadphase finalization.
        //         // This job must finish before integrating velocities. Until then the positions will not be updated nor will
        //         // bodies be added or removed.
        //         // Dependencies: All Find Collision Jobs, Broadphase Prepare, Finish Building Jobs 
        //         step.m_broadPhaseFinalize = pJobSystem->CreateJob("Update Broadphase Finalize", [&context, &step]()
        //         {
        //             // Validate that all find collision jobs have stopped.
        //             NES_ASSERT(step.m_activeFindCollisionJobs.load(std::memory_order_relaxed) == 0);
        //
        //             // Finalize the Broadphase update:
        //             context.m_pScene->m_pBroadphase->UpdateFinalize(step.m_broadPhaseUpdateState);
        //
        //             // Signal that it is done.
        //             step.m_preIntegrateVelocity.RemoveDependency();
        //         }, numFindCollisionsJobs + 2);
        //
        //         // The immediate jobs below are only immediate for the first step - then all finished jobs will
        //         // kick them off for the next step
        //         int previousStepDependencyCount = isFirstStep? 0 : 1;
        //
        //         // Start job immediately: Start prepare broadphase
        //         // Must be done under body lock protection since the order is body locks then broadphase mutex
        //         // If this is turned around the RemoveBody call will hang since it locks in that order
        //         step.m_broadPhasePrepare = pJobSystem->CreateJob("UpdateBroadphasePrepare", [&context, &step]()
        //         {
        //             // Prepare the broadphase update
        //             step.m_broadPhaseUpdateState = context.m_pScene->m_pBroadphase->UpdatePrepare();
        //
        //             // Now the finalize can run (if other dependencies are met too).
        //             step.m_broadPhaseFinalize.RemoveDependency();
        //             
        //         }, previousStepDependencyCount);
        //
        //         // This job will find all collisions
        //         step.m_bodyPairQueues.resize(maxConcurrency);
        //         step.m_maxBodyPairsPerQueue = m_settings.m_maxInFlightBodyPairs / maxConcurrency;
        //         step.m_activeFindCollisionJobs.store(~static_cast<PhysicsUpdateContext::JobMask>(0) >> (sizeof(PhysicsUpdateContext::JobMask) * 8 - numFindCollisionsJobs), std::memory_order_release);
        //         step.m_findCollisions.resize(numFindCollisionsJobs);
        //         for (int i = 0; i < numFindCollisionsJobs; ++i)
        //         {
        //             // Build islands from constraints.
        //         }
        //     }
        // }
        
        // Report any accumulated errors:
        //const auto errors = static_cast<PhysicsUpdateErrorCode>(context.m_errors.load(std::memory_order_acquire));
        //NES_ASSERTV(errors == PhysicsUpdateErrorCode::None, "An Error occurred during the physics update! Error: ", ToString(errors));
        //return errors;
        return EPhysicsUpdateErrorCode::None;
    }

    void PhysicsScene::JobStepListeners([[maybe_unused]] PhysicsUpdateContext::Step* pStep)
    {
        // [TODO]: 
    }

    void PhysicsScene::JobDetermineActiveConstraints([[maybe_unused]] PhysicsUpdateContext::Step* pStep) const
    {
        // [TODO]: 
    }

    void PhysicsScene::JobApplyGravity([[maybe_unused]] const PhysicsUpdateContext* pContext,[[maybe_unused]]  PhysicsUpdateContext::Step* pStep)
    {
        // [TODO]: 
    }

    void PhysicsScene::JobSetupVelocityContstraints([[maybe_unused]] float deltaTime,[[maybe_unused]]  PhysicsUpdateContext::Step* pStep) const
    {
        // [TODO]: 
    }

    void PhysicsScene::JobBuildIslandsFromConstraints([[maybe_unused]] PhysicsUpdateContext* pContext,[[maybe_unused]]  PhysicsUpdateContext::Step* pStep)
    {
        // [TODO]: 
    }

    void PhysicsScene::JobFindCollisions([[maybe_unused]] PhysicsUpdateContext::Step* pStep, [[maybe_unused]] const int jobIndex)
    {
        // [TODO]: 
    }

    void PhysicsScene::JobFinalizeIslands([[maybe_unused]] PhysicsUpdateContext* pContext)
    {
        // [TODO]: 
    }

    void PhysicsScene::JobBodySetIslandIndex()
    {
        // [TODO]: 
    }

    void PhysicsScene::JobSolveVelocityConstraints([[maybe_unused]] PhysicsUpdateContext* pContext, [[maybe_unused]] PhysicsUpdateContext::Step* pStep)
    {
        // [TODO]: 
    }

    void PhysicsScene::JobPreIntegrateVelocity([[maybe_unused]] const PhysicsUpdateContext* pContext, [[maybe_unused]] PhysicsUpdateContext::Step* pStep)
    {
        // [TODO]: 
    }

    void PhysicsScene::JobPostIntegrateVelocity([[maybe_unused]] PhysicsUpdateContext* pContext, [[maybe_unused]] PhysicsUpdateContext::Step* pStep) const
    {
        // [TODO]: 
    }

    void PhysicsScene::JobFindCCDContacts([[maybe_unused]] const PhysicsUpdateContext* pContext, [[maybe_unused]] PhysicsUpdateContext::Step* pStep)
    {
        // [TODO]: 
    }

    void PhysicsScene::JobResolveCCDContacts([[maybe_unused]] PhysicsUpdateContext* pContext, [[maybe_unused]] PhysicsUpdateContext::Step* pStep)
    {
        // [TODO]: 
    }

    void PhysicsScene::JobContactRemovedCallbacks([[maybe_unused]] const PhysicsUpdateContext::Step* pStep)
    {
        // [TODO]: 
    }

    void PhysicsScene::JobSolvePositionConstraints([[maybe_unused]] PhysicsUpdateContext* pContext, [[maybe_unused]] PhysicsUpdateContext::Step* pStep)
    {
        // [TODO]: 
    }

    void PhysicsScene::TrySpawnJobFindCollisions([[maybe_unused]] PhysicsUpdateContext::Step* pStep) const
    {
        // [TODO]: 
    }
}
