// PhysicsScene.h
#pragma once
#include "IslandBuilder.h"
#include "LargeIslandSplitter.h"
#include "PhysicsSettings.h"
#include "PhysicsUpdateContext.h"
#include "Body/BodyInterface.h"
#include "Collision/ContactListener.h"
#include "Collision/NarrowPhaseQuery.h"
#include "Collision/BroadPhase/BroadPhase.h"
#include "Constraints/ConstraintManager.h"
#include "Constraints/ContactConstraintManager.h"

namespace nes
{
    class PhysicsStepListener;
    class JobSystem;
    class StackAllocator;
    class SimShapeFilter;
    
    //----------------------------------------------------------------------------------------------------
    ///	@brief : Class that runs physics simulation for all registered Bodies.   
    //----------------------------------------------------------------------------------------------------
    class PhysicsScene
    {
        friend class World;
        using CCDBody = PhysicsUpdateContext::Step::CCDBody;
        
    public:
        /// Maximum number of bodies that is supported.
        static constexpr uint32   kMaxBodiesLimit = BodyID::kMaxBodyIndex + 1;
        static constexpr uint32   kMaxBodyPairsLimit = ContactConstraintManager::kMaxBodyPairsLimit;
        static constexpr uint32   kMaxContactConstraintsLimit = ContactConstraintManager::kMaxContactConstraintsLimit;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Struct with data to provide to PhysicsScene::Init(). 
        //----------------------------------------------------------------------------------------------------
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
            uint32                                  m_maxBodies;
            
            /// Number of Body Mutexes to use. Should be a power of 2 in the range [1, 64]. Use 0 to auto-detect.
            uint32                                  m_numBodyMutexes = 0;
            
            /// Maximum number of Body Pairs to process (anything else will fall through the world). This number should
            /// generally be much higher than the max number of contact points as there will be lots of bodies close
            /// that are not touching.
            uint32                                  m_maxNumBodyPairs;

            /// Maximum number of contact constraints to process (anything else will fall through the world).
            uint32                                  m_maxNumContactConstraints;
        };

        /// Combine function used to combine friction and restitution between bodies.
        using CombineFunction = ContactConstraintManager::CombineFunction;
        
        /// Advanced use only. This function is similar to CollisionSolver::CollideShapeVsShape but only used to collide bodies during simulation.
        /// body1: The first body to collide.
        /// body2: The second body to collide.
        /// centerOfMassTransform1: The center of mass transform for the first body (note this will not be the actual world space position of the body, it will be made relative to some position so we can drop to single precision). 
        /// centerOfMassTransform2: The center of mass transform for the second body.
        /// settings: Settings that control the collision detection. Note that the implementation can freely overwrite the shape settings as needed; the caller provides a temporary that will not be used after the function returns.
        /// collector: The collector that will receive the contact points.
        /// shapeFilter: The shape filter that can be used to exclude shapes from colliding with one another.
        using SimCollideBodyVsBody = std::function<void(const Body& body1, const Body& body2, const Mat44& centerOfMassTransform1, const Mat44& centerOfMassTransform2, CollideShapeSettings& settings, CollideShapeCollector& collector, const ShapeFilter& shapeFilter)>;
    
