// BroadPhaseQuadTree.h
#pragma once
#include <shared_mutex>
#include "BroadPhase.h"
#include "QuadTree.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Quadtree implementation of the Broadphase. 
    //----------------------------------------------------------------------------------------------------
    class BroadPhaseQuadTree final : public BroadPhase
    {
    public:
        virtual ~BroadPhaseQuadTree() override;

    public:
        virtual void        Init(BodyManager* pBodyManager, const BroadPhaseLayerInterface& layerInterface) override;
        virtual void        Optimize() override;
        virtual void        FrameSync() override;
        virtual void        LockModifications() override;
        virtual void        UnlockModifications() override;
        virtual UpdateState UpdatePrepare() override;
        virtual void        UpdateFinalize(const UpdateState& updateState) override;

        virtual AddState    AddBodiesPrepare(BodyID* pBodies, int number) override;
        virtual void        AddBodiesFinalize(BodyID* pBodies, int number, AddState addState) override;
        virtual void        AddBodiesAbort(BodyID* pBodies, int number, AddState addState) override;
        virtual void        RemoveBodies(BodyID* pBodies, int number) override;
        virtual void        NotifyBodiesAABBChanged(BodyID* pBodies, int number, bool takeLock) override;
        virtual void        NotifyBodiesLayerChanged(BodyID* pBodies, int number) override;
        
        virtual void        CastRay(const RayCast& ray, RayCastBodyCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter) const override;
        virtual void        CastAABox(const AABoxCast& box, CastShapeBodyCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter) const override;
                void        CastAABoxNoLock(const AABoxCast& box, CastShapeBodyCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter) const;
        virtual void        CollideAABox(const AABox& box, CollideShapeBodyCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter) const override;
        virtual void        CollideSphere(const Vec3& center, const float radius, CollideShapeBodyCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter) const override;
        virtual void        CollidePoint(const Vec3& point, CollideShapeBodyCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter) const override;
        virtual void        CollideOrientedBox(const OrientedBox& box, CollideShapeBodyCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter) const override;
        virtual void        FindCollidingPairs(BodyID* pActiveBodies, int numActiveBodies, float speculativeContactDistance, const CollisionVsBroadPhaseLayerFilter& collisionVsBroadPhaseLayerFilter, const CollisionLayerPairFilter& collisionLayerPairFilter, BodyPairCollector& pairCollector) const override;
        
        virtual AABox       GetBounds() const override;

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper struct for AddBodies void* handle.
        //----------------------------------------------------------------------------------------------------
        struct LayerState
        {
            NES_OVERRIDE_NEW_DELETE

            BodyID*             m_pBodyStart = nullptr;
            BodyID*             m_pBodyEnd = nullptr;
            QuadTree::AddState  m_addState;
        };
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : UpdateState implementation for this tree used during UpdatePrepare/Finalize().
        //----------------------------------------------------------------------------------------------------
        struct UpdateStateImpl
        {
            QuadTree*              m_pTree = nullptr;
            QuadTree::UpdateState  m_updateState;
        };

        using Tracker = QuadTree::BodyTracker;
        using TrackerArray = QuadTree::BodyTrackerArray;
    
    private:
        /// Array that for each BodyID, it keeps track of where it is located in which tree.
        TrackerArray                    m_trackers;

        /// Node Allocator for all Trees.
        QuadTree::Allocator             m_allocator{};

        /// The Maximum number of Bodies that are supported.
        size_t                          m_maxBodies = 0;

        /// Information about the Broadphase->Collision Layer mappings
        const BroadPhaseLayerInterface* m_broadPhaseLayerInterface = nullptr;

        /// One Quadtree per BroadPhaseLayer.
        QuadTree*                       m_layers = nullptr;
        uint32_t                        m_numLayers = 0;

        /// This is the next tree to update in UpdatePrepare();
        uint32_t                        m_nextLayerToUpdate = 0;

        /// Mutex that prevents object modification during UpdatePrepare()/UpdateFinalize().
        std::shared_mutex               m_updateMutex;

        /// We double buffer all trees so that we can query while building the next one, and we
        /// destroy the old tree on the next physics update.
        /// This structure ensures that we wait for queries that are still using the old tree.
        mutable std::shared_mutex       m_queryLocks[2];

        /// Index indicates which Query Lock is currently active. It alternates between 0 and 1.
        std::atomic<uint32_t>           m_queryLockIndex {0};
    };
}
