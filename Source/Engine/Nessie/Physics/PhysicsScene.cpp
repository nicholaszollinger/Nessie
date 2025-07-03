// PhysicsScene.cpp
#include "PhysicsScene.h"
#include "PhysicsStepListener.h"
#include "Nessie/Physics/Collision/BroadPhase/BroadPhaseQuadTree.h"
#include "Nessie/Physics/Collision/CollisionSolver.h"
#include "Nessie/Physics/Collision/AABoxCast.h"
#include "Nessie/Physics/Collision/ShapeCast.h"
#include "Nessie/Physics/Collision/CollideShape.h"
#include "Nessie/Physics/Collision/CollisionCollector.h"
#include "Nessie/Physics/Collision/CastResult.h"
//#include "Collision/CollideConvexVsTriangles.h" 
#include "Nessie/Physics/Collision/ManifoldBetweenTwoFaces.h"
#include "Nessie/Physics/Collision/Shapes/ConvexShape.h"
#include "Nessie/Physics/Collision/SimShapeFilterWrapper.h"
#include "Nessie/Physics/Collision/InternalEdgeRemovingCollector.h"
#include "Nessie/Physics/Constraints/CalculateSolverSteps.h"
#include "Nessie/Physics/Constraints/ContactConstraintManager.h"
#include "Nessie/Physics/Constraints/ConstraintPart/AxisConstraintPart.h"
#include "Nessie/Geometry/RayAABox.h"
#include "Nessie/Geometry/ClosestPoint.h"
#include "Nessie/Core/Jobs/JobSystem.h"
#include "Nessie/Core/Memory/StackAllocator.h"
#include "Nessie/Core/QuickSort.h"
#include "Nessie/Core/ScopeExit.h"

namespace nes
{
    static void FinalizeContactAllocator(PhysicsUpdateContext::Step& step, const ContactConstraintManager::ContactAllocator& allocator)
    {
        // Atomically accumulate the number of found manifolds and body pairs.
        step.m_numBodyPairs.fetch_add(allocator.m_numBodyPairs, std::memory_order_relaxed);
        step.m_numManifolds.fetch_add(allocator.m_numManifolds, std::memory_order_relaxed);

        // Combine update errors
        step.m_pContext->m_errors.fetch_or(static_cast<uint32>(allocator.m_errors), std::memory_order_relaxed);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper function to calculate the motion of a body during this CCD step.
    //----------------------------------------------------------------------------------------------------
    inline static Vec3 CalculateBodyMotion(const Body& body, const float deltaTime)
    {
        // If the body is linear casting, the body has not yet moved so we need to calculate its motion
        if (body.IsDynamic() && body.GetMotionProperties()->GetMotionQuality() == EBodyMotionQuality::LinearCast)
            return deltaTime * body.GetLinearVelocity();

        // The body has already moved, so we don't need to correct for anything.
        return Vec3::Zero();
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper function that finds the CCD body corresponding to a body (if it exists).
    //----------------------------------------------------------------------------------------------------
    inline static PhysicsUpdateContext::Step::CCDBody* GetCCDBody(const Body& body, PhysicsUpdateContext::Step* pStep)
    {
        // Only rigid bodies have a CCD body
        if (!body.IsRigidBody())
            return nullptr;

        // If the body has no motion properties, it cannot have a CCD body.
        const MotionProperties* pMotionProps = body.GetMotionPropertiesUnchecked();
        if (pMotionProps == nullptr)
            return nullptr;

        // If it is not active, it cannot have a CCD body.
        const uint32 activeIndex = pMotionProps->Internal_GetIndexInActiveBodies();
        if (activeIndex == Body::kInactiveIndex)
            return nullptr;

        // Check if the body has a corresponding CCD body.
        NES_ASSERT(activeIndex < pStep->m_numActiveBodyToCCDBodies); // Ensure that the body has a mapping to a CCD body.
        const int ccdIndex = pStep->m_pActiveBodyToCCDBody[activeIndex];
        if (ccdIndex < 0)
            return nullptr;

        PhysicsUpdateContext::Step::CCDBody* pCCDBody = &pStep->m_pCCDBodies[ccdIndex];
        NES_ASSERT(pCCDBody->m_bodyID1 == body.GetID(), "We found the wrong CCD body!");
        return pCCDBody;
    }
    
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

    EPhysicsUpdateErrorCode PhysicsScene::Update(const float deltaTime, const int collisionSteps, StackAllocator* pAllocator, JobSystem* pJobSystem)
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
                    context.m_pActiveConstraints = static_cast<Constraint**>(pAllocator->Allocate(m_constraintManager.GetNumConstraints() * sizeof(Constraint*)));

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
                
                // Finalize Islands Job: finalizes the simulation islands.
                // Dependencies: find Collisions, build islands from constraints, finish building jobs. 
                step.m_finalizeIslands = pJobSystem->CreateJob("Finalize Islands", [&context, &step]()
                {
                    // Validate that all find collision jobs have stopped.
                    NES_ASSERT(step.m_activeFindCollisionJobs.load(std::memory_order_relaxed) == 0);

                    context.m_pPhysicsScene->JobFinalizeIslands(&context);

                    JobHandle::RemovedDependencies(step.m_solveVelocityConstraints);
                    step.m_bodySetIslandIndex.RemoveDependency();
                }, numFindCollisionsJobs + 2);

                // Unblock previous job
                // Note: technically we could release find collision here, but we don't want to because that could make them run before
                // 'setup velocity constraints', which means that job won't have a thread left.
                step.m_buildIslandsFromConstraints.RemoveDependency();

                // Contact Removed Callbacks Job: call the contact removed callbacks.
                // Dependencies: find CCD contacts.
                step.m_contactRemovedCallbacks = pJobSystem->CreateJob("Contact Removed Callbacks", [&context, &step]()
                {
                    context.m_pPhysicsScene->JobContactRemovedCallbacks(&step);

                    if (step.m_startNextStep.IsValid())
                        step.m_startNextStep.RemoveDependency();
                }, 1);

                // Body Set Island Index Job: set the island index on each body (only used for debug drawing purposes).
                // It will also delete any bodies that have been destroyed in the last frame.
                // Dependencies: finalize islands, finish building jobs.
                step.m_bodySetIslandIndex = pJobSystem->CreateJob("Body Set Island Index", [&context, &step]()
                {
                    context.m_pPhysicsScene->JobBodySetIslandIndex();

                    JobHandle::RemovedDependencies(step.m_solvePositionConstraints);
                }, 2);

                // StartNextStep Job: kicks of the next collision step
                // Dependencies: update soft bodies [TODO], contact removed callbacks, finish building the previous setp 
                if (!isLastStep)
                {
                    PhysicsUpdateContext::Step* pNextStep = &context.m_steps[stepIndex + 1];
                    step.m_startNextStep = pJobSystem->CreateJob("Start Next Step", [this, pNextStep]()
                    {
                    #ifdef NES_DEBUG
                        // Validate that the cached bounds are correct
                        m_bodyManager.Internal_ValidateActiveBodyBounds();
                    #endif

                        // Store the number of active bodies at the start of the step
                        pNextStep->m_numActiveBodiesAtStepStart = m_bodyManager.GetNumActiveBodies();

                        // Clear the large island splitter
                        StackAllocator* pTempAllocator = pNextStep->m_pContext->m_pAllocator;
                        m_largeIslandSplitter.Reset(pTempAllocator);

                        // Clear the island builder
                        m_islandBuilder.ResetIslands(pTempAllocator);

                        // Setup island builder
                        m_islandBuilder.PrepareContactConstraints(m_contactManager.GetMaxConstraints(), pTempAllocator);

                        // Restart the contact manager.
                        m_contactManager.RecycleConstraintBuffer();

                        // Kick the jobs of the next step (in the same order as the first step)
                        pNextStep->m_broadPhasePrepare.RemoveDependency();
                        if (pNextStep->m_stepListeners.empty())
                        {
                            // Kick the gravity and active constraints jobs immediately.
                            JobHandle::RemovedDependencies(pNextStep->m_applyGravity);
                            JobHandle::RemovedDependencies(pNextStep->m_determineActiveConstraints);
                        }
                        else
                        {
                            // Kick the step listeners job first
                            JobHandle::RemovedDependencies(pNextStep->m_stepListeners);
                        }
                    }, 3);
                }

                // Solve Velocity Constraints Job
                // Dependencies: finalize islands, setup velocity constraints, finish building jobs.
                step.m_solveVelocityConstraints.resize(maxConcurrency);
                for (int i = 0; i < maxConcurrency; ++i)
                {
                    step.m_solveVelocityConstraints[i] = pJobSystem->CreateJob("Solve Velocity Constraints", [&context, &step]()
                    {
                        context.m_pPhysicsScene->JobSolveVelocityConstraints(&context, &step);

                        step.m_preIntegrateVelocity.RemoveDependency();
                    }, numSetupVelocityConstraintsJobs + 2);
                }
                
                // We prefer setup velocity constraints to finish first, so we kick it first.
                JobHandle::RemovedDependencies(step.m_setupVelocityConstraints);
                JobHandle::RemovedDependencies(step.m_findCollisions);

                // Finalize islands is a dependency on find collisions so it can go last.
                step.m_finalizeIslands.RemoveDependency();

                // PreIntegrate Velocity Job: This will prepare the position update of all active bodies.
                // Dependencies: broadphase update finalize, solve velocity constraints, finish building jobs.
                step.m_preIntegrateVelocity = pJobSystem->CreateJob("Pre Integrate Velocity", [&context, &step]()
                {
                    context.m_pPhysicsScene->JobPreIntegrateVelocity(&context, &step);
                    
                    JobHandle::RemovedDependencies(step.m_integrateVelocity);
                }, maxConcurrency + 2);

                // Unblock previous jobs
                step.m_broadPhaseFinalize.RemoveDependency();
                JobHandle::RemovedDependencies(step.m_solveVelocityConstraints);

                // Integrate Velocity Jobs: this will update the positions of all active bodies.
                // Dependencies: pre-integrate velocity, finish building jobs.
                step.m_integrateVelocity.resize(numIntegrateVelocityJobs);
                for (int i = 0; i < numIntegrateVelocityJobs; ++i)
                {
                    step.m_integrateVelocity[i] = pJobSystem->CreateJob("Integrate Velocity", [&context, &step]()
                    {
                        context.m_pPhysicsScene->JobIntegrateVelocity(&context, &step);
                        step.m_postIntegrateVelocity.RemoveDependency();
                    }, 2);
                }

                // Unblock the previous job
                step.m_preIntegrateVelocity.RemoveDependency();

                // Post Integrate Velocity Job: this will finish the position update of all active bodies.
                // Dependencies: integrate velocity, finish building jobs.
                step.m_postIntegrateVelocity = pJobSystem->CreateJob("Post Integrate Velocity", [&context, &step]()
                {
                    context.m_pPhysicsScene->JobPostIntegrateVelocity(&context, &step);
                    step.m_resolveCCDContacts.RemoveDependency();
                }, numIntegrateVelocityJobs + 1);

                // Unblock previous jobs
                JobHandle::RemovedDependencies(step.m_integrateVelocity);

                // Resolve CCD Contacts Job: This will update the positions and velocities for all bodies that need continuous collision detection.
                // Dependencies: integrate velocities, detect CCD contacts (added dynamically), finish building jobs.
                step.m_resolveCCDContacts = pJobSystem->CreateJob("Resolve CCD Contacts", [&context, &step]()
                {
                    context.m_pPhysicsScene->JobResolveCCDContacts(&context, &step);
                    JobHandle::RemovedDependencies(step.m_solvePositionConstraints);
                }, 2);

                // Unblock previous job
                step.m_postIntegrateVelocity.RemoveDependency();

                // Solve Position Constraint Jobs: Fixes drift in positions and updates the broadphase with the new body positions.
                // Dependencies: resolve CCD contacts, body set island index, finish building jobs.
                step.m_solvePositionConstraints.resize(maxConcurrency);
                for (int i = 0; i < maxConcurrency; ++i)
                {
                    step.m_solvePositionConstraints[i] = pJobSystem->CreateJob("Solve Position Constraints", [&context, &step]()
                    {
                        context.m_pPhysicsScene->JobSolvePositionConstraints(&context, &step);

                        // [TODO]: Soft Body:
                        // Start the next step
                        //if (step.m_softBodyPrepare.IsValid()
                        //  step.m_softBodyPrepare.RemoveDependency();
                    }, 3);
                }

                // Unblock previous jobs
                step.m_resolveCCDContacts.RemoveDependency();
                step.m_bodySetIslandIndex.RemoveDependency();

                // [TODO]: Soft Body Prepare

                // Unblock previous jobs
                JobHandle::RemovedDependencies(step.m_solvePositionConstraints);
            }
        }

