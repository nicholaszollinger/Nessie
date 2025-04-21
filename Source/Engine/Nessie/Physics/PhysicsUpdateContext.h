// PhysicsUpdateContext.h
#pragma once
#include "Collision/BroadPhase/BroadPhase.h"
#include "Core/Config.h"
#include "Core/Memory/STLStackAllocator.h"
#include "Math/Vector3.h"

namespace nes
{
    class PhysicsScene;
    class Constraint;

    // [TEMP]: This will be defined by the Job System.
    using JobHandle = uint32_t;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Information maintained during the PhysicsScene::Update(). 
    //----------------------------------------------------------------------------------------------------
    struct PhysicsUpdateContext
    {
        /// Maximum supported amount of concurrent jobs.
        static constexpr int kMaxConcurrency = 32;
        
        struct BodyPairQueue
        {
            /// Next index to write in m_bodyPair array.
            /// (Need to add thread index * m_maxBodyPairsQueue and modulo m_maxBodyPairsPerQueue).
            std::atomic<uint32_t> m_writeIndex{ 0 };
            /// Moved to own cache line to avoid conflicts with consumer jobs.
            uint8_t m_padding1[NES_CACHE_LINE_SIZE - sizeof(std::atomic<uint32_t>)];

            /// Next index to read in m_bodyPair array.
            /// (Need to add thread index * m_maxBodyPairsQueue and modulo m_maxBodyPairsPerQueue).
            std::atomic<uint32_t> m_readIndex{ 0 };
            /// Moved to own cache line to avoid conflicts with consumer jobs.
            uint8_t m_padding2[NES_CACHE_LINE_SIZE - sizeof(std::atomic<uint32_t>)];
        };

        using BodyPairQueues = std::array<BodyPairQueue, kMaxConcurrency>;
        using JobHandleArray = std::array<JobHandle, kMaxConcurrency>;
        using JobMask = uint32_t;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Structure that contains data needed for each collision step. 
        //----------------------------------------------------------------------------------------------------
        struct Step
        {
            //----------------------------------------------------------------------------------------------------
            /// @brief : "Continuous Collision Detection Body". Contains all the information needed to cast a Body
            ///     through the Scene to do continuous collision detection.
            //----------------------------------------------------------------------------------------------------
            struct CCDBody
            {
                explicit CCDBody(BodyID bodyID1, const Vector3& deltaPos, const float linearCastThresholdSqr, float maxPenetration);

                Vector3         m_deltaPosition;
                Vector3         m_contactNormal;
                Vector3         m_contactPointOn2;
                BodyID          m_bodyID1;
                BodyID          m_bodyID2;
                //SubShapeID      m_subShapeID2;
                float           m_hitFraction = 1.f;
                float           m_hitFractionPlusSlop = 1.f;
                float           m_linearCastThresholdSqr;
                float           m_maxPenetration;
                //ContactSettings m_contactSettings;
            };
            
            Step() = default;
            Step(const Step&) : Step() { NES_ASSERT(false); } // The Vector class needs a copy constructor, but it will never be called.

            PhysicsUpdateContext* m_pContext = nullptr;                                 /// The Physics Update Context associated with this Step.
            bool m_isFirst;                                                             /// If this is the first step.
            bool m_isLast;                                                              /// If this is the last step.
            
            BroadPhase::UpdateState m_broadPhaseUpdateState;                            /// Handle returned by BroadPhase::UpdatePrepare().
            uint32_t m_numActiveBodiesAtStepStart;                                      /// Number of bodies that were active at the start of the Step. Only these bodies will receive Gravity. They are the first N in the active Body List.

            std::atomic<uint32_t> m_determineActiveConstraintIndex{ 0 };          /// Next constraint for determining active constraints
            uint8_t m_padding1[NES_CACHE_LINE_SIZE - sizeof(std::atomic<uint32_t>)];    /// Padding to avoid sharing cache line with the next atomic.

            std::atomic<uint32_t> m_numActiveConstraints{ 0 };                    /// Number of Constraints in the m_activeConstraints array.
            uint8_t m_padding2[NES_CACHE_LINE_SIZE - sizeof(std::atomic<uint32_t>)];    /// Padding to avoid sharing cache line with the next atomic.

            std::atomic<uint32_t> m_setupVelocityConstraintsReadIndex{ 0 };       /// Next constraint for setting up velocity constraints.
            uint8_t m_padding3[NES_CACHE_LINE_SIZE - sizeof(std::atomic<uint32_t>)];    /// Padding to avoid sharing cache line with the next atomic.

            std::atomic<uint32_t> m_stepListenerReadIndex{ 0 };                   /// Next step listener to call.
            uint8_t m_padding4[NES_CACHE_LINE_SIZE - sizeof(std::atomic<uint32_t>)];    /// Padding to avoid sharing cache line with the next atomic.

            std::atomic<uint32_t> m_applyGravityReadIndex{ 0 };                   /// Next body to apply Gravity to.
            uint8_t m_padding5[NES_CACHE_LINE_SIZE - sizeof(std::atomic<uint32_t>)];    /// Padding to avoid sharing cache line with the next atomic.

            std::atomic<uint32_t> m_activeBodyReadIndex{ 0 };                     /// Index of the first active body that has not yet been processed by the broadphase.
            uint8_t m_padding6[NES_CACHE_LINE_SIZE - sizeof(std::atomic<uint32_t>)];    /// Padding to avoid sharing cache line with the next atomic.

