// PhysicsSettings.h
#pragma once
#include "Core/Memory/Memory.h"
#include "Math/Generic.h"

namespace nes
{
    namespace physics
    {
        /// If objects are closer than this distance, they are considered to be colliding. Used for GJK. Unit: meter.
        static constexpr float  kDefaultCollisionTolerance = 1.0e-4f;

        /// A factor that determines the accuracy of the penetration depth calculation. If the change of the
        /// squared distance is less than the tolerance * currentPenetrationDepth^2 the algorithm will terminate.
        /// - This stops when there's less than a 1% change.
        static constexpr float  kDefaultPenetrationTolerance = 1.0e-4f;

        /// How much padding to add around objects.
        static constexpr float  kDefaultConvexRadius = 0.05f;

        /// Used by (Tapered)CapsuleShape to determine when supporting phase is an edge rather than a point. Unit: meter.
        static constexpr float  kCapsuleProjectionSlop = 0.02f;

        /// Maximum number of jobs to allow.
        static constexpr int    kMaxPhysicsJobs = 2048;

        /// Maximum number of barriers to allow.
        static constexpr int    kMaxPhysicsBarriers = 8;
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Settings for the Physics Simulation. All distances are in meters unless otherwise specified.
    //----------------------------------------------------------------------------------------------------
    struct PhysicsSettings
    {
        NES_OVERRIDE_NEW_DELETE

        /// Size of body pairs array. Corresponds to the maximum amount of potential body pairs that can be
        /// in flight at any time. Setting this to a low value will use less memory but slow down simulation
        /// as threads may run out of narrow phase work.
        int         m_maxInFlightBodyPairs = 16384;

        /// How many PhysicsStepListeners to notify in 1 batch.
        int         m_stepListenersBatchSize = 8;

        /// How many step listener batches are needed before spawning another job. Set to INT_MAX if no parallelism is
        /// desired.
        int         m_stepListenersBatchesPerJob = 1;

        /// Baumgarte stabilization factor: how much of the position error to 'fix' in 1 update. 0 = 0%, 1 = 100%
        float       m_baumgarte = 0.2f;

        /// Radius around objects inside which speculative context points will be detected. Note that if this is too
        /// big you will get ghost collisions as speculative contacts are based on the closest points during the collision
        /// detection step which many not be actual closest points by the time the two objects hit.
        float       m_speculativeContactDistance = 0.02f;

        /// How much bodies are allowed to sink into each other.
        float       m_penetrationSlop = 0.02f;

        /// Fraction of a body's inner radius that it must move per step to enable casting for the LinearCast motion quality. 
        float       m_linearCastThreshold = 0.75f;

        /// Fraction of a body's inner radius that may penetrate another body for the LinearCast motion quality.
        float       m_linearCastMaxPenetration = 0.25f;

        /// Max distance to use to determine if two points are on the same plane for determining the contact manifold between
        /// two shape faces. Unit: meters.
        float       m_manifoldTolerance = 1.0e-3f;

        /// Max distance to correct in a single iteration when solving position constraints.
        float       m_maxPenetrationDistance = 0.2f;

        /// Max relative delta position for body pairs to be able to reuse collision results from last frame. Units: meters^2.
        float       m_bodyPairCacheMaxDeltaPositionSqr = math::Squared(0.001f); // 1 mm.

        /// Max relative delta rotation for body pairs to be able to reuse collision results from last frame. Stored as cos(max angle / 2).
        float       m_bodyPairCacheCosMaxDeltaRotationDiv2 = 0.99984769515639123915701155881391f; // cos(2 degrees / 2).

        /// Max angle between normals that allows manifolds between different sub shapes of the same body pair to be combined.
        float       m_contactNormalCosMaxDeltaRotation = 0.99619469809174553229501040247389f; // cos(5 degrees).

        /// Max allowed distance between old and new contact point to preserve contact forces for warm start. Units: meter^2.
        float       m_contactNormalPreserveLambdaMaxDistSqr = math::Squared(0.01f); // 1 cm

        /// Number of solver velocity iterations to run.
        /// Note that this needs to be >= 2 in order for friction to work. Friction is applied using the non-penetration impulse from the previous iteration. 
        uint        m_numVelocitySteps = 10;

        /// Number of solver position iterations to run.
        uint        m_numPositionSteps = 2;
        
        /// Minimal velocity needed before a collision can be elastic. If the relative velocity between colliding objects
        /// in the direction of the contact normal is lower than this, the restitution will be zero regardless of the configured
        /// value. This lets an object settle sooner. Must be a positive number.
        float       m_minVelocityForRestitution = 1.f;
        
        /// Time before a Body is allowed to go to sleep.
        float       m_timeBeforeSleep = 0.5f;

        /// To detect if a Body is sleeping, we use 3 points:
        /// - The center of mass.
        /// - The centers of the faces of the bounding box that are furthest away from the center.
        /// The movement of these points is tracked and if the velocity of all 3 points is lower than this value,
        /// the Body is allowed to go to sleep. Must be a positive number. (unit: m/s)
        float       m_pointVelocitySleepThreshold = 0.03f;

        /// By default, the simulation is deterministic. Setting this to false will make the simulation run faster, but it won't
        /// be deterministic.
        bool        m_simulationIsDeterministic = true;

        //----------------------------------------------------------------------------------------------------
        // The following variables are mainly for debugging purposes. They allow turning on/off certain subsystems.
        // You probably want to leave these alone.
        //----------------------------------------------------------------------------------------------------

        /// Whether to use warm starting for constraints. This initially applies previous frame impulses.
        bool        m_useConstraintWarmStart = true;
        
        /// Whether to use the body pair cache, which removes the need to for narrow phase collision detection when
        /// orientation between two bodies didn't change.
        bool        m_useBodyPairContactCache = true;

        /// Whether to reduce manifolds with similar contact normals into one contact normal.
        /// (see description at Body::SetUseManifoldReduction())
        bool        m_useManifoldReduction = true;

        /// Whether to split up large islands into smaller parallel batches of work (to improve performance).
        bool        m_useLargeIslandSplitter = true;

        /// Whether objects can go to sleep or not.
        bool        m_allowSleeping = true;

        /// When false, we prevent collision against non-active, shared, edges. Mainly for debugging the algorithm.
        bool        m_checkActiveEdges = true;
    };
}