        // Build the list of jobs to wait for:
        JobSystem::Barrier* pBarrier = context.m_pBarrier;
        {
            // [TODO]: Profile:

            StaticArray<JobHandle, physics::kMaxPhysicsJobs> handles;
            for (const PhysicsUpdateContext::Step& step : context.m_steps)
            {
                if (step.m_broadPhasePrepare.IsValid())
                    handles.push_back(step.m_broadPhasePrepare);

                for (const JobHandle& h : step.m_stepListeners)
                {
                    handles.push_back(h);
                }
                for (const JobHandle& h : step.m_determineActiveConstraints)
                {
                    handles.push_back(h);
                }
                for (const JobHandle& h : step.m_applyGravity)
                {
                    handles.push_back(h);
                }
                for (const JobHandle& h : step.m_findCollisions)
                {
                    handles.push_back(h);
                }
                
                if (step.m_broadPhaseFinalize.IsValid())
                    handles.push_back(step.m_broadPhaseFinalize);
                
                for (const JobHandle& h : step.m_setupVelocityConstraints)
                {
                    handles.push_back(h);
                }

                handles.push_back(step.m_buildIslandsFromConstraints);
                handles.push_back(step.m_finalizeIslands);
                handles.push_back(step.m_bodySetIslandIndex);

                for (const JobHandle& h : step.m_solveVelocityConstraints)
                {
                    handles.push_back(h);
                }

                handles.push_back(step.m_preIntegrateVelocity);
                
                for (const JobHandle& h : step.m_integrateVelocity)
                {
                    handles.push_back(h);
                }

                handles.push_back(step.m_postIntegrateVelocity);
                handles.push_back(step.m_resolveCCDContacts);
                
                for (const JobHandle& h : step.m_solvePositionConstraints)
                {
                    handles.push_back(h);
                }
                
                handles.push_back(step.m_contactRemovedCallbacks);
                // [TODO]: SoftBodyPrepare
                if (step.m_startNextStep.IsValid())
                    handles.push_back(step.m_startNextStep);
            }
            pBarrier->AddJobs(handles.data(), static_cast<uint32>(handles.size()));
        }

        // Wait until all jobs have finished.
        // Note: We don't just wait for the last job. If we did and another job
        // was scheduled in between, there is the possibility of a deadlock.
        // The other job could try to, for example, add/remove a body, which would
        // try to lock a body mutex while this thread has already locked the mutex.
        pJobSystem->WaitForJobs(pBarrier);

        // We're done with the barrier for this update.
        pJobSystem->DestroyBarrier(pBarrier);

    #ifdef NES_DEBUG
        // Validate that the cached bounds are correct.
        m_bodyManager.Internal_ValidateActiveBodyBounds();
    #endif

        // Clear the large island splitter.
        m_largeIslandSplitter.Reset(pAllocator);

        // Clear the island builder
        m_islandBuilder.ResetIslands(pAllocator);

        // Clear the contact manager
        m_contactManager.FinishConstraintBuffer();

        // Free active constraints
        pAllocator->Free(static_cast<void*>(context.m_pActiveConstraints), m_constraintManager.GetNumConstraints() * sizeof(Constraint*));
        context.m_pActiveConstraints = nullptr;

        // Free body pairs
        pAllocator->Free(context.m_pBodyPairs, sizeof(BodyPair) * m_physicsSettings.m_maxInFlightBodyPairs);
        context.m_pBodyPairs = nullptr;

        // Unlock the broadphase
        m_pBroadphase->UnlockModifications();

        // Unlock all constraints
        m_constraintManager.Internal_UnlockAllConstraints();

    #if NES_ASSERTS_ENABLED
        // Allow write operations to the active bodies array.
        m_bodyManager.Internal_SetActiveBodiesLocked(false);
    #endif

        // Unlock all bodies
        m_bodyManager.UnlockAllBodies();

        // Unlock step listeners
        m_stepListenersMutex.unlock();
        
