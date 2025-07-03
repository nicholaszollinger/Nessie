// ContactConstraintManager.h
#pragma once

#include "Nessie/Core/StaticArray.h"
#include "Nessie/Core/Thread/Containers/LockFreeHashMap.h"
#include "Nessie/Physics/PhysicsUpdateErrorCodes.h"
#include "Nessie/Physics/Body/BodyPair.h"
#include "Nessie/Physics/Collision/ManifoldBetweenTwoFaces.h"
#include "Nessie/Physics/Collision/Shapes/SubShapeIDPair.h"
#include "Nessie/Physics/Constraints/ConstraintPart/AxisConstraintPart.h"
#include "Nessie/Physics/Constraints/ConstraintPart/DualAxisConstraintPart.h"

namespace nes
{
    struct PhysicsSettings;
    struct PhysicsUpdateContext;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Class that manages constraints between two bodies in contact with each other. 
    //----------------------------------------------------------------------------------------------------
    class ContactConstraintManager
    {
    public:
        /// Max 4 contact points are needed for a stable manifold.
        static constexpr int            kMaxContactPoints = 4;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : The contacts that are allocated in the lock-free hash map.
        //----------------------------------------------------------------------------------------------------
        struct ContactAllocator : public LFHMAllocatorContext
        {
            using LFHMAllocatorContext::LFHMAllocatorContext;

            uint					    m_numBodyPairs = 0;									/// Total number of body pairs added using this allocator.
            uint					    m_numManifolds = 0;                                 /// Total number of manifolds added using this allocator.
            EPhysicsUpdateErrorCode	    m_errors = EPhysicsUpdateErrorCode::None;			/// Errors reported on this allocator.
        };
        
        /// Callback function to combine the restitution or friction of two bodies
        /// Note that when merging manifolds (when PhysicsSettings::m_useManifoldReduction is true) you will only get a callback for the merged manifold.
        /// It is not possible in that case to get all subshape ID pairs that were colliding, you'll get the first encountered pair.
        using CombineFunction = float (*)(const Body& body1, const SubShapeID& subShapeID1, const Body& body2, const SubShapeID& subShapeID2);

        /// Handle used to keep track of the current body pair.
        using BodyPairHandle = void*;
        
