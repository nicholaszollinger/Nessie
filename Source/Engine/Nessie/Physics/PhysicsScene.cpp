// PhysicsScene.cpp
#include "PhysicsScene.h"
#include "Collision/BroadPhase/BroadPhaseQuadTree.h"
#include "Collision/CollisionSolver.h"
#include "Collision/AABoxCast.h"
#include "Collision/ShapeCast.h"
#include "Collision/CollideShape.h"
#include "Collision/CollisionCollector.h"
#include "Collision/CastResult.h"
//#include "Collision/CollideConvexVsTriangles.h" 
#include "Collision/ManifoldBetweenTwoFaces.h"
#include "Collision/Shapes/ConvexShape.h"
//#include "Collision/SimShapeFilterWrapper.h"
#include "Collision/InternalEdgeRemovingCollector.h"
#include "Constraints/CalculateSolverSteps.h"
#include "Constraints/ConstraintPart/AxisConstraintPart.h"
#include "Geometry/RayAABox.h"
#include "Geometry/ClosestPoint.h"
#include "Core/Jobs/JobSystem.h"
#include "Core/Memory/StackAllocator.h"
#include "Core/QuickSort.h"

namespace nes
{
    PhysicsScene::PhysicsScene()
        : m_contactManager(m_physicsSettings)
        NES_IF_ASSERTS_ENABLED(, m_constraintManager(&m_bodyManager))
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
        m_contactManager.Init(createInfo.m_maxNumBodyPairs, createInfo.m_maxNumContactConstraints);
        
        // Init Islands Builder
        m_islandBuilder.Init(maxBodies);
        
        // Init the Body Interface
        m_bodyInterfaceLocking.Internal_Init(m_bodyLockInterfaceLocking, m_bodyManager, *m_pBroadphase);
        m_bodyInterfaceNoLock.Internal_Init(m_bodyLockInterfaceNoLock, m_bodyManager, *m_pBroadphase);
        
        // Init the NarrowPhase Query
        m_narrowPhaseQueryLocking.Internal_Init(m_bodyLockInterfaceLocking, *m_pBroadphase);
        m_narrowPhaseQueryNoLock.Internal_Init(m_bodyLockInterfaceNoLock, *m_pBroadphase);
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

    void PhysicsScene::AddStepListener(PhysicsStepListener* pListener)
    {
        std::lock_guard lock(m_stepListenersMutex);

        NES_ASSERT(std::ranges::find(m_stepListeners.begin(), m_stepListeners.end(), pListener) != m_stepListeners.end());
        m_stepListeners.push_back(pListener);
    }

    void PhysicsScene::RemoveStepListener(PhysicsStepListener* pListener)
    {
        std::lock_guard lock(m_stepListenersMutex);

        StepListeners::iterator it = std::ranges::find(m_stepListeners.begin(), m_stepListeners.end(), pListener);
        NES_ASSERT(it != m_stepListeners.end());
        *it = m_stepListeners.back();
        m_stepListeners.pop_back();
    }