        // Report any accumulated errors:
        const auto errors = static_cast<EPhysicsUpdateErrorCode>(context.m_errors.load(std::memory_order_acquire));
        NES_ASSERT(errors == EPhysicsUpdateErrorCode::None, "An Error occurred during the physics update! Error: {}", ToString(errors));
        return errors;
    }

    void PhysicsScene::JobStepListeners(PhysicsUpdateContext::Step* pStep)
    {
    #if NES_ASSERTS_ENABLED
        // Read positions (broadphase updates concurrently so we can't write), read/write velocities.
        BodyAccess::GrantScope grant(BodyAccess::EAccess::ReadWrite, BodyAccess::EAccess::Read);

        // Can activate bodies only (we cache the number of active bodies at the beginning of the step in m_numActiveBodiesAtStepStart so we cannot deactivate here).
        BodyManager::Internal_GrantActiveBodiesAccess grantActive(true, false);
    #endif

        PhysicsStepListenerContext context;
        context.m_deltaTime = pStep->m_pContext->m_stepDeltaTime;
        context.m_isFirstStep = pStep->m_isFirst;
        context.m_isLastStep = pStep->m_isLast;
        context.m_pPhysicsScene = this;

        const uint32 batchSize = m_physicsSettings.m_stepListenersBatchSize;
        for (;;)
        {
            // Get the start of a new batch
            const uint32 batch = pStep->m_stepListenerReadIndex.fetch_add(batchSize);
            if (batch >= m_stepListeners.size())
                break;

            // Call the listeners
            for (uint32 i = batch, end = math::Min(static_cast<uint32>(m_stepListeners.size()), batch + batchSize); i < end; ++i)
            {
                m_stepListeners[i]->OnStep(context);
            }
        }
    }

    void PhysicsScene::JobDetermineActiveConstraints(PhysicsUpdateContext::Step* pStep) const
    {
    #if NES_ASSERTS_ENABLED
        // No body access
        BodyAccess::GrantScope grant(BodyAccess::EAccess::None, BodyAccess::EAccess::None);
    #endif

        const uint32 numConstraints = m_constraintManager.GetNumConstraints();
        uint32 numActiveConstraints;
        Constraint** pActiveConstraints = static_cast<Constraint**>(NES_STACK_ALLOCATE(kDetermineActiveConstraintsBatchSize * sizeof(Constraint*)));

        for (;;)
        {
            // Atomically fetch a batch of constraints
            const uint32 constraintIndex = pStep->m_determineActiveConstraintsReadIndex.fetch_add(kDetermineActiveConstraintsBatchSize);
            if (constraintIndex >= numConstraints)
                break;

            // Calculate the end of the batch.
            const uint32 constraintIndexEnd = math::Min(numConstraints, constraintIndex + kDetermineActiveConstraintsBatchSize);

            // Store the active constraints at the start of the step (bodies get activated during the step which in turn may activate constraints
            // leading to an inconsistent snapshot).
            m_constraintManager.GetActiveConstraints(constraintIndex, constraintIndexEnd, pActiveConstraints, numActiveConstraints);

            // Copy the block of active constraints to the global list of active constraints
            if (numActiveConstraints > 0)
            {
                const uint32 activeConstraintIndex = pStep->m_numActiveConstraints.fetch_add(numActiveConstraints);
                memcpy(static_cast<void*>(pStep->m_pContext->m_pActiveConstraints + activeConstraintIndex), static_cast<void*>(pActiveConstraints), numActiveConstraints * sizeof(Constraint*));
            }
        }
    }

    void PhysicsScene::JobApplyGravity(const PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep)
    {
#if NES_ASSERTS_ENABLED
        // We update velocities and need to read the rotation to do so.
        BodyAccess::GrantScope grant(BodyAccess::EAccess::ReadWrite, BodyAccess::EAccess::Read);
#endif

        // Get the array of active bodies that we had at the start of the physics update.
        // Any body activated as a part of the simulation step does not receive gravity this frame.
        // Note that bodies may be activated during this job but not deactivated. This means that only
        // elements will be added to the array. Since the array is made to not reallocate, this is a safe operation.
        const BodyID* pActiveBodies = m_bodyManager.GetActiveBodiesUnsafe();
        const uint32 numActiveBodiesAtStepStart = pStep->m_numActiveBodiesAtStepStart;

        // Fetch delta time once outside the loop.
        const float deltaTime = pContext->m_stepDeltaTime;

        // Update velocities from forces.
        for (;;)
        {
            // Atomically fetch a batch of bodies.
            uint32 activeBodyIndex = pStep->m_applyGravityReadIndex.fetch_add(kApplyGravityBatchSize);
            if (activeBodyIndex >= numActiveBodiesAtStepStart)
                break;

            // Calculate the end of the batch
            const uint32 activeBodyIndexEnd = math::Min(numActiveBodiesAtStepStart, activeBodyIndex + kApplyGravityBatchSize);

            // Process the batch
            while (activeBodyIndex < activeBodyIndexEnd)
            {
                Body& body = m_bodyManager.GetBody(pActiveBodies[activeBodyIndex]);
                if (body.IsDynamic())
                {
                    MotionProperties* pMotionProps = body.GetMotionPropertiesUnchecked();
                    const Quat rotation = body.GetRotation();

                    if (body.GetApplyGyroscopicForce())
                        pMotionProps->Internal_ApplyGyroscopicForce(rotation, deltaTime);

                    pMotionProps->Internal_ApplyForceTorqueAndDrag(rotation, m_gravity, deltaTime);
                }

                ++activeBodyIndex;
            }
        }
    }

    void PhysicsScene::JobSetupVelocityConstraints(const float deltaTime, PhysicsUpdateContext::Step* pStep) const
    {
    #if NES_ASSERTS_ENABLED
        // We only read positions
        BodyAccess::GrantScope grant(BodyAccess::EAccess::None, BodyAccess::EAccess::Read);
    #endif

        const uint32 numConstraints = pStep->m_numActiveConstraints;

        for (;;)
        {
            // Atomically fetch a batch of constraints
            const uint32 constraintIndex = pStep->m_setupVelocityConstraintsReadIndex.fetch_add(kSetupVelocityConstraintsBatchSize);
            if (constraintIndex >= numConstraints)
                break;

            ConstraintManager::SetupVelocityConstraints(pStep->m_pContext->m_pActiveConstraints + constraintIndex, math::Min<uint32>(kSetupVelocityConstraintsBatchSize, numConstraints - constraintIndex), deltaTime);
        }
    }

    void PhysicsScene::JobBuildIslandsFromConstraints(PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep)
    {
    #if NES_ASSERTS_ENABLED
        // We read constraints and positions
        BodyAccess::GrantScope grant(BodyAccess::EAccess::None, BodyAccess::EAccess::Read);

        // Can only activate bodies
        BodyManager::Internal_GrantActiveBodiesAccess grantActive(true, false);
    #endif

        // Prepare the island builder.
        m_islandBuilder.PrepareNonContactConstraints(pStep->m_numActiveConstraints, pContext->m_pAllocator);

        // Build the islands
        ConstraintManager::BuildIslands(pStep->m_pContext->m_pActiveConstraints, pStep->m_numActiveConstraints, m_islandBuilder, m_bodyManager);
    }

    //----------------------------------------------------------------------------------------------------
    // Disable thread sanitization for this function. It detects a false positive race condition on m_pBodyPairs.
    // We have written m_pBodyPairs before doing m_writeIndex++ and we check m_writeIndex before reading
    // m_pBodyPairs, so this should be safe.
    //----------------------------------------------------------------------------------------------------
    NES_TSAN_NO_SANITIZE_ATTRIBUTE
    void PhysicsScene::JobFindCollisions(PhysicsUpdateContext::Step* pStep, const int jobIndex)
    {
    #if NES_ASSERTS_ENABLED
        // We read positions and read velocities (for elastic collisions).
        BodyAccess::GrantScope grant(BodyAccess::EAccess::Read, BodyAccess::EAccess::Read);

        // Can only activate bodies
        BodyManager::Internal_GrantActiveBodiesAccess grantActive(true, false);
    #endif

        // Get the allocation context for allocating new contact points.
        ContactAllocator contactAllocator(m_contactManager.GetContactAllocator());

        // Determine the initial queue to read pairs from if no broadphase work can be done
        // (always start looking at results from the next job).
        int readQueueIndex = (jobIndex + 1) % static_cast<int>(pStep->m_bodyPairQueues.size());

        // Allocate space to temporarily store a batch of active bodies.
        BodyID* pActiveBodies = static_cast<BodyID*>(NES_STACK_ALLOCATE(kActiveBodiesBatchSize * sizeof(BodyID)));

        for (;;)
        {
            // Check if there are active bodies to be processed.
            uint32 activeBodiesReadIndex = pStep->m_activeBodyReadIndex;
            const uint32 numActiveBodies = m_bodyManager.GetNumActiveBodies();
            if (activeBodiesReadIndex < numActiveBodies)
            {
                // Try to claim a batch of active bodies.
                const uint32 activeBodiesReadIndexEnd = math::Min(numActiveBodies, activeBodiesReadIndex + kActiveBodiesBatchSize);
                if (pStep->m_activeBodyReadIndex.compare_exchange_strong(activeBodiesReadIndex, activeBodiesReadIndexEnd))
                {
                    // Callback when a new body pair is found
                    class MyBodyPairCallback : public BodyPairCollector
                    {
                    public:
                        MyBodyPairCallback(PhysicsUpdateContext::Step* pStep, ContactAllocator& contactAllocator, const int jobIndex)
                            : m_pStep(pStep)
                            , m_contactAllocator(contactAllocator)
                            , m_jobIndex(jobIndex)
                        {
                            //
                        }

                        virtual void AddHit(const BodyPair& pair) override
                        {
                            // Check if we have space in our write queue.
                            PhysicsUpdateContext::BodyPairQueue& queue = m_pStep->m_bodyPairQueues[m_jobIndex];
                            const uint32 bodyPairsInQueue = queue.m_writeIndex - queue.m_readIndex;
                            if (bodyPairsInQueue >= m_pStep->m_maxBodyPairsPerQueue)
                            {
                                // The buffer is full, process the pair now:
                                m_pStep->m_pContext->m_pPhysicsScene->ProcessBodyPair(m_contactAllocator, pair);
                            }
                            else
                            {
                                // Store the pair in our own queue
                                m_pStep->m_pContext->m_pBodyPairs[m_jobIndex * m_pStep->m_maxBodyPairsPerQueue + queue.m_writeIndex % m_pStep->m_maxBodyPairsPerQueue] = pair;
                                ++queue.m_writeIndex;
                            }
                        }
                        

                    private:
                        PhysicsUpdateContext::Step* m_pStep;
                        ContactAllocator&           m_contactAllocator;
                        int                         m_jobIndex;
                    };
                    MyBodyPairCallback addPair(pStep, contactAllocator, jobIndex);

                    // Copy the active bodies to the temporary array; the broadphase will reorder them.
                    const uint32 batchSize = activeBodiesReadIndexEnd - activeBodiesReadIndex;
                    memcpy(pActiveBodies, m_bodyManager.GetActiveBodiesUnsafe() + activeBodiesReadIndex, batchSize * sizeof(BodyID));

                    // Find pairs in the broadphase
                    m_pBroadphase->FindCollidingPairs(pActiveBodies, static_cast<int>(batchSize), m_physicsSettings.m_speculativeContactDistance, *m_pCollisionVsBroadPhaseLayerFilter, *m_pCollisionLayerPairFilter, addPair);

                    // Check if we have enough pairs in the buffer to start a new job:
                    const PhysicsUpdateContext::BodyPairQueue& queue = pStep->m_bodyPairQueues[jobIndex];
                    const uint32 bodyPairsInQueue = queue.m_writeIndex - queue.m_readIndex;
                    if (bodyPairsInQueue >= kNarrowPhaseBatchSize)
                        TrySpawnJobFindCollisions(pStep);
                }
            }
            else
            {
                // There are no active bodies to process from the current read index.
                // Lockless loop to get the next body pair from the pairs buffer.
                const PhysicsUpdateContext* pContext = pStep->m_pContext;
                const int firstReadQueueIndex = readQueueIndex;
                for (;;)
                {
                    PhysicsUpdateContext::BodyPairQueue& queue = pStep->m_bodyPairQueues[readQueueIndex];

                    // Get the next pair to process
                    uint32 pairIndex = queue.m_readIndex;

                    // If the pair hasn't been written yet
                    if (pairIndex >= queue.m_writeIndex)
                    {
                        // Go to the next queue
                        readQueueIndex = (readQueueIndex + 1) % static_cast<int>(pStep->m_bodyPairQueues.size());

                        // If we're back at the first queue, we've looked at all of them and found nothing
                        if (readQueueIndex == firstReadQueueIndex)
                        {
                            // Collect information from the contact allocator and accumulate it in the step.
                            FinalizeContactAllocator(*pStep, contactAllocator);

                            // Mark this job as inactive
                            pStep->m_activeFindCollisionJobs.fetch_and(~static_cast<PhysicsUpdateContext::JobMask>(1 << jobIndex), std::memory_order_release);

                            // Trigger the next jobs
                            pStep->m_broadPhaseFinalize.RemoveDependency();
                            pStep->m_finalizeIslands.RemoveDependency();
                            return;
                        }

                        // Try again reading from the next queue
                        continue;
                    }

                    // Copy the body pair out of the buffer
                    const BodyPair bp = pContext->m_pBodyPairs[readQueueIndex * pStep->m_maxBodyPairsPerQueue + pairIndex % pStep->m_maxBodyPairsPerQueue];

                    // Mark this pair as taken
                    if (queue.m_readIndex.compare_exchange_strong(pairIndex, pairIndex + 1))
                    {
                        // Process the actual body pair
                        ProcessBodyPair(contactAllocator, bp);
                        break;
                    }
                }
            }
        }
    }

    void PhysicsScene::JobFinalizeIslands(PhysicsUpdateContext* pContext)
    {
    #if NES_ASSERTS_ENABLED
        // We only touch island data
        BodyAccess::GrantScope grant(BodyAccess::EAccess::None, BodyAccess::EAccess::None);
    #endif

        // Finish collecting the islands, at this point the active body list doesn't change, so it's safe to access.
        m_islandBuilder.Finalize(m_bodyManager.GetActiveBodiesUnsafe(), m_bodyManager.GetNumActiveBodies(), m_contactManager.GetNumConstraints(), pContext->m_pAllocator);

        // Prepare the large island splitter
        if (m_physicsSettings.m_useLargeIslandSplitter)
            m_largeIslandSplitter.Prepare(m_islandBuilder, m_bodyManager.GetNumActiveBodies(), pContext->m_pAllocator);
    }

    void PhysicsScene::JobBodySetIslandIndex()
    {
    #if NES_ASSERTS_ENABLED
        // We only touch island data
        BodyAccess::GrantScope grant(BodyAccess::EAccess::None, BodyAccess::EAccess::None);
    #endif

        // Loop through the result of the island builder and tag all bodies with an island index.
        for (uint islandIndex = 0, n = m_islandBuilder.GetNumIslands(); islandIndex < n; ++islandIndex)
        {
            BodyID* pBodyStart;
            BodyID* pBodyEnd;
            m_islandBuilder.GetBodiesInIsland(islandIndex, pBodyStart, pBodyEnd);
            for (const BodyID* pBody = pBodyStart; pBody < pBodyEnd; ++pBody)
            {
                m_bodyManager.GetBody(*pBody).GetMotionProperties()->Internal_SetIslandIndex(islandIndex);
            }
        }
    }

    NES_SUPPRESS_WARNINGS_BEGIN
    NES_CLANG_SUPPRESS_WARNING("-Wundefined-func-template") // ConstraintManager::WarmStartVelocityConstraints / ContactConstraintManager::WarmStartVelocityConstraints is instantiated in the cpp file
    void PhysicsScene::JobSolveVelocityConstraints([[maybe_unused]] PhysicsUpdateContext* pContext, [[maybe_unused]] PhysicsUpdateContext::Step* pStep)
    {
    #if NES_ASSERTS_ENABLED
        // We update velocities and need to read positions to do so.
        BodyAccess::GrantScope grant(BodyAccess::EAccess::ReadWrite, BodyAccess::EAccess::Read);
    #endif

        const float deltaTime = pContext->m_stepDeltaTime;
        Constraint** pActiveConstraints = pContext->m_pActiveConstraints;

        // Only correct the first step for the delta time difference in the previous update.
        const float warmStartImpulseRatio = pStep->m_isFirst? pContext->m_warmStartImpulseRatio : 1.0f;

        bool checkIslands = true;
        bool checkSplitIslands = m_physicsSettings.m_useLargeIslandSplitter;
        for (;;)
        {
            // First, try to get work from large islands.
            if (checkSplitIslands)
            {
                bool firstIteration;
                uint splitIslandIndex;
                uint32* pConstraintsBegin;
                uint32* pConstraintsEnd;
                uint32* pContactsBegin;
                uint32* pContactsEnd;
                switch (m_largeIslandSplitter.FetchNextBatch(splitIslandIndex, pConstraintsBegin, pConstraintsEnd, pContactsBegin, pContactsEnd, firstIteration))
                {
                    case LargeIslandSplitter::EStatus::BatchRetrieved:
                    {
                        if (firstIteration)
                        {
                            // Iteration 0 is used to warm start the batch (we added 1 to the number of iterations in LargeIslandSplitter::SplitIsland()).
                            DummyCalculateSolverSteps dummy;
                            ConstraintManager::WarmStartVelocityConstraints(pActiveConstraints, pConstraintsBegin, pConstraintsEnd, warmStartImpulseRatio, dummy);
                            m_contactManager.WarmStartVelocityConstraints(pContactsBegin, pContactsEnd, warmStartImpulseRatio, dummy);
                        }
                        else
                        {
                            // Solve velocity constraints
                            ConstraintManager::SolveVelocityConstraints(pActiveConstraints, pConstraintsBegin, pConstraintsEnd, deltaTime);
                            m_contactManager.SolveVelocityConstraints(pContactsBegin, pContactsEnd);
                        }

                        // Mark this batch as processed
                        bool lastIteration;
                        bool finalBatch;
                        m_largeIslandSplitter.MarkBatchProcessed(splitIslandIndex, pConstraintsBegin, pConstraintsEnd, pContactsBegin, pContactsEnd, lastIteration, finalBatch);

                        // Save back the lambdas in the contact cache for the warm start of the next physics update
                        if (lastIteration)
                            m_contactManager.StoreAppliedImpulses(pContactsBegin, pContactsEnd);

                        // We processed work, loop again.
                        continue;
                    }
                        
                    case LargeIslandSplitter::EStatus::WaitingForBatch:
                        break;

                    case LargeIslandSplitter::EStatus::AllBatchesDone:
                    {
                        checkSplitIslands = false;
                        break;
                    }
                }
            }

            // If that didn't succeed, try to process an island
            if (checkIslands)
            {
                // Next Island
                uint32 islandIndex = pStep->m_solveVelocityConstraintsNextIsland++;
                if (islandIndex >= m_islandBuilder.GetNumIslands())
                {
                    // We processed all islands, stop checking islands
                    checkIslands = false;
                    continue;
                }

                // Get iterators for this island
                uint32* pConstraintsBegin;
                uint32* pConstraintsEnd;
                uint32* pContactsBegin;
                uint32* pContactsEnd;
                const bool hasConstraints = m_islandBuilder.GetConstraintsInIsland(islandIndex, pConstraintsBegin, pConstraintsEnd);
                const bool hasContacts = m_islandBuilder.GetContactsInIsland(islandIndex, pContactsBegin, pContactsEnd);

                // If we don't have any contacts or constraints, we know that none of the following islands have any contacts or constraints.
                // This is because they are sorted by the most constraints first. This means we are done.
                if (!hasConstraints && !hasContacts)
                {
                #if NES_ASSERTS_ENABLED
                    // Validate our assumption that the next islands don't have any constraints or contacts
                    for (; islandIndex < m_islandBuilder.GetNumIslands(); ++islandIndex)
                    {
                        NES_ASSERT(!m_islandBuilder.GetConstraintsInIsland(islandIndex, pConstraintsBegin, pConstraintsEnd));
                        NES_ASSERT(!m_islandBuilder.GetContactsInIsland(islandIndex, pContactsBegin, pContactsEnd));
                    }
                #endif

                    checkIslands = false;
                    continue;
                }

                // Sorting is costly but needed for a deterministic simulation. Allow the user to turn this off.
                if (m_physicsSettings.m_simulationIsDeterministic)
                {
                    // Sort the constraints to give a deterministic simulation
                    ConstraintManager::SortConstraints(pActiveConstraints, pConstraintsBegin, pConstraintsEnd);

                    // Sort the contacts to give a deterministic simulation
                    m_contactManager.SortContacts(pContactsBegin, pContactsEnd);
                }

                // Split up the large islands
                CalculateSolverSteps stepsCalculator(m_physicsSettings);
                if (m_physicsSettings.m_useLargeIslandSplitter
                    && m_largeIslandSplitter.SplitIsland(islandIndex, m_islandBuilder, m_bodyManager, m_contactManager, pActiveConstraints, stepsCalculator))
                    continue; // If this is split, loop again to fetch the newly split island.

                // We didn't create a split, so run the solver now for this entire island. Begin by warm starting.
                ConstraintManager::WarmStartVelocityConstraints(pActiveConstraints, pConstraintsBegin, pConstraintsEnd, warmStartImpulseRatio, stepsCalculator);
                m_contactManager.WarmStartVelocityConstraints(pContactsBegin, pContactsEnd, warmStartImpulseRatio, stepsCalculator);
                stepsCalculator.Finalize();

                // Store the number of positions steps for later.
                m_islandBuilder.SetNumPositionSteps(islandIndex, stepsCalculator.GetNumPositionSteps());

                // Solve velocity constraints
                for (uint velocityStep = 0; velocityStep < stepsCalculator.GetNumVelocitySteps(); ++velocityStep)
                {
                    bool appliedImpulse = ConstraintManager::SolveVelocityConstraints(pActiveConstraints, pConstraintsBegin, pConstraintsEnd, deltaTime);
                    appliedImpulse |= m_contactManager.SolveVelocityConstraints(pContactsBegin, pContactsEnd);
                    if (!appliedImpulse)
                        break;
                }

                // Save back the lambdas in the contact cache for the warm start of the next physics update.
                m_contactManager.StoreAppliedImpulses(pContactsBegin, pContactsEnd);

                // We processed work, loop again.
                continue;
            }

            if (checkIslands)
            {
                // If there are islands, we don't need to wait and can pick up new work.
                continue;
            }
            else if (checkSplitIslands)
            {
                // If there are split islands, but we didn't do any work, give up a time slice.
                std::this_thread::yield();
            }
            else
            {
                // No more work.
                break;
            }
        }
    }
    NES_SUPPRESS_WARNINGS_END

    void PhysicsScene::JobPreIntegrateVelocity(PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep)
    {
        // Reserve enough space for all bodies that may need a cast.
        StackAllocator* pAllocator = pContext->m_pAllocator;
        NES_ASSERT(pStep->m_pCCDBodies == nullptr);
        pStep->m_CCDBodiesCapacity = m_bodyManager.GetNumActiveCCDBodies();
        pStep->m_pCCDBodies = static_cast<CCDBody*>(pAllocator->Allocate(pStep->m_CCDBodiesCapacity * sizeof(CCDBody)));

        // Initialize the mapping table between the active body and the CCD body.
        NES_ASSERT(pStep->m_pActiveBodyToCCDBody == nullptr);
        pStep->m_numActiveBodyToCCDBodies = m_bodyManager.GetNumActiveBodies();
        pStep->m_pActiveBodyToCCDBody = static_cast<int*>(pAllocator->Allocate(pStep->m_numActiveBodyToCCDBodies * sizeof(int)));

        // Prepare the split island build for solving the position constraints.
        m_largeIslandSplitter.PrepareForSolverPositions();
    }

    void PhysicsScene::JobIntegrateVelocity(const PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep)
    {
    #if NES_ASSERTS_ENABLED
        // We update positions and need velocity to do so, we also clam velocities so we need to write to them.
        BodyAccess::GrantScope grant(BodyAccess::EAccess::ReadWrite, BodyAccess::EAccess::ReadWrite);
    #endif

        const float deltaTime = pContext->m_stepDeltaTime;
        const BodyID* pActiveBodies = m_bodyManager.GetActiveBodiesUnsafe();
        uint32 numActiveBodies = m_bodyManager.GetNumActiveBodies();
        uint32 numActiveBodiesAfterFindCollisions = pStep->m_activeBodyReadIndex;

        // We can now move bodies that are not part of an island. In this case,we need to notify the broadphase of the movement.
        static constexpr int kBodiesBatchSize = 64;
        BodyID* pBodiesToUpdateBounds = static_cast<BodyID*>(NES_STACK_ALLOCATE(kBodiesBatchSize * sizeof(BodyID)));
        int numBodiesToUpdateBounds = 0;

        for (;;)
        {
            // Atomically fetch a batch of bodies.
            uint32 activeBodyIndex = pStep->m_integrateVelocityReadIndex.fetch_add(kIntegrateVelocityBatchSize);
            if (activeBodyIndex >= numActiveBodies)
                break;

            // Calculate the end of the batch
            const uint32 activeBodyIndexEnd = math::Min(numActiveBodies, activeBodyIndex + kIntegrateVelocityBatchSize);

            // Process the batch
            while (activeBodyIndex < activeBodyIndexEnd)
            {
                // Update the positions using a Symplectic Euler step (which integrates using the updated velocity v1' rather
                // than the original velocity v1):
                // x1' = x1 + h * v1'
                // At this point the active bodies array does not change, so it is safe to access the array.
                BodyID bodyID = pActiveBodies[activeBodyIndex];
                Body& body = m_bodyManager.GetBody(bodyID);
                MotionProperties* pMotionProps = body.GetMotionProperties();

                // Clamp velocities (not for kinematic bodies).
                if (body.IsDynamic())
                {
                    pMotionProps->ClampLinearVelocity();
                    pMotionProps->ClampAngularVelocity();
                }

                // Update the rotation of the body according to the angular velocity.
                // For motion type discrete we need to do this anyway, for motion type linear cast we have multiple options:
                // 1. Rotate the body first, then sweep.
                // 2. First sweep and then rotation the body at the end.
                // 3. Pick some in between rotation (e.g., half-way), then sweep and finally rotate the remainder.
                // (1) has some clear advantages as when a long, thin body hits a surface away from the center of mass, this will result in a large
                // angular velocity and a limited reduction in linear velocity.
                // When simulating the rotation first before doing the translation, the body will be able to rotate away from the contact point allowing
                // the center of mass to approach the surface. When using approach (2), in this case, we will immediately detect the same collision again
                // (the body has not rotated, and the body was already colliding at the end of the previous time step) resulting in a lot of stolen time.
                // Plus, the body will appear to be frozen in an unnatural pose (like it is glued at an angle to the surface). (1) obviously has some
                // negative side effects too as simulating the rotation first may cause it to tunnel through a small object that the linear cast might
                // have otherwise detected. In any case, a linear cast is not good for detecting tunneling due to angular rotation, so we don't care
                // about that too much (you'd need a full cast to take angular effects into account).
                body.Internal_AddRotationStep(body.GetAngularVelocity() * deltaTime);

                // Get the delta position
                Vec3 deltaPos = body.GetLinearVelocity() * deltaTime;

                // If the position should be updated (or if it is delayed because of CCD).
                bool updatePosition = true;

                switch (pMotionProps->GetMotionQuality())
                {
                    case EBodyMotionQuality::Discrete:
                    {
                        // No additional collision checking to be done.
                        break;
                    }

                    case EBodyMotionQuality::LinearCast:
                    {
                        if (body.IsDynamic()        // Kinematic bodies cannot be stopped.
                            && !body.IsSensor())    // We don't support CCD sensors.
                        {
                            // Determine the inner radius (the smallest sphere that fits into the shape)
                            const float innerRadius = body.GetShape()->GetInnerRadius();
                            NES_ASSERT(innerRadius > 0.f, "The shape has no inner radius, this makes the shape unsuitable for the linear cast motion quality as we cannot move it with risking tunneling.");

                            // Measure translation in this step and check if is above the threshold to perform a linear cast.
                            const float linearCastThresholdSqr = math::Squared(m_physicsSettings.m_linearCastThreshold * innerRadius);
                            if (deltaPos.LengthSqr() > linearCastThresholdSqr)
                            {
                                // This body needs a cast.
                                const uint32 ccdBodyIndex = pStep->m_numCCDBodies++;
                                NES_ASSERT(activeBodyIndex < pStep->m_numActiveBodyToCCDBodies);
                                pStep->m_pActiveBodyToCCDBody[activeBodyIndex] = static_cast<int>(ccdBodyIndex);
                                new (&pStep->m_pCCDBodies[ccdBodyIndex]) CCDBody(bodyID, deltaPos, linearCastThresholdSqr, math::Min(m_physicsSettings.m_penetrationSlop, m_physicsSettings.m_linearCastMaxPenetration * innerRadius));

                                updatePosition = false;
                            }
                        }

                        break;
                    }
                }

                if (updatePosition)
                {
                    // Move the body now
                    body.Internal_AddPositionStep(deltaPos);

                    // If the body was activated due to an earlier CCD step, it will have an index in the active
                    // body array that is higher than the highest one we processed during FindCollisions.
                    // This means that it hasn't been assigned to an island and will not be updated by an island.
                    // So, we need to update its bounds manually.
                    if (pMotionProps->Internal_GetIndexInActiveBodies() >= numActiveBodiesAfterFindCollisions)
                    {
                        body.Internal_CalculateWorldSpaceBounds();
                        pBodiesToUpdateBounds[numBodiesToUpdateBounds++] = body.GetID();
                        if (numBodiesToUpdateBounds == kBodiesBatchSize)
                        {
                            // Buffer full, flush now
                            m_pBroadphase->NotifyBodiesAABBChanged(pBodiesToUpdateBounds, numBodiesToUpdateBounds, false);
                            numBodiesToUpdateBounds = 0;
                        }
                    }

                    // We did not create a CCD body.
                    pStep->m_pActiveBodyToCCDBody[activeBodyIndex] = -1;
                }

                ++activeBodyIndex;
            }
        }

        // Notify change bounds on requested bodies
        if (numBodiesToUpdateBounds > 0)
            m_pBroadphase->NotifyBodiesAABBChanged(pBodiesToUpdateBounds, numBodiesToUpdateBounds, false);
    }

    void PhysicsScene::JobPostIntegrateVelocity(PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep) const
    {
        // Validate that our reservations were correct.
        NES_ASSERT(pStep->m_numCCDBodies <= m_bodyManager.GetNumActiveCCDBodies());

        if (pStep->m_numCCDBodies == 0)
        {
            // No continuous collision detection jobs -> kick the next job ourselves
            pStep->m_contactRemovedCallbacks.RemoveDependency();
        }
        else
        {
            // Run the continuous collision detection jobs
            const int numCCDJobs = math::Min(static_cast<int>(pStep->m_numCCDBodies + kNumCCDBodiesPerJob - 1) / kNumCCDBodiesPerJob, pContext->GetMaxConcurrency());
            pStep->m_resolveCCDContacts.AddDependency(numCCDJobs);
            pStep->m_contactRemovedCallbacks.AddDependency(numCCDJobs - 1); // Already had 1 dependency.
            for (int i = 0; i < numCCDJobs; ++i)
            {
                JobHandle job = pContext->m_pJobSystem->CreateJob("Find CCD Contacts", [pContext, pStep]
                {
                    pContext->m_pPhysicsScene->JobFindCCDContacts(pContext, pStep);
                    
                    pStep->m_resolveCCDContacts.RemoveDependency();
                    pStep->m_contactRemovedCallbacks.RemoveDependency();
                    
                });
                pContext->m_pBarrier->AddJob(job);
            }
        }
    }

    void PhysicsScene::JobFindCCDContacts(const PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep)
    {
    #if NES_ASSERTS_ENABLED
        // We only read positions, but the validation callback may read body positions and velocities.
        BodyAccess::GrantScope grant(BodyAccess::EAccess::Read, BodyAccess::EAccess::Read);
    #endif

        // Allocation context for allocating new contact points.
        ContactAllocator contactAllocator(m_contactManager.GetContactAllocator());

        // Settings
        ShapeCastSettings settings;
        settings.m_useShrunkenShapeAndConvexRadius = true;
        settings.m_backfaceModeTriangles = EBackFaceMode::IgnoreBackFaces;
        settings.m_backfaceModeConvex = EBackFaceMode::IgnoreBackFaces;
        settings.m_returnDeepestPoint = true;
        settings.m_collectFacesMode = ECollectFacesMode::CollectFaces;
        settings.m_activeEdgeMode = m_physicsSettings.m_checkActiveEdges? EActiveEdgeMode::CollideOnlyWithActive : EActiveEdgeMode::CollideWithAll;

        for (;;)
        {
            // Fetch the next body to cast
            uint32 index = pStep->m_nextCCDBody++;
            if (index >= pStep->m_numCCDBodies)
                break;

            CCDBody& ccdBody = pStep->m_pCCDBodies[index];
            const Body& body = m_bodyManager.GetBody(ccdBody.m_bodyID1);

            // Filter out layers
            DefaultBroadPhaseLayerFilter broadPhaseFilter = GetDefaultBroadPhaseFilter(body.GetCollisionLayer());
            DefaultCollisionLayerFilter collisionLayerFilter = GetDefaultCollisionLayerFilter(body.GetCollisionLayer());

            // Create a collector that will find the maximum distance allowed to travel while not penetrating more than 'max penetration'.
            class CCDNarrowPhaseCollector : public CastShapeCollector
            {
            public:
                CCDNarrowPhaseCollector(const BodyManager& bodyManager, ContactConstraintManager& contactConstraintManager, CCDBody& ccdBody, ShapeCastResult& result, const float deltaTime)
                    : m_bodyManager(bodyManager)
                    , m_contactConstraintManager(contactConstraintManager)
                    , m_ccdBody(ccdBody)
                    , m_result(result)
                    , m_deltaTime(deltaTime)
                {
                    //
                }

                virtual void AddHit(const ShapeCastResult& result) override
                {
                    // Check if this is a possible earlier hit than the one before
                    const float fraction = result.m_fraction;
                    if (fraction < m_ccdBody.m_hitFractionPlusSlop)
                    {
                        // Normalize the normal
                        Vec3 normal = result.m_penetrationAxis.Normalized();

                        // Calculate how much we can add to the fraction to penetrate the collision point by m_maxPenetration.
                        // Note that the normal is pointing to Body 2!
                        // Let the extra distance that we can travel along deltaPos be 'dist' : m_maxPenetration / dist = cos(angle between normal and deltaPos) = normal . deltaPos / |deltaPos|
                        // <=> dist = m_maxPenetration * |deltaPos| / normal . deltaPos
                        // Converting to a fraction: deltaFraction = dist / |deltaPos| = m_linearCastThreshold / normal . delatPos
                        const float denominator = normal.Dot(m_ccdBody.m_deltaPosition);
                        if (denominator > m_ccdBody.m_maxPenetration) // avoid dividing by zero, if extra hit fraction > 1 there's also no point in continuing.
                        {
                            const float fractionPlusSlop = fraction + m_ccdBody.m_maxPenetration / denominator;
                            if (fractionPlusSlop < m_ccdBody.m_hitFractionPlusSlop)
                            {
                                const Body& body2 = m_bodyManager.GetBody(result.m_bodyID2);

                                // Check if we've already accepted all hits from this body
                                if (m_validateBodyPair)
                                {
                                    // Validate the contact result
                                    const Body& body1 = m_bodyManager.GetBody(m_ccdBody.m_bodyID1);
                                    // Note that the center of mass of body 1 is the start of the sweep and is used as the base offset below.
                                    const EValidateContactResult validateResult = m_contactConstraintManager.ValidateContactPoint(body1, body2, body1.GetCenterOfMassPosition(), result);
                                    switch (validateResult)
                                    {
                                        case EValidateContactResult::AcceptContact:
                                        {
                                            // Continue
                                            break;
                                        }

                                        case EValidateContactResult::AcceptAllContactsForThisBodyPair:
                                        {
                                            // Accept this and all following contacts from this body
                                            m_validateBodyPair = true;
                                            break;
                                        }

                                        case EValidateContactResult::RejectContact:
                                            return;

                                        case EValidateContactResult::RejectAllContactsForThisBodyPair:
                                        {
                                            // Reject this an all following contacts from this body.
                                            m_rejectAll = true;
                                            ForceEarlyOut();
                                            return;
                                        }
                                    }
                                }

                                // This is the earliest hit so far, store it.
                                m_ccdBody.m_contactNormal = normal;
                                m_ccdBody.m_bodyID2 = result.m_bodyID2;
                                m_ccdBody.m_subShapeID2 = result.m_subShapeID2;
                                m_ccdBody.m_hitFraction = fraction;
                                m_ccdBody.m_hitFractionPlusSlop = fractionPlusSlop;
                                m_result = result;

                                // Result was assuming that body 2 is not moving, but it is, so we need to correct for it.
                                Vec3 movement2 = fraction * CalculateBodyMotion(body2, m_deltaTime);
                                if (!movement2.IsNearZero())
                                {
                                    m_result.m_contactPointOn1 += movement2;
                                    m_result.m_contactPointOn2 += movement2;
                                    for (Vec3& v : m_result.m_shape1Face)
                                    {
                                        v += movement2;
                                    }
                                    for (Vec3& v : m_result.m_shape2Face)
                                    {
                                        v += movement2;
                                    }
                                }

                                // Update the early out fraction
                                UpdateEarlyOutFraction(fractionPlusSlop);
                            }
                        }
                    }
                }

                bool                        m_validateBodyPair; /// If we still have to call the ValidateContactPoint for this body pair.
                bool                        m_rejectAll;        /// Reject all further contacts between this body pair.
                
            private:
                const BodyManager&          m_bodyManager;
                ContactConstraintManager&   m_contactConstraintManager;
                CCDBody&                    m_ccdBody;
                ShapeCastResult&            m_result;
                float                       m_deltaTime;
                BodyID                      m_acceptedBodyID;
            };

            // Narrow phase collector
            ShapeCastResult castShapeResult;
            CCDNarrowPhaseCollector npCollector(m_bodyManager, m_contactManager, ccdBody, castShapeResult, pContext->m_stepDeltaTime);

            // This collector wraps the narrow phase collector and collects the closest hit.
            class CCDBroadPhaseCollector : public CastShapeBodyCollector
            {
            public:
                CCDBroadPhaseCollector(const CCDBody& ccdBody, const Body& body1, const RShapeCast& shapeCast, ShapeCastSettings& shapeCastSettings, Internal_SimShapeFilterWrapper& shapeFilter, CCDNarrowPhaseCollector& collector, const BodyManager& bodyManager, PhysicsUpdateContext::Step* pStep, float deltaTime)
                    : m_ccdBody(ccdBody)
                    , m_body1(body1)
                    , m_shapeCast(shapeCast)
                    , m_shapeCastSettings(shapeCastSettings)
                    , m_shapeFilter(shapeFilter)
                    , m_collector(collector)
                    , m_bodyManager(bodyManager)
                    , m_pStep(pStep)
                    , m_deltaTime(deltaTime)
                {
                    //
                }

                virtual void AddHit(const BroadPhaseCastResult& result) override
                {
                    NES_ASSERT(result.m_fraction <= GetEarlyOutFraction(), "This hit should not have been passed on to the collector!");

                    // Test if we're colliding with ourselves
                    if (m_body1.GetID() == result.m_bodyID)
                        return;

                    // Avoid treating duplicates, if both bodies are doing CCD then only consider collision detection if bodyID < other bodyID.
                    const Body& body2 = m_bodyManager.GetBody(result.m_bodyID);
                    const CCDBody* pCCDBody2 = GetCCDBody(body2, m_pStep);
                    if (pCCDBody2 != nullptr && m_ccdBody.m_bodyID1 > pCCDBody2->m_bodyID1)
                        return;

                    // Test group filter
                    if (!m_body1.GetCollisionGroup().CanCollide(body2.GetCollisionGroup()))
                        return;

                    // TODO: For now, we ignore sensors.
                    if (body2.IsSensor())
                        return;

                    // Get relative movement of these two bodies
                    Vec3 direction = m_shapeCast.m_direction - CalculateBodyMotion(body2, m_deltaTime);

                    // Test if the remaining movement is less than our movement threshold
                    if (direction.LengthSqr() < m_ccdBody.m_linearCastThresholdSqr)
                        return;

                    // Get the bounds of 2, widen it by the extent of 1 and test a ray to see if it hits earlier than the current early out fraction.
                    AABox bounds = body2.GetWorldSpaceBounds();
                    bounds.m_min -= m_body1Extent;
                    bounds.m_max += m_body1Extent;
                    const float hitFraction = RayAABox(Vec3(m_shapeCast.m_centerOfMassStart.GetTranslation()), RayInvDirection(direction), bounds.m_min, bounds.m_max);
                    if (hitFraction > GetPositiveEarlyOutFraction()) // If the early out fraction was <= 0, we have the possibility of finding a deeper hit so we need to clamp the early out fraction
                        return;

                    // Reset the collector (this is a new body pair).
                    m_collector.ResetEarlyOutFraction(GetEarlyOutFraction());
                    m_collector.m_validateBodyPair = true;
                    m_collector.m_rejectAll = false;

                    // Set the body ID on the shape filter
                    m_shapeFilter.SetBody2(&body2);

                    // Provide the direction as a hint for the active-edges algorithm.
                    m_shapeCastSettings.m_activeEdgeMovementDirection = direction;

                    // Do the narrow phase collision check:
                    RShapeCast relativeCast(m_shapeCast.m_pShape, m_shapeCast.m_scale, m_shapeCast.m_centerOfMassStart, direction, m_shapeCast.m_shapeWorldBounds);
                    body2.GetTransformedShape().CastShape(relativeCast, m_shapeCastSettings, m_shapeCast.m_centerOfMassStart.GetTranslation(), m_collector, m_shapeFilter.GetFilter());

                    // Update the early out fraction
                    if (!m_collector.m_rejectAll)
                        UpdateEarlyOutFraction(m_collector.GetEarlyOutFraction());
                }
                
                const CCDBody&                  m_ccdBody;
                const Body&                     m_body1;
                Vec3                            m_body1Extent;
                RShapeCast                      m_shapeCast;
                ShapeCastSettings&              m_shapeCastSettings;
                Internal_SimShapeFilterWrapper& m_shapeFilter;
                CCDNarrowPhaseCollector&        m_collector;
                const BodyManager&              m_bodyManager;
                PhysicsUpdateContext::Step*     m_pStep;
                float                           m_deltaTime;
            };

            // Create the shape filter
            Internal_SimShapeFilterWrapper shapeFilter(m_pSimShapeFilter, &body);

            // Check if we collide with any other body. Note that we use the non-locking interface as we know the broadphase cannot be modified at this point.
            RShapeCast shapeCast(body.GetShape(), Vec3::One(), body.GetCenterOfMassTransform(), ccdBody.m_deltaPosition);
            CCDBroadPhaseCollector bpCollector(ccdBody, body, shapeCast, settings, shapeFilter, npCollector, m_bodyManager, pStep, pContext->m_stepDeltaTime);
            m_pBroadphase->CastAABoxNoLock({ shapeCast.m_shapeWorldBounds, shapeCast.m_direction }, bpCollector, broadPhaseFilter, collisionLayerFilter);

            // Check if there was a hit.
            if (ccdBody.m_hitFractionPlusSlop < 1.f)
            {
                const Body& body2 = m_bodyManager.GetBody(ccdBody.m_bodyID2);

                // Determine the contact manifold
                ContactManifold manifold;
                manifold.m_baseOffset = shapeCast.m_centerOfMassStart.GetTranslation();
                ManifoldBetweenTwoFaces(castShapeResult.m_contactPointOn1, castShapeResult.m_contactPointOn2, castShapeResult.m_penetrationAxis, m_physicsSettings.m_manifoldTolerance, castShapeResult.m_shape1Face, castShapeResult.m_shape2Face, manifold.m_relativeContactPointsOn1, manifold.m_relativeContactPointsOn2);
                manifold.m_subShapeID1 = castShapeResult.m_subShapeID1;
                manifold.m_subShapeID2 = castShapeResult.m_subShapeID2;
                manifold.m_penetrationDepth = castShapeResult.m_penetrationDepth;
                manifold.m_worldSpaceNormal = ccdBody.m_contactNormal;

                // Call contact point callbacks
                m_contactManager.OnCCDContactAdded(contactAllocator, body, body2, manifold, ccdBody.m_contactSettings);

                if (ccdBody.m_contactSettings.m_isSensor)
                {
                    // If this is a sensor, we don't want to solve the contact.
                    ccdBody.m_hitFractionPlusSlop = 1.f;
                    ccdBody.m_bodyID2 = BodyID();
                }
                else
                {
                    // Calculate the average position from the manifold (this will result in the same impulse applied as when we apply impulses to all contact points).
                    if (manifold.m_relativeContactPointsOn2.size() > 1)
                    {
                        Vec3 averageContactPoint = Vec3::Zero();
                        for (const Vec3& v : manifold.m_relativeContactPointsOn2)
                        {
                            averageContactPoint += v;
                        }
                        averageContactPoint /= static_cast<float>(manifold.m_relativeContactPointsOn2.size());
                        ccdBody.m_contactPointOn2 = manifold.m_baseOffset + averageContactPoint;
                    }
                    else
                    {
                        ccdBody.m_contactPointOn2 = manifold.m_baseOffset + castShapeResult.m_contactPointOn2;
                    }
                }
            }
        }

        // Collect information from the contact allocator and accumulate it in the step.
        FinalizeContactAllocator(*pStep, contactAllocator);
    }

    void PhysicsScene::JobResolveCCDContacts(PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep)
    {
    #if NES_ASSERTS_ENABLED
        // Read/write body access.
        BodyAccess::GrantScope grant(BodyAccess::EAccess::ReadWrite, BodyAccess::EAccess::ReadWrite);

        // We activate bodies that we collide with.
        BodyManager::Internal_GrantActiveBodiesAccess grantActive(true, false);
    #endif

        const uint32 numActiveBodiesAfterFindCollisions = pStep->m_activeBodyReadIndex;
        StackAllocator* pAllocator = pContext->m_pAllocator;

        // Check if there is anything to do.
        const uint numCCDBodies = pStep->m_numCCDBodies;
        if (numCCDBodies > 0)
        {
            // Sort on fraction so that we process the earliest collisions first.
            // This is needed to make the simulation deterministic and also to be able to stop contact processing
            // between body pairs if an earlier hit was found involving the body by another CCD body
            // (if it's a body ID < this CCD body's body ID - see filtering logic in CCDBroadPhaseCollector).
            CCDBody** pSortedCCDBodies = static_cast<CCDBody**>(pAllocator->Allocate(numCCDBodies * sizeof(CCDBody*)));
            NES_ON_SCOPE_EXIT([pAllocator, pSortedCCDBodies, numCCDBodies]() { pAllocator->Free(static_cast<void*>(pSortedCCDBodies), numCCDBodies * sizeof(CCDBody*)); });
            {
                // We don't to copy the entire struct (it's quite big), so we create a pointer array first.
                CCDBody* pSrcCCDBodies = pStep->m_pCCDBodies;
                CCDBody** pDstCCDBodies = pSortedCCDBodies;
                CCDBody** pDstCCDBodiesEnd = pDstCCDBodies + numCCDBodies;
                while (pDstCCDBodies < pDstCCDBodiesEnd)
                {
                    *(pDstCCDBodies++) = pSrcCCDBodies++;
                }

                // Which we then sort
                QuickSort(pSortedCCDBodies, pSortedCCDBodies + numCCDBodies, [](const CCDBody* pBody1, const CCDBody* pBody2)
                {
                   if (pBody1->m_hitFractionPlusSlop != pBody2->m_hitFractionPlusSlop)
                       return pBody1->m_hitFractionPlusSlop < pBody2->m_hitFractionPlusSlop;

                    return pBody1->m_bodyID1 < pBody2->m_bodyID1;
                });
            }

            // We can collide with bodies that are not active; we track them here so we can activate them in one go at the end.
            // This is also needed because we can't modify the active body array while we iterate it.
            static constexpr int kBodiesBatchSize = 64;
            BodyID* pBodiesToActivate = static_cast<BodyID*>(NES_STACK_ALLOCATE(kBodiesBatchSize * sizeof(BodyID)));
            int numBodiesToActivate = 0;

            // We can move bodies that are not part of an island. In this case, we need to notify the broadphase of the movement.
            BodyID* pBodiesToUpdateBounds = static_cast<BodyID*>(NES_STACK_ALLOCATE(kBodiesBatchSize * sizeof(BodyID)));
            int numBodiesToUpdateBounds = 0;

            for (uint i = 0; i < numCCDBodies; ++i)
            {
                const CCDBody* pCCDBody = pSortedCCDBodies[i];
                Body& body1 = m_bodyManager.GetBody(pCCDBody->m_bodyID1);
                MotionProperties* pMotionProps = body1.GetMotionProperties();

                // If there was a hit:
                if (pCCDBody->m_bodyID2.IsValid())
                {
                    Body& body2 = m_bodyManager.GetBody(pCCDBody->m_bodyID2);

                    // Determine if the other body has a CCD body
                    CCDBody* pCCDBody2 = GetCCDBody(body2, pStep);
                    if (pCCDBody2 != nullptr)
                    {
                        NES_ASSERT(pCCDBody2->m_bodyID2 != pCCDBody->m_bodyID1, "If we collided with another body, that other body should have ignored collisions with us!");

                        // Check if the other body found a hit that is further away:
                        if (pCCDBody2->m_hitFraction > pCCDBody->m_hitFraction)
                        {
                            // Reset the colliding body of the other CCD body. The other body will shorten its distance traveled and will not do any collision response (we'll do that).
                            // This means that at this point we have triggered a contact point add/persist for our further hit by accident for the other body.
                            // We accept this, as calling the contact point callbacks here would require persisting the manifolds up to this point and doing the callbacks single threaded.
                            pCCDBody2->m_bodyID2 = BodyID();
                            pCCDBody2->m_hitFractionPlusSlop = pCCDBody->m_hitFraction;
                        }
                    }

                    // If the other body moved less than us before hitting something, we're not colliding with it. So, we again have triggered a contact point add/persist callbacks
                    // by accident. We'll just move to the collision position anyway (as that's the last position we know is good), but we won't do any collision response.
                    if (pCCDBody2 == nullptr || pCCDBody2->m_hitFraction >= pCCDBody->m_hitFraction)
                    {
                        const ContactSettings& contactSettings = pCCDBody->m_contactSettings;

                        // Calculate the contact point velocity for body 1.
                        const Vec3 r1PlusU = Vec3(pCCDBody->m_contactPointOn2 - (body1.GetCenterOfMassPosition() + pCCDBody->m_hitFraction * pCCDBody->m_deltaPosition));
                        Vec3 v1 = body1.GetPointVelocityCOM(r1PlusU);

                        // Calculate the inverse mass for body 1.
                        const float invMass1 = contactSettings.m_inverseMassScale1 * pMotionProps->GetInverseMass();

                        if (body2.IsRigidBody())
                        {
                            // Calculate the contact point velocity for body 2.
                            const Vec3 r2 = Vec3(pCCDBody->m_contactPointOn2 - body2.GetCenterOfMassPosition());
                            Vec3 v2 = body2.GetPointVelocityCOM(r2);

                            // Calculate relative contact velocity
                            const Vec3 relativeVelocity = v2 - v1;
                            const float normalVelocity = relativeVelocity.Dot(pCCDBody->m_contactNormal);

                            // Calculate the velocity bias due to restitution.
                            float normalVelocityBias;
                            if (contactSettings.m_combinedRestitution > 0.f && normalVelocity < -m_physicsSettings.m_minVelocityForRestitution)
                                normalVelocityBias = contactSettings.m_combinedRestitution * normalVelocity;
                            else
                                normalVelocityBias = 0.f;

                            // Get the inverse mass of body 2
                            const float invMass2 = body2.GetMotionPropertiesUnchecked() != nullptr ? contactSettings.m_inverseMassScale2 * body2.GetMotionPropertiesUnchecked()->GetInverseMassUnchecked() : 0.f;

                            // Solve the contact constraint
                            AxisConstraintPart contactConstraint;
                            contactConstraint.CalculateConstraintPropertiesWithMassOverride(body1, invMass1, contactSettings.m_inverseInertiaScale1, r1PlusU, body2, invMass2, contactSettings.m_inverseInertiaScale2, r2, pCCDBody->m_contactNormal, normalVelocityBias);
                            contactConstraint.SolveVelocityConstraintWithMassOverride(body1, invMass1, body2, invMass2, pCCDBody->m_contactNormal, -FLT_MAX, FLT_MAX);

                            // Apply friction
                            if (contactSettings.m_combinedFriction > 0.f)
                            {
                                // Calculate the friction direction by removing the normal velocity form the relative velocity.
                                Vec3 frictionDirection = relativeVelocity - normalVelocity * pCCDBody->m_contactNormal;
                                const float frictionDirectionLengthSqr = frictionDirection.LengthSqr();
                                if (frictionDirectionLengthSqr > 1.0e-12f)
                                {
                                    // Normalize the friction direction.
                                    frictionDirection /= std::sqrt(frictionDirectionLengthSqr);

                                    // Calculate the max friction impulse
                                    float maxLambdaF = contactSettings.m_combinedFriction * contactConstraint.GetTotalLambda();

                                    AxisConstraintPart friction;
                                    friction.CalculateConstraintPropertiesWithMassOverride(body1, invMass1, contactSettings.m_inverseInertiaScale1, r1PlusU, body2, invMass2, contactSettings.m_inverseInertiaScale2, r2, frictionDirection);
                                    friction.SolveVelocityConstraintWithMassOverride(body1, invMass1, body2, invMass2, frictionDirection, -maxLambdaF, maxLambdaF);
                                }
                            }

                            // Clamp velocity of Body 2
                            if (body2.IsDynamic())
                            {
                                MotionProperties* pMotionProps2 = body2.GetMotionProperties();
                                pMotionProps2->ClampLinearVelocity();
                                pMotionProps2->ClampAngularVelocity();
                            }
                        }
                        else
                        {
                            // [TODO]: Soft body logic goes here.
                            NES_ASSERT(false, "Soft bodies not implemented yet!");   
                        }

                        // Clamp the velocity of body 1.
                        pMotionProps->ClampLinearVelocity();
                        pMotionProps->ClampAngularVelocity();

                        // Activate the 2nd body if it is not already active
                        if (body2.IsDynamic() && !body2.IsActive())
                        {
                            pBodiesToActivate[numBodiesToActivate++] = pCCDBody->m_bodyID2;
                            if (numBodiesToActivate == kBodiesBatchSize)
                            {
                                // Batch is full, activate now:
                                m_bodyManager.ActivateBodies(pBodiesToActivate, numBodiesToActivate);
                                numBodiesToActivate = 0;
                            }
                        }
                    }
                }

                // Update body position:
                body1.Internal_AddPositionStep(pCCDBody->m_deltaPosition * pCCDBody->m_hitFractionPlusSlop);

                // If the body was activated due to an earlier CCD step, it will have an index in the active
                // body array that is higher than the highest one we processed during FindCollisions.
                // This means that it hasn't been assigned to an island and will not be updated by an island.
                // So, we need to update its bounds manually.
                if (pMotionProps->Internal_GetIndexInActiveBodies() >= numActiveBodiesAfterFindCollisions)
                {
                    body1.Internal_CalculateWorldSpaceBounds();
                    pBodiesToUpdateBounds[numBodiesToUpdateBounds++] = body1.GetID();
                    if (numBodiesToUpdateBounds == kBodiesBatchSize)
                    {
                        // Buffer is full, flush now.
                        m_pBroadphase->NotifyBodiesAABBChanged(pBodiesToUpdateBounds, numBodiesToUpdateBounds, false);
                        numBodiesToUpdateBounds = 0;
                    }
                }
            }

            // Activate the requested bodies
            if (numBodiesToActivate > 0)
                m_bodyManager.ActivateBodies(pBodiesToActivate, numBodiesToActivate);
            
            // Notify the changed bounds on requested bodies
            if (numBodiesToUpdateBounds > 0)
                m_pBroadphase->NotifyBodiesAABBChanged(pBodiesToUpdateBounds, numBodiesToUpdateBounds, false);
        }

        // Ensure we free the CCD bodies array now, will not call the destructor!
        pAllocator->Free(pStep->m_pActiveBodyToCCDBody, pStep->m_numActiveBodyToCCDBodies * sizeof(int));
        pStep->m_pActiveBodyToCCDBody = nullptr;
        pStep->m_numActiveBodyToCCDBodies = 0;
        
        pAllocator->Free(pStep->m_pCCDBodies, pStep->m_CCDBodiesCapacity * sizeof(CCDBody));
        pStep->m_pCCDBodies = nullptr;
        pStep->m_CCDBodiesCapacity = 0;
    }

    void PhysicsScene::JobContactRemovedCallbacks(const PhysicsUpdateContext::Step* pStep)
    {
    #if NES_ASSERTS_ENABLED
        // We don't touch any bodies
        BodyAccess::GrantScope grant(BodyAccess::EAccess::None, BodyAccess::EAccess::None);
    #endif

        // Reset the Body::EFlags::InvalidateContactCache flag for all bodies.
        m_bodyManager.ValidateContactCacheForAllBodies();

        // Finalize the contact cache (this swaps the read and write versions of the contact cache).
        // Trigger all contact removed callbacks by looking at the last step contact points that have not been flagged as reused.
        m_contactManager.FinalizeContactCacheAndCallContactPointRemovedCallback(pStep->m_numBodyPairs, pStep->m_numManifolds);
    }

    class PhysicsScene::BodiesToSleep
    {
    public:
        static constexpr int    kBodiesToSleepSize = 512;
        static constexpr int    kMaxBodiesToPutInBuffer = 128;

        inline                  BodiesToSleep(BodyManager& bodyManager, BodyID* pBodiesToSleepBuffer) : m_bodyManager(bodyManager), m_pBodiesToSleepBuffer(pBodiesToSleepBuffer), m_pBodiesToSleepCurrent(pBodiesToSleepBuffer) {}
        
        inline                  ~BodiesToSleep()
        {
            // Flush the bodies to sleep buffer
            const int numBodiesInBuffer = static_cast<int>(m_pBodiesToSleepCurrent - m_pBodiesToSleepBuffer);
            if (numBodiesInBuffer > 0)
                m_bodyManager.DeactivateBodies(m_pBodiesToSleepBuffer, numBodiesInBuffer);
        }

        inline void             PutToSleep(const BodyID* pBegin, const BodyID* pEnd)
        {
            int numBodiesToSleep = static_cast<int>(pEnd - pBegin);
            if (numBodiesToSleep > kMaxBodiesToPutInBuffer)
            {
                // Too many bodies, deactivate immediately
                m_bodyManager.DeactivateBodies(pBegin, numBodiesToSleep);
            }
            else
            {
                // Check if there's enough space in the bodies to sleep buffer
                int numBodiesInBuffer = static_cast<int>(m_pBodiesToSleepCurrent - m_pBodiesToSleepBuffer);
                if (numBodiesInBuffer + numBodiesToSleep > kBodiesToSleepSize)
                {
                    // Flush the bodies to sleep buffer
                    m_bodyManager.DeactivateBodies(m_pBodiesToSleepBuffer, numBodiesInBuffer);
                    m_pBodiesToSleepCurrent = m_pBodiesToSleepBuffer;
                }

                // Copy the bodies to the buffer
                memcpy(m_pBodiesToSleepCurrent, pBegin, numBodiesToSleep * sizeof(BodyID));
                m_pBodiesToSleepCurrent += numBodiesToSleep;
            }
        }

    private:
        BodyManager&            m_bodyManager;
        BodyID*                 m_pBodiesToSleepBuffer;
        BodyID*                 m_pBodiesToSleepCurrent;
    };

    void PhysicsScene::JobSolvePositionConstraints(PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep)
    {
    #if NES_ASSERTS_ENABLED
        // We are fixing up position errors.
        BodyAccess::GrantScope grant(BodyAccess::EAccess::None, BodyAccess::EAccess::ReadWrite);

        // Can only deactivate bodies.
        BodyManager::Internal_GrantActiveBodiesAccess grantActive(false, true);
    #endif

        const float deltaTime = pContext->m_stepDeltaTime;
        const float baumgarte = m_physicsSettings.m_baumgarte;
        Constraint** pActiveConstraints = pContext->m_pActiveConstraints;

        // Keep a buffer of bodies that need to go to sleep to not constantly lock the active bodies mutex and create contention between all solving threads.
        BodiesToSleep bodiesToSleep(m_bodyManager, static_cast<BodyID*>(NES_STACK_ALLOCATE(BodiesToSleep::kBodiesToSleepSize * sizeof(BodyID))));

        bool checkIslands = true;
        bool checkSplitIslands = m_physicsSettings.m_useLargeIslandSplitter;
        for (;;)
        {
            // First, try to get work from large islands
            if (checkSplitIslands)
            {
                bool firstIteration;
                uint splitIslandIndex;
                uint32* pConstraintsBegin;
                uint32* pConstraintsEnd;
                uint32* pContactsBegin;
                uint32* pContactsEnd;
                switch (m_largeIslandSplitter.FetchNextBatch(splitIslandIndex, pConstraintsBegin, pConstraintsEnd, pContactsBegin, pContactsEnd, firstIteration))
                {
                    case LargeIslandSplitter::EStatus::BatchRetrieved:
                    {
                        // Solve the batch
                        ConstraintManager::SolvePositionConstraints(pActiveConstraints, pConstraintsBegin, pConstraintsEnd, deltaTime, baumgarte);
                        m_contactManager.SolvePositionConstraints(pContactsBegin, pContactsEnd);

                        // Mark the batch as processed.
                        bool lastIteration;
                        bool finalBatch;
                        m_largeIslandSplitter.MarkBatchProcessed(splitIslandIndex, pConstraintsBegin, pConstraintsEnd, pContactsBegin, pContactsEnd, lastIteration, finalBatch);

                        // The final batch will update all bounds and check sleeping
                        if (finalBatch)
                            CheckSleepAndUpdateBounds(m_largeIslandSplitter.GetIslandIndex(splitIslandIndex), pContext, pStep, bodiesToSleep);
                        
                        // We processed work, loop again.
                        continue;
                    }

                    case LargeIslandSplitter::EStatus::WaitingForBatch:
                        break;

                    case LargeIslandSplitter::EStatus::AllBatchesDone:
                    {
                        checkSplitIslands = false;
                        break;
                    }
                }
            }

            // If that didn't succeed, try to process an island
            if (checkIslands)
            {
                // Next island
                const uint32 islandIndex = pStep->m_solvePositionConstraintsNextIsland++;
                if (islandIndex >= m_islandBuilder.GetNumIslands())
                {
                    // We processed all islands, stop checking.
                    checkIslands = false;
                    continue;
                }

                // Get iterators for this island
                uint32* pConstraintsBegin;
                uint32* pConstraintsEnd;
                uint32* pContactsBegin;
                uint32* pContactsEnd;
                m_islandBuilder.GetConstraintsInIsland(islandIndex, pConstraintsBegin, pConstraintsEnd);
                m_islandBuilder.GetContactsInIsland(islandIndex, pContactsBegin, pContactsEnd);

                // If this island is a large island, it will be picked up as a batch, and we don't need to to anything here.
                const uint numItems = static_cast<uint>(pConstraintsEnd - pConstraintsBegin) + static_cast<uint>(pContactsEnd - pContactsBegin);
                if (m_physicsSettings.m_useLargeIslandSplitter && numItems >= LargeIslandSplitter::kLargeIslandThreshold)
                    continue;

                // Check if this island needs solving.
                if (numItems > 0)
                {
                    // Iterate
                    const uint numPositionSteps = m_islandBuilder.GetNumPositionSteps(islandIndex);
                    for (uint positionStep = 0; positionStep < numPositionSteps; ++positionStep)
                    {
                        bool appliedImpulse = ConstraintManager::SolvePositionConstraints(pActiveConstraints, pConstraintsBegin, pConstraintsEnd, deltaTime, baumgarte);
                        appliedImpulse |= m_contactManager.SolvePositionConstraints(pContactsBegin, pContactsEnd);

                        if (!appliedImpulse)
                            break;
                    }
                }

                // After solving, we will update all bounds and check sleeping
                CheckSleepAndUpdateBounds(islandIndex, pContext, pStep, bodiesToSleep);

                // We processed work, loop again.
                continue;
            }

            if (checkIslands)
            {
                // If there are islands, we don't need to wait and can pick up new work.
                continue; 
            }
            else if (checkSplitIslands)
            {
                // If there are split islands, but we didn't do any work, give up a time slice.
                std::this_thread::yield();
            }
            else
            {
                // No more work
                break;
            }
        }
    }

    void PhysicsScene::TrySpawnJobFindCollisions(PhysicsUpdateContext::Step* pStep) const
    {
        // Get how many jobs we can spawn and check if we can spawn more.
        const uint maxJobs = static_cast<uint>(pStep->m_bodyPairQueues.size());
        if (math::CountBits(pStep->m_activeFindCollisionJobs.load(std::memory_order_relaxed)) >= maxJobs)
            return;

        // Count how many body pairs we have waiting.
        uint32 numBodyPairs = 0;
        for (const PhysicsUpdateContext::BodyPairQueue& queue : pStep->m_bodyPairQueues)
        {
            numBodyPairs += queue.m_writeIndex - queue.m_readIndex;
        }

        // Count how many active bodies we have waiting.
        const uint32 numActiveBodies = m_bodyManager.GetNumActiveBodies() - pStep->m_activeBodyReadIndex;

        // Calculate how many jobs that we would like: 
        const uint32 desiredNumJobs = math::Min((numBodyPairs + kNarrowPhaseBatchSize - 1) / kNarrowPhaseBatchSize + (numActiveBodies + kActiveBodiesBatchSize - 1) / kActiveBodiesBatchSize, maxJobs);

        for (;;)
        {
            // Get the bit mask of active jobs and see if we can spawn more.
            PhysicsUpdateContext::JobMask currentActiveJobs = pStep->m_activeFindCollisionJobs.load(std::memory_order_relaxed);
            const uint jobIndex = math::CountTrailingZeros(~currentActiveJobs);
            if (jobIndex >= desiredNumJobs)
                break;

            // Try to claim the job index.
            const PhysicsUpdateContext::JobMask jobMask = static_cast<PhysicsUpdateContext::JobMask>(1) << jobIndex;
            const PhysicsUpdateContext::JobMask prevValue = pStep->m_activeFindCollisionJobs.fetch_or(jobMask, std::memory_order_acquire);

            if ((prevValue & jobMask) == 0)
            {
                // Add dependencies from the find collisions job to the next jobs
                pStep->m_broadPhaseFinalize.AddDependency();
                pStep->m_finalizeIslands.AddDependency();

                // Start the job
                JobHandle job = pStep->m_pContext->m_pJobSystem->CreateJob("Find Collisions", [step = pStep, jobIndex]()
                {
                    step->m_pContext->m_pPhysicsScene->JobFindCollisions(step, static_cast<int>(jobIndex));   
                });

                // Add the job to the job barrier so the main updating thread can execute the job too.
                pStep->m_pContext->m_pBarrier->AddJob(job);

                // Spawn only 1 extra job at a time.
                return;
            }
        }
    }

    void PhysicsScene::ProcessBodyPair(ContactAllocator& contactAllocator, const BodyPair& bodyPair)
    {
        // [TODO]: Profile function

        // Fetch the body pair
        Body* pBody1 = &m_bodyManager.GetBody(bodyPair.m_bodyA);
        Body* pBody2 = &m_bodyManager.GetBody(bodyPair.m_bodyB);
        NES_ASSERT(pBody1->IsActive());

        // [TODO]: Check for soft bodies

        // Ensure that pBody1 has the higher motion type (i.e., dynamic trumps kinematic), this ensures that we do the collision detection in the space of a moving
        // body, which avoids accuracy problems when testing a very large static object against a small dynamic object.
        // Ensure that pBody1 id < pBody2 id when motion types are the same.
        if (pBody1->GetMotionType() < pBody2->GetMotionType()
            || (pBody1->GetMotionType() == pBody2->GetMotionType() && bodyPair.m_bodyB < bodyPair.m_bodyA))
        {
            std::swap(pBody1, pBody2);
        }

        // Check if the contact points from the previous frame are reusable and if so, copy them.
        bool pairHandled = false;
        bool constraintCreated = false;
        if (m_physicsSettings.m_useBodyPairContactCache && !(pBody1->IsCollisionCacheInvalid() || pBody2->IsCollisionCacheInvalid()))
        {
            m_contactManager.GetContactsFromCache(contactAllocator, *pBody1, *pBody2, pairHandled, constraintCreated);
        }

        // If the cache hasn't handled this body pair, do the actual collision detection
        if (!pairHandled)
        {
            // Create an entry in the cache for this body pair.
            // Needs to happen regardless if we found a collision or not (we want to remember that no collision was found, too).
            ContactConstraintManager::BodyPairHandle bodyPairHandle = m_contactManager.AddBodyPair(contactAllocator, *pBody1, *pBody2);
            if (bodyPairHandle == nullptr)
                return; // Out of space.

            // Create the query settings
            CollideShapeSettings settings;
            settings.m_collectFacesMode = ECollectFacesMode::CollectFaces;
            settings.m_activeEdgeMode = m_physicsSettings.m_checkActiveEdges? EActiveEdgeMode::CollideOnlyWithActive : EActiveEdgeMode::CollideWithAll;
            settings.m_maxSeparationDistance = pBody1->IsSensor() || pBody2->IsSensor() ? 0.f : m_physicsSettings.m_speculativeContactDistance;
            settings.m_activeEdgeMovementDirection = pBody1->GetLinearVelocity() - pBody2->GetLinearVelocity();

            // Create the shape filter
            Internal_SimShapeFilterWrapper shapeFilter(m_pSimShapeFilter, pBody1);
            shapeFilter.SetBody2(pBody2);

            // Get transforms relative to Body 1
            const RVec3 offset = pBody1->GetCenterOfMassPosition();
            const Mat44 transform1 = Mat44::MakeRotation(pBody1->GetRotation());
            const Mat44 transform2 = pBody2->GetCenterOfMassTransform().PostTranslated(-offset); // .ToMat44() - for when we have DMat44 as an option.

            if (m_physicsSettings.m_useManifoldReduction                // Check the global flag.
                && pBody1->GetUseManifoldReductionWithBody(*pBody2))    // Check the body flag.
            {
                // Version *with* contact manifold reduction

                class MyManifold : public ContactManifold
                {
                public:
                    Vec3 m_firstWorldSpaceNormal;
                };

                // A temporary structure that allows us to keep track of all manifolds between this body pair.
                using Manifolds = StaticArray<MyManifold, 32>;

                // Collector Class
                struct ReductionCollideShapeCollector : public CollideShapeCollector
                {
                    PhysicsScene*   m_pPhysicsScene;
                    const Body*     m_pBody1;
                    const Body*     m_pBody2;
                    bool            m_validateBodyPair = true;
                    Manifolds       m_manifolds;

                    ReductionCollideShapeCollector(PhysicsScene* pPhysicsScene, const Body* pBody1, const Body* pBody2)
                        : m_pPhysicsScene(pPhysicsScene)
                        , m_pBody1(pBody1)
                        , m_pBody2(pBody2)
                    {
                        //
                    }

                    virtual void AddHit(const CollideShapeResult& result) override
                    {
                        // The first body should be the one with the highest motion type.
                        NES_ASSERT(m_pBody1->GetMotionType() >= m_pBody2->GetMotionType());
                        NES_ASSERT(!ShouldEarlyOut());

                        // Test if we want ot accept this hit
                        if (m_validateBodyPair)
                        {
                            switch (m_pPhysicsScene->m_contactManager.ValidateContactPoint(*m_pBody1, *m_pBody2, m_pBody1->GetCenterOfMassPosition(), result))
                            {
                                case EValidateContactResult::AcceptContact:
                                {
                                    // We're just accepting this one, nothing to do.
                                    break;
                                }

                                case EValidateContactResult::AcceptAllContactsForThisBodyPair:
                                {
                                    // Accept and stop calling the validation callback.
                                    m_validateBodyPair = false;
                                    break;
                                }

                                case EValidateContactResult::RejectContact:
                                {
                                    // Skip this contact
                                    return;
                                }

                                case EValidateContactResult::RejectAllContactsForThisBodyPair:
                                {
                                    // Skip this and early out.
                                    ForceEarlyOut();
                                    return;
                                }  
                            }
                        }

                        // Calculate the normal
                        const Vec3 worldSpaceNormal = result.m_penetrationAxis.Normalized();

                        // Check if we can add it to an existing manifold.
                        Manifolds::iterator manifold;
                        const float contactNormalCosMaxDeltaRot = m_pPhysicsScene->m_physicsSettings.m_contactNormalCosMaxDeltaRotation;
                        for (manifold = m_manifolds.begin(); manifold != m_manifolds.end(); ++manifold)
                        {
                            if (worldSpaceNormal.Dot(manifold->m_firstWorldSpaceNormal) >= contactNormalCosMaxDeltaRot)
                            {
                                // Update the average normal
                                manifold->m_worldSpaceNormal += worldSpaceNormal;
                                manifold->m_penetrationDepth = math::Max(manifold->m_penetrationDepth, result.m_penetrationDepth);
                                break;
                            }
                        }
                        if (manifold == m_manifolds.end())
                        {
                            // Check if the array is full
                            if (m_manifolds.size() == m_manifolds.capacity())
                            {
                                // Full, find the manifold with the least amount of penetration.
                                manifold = m_manifolds.begin();
                                for (Manifolds::iterator m = m_manifolds.begin() + 1; m < m_manifolds.end(); ++m)
                                {
                                    if (m->m_penetrationDepth < manifold->m_penetrationDepth)
                                        manifold = m;
                                }

                                // If this contact penetration is smaller than the smallest manifold, we skip this contact.
                                if (result.m_penetrationDepth < manifold->m_penetrationDepth)
                                    return;

                                // Replace the manifold
                                *manifold = { { m_pBody1->GetCenterOfMassPosition(), worldSpaceNormal, result.m_penetrationDepth, result.m_subShapeID1, result.m_subShapeID2, {}, {}}, worldSpaceNormal };
                            }
                            else
                            {
                                // Not full, create a new manifold
                                m_manifolds.push_back({ { m_pBody1->GetCenterOfMassPosition(), worldSpaceNormal, result.m_penetrationDepth, result.m_subShapeID1, result.m_subShapeID2, {}, {}}, worldSpaceNormal });
                                manifold = m_manifolds.end() - 1;
                            }
                        }

                        // Determine the contact points
                        const PhysicsSettings& settings = m_pPhysicsScene->m_physicsSettings;
                        ManifoldBetweenTwoFaces(result.m_contactPointOn1, result.m_contactPointOn2, result.m_penetrationAxis, settings.m_speculativeContactDistance + settings.m_manifoldTolerance, result.m_shape1Face, result.m_shape2Face, manifold->m_relativeContactPointsOn1, manifold->m_relativeContactPointsOn2);

                        // Prune if we have more than 32 points (this means we could run out of space in the next iteration).
                        if (manifold->m_relativeContactPointsOn1.size() > 32)
                            PruneContactPoints(manifold->m_firstWorldSpaceNormal, manifold->m_relativeContactPointsOn1, manifold->m_relativeContactPointsOn2);
                    }
                };

                ReductionCollideShapeCollector collector(this, pBody1, pBody2);

                // Perform collision detection between the two shapes
                m_simCollideBodyVsBody(*pBody1, *pBody2, transform1, transform2, settings, collector, shapeFilter.GetFilter());

                // Add the contacts
                for (ContactManifold& manifold : collector.m_manifolds)
                {
                    // Normalize the normal (it is a sum of all normals from the merge manifolds
                    manifold.m_worldSpaceNormal.Normalize();

                    // If we still have too many points, prune them now.
                    if (manifold.m_relativeContactPointsOn1.size() > 4)
                        PruneContactPoints(manifold.m_worldSpaceNormal, manifold.m_relativeContactPointsOn1, manifold.m_relativeContactPointsOn2);

                    // Add the contact points to the manager.
                    constraintCreated |= m_contactManager.AddContactConstraint(contactAllocator, bodyPairHandle, *pBody1, *pBody2, manifold);
                }
            }
            else
            {
                // Version WITHOUT contact manifold reduction.

                // Collector class
                struct NonReductionCollideShapeCollector : public CollideShapeCollector
                {
                    PhysicsScene*       m_pPhysicsScene;
                    ContactAllocator&   m_contactAllocator;
                    Body*               m_pBody1;
                    Body*               m_pBody2;
                    ContactConstraintManager::BodyPairHandle m_bodyPairHandle;
                    bool                m_validateBodyPair = true;
                    bool                m_constraintCreated = false;

                    NonReductionCollideShapeCollector(PhysicsScene* pPhysicsScene, ContactAllocator& allocator, Body* pBody1, Body* pBody2, const ContactConstraintManager::BodyPairHandle& bodyPairHandle)
                        : m_pPhysicsScene(pPhysicsScene)
                        , m_contactAllocator(allocator)
                        , m_pBody1(pBody1)
                        , m_pBody2(pBody2)
                        , m_bodyPairHandle(bodyPairHandle)
                    {
                        //
                    }

                    virtual void AddHit(const CollideShapeResult& result) override
                    {
                        // The first body should be the one with the highest motion type.
                        NES_ASSERT(m_pBody1->GetMotionType() >= m_pBody2->GetMotionType());
                        NES_ASSERT(!ShouldEarlyOut());

                        // Test if we want ot accept this hit
                        if (m_validateBodyPair)
                        {
                            switch (m_pPhysicsScene->m_contactManager.ValidateContactPoint(*m_pBody1, *m_pBody2, m_pBody1->GetCenterOfMassPosition(), result))
                            {
                                case EValidateContactResult::AcceptContact:
                                {
                                    // We're just accepting this one, nothing to do.
                                    break;
                                }

                                case EValidateContactResult::AcceptAllContactsForThisBodyPair:
                                {
                                    // Accept and stop calling the validation callback.
                                    m_validateBodyPair = false;
                                    break;
                                }

                                case EValidateContactResult::RejectContact:
                                {
                                    // Skip this contact
                                    return;
                                }

                                case EValidateContactResult::RejectAllContactsForThisBodyPair:
                                {
                                    // Skip this and early out.
                                    ForceEarlyOut();
                                    return;
                                }
                            }
                        }

                        // Determine the contact points
                        ContactManifold manifold;
                        manifold.m_baseOffset = m_pBody1->GetCenterOfMassPosition();
                        const PhysicsSettings& settings = m_pPhysicsScene->m_physicsSettings;
                        ManifoldBetweenTwoFaces(result.m_contactPointOn1, result.m_contactPointOn2, result.m_penetrationAxis, settings.m_speculativeContactDistance + settings.m_manifoldTolerance, result.m_shape1Face, result.m_shape2Face, manifold.m_relativeContactPointsOn1, manifold.m_relativeContactPointsOn2);

                        // Calculate normal
                        manifold.m_worldSpaceNormal = result.m_penetrationAxis.Normalized();

                        // Store the penetration depth.
                        manifold.m_penetrationDepth = result.m_penetrationDepth;

                        // Prune if we have more than 4 points
                        if (manifold.m_relativeContactPointsOn1.size() > 4)
                            PruneContactPoints(manifold.m_worldSpaceNormal, manifold.m_relativeContactPointsOn1, manifold.m_relativeContactPointsOn2);

                        // Set other properties
                        manifold.m_subShapeID1 = result.m_subShapeID1;
                        manifold.m_subShapeID2 = result.m_subShapeID2;

                        // Add the contact points to the manager.
                        m_constraintCreated |= m_pPhysicsScene->m_contactManager.AddContactConstraint(m_contactAllocator, m_bodyPairHandle, *m_pBody1, *m_pBody2, manifold);
                    }
                };

                NonReductionCollideShapeCollector collector(this, contactAllocator, pBody1, pBody2, bodyPairHandle);

                // Perform collision detection between the two shapes.
                m_simCollideBodyVsBody(*pBody1, *pBody2, transform1, transform2, settings, collector, shapeFilter.GetFilter());

                constraintCreated = collector.m_constraintCreated;
            }
        }

        /// If a contact constraint was created, we need to ensure that they are awake, and link them in the island builder.
        if (constraintCreated)
        {
            // Wake up the sleeping bodies.
            BodyID bodyIDs[2];
            int numBodies = 0;
            if (pBody1->IsDynamic() && !pBody1->IsActive())
                bodyIDs[numBodies++] = pBody1->GetID();
            if (pBody2->IsDynamic() && !pBody2->IsActive())
                bodyIDs[numBodies++] = pBody2->GetID();

            if (numBodies > 0)
                m_bodyManager.ActivateBodies(bodyIDs, numBodies);

            // Link the two bodies
            m_islandBuilder.LinkBodies(pBody1->Internal_GetIndexInActiveBodies(), pBody2->Internal_GetIndexInActiveBodies());
        }
    }

    void PhysicsScene::CheckSleepAndUpdateBounds(const uint32 islandIndex, const PhysicsUpdateContext* pContext, const PhysicsUpdateContext::Step* pStep, BodiesToSleep& bodiesToSleep)
    {
        // Get the bodies that belong to this island.
        BodyID* pBodiesBegin;
        BodyID* pBodiesEnd;
        m_islandBuilder.GetBodiesInIsland(islandIndex, pBodiesBegin, pBodiesEnd);

        // Only check sleeping in the last step
        if (pStep->m_isLast)
        {
            static_assert(static_cast<int>(ECanSleep::CannotSleep) == 0 && static_cast<int>(ECanSleep::CanSleep) == 1, "Loop below makes this assumption");
            int allCanSleep = m_physicsSettings.m_allowSleeping? static_cast<int>(ECanSleep::CanSleep) : static_cast<int>(ECanSleep::CannotSleep);

            const float timeBeforeSleep = m_physicsSettings.m_timeBeforeSleep;
            const float maxMovement = m_physicsSettings.m_pointVelocitySleepThreshold * timeBeforeSleep;

            for (const BodyID* pBodyID = pBodiesBegin; pBodyID < pBodiesEnd; ++pBodyID)
            {
                Body& body = m_bodyManager.GetBody(*pBodyID);

                // Update the bounding box
                body.Internal_CalculateWorldSpaceBounds();

                // Update Sleeping
                allCanSleep &= static_cast<int>(body.Internal_UpdateSleepState(pContext->m_stepDeltaTime, maxMovement, timeBeforeSleep));

                // Reset force and torque
                MotionProperties* pMotionProps = body.GetMotionProperties();
                pMotionProps->ResetForce();
                pMotionProps->ResetTorque();
            }

            if (allCanSleep == static_cast<int>(ECanSleep::CanSleep))
                bodiesToSleep.PutToSleep(pBodiesBegin, pBodiesEnd);
        }
        else
        {
            // Update the bounding box only for all other steps.
            for (const BodyID* pBodyID = pBodiesBegin; pBodyID < pBodiesEnd; ++pBodyID)
            {
                Body& body = m_bodyManager.GetBody(*pBodyID);
                body.Internal_CalculateWorldSpaceBounds();
            }
        }

        // Notify the broadphase of the changed objects (FindCCDContacts can do linear casts in the next step, so we need to do this every step).
        m_pBroadphase->NotifyBodiesAABBChanged(pBodiesBegin, static_cast<int>(pBodiesEnd - pBodiesBegin), false);
    }

    void PhysicsScene::Internal_DefaultSimCollideBodyVsBody(const Body& body1, const Body& body2, const Mat44& centerOfMassTransform1, const Mat44& centerOfMassTransform2, CollideShapeSettings& settings, CollideShapeCollector& collector, const ShapeFilter& shapeFilter)
    {
        SubShapeIDCreator part1, part2;

        if (body1.GetEnhancedInternalEdgeRemovalWithBody(body2))
        {
            // Collide with enhanced internal-edge removal
            settings.m_activeEdgeMode = EActiveEdgeMode::CollideWithAll;
            InternalEdgeRemovingCollector::CollideShapeVsShape(body1.GetShape(), body2.GetShape(), Vec3::One(), Vec3::One(), centerOfMassTransform1, centerOfMassTransform2, part1, part2, settings, collector, shapeFilter);
        }
        else
        {
            // Regular collide
            CollisionSolver::CollideShapeVsShape(body1.GetShape(), body2.GetShape(), Vec3::One(), Vec3::One(), centerOfMassTransform1, centerOfMassTransform2, part1, part2, settings, collector, shapeFilter);
        }
    }
}
