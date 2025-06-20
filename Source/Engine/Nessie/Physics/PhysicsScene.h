﻿// PhysicsScene.h
#pragma once
#include "IslandBuilder.h"
#include "LargeIslandSplitter.h"
#include "PhysicsSettings.h"
#include "PhysicsUpdateContext.h"
#include "PhysicsUpdateErrorCodes.h"
#include "Body/BodyInterface.h"
#include "Body/BodyLockInterface.h"
#include "Body/BodyManager.h"
#include "Collision/BroadPhase/BroadPhase.h"
#include "Constraints/ConstraintManager.h"
#include "Core/Jobs/JobSystem.h"

namespace nes
{
    class PhysicsStepListener;
    
    //----------------------------------------------------------------------------------------------------
    ///	@brief : Class that runs physics simulation for all registered Bodies.   
    //----------------------------------------------------------------------------------------------------
    class PhysicsScene
    {
        friend class World;
        
    public:
        /// Maximum number of bodies that is supported.
        static constexpr uint32_t   kMaxBodiesLimit = BodyID::kMaxBodyIndex + 1;
        //static constexpr uint32_t kMaxBodyPairsLimit = ContactConstraintManager::kMaxBodyPairsLimit;
        //static constexpr uint32_t kMaxContactConstraintsLimit = ContactConstraints::kMaxContactConstraintsLimit;
        
        struct CreateInfo
        {
            /// Maps Collision layers to the Broadphase Layers.
            /// The instance needs to stay around for the duration of the program.
            const BroadPhaseLayerInterface*         m_pLayerInterface;

            /// Filter function that is used to determine if a Collision Layer collides with a certain broad phase layer.
            /// The instance needs to stay around for the duration of the program.
            const CollisionVsBroadPhaseLayerFilter* m_pCollisionVsBroadPhaseLayerFilter;

            /// Filter function that is used to determine if two Collision Layers should collide.
            /// The instance needs to stay around for the duration of the program.
            const CollisionLayerPairFilter*         m_pCollisionLayerPairFilter;
            
            /// Maximum number of Bodies that is supported.
            uint32_t                                m_maxBodies;
            
            /// Number of Body Mutexes to use. Should be a power of 2 in the range [1, 64]. Use 0 to auto detect.
            uint32_t                                m_numBodyMutexes = 0;
            
            /// Maximum number of Body Pairs to process (anything else will fall through the world). This number should
            /// generally be much higher than the max amount of contact points as there will be lots of bodies close
            /// that are not actually touching.
            uint32_t                                m_maxNumBodyPairs;

            /// Maximum amount of contact constraints to process (anything else will fall through the world).
            uint32_t                                m_maxNumContactConstraints;
        };
    