    EPhysicsUpdateErrorCode PhysicsScene::Update([[maybe_unused]] const float deltaTime, [[maybe_unused]] const int collisionSteps, [[maybe_unused]] StackAllocator* pAllocator, [[maybe_unused]] JobSystem* pJobSystem)
    {
        NES_ASSERT(m_pBroadphase != nullptr);
        NES_ASSERT(collisionSteps > 0);
        NES_ASSERT(deltaTime > 0.0f);

        /*
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
            
            // If time has passed, call contact removal callbacks from contacts that existed in the previous update
            if (deltaTime > 0.0f)
              m_contactManager.FinalizeContactCacheAndCallContactPointRemovedCallback(0, 0);
        
            m_bodyManager.UnlockAllBodies();
            return EPhysicsUpdateErrorCode::None;
        }
        
        // Calculate the ratio between the current and previous frame deltaTime to scale initial constraint forces
        const float stepDeltaTime = deltaTime / static_cast<float>(collisionSteps);
        const float warmStartImpulseRatio = m_physicsSettings.m_useConstraintWarmStart && m_previousStepDeltaTime > 0.f ? stepDeltaTime / m_previousStepDeltaTime : 0.0f;
        m_previousStepDeltaTime = stepDeltaTime;
        
        // Create the Context used for passing information between Jobs:
        PhysicsUpdateContext context = PhysicsUpdateContext(*pAllocator);
        context.m_pPhysicsScene = this;
        context.m_pJobSystem = pJobSystem;
        context.m_pBarrier = pJobSystem->CreateBarrier();
        context.m_pIslandBuilder = &m_islandBuilder;
        context.m_stepDeltaTime = stepDeltaTime;
        context.m_warmStartImpulseRatio = warmStartImpulseRatio;
        context.m_steps.resize(collisionSteps);
        
        // Allocate space for body pairs
        NES_ASSERT(context.m_pBodyPairs == nullptr);
        context.m_pBodyPairs = static_cast<BodyPair*>(pAllocator->Allocate(sizeof(BodyPair) * m_physicsSettings.m_maxInFlightBodyPairs));
        
        // Lock all bodies for write, so that we can freely touch them.
        m_stepListenersMutex.lock();
        m_bodyManager.LockAllBodies();
        m_pBroadphase->LockModifications();

        // Get the max number of concurrent jobs
        const int maxConcurrency = context.GetMaxConcurrency();
        
        // Calculate how many step listener jobs we need to spawn:
        const int numStepListenerJobs = m_stepListeners.empty()? 0 : math::Max(static_cast<int>(m_stepListeners.size()) / m_physicsSettings.m_stepListenersBatchSize / m_physicsSettings.m_stepListenersBatchesPerJob, maxConcurrency);

        // The number of gravity jobs depends on the number of active bodies.
        // Launch max 1 job per batch of active bodies.
        // Leave 1 thread for update broadphase prepare and 1 for determine active constraints.
        const int numApplyGravityJobs = math::Max(1, math::Min((static_cast<int>(numActiveRigidBodies) + kApplyGravityBatchSize - 1) / kApplyGravityBatchSize, maxConcurrency - 2));

        // The number of determine active constraints jobs to run depends on the number of constraints.
        // Leave 1 thread for update broadphase prepare and 1 thread for apply gravity.
        const int numDetermineActiveConstraintsJobs = math::Max(1, math::Min((static_cast<int>(m_constraintManager.GetNumConstraints()) + kDetermineActiveConstraintsBatchSize - 1) / kDetermineActiveConstraintsBatchSize, maxConcurrency - 2));

        // The number of setup velocity constraints jobs to run depends on the number of constraints.
        const int numSetupVelocityConstraintsJobs = math::Max(1, math::Min((static_cast<int>(m_constraintManager.GetNumConstraints()) + kSetupVelocityConstraintsBatchSize - 1) / kSetupVelocityConstraintsBatchSize, maxConcurrency));

        // The number of find collisions jobs to run depends on the number of active bodies.
        // Note that when we have more than 1 thread, we always spawn at least 2 find collisions jobs so that the first job can wait for build islands from
        // constraints (Which may activate additional bodies that need to be processed) while the second job can start processing the collision work.
        const int numFindCollisionsJobs = math::Max(maxConcurrency == 1? 1 : 2, math::Min((static_cast<int>(numActiveRigidBodies) + kActiveBodiesBatchSize - 1) / kActiveBodiesBatchSize, maxConcurrency));

        // The number of integrate velocities jobs depends on the number of active bodies.
        const int numIntegrateVelocityJobs = math::Max(1, math::Min((static_cast<int>(numActiveRigidBodies) + kIntegrateVelocityBatchSize - 1) / kIntegrateVelocityBatchSize, maxConcurrency));
        
        // Build and Run Jobs:
        {
            // [TODO]: Scoped Profile: "Build Jobs"
            for (int stepIndex = 0; stepIndex < collisionSteps; ++stepIndex)
            {
                const bool isFirstStep = stepIndex == 0;
                const bool isLastStep = stepIndex == collisionSteps - 1;
        
                PhysicsUpdateContext::Step& step = context.m_steps[stepIndex];
                step.m_pContext = &context;
                step.m_isFirst = isFirstStep;
                step.m_isLast = isLastStep;
        
                // Create a Job to do the broadphase finalization.
                // This job must finish before integrating velocities. Until then, the positions will not be updated, nor will
                // bodies be added or removed.
                // Dependencies: All Find Collision Jobs, Broadphase Prepare, Finish Building Jobs 
                step.m_broadPhaseFinalize = pJobSystem->CreateJob("Update Broadphase Finalize", [&context, &step]()
                {
                    // Validate that all find collision jobs have stopped.
                    NES_ASSERT(step.m_activeFindCollisionJobs.load(std::memory_order_relaxed) == 0);
        
                    // Finalize the Broadphase update:
                    context.m_pPhysicsScene->m_pBroadphase->UpdateFinalize(step.m_broadPhaseUpdateState);
        
                    // Signal that it is done.
                    step.m_preIntegrateVelocity.RemoveDependency();
                }, numFindCollisionsJobs + 2);
        
                // The immediate jobs below are only immediate for the first step - then all finished jobs will
                // kick them for the next step
                const int previousStepDependencyCount = isFirstStep? 0 : 1;
        
                // Start this job immediately: Start the PrepareBroadphase
                // This must be done under body lock protection since the order is body locks then broadphase mutex
                // If this is turned around, the RemoveBody call will hang since it locks in that order
                step.m_broadPhasePrepare = pJobSystem->CreateJob("UpdateBroadphasePrepare", [&context, &step]()
                {
                    // Prepare the broadphase update
                    step.m_broadPhaseUpdateState = context.m_pPhysicsScene->m_pBroadphase->UpdatePrepare();
        
                    // Now the Finalize job can run (if other dependencies are met too).
                    step.m_broadPhaseFinalize.RemoveDependency();
                    
                }, previousStepDependencyCount);
        
                // This job will find all collisions
                step.m_bodyPairQueues.resize(maxConcurrency);
                step.m_maxBodyPairsPerQueue = m_physicsSettings.m_maxInFlightBodyPairs / maxConcurrency;
                step.m_activeFindCollisionJobs.store(~static_cast<PhysicsUpdateContext::JobMask>(0) >> (sizeof(PhysicsUpdateContext::JobMask) * 8 - numFindCollisionsJobs), std::memory_order_release);
                step.m_findCollisions.resize(numFindCollisionsJobs);
                for (int i = 0; i < numFindCollisionsJobs; ++i)
                {
                    // Build islands from constraints may activate additional bodies, so the first job will wait for this to finish to not miss
                    // any active bodies.
                    const int numDepBuildIslandsFromConstraints = i == 0? 1 : 0;
                    step.m_findCollisions[i] = pJobSystem->CreateJob("Find Collisions", [&step, i]()
                    {
                        step.m_pContext->m_pPhysicsScene->JobFindCollisions(&step, i);
                    }, numApplyGravityJobs + numDetermineActiveConstraintsJobs + 1 + numDepBuildIslandsFromConstraints);
                }

                if (isFirstStep)
                {
                #ifdef NES_ASSERTS_ENABLED
                    // Don't all write operations to the active bodies list.
                    m_bodyManager.Internal_SetActiveBodiesLocked(true);
                #endif

                    // Store the number of active bodies at the start of the step
                    step.m_numActiveBodiesAtStepStart = m_bodyManager.GetNumActiveBodies();
                    
                    // Lock all constraints
                    m_constraintManager.Internal_LockAllConstraints();

                    // Allocate memory for storing the active constraints.
                    NES_ASSERT(context.m_pActiveConstraints == nullptr);
                    context.m_pActiveConstraints = pAllocator->Allocate<Constraint*>(m_constraintManager.GetNumConstraints() * sizeof(Constraint*));

                    // Prepare the contact buffer
                    m_contactManager.PrepareConstraintBuffer(&context);

                    // Setup island builder
                    m_islandBuilder.PrepareContactConstraints(m_contactManager.GetMaxConstraints(), context.m_pAllocator);
                }

                // Apply Gravity Jobs: applies gravity to all bodies.
                // Dependencies: Step Listeners (or previous step if no step listeners).
                step.m_applyGravity.resize(numApplyGravityJobs);
                for (int i = 0; i < numApplyGravityJobs; ++i)
                {
                    step.m_applyGravity[i] = pJobSystem->CreateJob("Apply Gravity", [&context, &step]()
                    {
                        context.m_pPhysicsScene->JobApplyGravity(&context, &step);
                        JobHandle::RemovedDependencies(step.m_findCollisions);
                        
                    }, numStepListenerJobs > 0? numStepListenerJobs : previousStepDependencyCount);
                }

                // Setup Velocity Jobs: sets up velocity constraints for non-collision constraints.
                // Dependencies: determine active constraints, finishing building jobs.
                step.m_setupVelocityConstraints.resize(numSetupVelocityConstraintsJobs);
                for (int i = 0; i < numSetupVelocityConstraintsJobs; ++i)
                {
                    step.m_setupVelocityConstraints[i] = pJobSystem->CreateJob("Setup Velocity Constraints", [&context, &step]()
                    {
                        context.m_pPhysicsScene->JobSetupVelocityConstraints(context.m_stepDeltaTime, &step);
                        JobHandle::RemovedDependencies(step.m_solveVelocityConstraints);
                        
                    }, numDetermineActiveConstraintsJobs + 1);
                }

                // Build Islands Job: builds the islands from the constraints.
                // Dependencies: determine active constraints, finishing building jobs.
                step.m_buildIslandsFromConstraints = pJobSystem->CreateJob("Build Islands From Constraints", [&context, &step]()
                {
                    context.m_pPhysicsScene->JobBuildIslandsFromConstraints(&context, &step);

                    step.m_findCollisions[0].RemoveDependency(); // The first collisions job cannot start running until we've finished building islands and activated all bodies.
                    step.m_finalizeIslands.RemoveDependency();
                    
                }, numDetermineActiveConstraintsJobs + 1);

                // Determine Active Constraints Jobs:
                // Dependencies: step listeners (or previous step if no step listeners).
                step.m_determineActiveConstraints.resize(numDetermineActiveConstraintsJobs);
                for (int i = 0; i < numDetermineActiveConstraintsJobs; ++i)
                {
                    step.m_determineActiveConstraints[i] = pJobSystem->CreateJob("Determine Active Constraints", [&context, &step]()
                    {
                        context.m_pPhysicsScene->JobDetermineActiveConstraints(&step);
                        step.m_buildIslandsFromConstraints.RemoveDependency();

                        // Kick these jobs last as they will use up all CPU cores leaving no space for the previous job,
                        // we prefer setup velocity constraints to finish first so we kick it first.
                        JobHandle::RemovedDependencies(step.m_setupVelocityConstraints);
                        JobHandle::RemovedDependencies(step.m_findCollisions);
                        
                    }, numStepListenerJobs > 0? numStepListenerJobs : previousStepDependencyCount);
                }

                // Step Listeners Jobs
                step.m_stepListeners.resize(numStepListenerJobs);
                for (int i = 0; i < numStepListenerJobs; ++i)
                {
                    step.m_stepListeners[i] = pJobSystem->CreateJob("Step Listeners", [&context, &step]()
                    {
                        // Call the step listeners
                        context.m_pPhysicsScene->JobStepListeners(&step);

                        // Kick ApplyGravity and DetermineActiveConstraints jobs
                        JobHandle::RemovedDependencies(step.m_applyGravity);
                        JobHandle::RemovedDependencies(step.m_determineActiveConstraints);
                        
                    }, previousStepDependencyCount);
                }

                // Unblock the previous step
                if (!isFirstStep)
                    context.m_steps[stepIndex - 1].m_startNextStep.RemoveDependency();

                // [TODO]: Finished on line 357 - Finalize Islands
            }
        }
        
        // Report any accumulated errors:
        //const auto errors = static_cast<PhysicsUpdateErrorCode>(context.m_errors.load(std::memory_order_acquire));
        //NES_ASSERTV(errors == PhysicsUpdateErrorCode::None, "An Error occurred during the physics update! Error: ", ToString(errors));
        //return errors;

        */
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

    void PhysicsScene::JobSetupVelocityConstraints([[maybe_unused]] float deltaTime,[[maybe_unused]]  PhysicsUpdateContext::Step* pStep) const
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

    void PhysicsScene::JobPreIntegrateVelocity([[maybe_unused]] PhysicsUpdateContext* pContext, [[maybe_unused]] PhysicsUpdateContext::Step* pStep)
    {
        // [TODO]: 
    }

    void PhysicsScene::JobIntegrateVelocity([[maybe_unused]] const PhysicsUpdateContext* pContext, [[maybe_unused]] PhysicsUpdateContext::Step* pStep)
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

    void PhysicsScene::ProcessBodyPair([[maybe_unused]] ContactAllocator& contactAllocator, [[maybe_unused]] const BodyPair& bodyPair)
    {
        // [TODO]: 
    }

    void PhysicsScene::CheckSleepAndUpdateBounds([[maybe_unused]] const uint32 islandIndex, [[maybe_unused]] const PhysicsUpdateContext* pContext, [[maybe_unused]] const PhysicsUpdateContext::Step* pStep, [[maybe_unused]] BodiesToSleep& bodiesToSleep)
    {
        // [TODO]: 
    }
}