    public:
        PhysicsScene();
        ~PhysicsScene();
        PhysicsScene(const PhysicsScene&) = delete;
        PhysicsScene& operator=(const PhysicsScene&) = delete;
        PhysicsScene(PhysicsScene&&) = delete;
        PhysicsScene& operator=(PhysicsScene&&) = delete;

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
        EPhysicsUpdateErrorCode         Update(const float deltaTime, int collisionSteps, StackAllocator* pAllocator, JobSystem* pJobSystem);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the listener which is notified whenever a body is activated or deactivated.
        //----------------------------------------------------------------------------------------------------
        void                            SetBodyActivationListener(BodyActivationListener* pListener)    { m_bodyManager.SetBodyActivationListener(pListener); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the listener which is notified whenever a body is activated or deactivated.
        //----------------------------------------------------------------------------------------------------
        BodyActivationListener*         GetBodyActivationListener() const                               { return m_bodyManager.GetBodyActivationListener(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the listener which is notified whenever a contact point between two bodies is
        ///     added/updated/removed. You can't change the contact listener during a PhysicsScene::Update,
        ///     but it can be changed at any other time.
        //----------------------------------------------------------------------------------------------------
        void                            SetContactListener(ContactListener* pListener)                  { m_contactManager.SetContactListener(pListener); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the listener which is notified whenever a contact point between two bodies is
        ///     added/updated/removed. 
        //----------------------------------------------------------------------------------------------------
        ContactListener*                GetContactListener() const                                      { return m_contactManager.GetContactListener(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the function that combines the friction of two bodies and returns it.
        ///     The default method is the geometric mean: sqrt(friction1 * friction2).
        //----------------------------------------------------------------------------------------------------
        void                            SetCombineFriction(const CombineFunction& func)                 { m_contactManager.SetCombineFriction(func); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the function that combines the friction of two bodies and returns it.
        ///     The default method is the geometric mean: sqrt(friction1 * friction2).
        //----------------------------------------------------------------------------------------------------
        CombineFunction                 GetCombineFriction() const                                      { return m_contactManager.GetCombineFriction(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the function that combines the restitution of two bodies and returns it.
        /// The default method is max(restitution1, restitution2).
        //----------------------------------------------------------------------------------------------------
        void                            SetCombineRestitution(const CombineFunction& func)              { m_contactManager.SetCombineRestitution(func); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the function that combines the restitution of two bodies and returns it.
        /// The default method is max(restitution1, restitution2).
        //----------------------------------------------------------------------------------------------------
        CombineFunction                 GetCombineRestitution() const                                   { return m_contactManager.GetCombineRestitution(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the shape filter that will be used during simulation. This can be used to exclude
        /// shapes within a body from colliding with each other. For example, if you have a high detail
        /// collision model when simulating to exclude a low detail collision model when casting rays.
        /// Note that in this case, you would need to pass the inverse of pFilter to the CastRay() function.
        ///
        ///	@param pFilter : Filter to set. Pass in nullptr to disable the shape filter.
        ///
        /// @note : The PhysicsScene does not own the ShapeFilter, so make sure that it stays in memory during
        /// the lifetime of the PhysicsScene!
        //----------------------------------------------------------------------------------------------------
        void                            SetSimShapeFilter(const SimShapeFilter* pFilter)                { m_pSimShapeFilter = pFilter; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the shape filter used during simulation. See SetSimShapeFilter() for more details. 
        //----------------------------------------------------------------------------------------------------
        const SimShapeFilter*           GetSimShapeFilter() const                                       { return m_pSimShapeFilter; }
        
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
        /// @brief : Access to the broadphase interface that allows coarse collision queries.
        //----------------------------------------------------------------------------------------------------
        const BroadPhaseQuery&          GetBroadPhaseQuery() const                                      { return *m_pBroadphase; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the interface that allows fine collision queries against the broadphase and then the
        ///     narrow phase.
        //----------------------------------------------------------------------------------------------------
        const NarrowPhaseQuery&         GetNarrowPhaseQuery() const                                     { return m_narrowPhaseQueryLocking; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the interface that allows fine collision queries against the broadphase and then the
        ///     narrow phase.
        /// @note : This version does not lock the bodies, use with great care!
        //----------------------------------------------------------------------------------------------------
        const NarrowPhaseQuery&         GetNarrowPhaseQueryNoLock() const                               { return m_narrowPhaseQueryNoLock; }
        
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
        /// @brief : Get the array of all constraints. 
        //----------------------------------------------------------------------------------------------------
        Constraints                     GetConstraints() const                                          { return m_constraintManager.GetConstraints(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Optimize the Broadphase. This is needed only if you've added many bodies prior to calling
        /// Update() for the first time. Don't call this every frame as Update() spreads out the same work over
        /// multiple frames.
        /// @note : Don't call this function while bodies are being modified from another thread.
        //----------------------------------------------------------------------------------------------------
        void                            OptimizeBroadPhase();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a new step listener. 
        //----------------------------------------------------------------------------------------------------
        void                            AddStepListener(PhysicsStepListener* pListener);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove a step listener.
        //----------------------------------------------------------------------------------------------------
        void                            RemoveStepListener(PhysicsStepListener* pListener);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Physics Settings that govern the simulation. 
        //----------------------------------------------------------------------------------------------------
        void                            SetSettings(const PhysicsSettings& settings)                    { m_physicsSettings = settings; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Physics Settings that govern the simulation.
        //----------------------------------------------------------------------------------------------------
        const PhysicsSettings&          GetSettings() const                                             { return m_physicsSettings; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the global gravity for the physics scene.
        //----------------------------------------------------------------------------------------------------
        void                            SetGravity(const Vec3 gravity)                                  { m_gravity = gravity; } 
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the global gravity for the physics scene. 
        //----------------------------------------------------------------------------------------------------
        Vec3                            GetGravity() const                                              { return m_gravity; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the locking interface that won't actually lock the body.
        /// @note : Use with great care!
        //----------------------------------------------------------------------------------------------------
        inline const BodyLockInterfaceNoLock& GetBodyLockInterfaceNoLock() const                        { return m_bodyLockInterfaceNoLock; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the locking interface that locks the body so other threads cannot modify it. 
        //----------------------------------------------------------------------------------------------------
        inline const BodyLockInterfaceLocking& GetBodyLockInterface() const                             { return m_bodyLockInterfaceLocking; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Broadphase layer filter that decides if two objects can collide. This was passed into the init function.
        //----------------------------------------------------------------------------------------------------
        const CollisionVsBroadPhaseLayerFilter& GetCollisionVsBroadPhaseLayerFilter() const             { return *m_pCollisionVsBroadPhaseLayerFilter; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Collision layer filter that decides if two objects can collide. This was passed into the init function.
        //----------------------------------------------------------------------------------------------------
        const CollisionLayerPairFilter& GetCollisionLayerPairFilter() const                             { return *m_pCollisionLayerPairFilter; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a broadphase layer filter that uses the default pair filter and a specified collision
        ///     layer to determine if broadphase layers collide.
        //----------------------------------------------------------------------------------------------------
        DefaultBroadPhaseLayerFilter    GetDefaultBroadPhaseFilter(CollisionLayer layer) const          { return DefaultBroadPhaseLayerFilter(*m_pCollisionVsBroadPhaseLayerFilter, layer); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a collision layer filter that uses the default pair filter and a specified layer to
        ///     determine if layers collide.
        //----------------------------------------------------------------------------------------------------
        DefaultCollisionLayerFilter     GetDefaultCollisionLayerFilter(CollisionLayer layer) const               { return DefaultCollisionLayerFilter(*m_pCollisionLayerPairFilter, layer); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of bodies that are in the body manager. 
        //----------------------------------------------------------------------------------------------------
        uint                            GetNumBodies() const                                            { return m_bodyManager.GetNumBodies(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of active bodies that are in the body manager. 
        //----------------------------------------------------------------------------------------------------
        uint32                          GetNumActiveBodies() const                                      { return m_bodyManager.GetNumActiveBodies(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the maximum number of bodies that this physics scene supports. 
        //----------------------------------------------------------------------------------------------------
        uint                            GetMaxBodies() const                                            { return m_bodyManager.GetMaxNumBodies(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a copy of the array of all bodies under protection of a lock. 
        ///	@param outBodyIDs : On return, this will contain the list of BodyIDs.
        //----------------------------------------------------------------------------------------------------
        void                            GetBodies(BodyIDVector& outBodyIDs) const                       { return m_bodyManager.GetBodyIDs(outBodyIDs); }

        //----------------------------------------------------------------------------------------------------
        // [TODO]: Rigid vs Soft bodies.
        /// @brief : Get a copy of the array of all active bodies under protection of a lock.
        ///	@param outBodyIDs : On return, this will contain the list of BodyIDs.
        //----------------------------------------------------------------------------------------------------
        void                            GetActiveBodies(/*EBodyType type,*/ BodyIDVector& outBodyIDs) const { return m_bodyManager.GetActiveBodies(outBodyIDs); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the array of active bodies; use GetNumActiveBodies() to find out the array size.
        /// @note : Not thread safe. The active bodies list can change at any moment when other threads are
        ///     doing work. Use GetActiveBodies() if you need a thread safe version. 
        //----------------------------------------------------------------------------------------------------
        const BodyID*                   GetActiveBodiesUnsafe(/*EBodyType type*/) const                 { return m_bodyManager.GetActiveBodiesUnsafe(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if 2 bodies were in contact during the last simulation step. Contacts are only
        ///     detected between active bodies, so at least one of the bodies must be active in order for
        ///     this function to work. This queries the state at the time of the last PhysicsScene::Update()
        ///     and will return true if the bodies were in contact, even if one of the bodies was moved/removed afterwards.
        ///
        ///     This function can be called from any thread when the PhysicsScene::Update is not running.
        ///     During PhysicsScene::Update() this function is only valid during contact callbacks:
        ///     - During the ContactListener::OnContactAdded() callback this function can be used to determine if a different
        ///       contact pair between the bodies was active in the previous simulation step (function returns true) or
        ///       if this is the first time that the bodies are touching.
        ///     - During the ContactListener::OnContactRemoved() callback this function can be used to determine if this is the
        ///       last contact pair between the bodies (function returns false) or if there are other contacts still present
        ///       (function returns true).
        //----------------------------------------------------------------------------------------------------
        bool                            WereBodiesInContact(const BodyID& bodyID1, const BodyID& bodyID2) const { return m_contactManager.WereBodiesInContact(bodyID1, bodyID2); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the bounding box of all bodies in the physics system. 
        //----------------------------------------------------------------------------------------------------
        AABox                           GetBounds() const                                               { return m_pBroadphase->GetBounds(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use only. Set the function that will be used to collide two bodies during simulation.
        //----------------------------------------------------------------------------------------------------
        void                            SetSimCollideBodyVsBody(const SimCollideBodyVsBody& func)       { m_simCollideBodyVsBody = func; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use only. Get the function that will be used to collide two bodies during simulation.
        //----------------------------------------------------------------------------------------------------
        const SimCollideBodyVsBody&     GetSimCollideBodyVsBody() const                                 { return m_simCollideBodyVsBody; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use only. Default function that is used to collide two bodies during simulation.
        //----------------------------------------------------------------------------------------------------
        static void                     Internal_DefaultSimCollideBodyVsBody(const Body& body1, const Body& body2, const Mat44& centerOfMassTransform1, const Mat44& centerOfMassTransform2, CollideShapeSettings& settings, CollideShapeCollector& collector, const ShapeFilter& shapeFilter);
    
    private:
        using StepListeners = std::vector<PhysicsStepListener*>;
        using ContactAllocator = ContactConstraintManager::ContactAllocator;

        //----------------------------------------------------------------------------------------------------
        /// @brief : This helper batches up bodies that need to be put to sleep to avoid contention on the
        ///     activation mutex.
        //----------------------------------------------------------------------------------------------------
        class BodiesToSleep;
        
    private:
        // Job Entry Points.
        void                            JobStepListeners(PhysicsUpdateContext::Step* pStep);
        void                            JobDetermineActiveConstraints(PhysicsUpdateContext::Step* pStep) const;
        void                            JobApplyGravity(const PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep);
        void                            JobSetupVelocityConstraints(float deltaTime, PhysicsUpdateContext::Step* pStep) const;
        void                            JobBuildIslandsFromConstraints(PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep);
        void                            JobFindCollisions(PhysicsUpdateContext::Step* pStep, const int jobIndex);
        void                            JobFinalizeIslands(PhysicsUpdateContext* pContext);
        void                            JobBodySetIslandIndex();
        void                            JobSolveVelocityConstraints(PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep);
        void                            JobPreIntegrateVelocity(PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep);
        void                            JobIntegrateVelocity(const PhysicsUpdateContext* pContext, PhysicsUpdateContext::Step* pStep);
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

        //----------------------------------------------------------------------------------------------------
        /// @brief : Process narrow phase for a single body pair.
        //----------------------------------------------------------------------------------------------------
        void                            ProcessBodyPair(ContactAllocator& contactAllocator, const BodyPair& bodyPair);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called at the end of JobSolveVelocityConstraints() to check if bodies need to go to sleep
        ///     and to update their bounding box in the broadphase.
        //----------------------------------------------------------------------------------------------------
        void                            CheckSleepAndUpdateBounds(const uint32 islandIndex, const PhysicsUpdateContext* pContext, const PhysicsUpdateContext::Step* pStep, BodiesToSleep& bodiesToSleep);
        
    private:
        /// Number of constraints to process at once in JobDetermineActiveConstraints().
        static constexpr int            kDetermineActiveConstraintsBatchSize = 64;
        
        /// Number of constraints to process at once in JobSetupVelocityConstraints(). We want a low number
        /// of threads working on this, so we take fairly large batches.
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

        /// Broadphase layer filter that decides if two objects can collide.
        const CollisionVsBroadPhaseLayerFilter* m_pCollisionVsBroadPhaseLayerFilter = nullptr;

        /// Collision layer filter that decides if two objects can collide.
        const CollisionLayerPairFilter* m_pCollisionLayerPairFilter = nullptr;

        /// Keeps track of the Bodies in the Scene.
        BodyManager                     m_bodyManager{};
        
        /// Body Locking Interfaces
        BodyLockInterfaceNoLock         m_bodyLockInterfaceNoLock    { m_bodyManager };
        BodyLockInterfaceLocking        m_bodyLockInterfaceLocking   { m_bodyManager };
        
        /// Body Interfaces
        BodyInterface                   m_bodyInterfaceNoLock;
        BodyInterface                   m_bodyInterfaceLocking;

        /// Narrow Phase Query interface
        NarrowPhaseQuery                m_narrowPhaseQueryNoLock;
        NarrowPhaseQuery                m_narrowPhaseQueryLocking;

        /// The Broadphase does quick collision detection between body pairs.
        BroadPhase*                     m_pBroadphase = nullptr;
        
        /// The shape filter that is used to filter out sub shapes during simulation.
        const SimShapeFilter*           m_pSimShapeFilter = nullptr; 

        /// The collision function that is used to collide two shapes during simulation.
        SimCollideBodyVsBody            m_simCollideBodyVsBody = &Internal_DefaultSimCollideBodyVsBody; 

        /// Simulation Settings.
        PhysicsSettings                 m_physicsSettings;
        
        /// The contact manager resolves all contacts during a simulation step.
        ContactConstraintManager        m_contactManager;

        /// All non-contact constraints.
        ConstraintManager               m_constraintManager;
        
        /// Keeps track of connected bodies and build islands for multithreaded velocity/position update.
        IslandBuilder                   m_islandBuilder;
        
        /// Will split large islands into smaller groups of bodies that can be processed in parallel.
        LargeIslandSplitter             m_largeIslandSplitter;

        /// Mutex for protecting m_stepListeners.
        Mutex                           m_stepListenersMutex;

        /// List of physics step listeners.
        StepListeners                   m_stepListeners;

        /// Global gravity value for the Physics Scene.
        Vec3                            m_gravity = Vec3(0.0f, -9.81f, 0.0f);

        /// Previous frame's delta time of one sub step to allow scaling previous frame's constraint impulses.
        float                           m_previousStepDeltaTime = 0.0f;
    };
}