    public:
        PhysicsScene();
        ~PhysicsScene();
        PhysicsScene(const PhysicsScene&) = delete;
        PhysicsScene& operator=(const PhysicsScene&) = delete;
        PhysicsScene(PhysicsScene&&) = delete;
        PhysicsScene& operator=(PhysicsScene&&) = delete;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the listener that is notified whenever a body is activated or deactivated.
        //----------------------------------------------------------------------------------------------------
        void                            SetBodyActivationListener(BodyActivationListener* pListener)    { m_bodyManager.SetBodyActivationListener(pListener); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the listener that is notified whenever a body is activated or deactivated.
        //----------------------------------------------------------------------------------------------------
        BodyActivationListener*         GetBodyActivationListener() const                               { return m_bodyManager.GetBodyActivationListener(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Body Interface. This interface allows you to create, remove bodies from the simulation
        ///     as well as change their properties.
        //----------------------------------------------------------------------------------------------------
        const BodyInterface&            GetBodyInterface() const                                        { return m_bodyInterfaceLocking; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Body Interface. This interface allows you to create, remove bodies from the simulation
        ///     as well as change their properties.
        //----------------------------------------------------------------------------------------------------
        BodyInterface&                  GetBodyInterface()                                              { return m_bodyInterfaceLocking; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Body Interface. This interface allows you to create, remove bodies from the simulation
        ///     as well as change their properties.
        /// @note : This version does not lock the bodies, use with great care!
        //----------------------------------------------------------------------------------------------------
        const BodyInterface&            GetBodyInterfaceNoLock() const                                  { return m_bodyInterfaceNoLock; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Body Interface. This interface allows you to create, remove bodies from the simulation
        ///     as well as change their properties.
        /// @note : This version does not lock the bodies, use with great care!
        //----------------------------------------------------------------------------------------------------
        BodyInterface&                  GetBodyInterfaceNoLock()                                        { return m_bodyInterfaceNoLock; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a Constraint to the Scene. 
        //----------------------------------------------------------------------------------------------------
        void                            AddConstraint(Constraint* pConstraint);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Add an array of Constraints to the Scene. 
        //----------------------------------------------------------------------------------------------------
        void                            AddConstraints(Constraint** constraintsArray, const int numConstraints);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove a Constraint from the Scene
        //----------------------------------------------------------------------------------------------------
        void                            RemoveConstraint(Constraint* pConstraint);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove an array of constraints to the Scene. 
        //----------------------------------------------------------------------------------------------------
        void                            RemoveConstraints(Constraint** constraintsArray, const int numConstraints);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access to the broadphase interface that allows coarse collision queries.
        //----------------------------------------------------------------------------------------------------
        const BroadPhaseQuery&          GetBroadPhaseQuery() const                          { return *m_pBroadphase; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Optimize the Broadphase. This is needed only if you've added many bodies prior to calling
        /// Update() for the first time. Don't call this every frame as Update() spreads out the same work over
        /// multiple frames.
        /// @note : Don't call this function while bodies are being modified from another thread.
        //----------------------------------------------------------------------------------------------------
        void                            OptimizeBroadPhase();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Physics Settings that govern the simulation. 
        //----------------------------------------------------------------------------------------------------
        void                            SetSettings(const PhysicsSettings& settings)        { m_settings = settings; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Physics Settings that govern the simulation.
        //----------------------------------------------------------------------------------------------------
        const PhysicsSettings&          GetSettings() const                                 { return m_settings; } 
    
    private:
        using StepListeners = std::vector<PhysicsStepListener*>;
        
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the Physics Scene. Must be called before using the scene. 
        //----------------------------------------------------------------------------------------------------
        void                            Init(const CreateInfo& createInfo);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Runs the simulation.
        ///     The World steps for a total of deltaTime seconds. This is divided in collisionSteps iterations.
        ///     Each iteration consists of collision detection followed by an integration step.
        ///     This function internally spawns jobs using the pJobSystem and waits for them to complete, so no
        ///     jobs will be running when this function returns.
        ///     The stack allocator is used, for example, to store a list of bodies that are in contact, how they
        ///     form islands together and the data to solve contacts between bodies. At the end of the function,
        ///     all allocated memory will have been freed.
        //----------------------------------------------------------------------------------------------------
        EPhysicsUpdateErrorCode          Update(const float deltaTime, int collisionSteps, StackAllocator* pAllocator, JobSystem* pJobSystem);
        
        // Job Entry Points.
        void                            JobStepListeners(PhysicsUpdateContext::Step* pStep);
        void                            JobDetermineActiveConstraints(PhysicsUpdateContext::Step* pStep) const;
        void                            JobApplyGravity(const PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep);
        void                            JobSetupVelocityContstraints(float deltaTime, PhysicsUpdateContext::Step* pStep) const;
        void                            JobBuildIslandsFromConstraints(PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep);
        void                            JobFindCollisions(PhysicsUpdateContext::Step* pStep, const int jobIndex);
        void                            JobFinalizeIslands(PhysicsUpdateContext* pContext);
        void                            JobBodySetIslandIndex();
        void                            JobSolveVelocityConstraints(PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep);
        void                            JobPreIntegrateVelocity(const PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep);
        void                            JobPostIntegrateVelocity(PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep) const;
        void                            JobFindCCDContacts(const PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep);
        void                            JobResolveCCDContacts(PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep);
        void                            JobContactRemovedCallbacks(const PhysicsUpdateContext::Step* pStep);
        void                            JobSolvePositionConstraints(PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep);
        // Ignoring Soft Body for now.
        //void        JobSoftBodyPrepare(PhysicsUpdateContext *pContext, PhysicsUpdateContext::Step *pStep);
        //void        JobSoftBodyCollide(PhysicsUpdateContext *pContext) const;
        //void        JobSoftBodySimulate(PhysicsUpdateContext *pContext, uint32_t threadIndex) const;
        //void        JobSoftBodyFinalize(PhysicsUpdateContext *pContext);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Tries to spawn a new FindCollisions job if max concurrency hasn't been reached yet. 
        //----------------------------------------------------------------------------------------------------
        void                            TrySpawnJobFindCollisions(PhysicsUpdateContext::Step* pStep) const;

    
        //using ContactAllocator = ContactConstraintManager::ContactAllocator; 
        
        /// Number of constraints to process at once in JobDetermineActiveConstraints().
        static constexpr int            kDetermineActiveConstraintsBatchSize = 64;
        
        /// Number of constraints to process at once in JobSetupVelocityConstraints(). We want a low number
        /// of threads working on this so we take fairly large batches.
        static constexpr int            kSetupVelocityConstraintsBatchSize = 256;

        /// Number of bodies to process at once in JobApplyGravity().
        static constexpr int            kApplyGravityBatchSize = 64;
        
        /// Number of active bodies to test for collisions per batch. 
        static constexpr int            kActiveBodiesBatchSize = 16;

        /// Number of active bodies to integrate velocities for, per batch.
        static constexpr int            kIntegrateVelocityBatchSize = 64;

        /// Number of contacts that need to queued before another narrow phase job is started.
        static constexpr int            kNarrowPhaseBatchSize = 16;

        /// Number of continuous collision shape casts that need to be queued before another job is started.
        static constexpr int            kNumCCDBodiesPerJob = 4;
        
        /// The Broadphase does quick collision detection between body pairs.
        BroadPhase*                     m_pBroadphase = nullptr;

        // [TODO]: 
        /// Narrow Phase Query interface
        //NarrowPhaseQuery m_narrowPhaseQueryNoLock;
        //NarrowPhaseQuery m_narrowPhaseQueryLocking;

        /// Broadphase layer filter that decides if two objects can collide.
        const CollisionVsBroadPhaseLayerFilter* m_pCollisionVsBroadPhaseLayerFilter = nullptr;

        /// Collision layer filter that decides if two objects can collide.
        const CollisionLayerPairFilter* m_pCollisionLayerPairFilter = nullptr;

        /// Simulation Settings.
        PhysicsSettings                 m_settings;

        /// Keeps track of the Bodies in the Scene.
        BodyManager                     m_bodyManager{};
        
        /// Body Locking Interfaces
        BodyLockInterfaceNoLock         m_bodyLockInterfaceNoLock     { m_bodyManager };
        BodyLockInterfaceLocking        m_bodyLockInterfaceLocking   { m_bodyManager };
        
        /// Body Interfaces
        BodyInterface                   m_bodyInterfaceNoLock;
        BodyInterface                   m_bodyInterfaceLocking;

        // [TODO]:
        /// The contact manager resolves all contacts during a simulation step.
        // ContactConstraintsManager m_contactManager;

        /// All non-contact constraints.
        ConstraintManager               m_constraintManager{};
        
        /// Keeps track of connected bodies and build islands for multithreaded velocity/position update.
        IslandBuilder                   m_islandBuilder;
        
        /// Will split large islands into smaller groups of bodies that can be processed in parallel.
        LargeIslandSplitter             m_largeIslandSplitter;

        /// Mutex for protecting m_stepListeners.
        std::mutex                      m_stepListenersMutex;

        /// List of physics step listeners.
        StepListeners                   m_stepListeners;

        /// Global gravity value for the Physics Scene.
        Vec3                            m_gravity = Vec3(0.0f, -9.81f, 0.0f);

        /// Previous frame's delta time of one sub step to allow scaling previous frame's constraint impulses.
        float                           m_previousStepDeltaTime = 0.0f;
        
    };
}