    public:
        explicit ContactConstraintManager(const PhysicsSettings& settings);
        ContactConstraintManager(const ContactConstraintManager&) = delete;
        ContactConstraintManager(ContactConstraintManager&&) noexcept = delete;
        ContactConstraintManager& operator=(const ContactConstraintManager&) = delete;
        ContactConstraintManager& operator=(ContactConstraintManager&&) noexcept = delete;
        ~ContactConstraintManager();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the system.
        ///	@param maxBodyPairs : Maximum number of body pairs to process (anything else will fall through the world).
        ///     This number should generally be much higher than the max number of contact points as there will
        ///     be lots of bodies close that are not touching.
        ///	@param maxContactConstraints : Maximum number of contact constraints to process (anything else will fall through the world).
        //----------------------------------------------------------------------------------------------------
        void                            Init(uint32_t maxBodyPairs, uint32_t maxContactConstraints);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the listener that is notified whenever a contact point between two bodies is added/updated/removed.
        //----------------------------------------------------------------------------------------------------
        void                            SetContactListener(ContactListener* pListener)          { m_pContactListener = pListener; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the listener that is notified whenever a contact point between two bodies is added/updated/removed.
        //----------------------------------------------------------------------------------------------------
        ContactListener*                GetContactListener() const                              { return m_pContactListener; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the function that combines the friction of the two bodies and returns it.
        ///     The default method is the geometric mean: sqrt(friction1 * friction2).
        //----------------------------------------------------------------------------------------------------
        void                            SetCombineFriction(CombineFunction combineFriction)     { m_combineFriction = combineFriction; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the function that combines the friction of the two bodies and returns it.
        //----------------------------------------------------------------------------------------------------
        CombineFunction                 GetCombineFriction() const                              { return m_combineFriction; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the function that combines the restitution of the two bodies and returns it.
        ///     The default method is max(restitution1, restitution2).
        //----------------------------------------------------------------------------------------------------
        void                            SetCombineRestitution(CombineFunction combineFunction)  { m_combineRestitution = combineFunction; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the function that combines the restitution of the two bodies and returns it.
        //----------------------------------------------------------------------------------------------------
        CombineFunction                 GetCombineRestitution() const                           { return m_combineRestitution; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the maximum number of constraints that are allowed.
        //----------------------------------------------------------------------------------------------------
        uint32                          GetMaxConstraints() const                               { return m_maxConstraints; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check with the listener if body1 and body2 could collide. Returns false if not.
        //----------------------------------------------------------------------------------------------------
        inline EValidateContactResult   ValidateContactPoint(const Body& body1, const Body& body2, const RVec3 baseOffset, const CollideShapeResult& collisionResult) const
        {
            if (m_pContactListener == nullptr)
                return EValidateContactResult::AcceptAllContactsForThisBodyPair;
            
            return m_pContactListener->OnContactValidate(body1, body2, baseOffset, collisionResult);
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sets up the constraint buffer. Should be called before starting collision detection. 
        //----------------------------------------------------------------------------------------------------
        void                            PrepareConstraintBuffer(PhysicsUpdateContext* pContext);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a new allocator contact for storing contacts.
        /// @note : You should call this once and then add multiple contacts using the context.
        //----------------------------------------------------------------------------------------------------
        ContactAllocator                GetContactAllocator()                                   { return m_cache[m_cacheWriteIndex].GetContactAllocator(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the contact points from the previous frame are reusable and if so, copy them.
        ///     When a cache was usable and the pair has been handled: outPairHandled = true.
        ///     When a contact constraint was produced: outConstraintCreated = true.
        //----------------------------------------------------------------------------------------------------
        void                            GetContactsFromCache(ContactAllocator& contactAllocator, Body& body1, Body& body2, bool& outPairHandled, bool& outConstraintCreated);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a handle for a colliding pair so that contact constraints can be added between them.
        /// Needs to be called once per body pair per frame before calling AddContactConstraint.
        //----------------------------------------------------------------------------------------------------
        BodyPairHandle                  AddBodyPair(ContactAllocator& contactAllocator, const Body& body1, const Body& body2);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a contact constraint this frame.
        ///	@param contactAllocator : The allocator that reserves memory for the contacts.
        ///	@param bodyPairHandle : The handle for the contact cache for this body pair.
        ///	@param body1 : The first body that is colliding.
        ///	@param body2 : The second body that is colliding.
        ///	@param manifold : The manifold that describes the collision.
        ///	@returns : True if a contact constraint was created (can be false in the case of a sensor).
        //
        // This is using the approach described in 'Modeling and Solving Constraints' by Erin Catto presented at GDC 2009 (and later years with slight modifications).
        // We're using the formulas from slide 50 - 53 combined.
        //
        // Euler velocity integration:
        //
        // v1' = v1 + M^-1 P
        //
        // Impulse:
        //
        // P = J^T lambda
        //
        // Constraint force:
        //
        // lambda = -K^-1 J v1
        //
        // Inverse effective mass:
        //
        // K = J M^-1 J^T
        //
        // Constraint equation (limits movement in 1 axis):
        //
        // C = (p2 - p1) . n
        //
        // Jacobian (for position constraint)
        //
        // J = [-n, -r1 x n, n, r2 x n]
        //
        // n = contact normal (pointing away from body 1).
        // p1, p2 = positions of collision on body 1 and 2.
        // r1, r2 = contact point relative to the center of mass of body 1 and body 2 (r1 = p1 - x1, r2 = p2 - x2).
        // v1, v2 = (linear velocity, angular velocity): 6 vectors containing linear and angular velocity for body 1 and 2.
        // M = mass matrix, a diagonal matrix of the mass and inertia with diagonal [m1, I1, m2, I2].
        //----------------------------------------------------------------------------------------------------
        bool                            AddContactConstraint(ContactAllocator& contactAllocator, BodyPairHandle bodyPairHandle, Body& body1, Body& body2, const ContactManifold& manifold);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Finalizes the contact cache - the contact cache that was generated during the calls to
        ///     AddContactConstraints in this update will be used from now on to read from. After finalizing the
        ///     contact cache, the contact removed callbacks will be called.
        /// @param expectedNumBodyPairs : The number of body pairs found in the previous step and is used to
        ///     determine the number of buckets the contact cache will use in the next update.
        /// @param expectedNumManifolds : The number of manifolds found in the previous step and is used to
        ///     determine the number of buckets the contact cache will use in the next update.
        //----------------------------------------------------------------------------------------------------
        void                            FinalizeContactCacheAndCallContactPointRemovedCallback(const uint expectedNumBodyPairs, const uint expectedNumManifolds);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if two bodies were in contact during the last simulation step. Since contacts are
        ///     only created between active bodies, at least one of the bodies must be active.
        ///     This uses the read collision cache to determine if the two bodies are in contact.
        //----------------------------------------------------------------------------------------------------
        bool                            WereBodiesInContact(const BodyID& bodyID1, const BodyID& bodyID2) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of contact constraints that were found.  
        //----------------------------------------------------------------------------------------------------
        uint32                          GetNumConstraints() const                   { return math::Min<uint32>(m_numConstraints, m_maxConstraints); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sort the contact constraints deterministically. 
        //----------------------------------------------------------------------------------------------------
        void                            SortContacts(uint32* constraintIndexBegin, uint32* constraintIndexEnd) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the two affected bodies for a given constraint.
        //----------------------------------------------------------------------------------------------------
        inline void                     GetAffectedBodies(const uint32 constraintIndex, const Body*& outBody1, const Body*& outBody2) const
        {
            const ContactConstraint& constraint = m_constraints[constraintIndex];
            outBody1 = constraint.m_pBody1;
            outBody2 = constraint.m_pBody2;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Apply last frame's impulses as an initial guess for this frame's impulses.
        //----------------------------------------------------------------------------------------------------
        template <typename MotionPropertiesCallback>
        void                            WarmStartVelocityConstraints(const uint32* constraintIndexBegin, const uint32* constraintIndexEnd, const float warmStartImpulseRatio, MotionPropertiesCallback& motionPropertiesCallback);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Solve velocity constraints. When almost nothing changes, this should only apply very small
        /// impulses since we're warm starting with the total impulse applied in the last frame above.
        //
        // Friction wise we're using the Coulomb friction model which says that:
        //
        // |F_T| <= mu |F_N|
        //
        // Where F_T is the tangential force, F_N is the normal force and mu is the friction coefficient
        //
        // In impulse terms this becomes:
        //
        // |lambda_T| <= mu |lambda_N|
        //
        // And the constraint that needs to be applied is exactly the same as a non penetration constraint
        // except that we use a tangent instead of a normal. The tangent should point in the direction of the
        // tangential velocity of the point:
        //
        // J = [-T, -r1 x T, T, r2 x T]
        //
        // Where T is the tangent.
        //
        // See slide 42 and 43.
        //
        // Restitution is implemented as a velocity bias (see slide 41):
        //
        // b = e v_n^-
        //
        // e = the restitution coefficient, v_n^- is the normal velocity prior to the collision
        //
        // Restitution is only applied when v_n^- is large enough, and the points are moving towards collision.
        //----------------------------------------------------------------------------------------------------
        bool                            SolveVelocityConstraints(const uint32* constraintIndexBegin, const uint32* constraintIndexEnd);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Save back the lambdas to the contact cache for the next warm start.
        //----------------------------------------------------------------------------------------------------
        void                            StoreAppliedImpulses(const uint32* constraintIndexBegin, const uint32* constraintIndexEnd) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Solve position constraints.
        //
        // This is using the approach described in 'Modeling and Solving Constraints' by Erin Catto presented at GDC 2007.
        // On slide 78, it is suggested to split up the Baumgarte stabilization for positional drift so that it does not
        // add to the momentum. We combine an Euler velocity integrate + a position integrate and then discard the velocity
        // change.
        //
        // Constraint force:
        //
        // lambda = -K^-1 b
        //
        // Baumgarte stabilization:
        //
        // b = beta / dt C
        //
        // beta = baumgarte stabilization factor.
        // dt = delta time.
        //----------------------------------------------------------------------------------------------------
        bool                            SolvePositionConstraints(const uint32* constraintIndexBegin, const uint32* constraintIndexEnd);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Recycle the constraint buffer. Should be called between collision simulation steps. 
        //----------------------------------------------------------------------------------------------------
        void                            RecycleConstraintBuffer();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Terminate the constraint buffer. Should be called after the simulation ends. 
        //----------------------------------------------------------------------------------------------------
        void                            FinishConstraintBuffer();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called by continuous collision detection to notify the contact listener that a contact was added.
        ///	@param contactAllocator : The allocator that reserves memory for the contacts.
        ///	@param body1 : The first body that is colliding.
        ///	@param body2 : The second body that is colliding.
        ///	@param manifold : The manifold that describes the collision.
        ///	@param outSettings : The calculated contact settings (which might be overridden by the contact listener).
        //----------------------------------------------------------------------------------------------------
        void                            OnCCDContactAdded(ContactAllocator& contactAllocator, const Body& body1, const Body& body2, const ContactManifold& manifold, ContactSettings& outSettings);
        
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Local space contact point. Used for caching impulses.
        //----------------------------------------------------------------------------------------------------
        struct CachedContactPoint
        {
        public:
            /// Local space positions of Body 1 and 2.
            /// Note: these values are read through LoadFloat3Unsafe.
            Float3                      m_position1;
            Float3                      m_position2;

            // Total applied impulse during the last update that it was used.
            float                       m_nonPenetrationLambda;
            Vec2                        m_frictionLambda;
        };

        static_assert(sizeof(CachedContactPoint) == 36, "Unexpected size");
        static_assert(alignof(CachedContactPoint) == 4, "Assuming 4 byte aligned");

        //----------------------------------------------------------------------------------------------------
        /// @brief : A single cached manifold. 
        //----------------------------------------------------------------------------------------------------
        struct CachedManifold
        {
            enum class EFlags : uint16
            {
                ContactPersisted    = 1,    /// If this cache entry was reused in the next simulation update.
                CCDContact          = 2,    /// This is a cached manifold reported by continuous collision detection and was only used to create a contact callback.
            };
            
            /// Handle to the next cached contact points in ManifoldCache::m_cachedManifolds for the same body pair.
            uint32                      m_nextWithSameBodyPair;

            /// Contact normal in the space of Body 2.
            /// Note: this value is read through LoadFloat3Unsafe. 
            Float3                      m_contactNormal;

            /// @see EFlags
            mutable std::atomic<uint16> m_flags { 0 };

            /// Number of contact points in the array below.
            uint16                      m_numContactPoints;

            /// Contact points that this manifold consists of.
            CachedContactPoint          m_contactPoints[1];
            
            //----------------------------------------------------------------------------------------------------
            /// @brief : Calculate the size, in bytes, needed beyond the size of the class to store numContactPoints. 
            //----------------------------------------------------------------------------------------------------
            static int                  GetRequiredExtraSize(const int numContactPoints)     { return static_cast<int>(math::Max(0, numContactPoints - 1) * sizeof(CachedContactPoint)); }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Calculate the total class size needed for storing numContactPoints. 
            //----------------------------------------------------------------------------------------------------
            static int                  GetRequiredTotalSize(const int numContactPoints)     { return static_cast<int>(sizeof(CachedManifold) + GetRequiredExtraSize(numContactPoints)); }
        };

        static_assert(sizeof(CachedManifold) == 56, "This structure is expect to not contain any waste due to alignment");
        static_assert(alignof(CachedManifold) == 4, "Assuming 4 byte aligned");

        using ManifoldMap = LockFreeHashMap<SubShapeIDPair, CachedManifold>;
        using MKeyValue = ManifoldMap::KeyValuePair;
        using MKeyValueAndCreated = std::pair<MKeyValue*, bool>;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Start of the array of contact points for a particular pair of bodies. 
        //----------------------------------------------------------------------------------------------------
        struct CachedBodyPair
        {
            /// Local space position difference between body 1 to body 2
            /// Note: this value is read through LoadFloat3Unsafe. 
            Float3                      m_deltaPosition;

            /// Local space rotation difference between body 1 to body 2. The 4th component of the quaternion is not
            /// stored but is guaranteed to be >= 0. Note: this value is read through LoadFloat3Unsafe. 
            Float3                      m_deltaRotation;

            /// Handle to the first manifold in ManifoldCache::m_cachedManifolds.
            uint32                      m_firstCachedManifold;
        };

        static_assert(sizeof(CachedBodyPair) == 28, "Unexpected size");
        static_assert(alignof(CachedBodyPair) == 4, "Assuming 4 byte aligned");

        using BodyPairMap = LockFreeHashMap<BodyPair, CachedBodyPair>;
        using BPKeyValue = BodyPairMap::KeyValuePair;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Holds all caches that are need to quickly find cached body pairs / manifolds. 
        //----------------------------------------------------------------------------------------------------
        class ManifoldCache
        {
        public:
            //----------------------------------------------------------------------------------------------------
            /// @brief : Initialize the cache. 
            //----------------------------------------------------------------------------------------------------
            void                        Init(const uint inMaxBodyPairs, const uint inMaxContactConstraints, const uint cachedManifoldSize);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Reset all entries from the cache.
            //----------------------------------------------------------------------------------------------------
            void                        Clear();

            //----------------------------------------------------------------------------------------------------
            /// @brief : Prepare cache before creating new contacts.
            ///	@param expectedNumBodyPairs : The number of body pairs found in the previous step and is used to determine
            ///     the number of buckets the contact cache hash map will use.
            ///	@param expectedNumManifolds : The number of manifolds found in the previous step and is used to determine
            ///     the number of buckets the contact cache hash map will use.
            //----------------------------------------------------------------------------------------------------
            void                        Prepare(const uint expectedNumBodyPairs, const uint expectedNumManifolds);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Get a new allocator context for storing contacts.
            /// @note : You should call this once and then add multiple contacts using this context. 
            //----------------------------------------------------------------------------------------------------
            ContactAllocator            GetContactAllocator()       { return ContactAllocator(m_allocator, kAllocatorBlockSize); }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Find a CachedManifold from a SubShapeIDPair.
            //----------------------------------------------------------------------------------------------------
            const MKeyValue*            Find(const SubShapeIDPair& key, uint64 keyHash) const;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Create a CachedManifold from a SubShapeIDPair.
            //----------------------------------------------------------------------------------------------------
            MKeyValue*                  Create(ContactAllocator& contactAllocator, const SubShapeIDPair& key, uint64 keyHash, const int numContactPoints);

            //----------------------------------------------------------------------------------------------------
            /// @brief : If not found, create a CachedManifold from a SubShapeIDPair. The boolean value will be true if created.
            //----------------------------------------------------------------------------------------------------
            MKeyValueAndCreated         FindOrCreate(ContactAllocator& contactAllocator, const SubShapeIDPair& key, uint64 keyHash, const int numContactPoints);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Convert a (SubShapeIDPair, CachedManifold) pair to a handle.
            //----------------------------------------------------------------------------------------------------
            uint32                      ToHandle(const MKeyValue* keyValue) const;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Convert a handle to a (SubShapeIDPair, CachedManifold) pair.
            //----------------------------------------------------------------------------------------------------
            const MKeyValue*            FromHandle(const uint32 handle) const;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Find a CachedBodyPair from a BodyPair.
            //----------------------------------------------------------------------------------------------------
            const BPKeyValue*           Find(const BodyPair& key, uint64 keyHash) const;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Create a CachedBodyPair from a BodyPair.
            //----------------------------------------------------------------------------------------------------
            BPKeyValue*                 Create(ContactAllocator& contactAllocator, const BodyPair& key, uint64 keyHash);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Get all cached body pairs, sorted by key value.
            //----------------------------------------------------------------------------------------------------
            void                        GetAllBodyPairsSorted(std::vector<const BPKeyValue*>& outAll) const;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Get all cached manifolds for a particular body pair, sorted by key value.
            //----------------------------------------------------------------------------------------------------
            void                        GetAllManifoldsSorted(const CachedBodyPair& bodyPair, std::vector<const MKeyValue*>& outAll) const;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Get all continuous collision detection manifolds, sorted by key value.
            //----------------------------------------------------------------------------------------------------
            void                        GetAllCCDManifoldsSorted(std::vector<const MKeyValue*>& outAll) const;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Call pListener->OnContactRemoved() for all non-persisting contacts. 
            //----------------------------------------------------------------------------------------------------
            void                        ContactPointRemovedCallbacks(ContactListener* pListener);

        #ifdef NES_ASSERTS_ENABLED
            //----------------------------------------------------------------------------------------------------
            /// @brief : Get the number of manifolds in the cache.
            //----------------------------------------------------------------------------------------------------
            uint                        GetNumManifolds() const             { return m_cachedManifolds.GetNumKeyValues(); }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Get the number of body pairs in the cache. 
            //----------------------------------------------------------------------------------------------------
            uint                        GetNumBodyPairs() const             { return m_cachedBodyPairs.GetNumKeyValues(); }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Before a cache is finalized, you can only do Create(). After, only Find() and Clear().
            //----------------------------------------------------------------------------------------------------
            void                        Finalize();
        #endif
            
        private:
            /// Block size used when allocating new blocks in the contact cache.
            static constexpr uint32     kAllocatorBlockSize = 4096;

            /// Allocator used by both m_cachedManifolds and m_cachedBodyPairs, this makes it more likely that a body pair and its manifolds
            /// are close in memory.
            LFHMAllocator               m_allocator;

            /// Simple hash map for SubShapeIDPair -> CachedManifold
            ManifoldMap                 m_cachedManifolds { m_allocator };

            /// Simple hash map for BodyPair -> CachedBodyPair
            BodyPairMap                 m_cachedBodyPairs { m_allocator };

        #ifdef NES_ASSERTS_ENABLED
            bool                        m_isFinalized = false;      /// Marks if the buffer is complete.
        #endif
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : World space contact point, used for solving penetrations. 
        //----------------------------------------------------------------------------------------------------
        struct WorldContactPoint
        {
            /// The constraint parts.
            AxisConstraintPart          m_nonPenetrationConstraint;
            AxisConstraintPart          m_frictionConstraint1;
            AxisConstraintPart          m_frictionConstraint2;

            /// Contact Cache
            CachedContactPoint*         m_pContactPoint;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Calculate the constraint properties above.
            //----------------------------------------------------------------------------------------------------
            void                        CalculateNonPenetrationConstraintProperties(const Body& body1, const float invMass1, const float invInertiaScale1, const Body& body2, const float invMass2, const float invInertiaScale2, const RVec3 worldSpacePosition1, const RVec3 worldSpacePosition2, const Vec3 worldSpaceNormal);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Calculate the constraint properties above, with the body motion types baked in.
            //----------------------------------------------------------------------------------------------------
            template <EBodyMotionType Type1, EBodyMotionType Type2>
            NES_INLINE void             TemplatedCalculateFrictionAndNonPenetrationConstraintProperties(float deltaTime, float inGravityDeltaTimeDotNormal, const Body& body1, const Body& body2, float invMass1, float invMass2, const Mat44& invI1, const Mat44& invI2, const RVec3 worldSpacePosition1, const RVec3 worldSpacePosition2, const Vec3 worldSpaceNormal, const Vec3 worldSpaceTangent1, const Vec3 worldSpaceTangent2, const ContactSettings& settings, float minVelocityForRestitution);
        };
        
        using WorldContactPoints = StaticArray<WorldContactPoint, kMaxContactPoints>;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Contact constraints are used for solving penetrations between bodies. 
        //----------------------------------------------------------------------------------------------------
        struct ContactConstraint
        {
            Body*                       m_pBody1;
            Body*                       m_pBody2;
            uint64                      m_sortKey;
            Float3                      m_worldSpaceNormal;
            float                       m_combinedFriction;
            float                       m_inverseMass1;
            float                       m_inverseInertiaScale1;
            float                       m_inverseMass2;
            float                       m_inverseInertiaScale2;
            WorldContactPoints          m_contactPoints;
            
            //----------------------------------------------------------------------------------------------------
            /// @brief : Convert world space normal to a Vec3 
            //----------------------------------------------------------------------------------------------------
            NES_INLINE Vec3             GetWorldSpaceNormal() const { return Vec3::LoadFloat3Unsafe(m_worldSpaceNormal); }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Calculate the tangents for this contact constraint. 
            //----------------------------------------------------------------------------------------------------
            NES_INLINE void             GetTangents(Vec3& outTangent1, Vec3& outTangent2) const
            {
                Vec3 worldSpaceNormal = GetWorldSpaceNormal();
                outTangent1 = worldSpaceNormal.NormalizedPerpendicular();
                outTangent2 = worldSpaceNormal.Cross(outTangent1);
            }
        };

    public:
        /// The maximum value that can be passed to Init for maxContactConstraints. Note: you should really use a lower value; using this value will cost a lot of memory!
        static constexpr uint           kMaxContactConstraintsLimit = ~static_cast<uint>(0) / sizeof(ContactConstraint);
        
        /// The maximum value that can be passed to Init for maxBodyPairs. Note: you should really use a lower value; using this value will cost a lot of memory!
        static constexpr uint           kMaxBodyPairsLimit = ~static_cast<uint>(0) / sizeof(BodyPairMap::KeyValuePair);

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Internal helper function to calculate the friction and non-penetration constraint properties.
        ///     Templated to the motion type to reduce the number of branches and calculations.
        //----------------------------------------------------------------------------------------------------
        template <EBodyMotionType Type1, EBodyMotionType Type2>
        NES_INLINE void                 TemplatedCalculateFrictionAndNonPenetrationConstraintProperties(ContactConstraint& constraint, const ContactSettings& settings, float deltaTime, const Vec3 gravityDeltaTime, const Mat44& transformBody1, const Mat44& transformBody2, const Body& body1, const Body& body2);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Internal helper function to calculate the friction and non-penetration constraint properties.
        //----------------------------------------------------------------------------------------------------
        inline void                     CalculateFrictionAndNonPenetrationConstraintProperties(ContactConstraint& constraint, const ContactSettings& settings, float deltaTime, const Vec3 gravityDeltaTime, const Mat44& transformBody1, const Mat44& transformBody2, const Body& body1, const Body& body2);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Internal helper function to add a contact constraint.
        ///     Templated to the motion type to reduce the number of branches and calculations.
        //----------------------------------------------------------------------------------------------------
        template <EBodyMotionType Type1, EBodyMotionType Type2>
        bool                            TemplatedAddContactConstraint(ContactAllocator& contactAllocator, BodyPairHandle bodyPairHandle, Body& body1, Body& body2, const ContactManifold& manifold);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Internal helper function to warm start a contact constraint.
        ///     Templated to the motion type to reduce the number of branches and calculations. 
        //----------------------------------------------------------------------------------------------------
        template <EBodyMotionType Type1, EBodyMotionType Type2>
        NES_INLINE static void          WarmStartConstraint(ContactConstraint& constraint, MotionProperties* pMotionProps1, MotionProperties* pMotionProps2, const float warmStartImpulseRatio);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Internal helper function to solve a single contact constraint.
        ///     Templated to the motion type to reduce the number of branches and calculations. 
        //----------------------------------------------------------------------------------------------------
        template <EBodyMotionType Type1, EBodyMotionType Type2>
        NES_INLINE static bool          SolveVelocityConstraint(ContactConstraint& constraint, MotionProperties* pMotionProps1, MotionProperties* pMotionProps2);
        
    private:
        /// We have one cache to read from, and one to write to.
        ManifoldCache                   m_cache[2];
        
        /// Which cache we are currently writing to.
        int                             m_cacheWriteIndex = 0;
        
        /// The main physics settings instance.
        const PhysicsSettings&          m_physicsSettings;

        /// Listener that is notified whenever a contact point between two bodies is added/updated/removed.
        ContactListener*                m_pContactListener = nullptr;

        /// Functions used to combine the friction and restitution between two bodies.
        CombineFunction                 m_combineFriction = [](const Body& body1, const SubShapeID&, const Body& body2, const SubShapeID&) { return std::sqrt(body1.GetFriction() * body2.GetFriction()); };
        CombineFunction                 m_combineRestitution = [](const Body& body1, const SubShapeID&, const Body& body2, const SubShapeID&) { return math::Max(body1.GetRestitution(), body2.GetRestitution()); };

        /// The constraints that were added this frame.
        ContactConstraint*              m_constraints = nullptr;
        uint32                          m_maxConstraints = 0;
        std::atomic<uint32>             m_numConstraints { 0 };

        /// Context used for this physics update
        PhysicsUpdateContext*           m_pUpdateContext = nullptr;
    };
}