            BodyPairQueues m_bodyPairQueues;                                            /// Queues in which to put body pairs that need to be tested by the narrow phase.

            std::atomic<JobMask> m_activeFindCollisionJobs;                             /// A bitmask that indicates which jobs are still active.

            std::atomic<unsigned int> m_numBodyPairs { 0 };                       /// The number of Body Pairs found during this Step. This is used to size the Contact Cache in the next step. 
            std::atomic<unsigned int> m_numManifolds { 0 };                       /// The number of Manifolds found during this Step. This is used to size the Contact Cache in the next step.

            std::atomic<uint32_t> m_solveVelocityConstraintsNextIsland { 0 };     /// Next Island that needs to be processed for the velocity constraints step. (Doesn't need its own cache line as position jobs won't run at the same time).
            std::atomic<uint32_t> m_solvePositionConstraintsNextIsland { 0 };     /// Next Island that needs to be processed for the velocity constraints step. (Doesn't need its own cache line as position jobs won't run at the same time).
            std::atomic<uint32_t> m_integrateVelocityReadIndex { 0 };             /// Next active body index to take when integrating velocities.

            // CCD Bodies
            CCDBody* m_pCCDBodies = nullptr;                                            /// List of Bodies that need to do continuous collision detection.
            uint32_t m_CCDBodiesCapacity = 0;                                           /// Capacity of the m_pCCDBodies List.
            std::atomic<uint32_t> m_numCCDBodies = 0;                                   /// Size of the m_pCCDBodies List.
            std::atomic<uint32_t> m_nextCCDBody = 0;                                    /// Next unprocessed body index in m_pCCDBodies;
            int* m_activeBodyToCCDBody = nullptr;                                       /// Mappings between an index in BodyManager::m_activeBodies and the index in m_pCCDBodies.
            uint32_t m_numActiveBodyToCCDBodies = 0;                                    /// Number of indices in m_activeBodyToCCDBody.

            // Jobs in Order of Execution. Some run in parallel.
            // BROADPHASE
            JobHandle       m_broadPhasePrepare;                                        /// Prepares the new tree in the background.
            JobHandleArray  m_stepListeners;                                            /// Listeners to notify of the beginning of the physics step.
            JobHandleArray  m_determineActiveConstraints;                               /// Determine which constraints will be active during this step.
            JobHandleArray  m_applyGravity;                                             /// Update velocities of bodies with gravity.
            JobHandleArray  m_findCollisions;                                           /// Find all collisions between active bodies in the world.
            JobHandle       m_broadPhaseFinalize;                                       /// Swap the newly built tree with the current tree.

            // NARROW PHASE
            // - Build Simulation Islands
            JobHandleArray  m_setupVelocityConstraints;                                 /// Calculate properties for all constraints in the constraint manager.
            JobHandle       m_buildIslandsFromConstraints;                              /// Go over all constraints and assign the bodies they're attached to to an Island.
            JobHandle       m_finalizeIslands;                                          /// Finalize calculation of simulation Islands.
            JobHandleArray  m_bodySetIslandIndex;                                       /// Set the current island index on each body. (Not used by the simulation, only for drawing purposes).

            // - Solve Velocity
            JobHandleArray  m_solveVelocityConstraints;                                 /// Solve the constraints in the velocity domain.
            JobHandle       m_preIntegrateVelocity;                                     /// Setup integration of all body positions.
            JobHandleArray  m_integrateVelocity;                                        /// Integrate all body positions.
            JobHandle       m_postIntegrateVelocity;                                    /// Finalize integration of all body positions.

            // - Solve Position
            JobHandle       m_resolveCCDContacts;                                       /// Updates the positions and velocities that need continuous collision detection.
            JobHandleArray  m_solvePositionConstraints;                                 /// Solve all constraints in the position domain.
            JobHandle       m_contactRemovedCallbacks;                                  /// Calls the contact removed callbacks.

            // [TODO]: 
            /// SOFT BODY COLLISION
            //JobHandle       m_softBodyPrepare;
            //JobHandleArray  m_softBodyCollide;
            //JobHandleArray  m_softBodySimulate;
            //JobHandle       m_softBodyFinalize;

            /// NEXT STEP
            JobHandle       m_startNextStep;                                            /// Job that kicks off the next step. This is empty for the last step.
        };

        using Steps = std::vector<Step, STLStackAllocator<Step>>;

        PhysicsScene*           m_pScene = nullptr;
        StackAllocator*         m_pAllocator = nullptr;
        //JobSystem*              m_pJobSystem = nullptr;
        //JobSystem::Barrier      m_barrier;
        float                   m_stepDeltaTime = 0.f;
        float                   m_warmStartImpulseRatio = 0.f;
        std::atomic<uint32_t>   m_errors = 0;
        Constraint**            m_pActiveConstraints = nullptr;
        BodyPair*               m_pBodyPairs = nullptr;
        //IslandBuilder*          m_pIslandBuilder;
        Steps                   m_steps;
        //unsigned int            m_numSoftBodies;
        //SoftBodyUpdateContext*  m_softBodyUpdateContexts = nullptr;
        //std::atomic<unsigned int> m_softBodyToCollide = 0;
        
        explicit PhysicsUpdateContext(StackAllocator& allocator);
        ~PhysicsUpdateContext();

        int GetMaxConcurrency() const { return kMaxConcurrency; /**return math::Min(kMaxConcurrency, m_pJobSystem->GetMaxConcurrency()); */}
    };
}
