// BroadPhaseQuadTree.cpp
#include "BroadPhaseQuadTree.h"

#include <shared_mutex>
#include "Core/QuickSort.h"
#include "Physics/Body/BodyManager.h"

namespace nes
{
    BroadPhaseQuadTree::~BroadPhaseQuadTree()
    {
        NES_DELETE_ARRAY(m_layers);
    }

    void BroadPhaseQuadTree::Init(BodyManager* pBodyManager, const BroadPhaseLayerInterface& layerInterface)
    {
        BroadPhase::Init(pBodyManager, layerInterface);

        // Store the input parameters
        m_broadPhaseLayerInterface = &layerInterface;
        m_numLayers = layerInterface.GetNumBroadPhaseLayers();
        NES_ASSERT(m_numLayers < static_cast<BroadPhaseLayer::Type>(kInvalidBroadPhaseLayer));

#if NES_ASSERTS_ENABLED
        m_lockContext = pBodyManager;
#endif

        m_maxBodies = m_pBodyManager->GetMaxNumBodies();

        // Initialize Tracking Data
        m_trackers.resize(m_maxBodies);

        // Initialize the Node Allocator
        uint32_t numLeaves = static_cast<uint32_t>(m_maxBodies + 1) / 2; // Assume 50% fill.
        uint32_t numLeavesPlusInternalNodes = numLeaves + (numLeaves + 2) / 3; // Sum(numLeaves * 4^-i) with i = [0, inf]
        m_allocator.Init(2 * numLeavesPlusInternalNodes, 256); // We use double the amount o f odes while rebuilding the tree during Update().

        // Initialize Sub-Trees
        m_layers = NES_NEW_ARRAY(QuadTree, m_numLayers);

        for (uint32_t i = 0; i < m_numLayers; ++i)
        {
            m_layers[i].Init(m_allocator);

            // [TODO]: Set the name of the layer using the BroadPhaseLayerInterface if Debugging.
        }
    }

    void BroadPhaseQuadTree::Optimize()
    {
        FrameSync();
        LockModifications();

        for (uint32_t i = 0; i < m_numLayers; ++i)
        {
            QuadTree& tree = m_layers[i];
            if (tree.HasBodies())
            {
                QuadTree::UpdateState updateState;
                tree.UpdatePrepare(m_pBodyManager->GetBodies(), m_trackers, updateState, true);
                tree.UpdateFinalize(m_pBodyManager->GetBodies(), m_trackers, updateState);
            }
        }

        UnlockModifications();
        m_nextLayerToUpdate = 0;
    }

    void BroadPhaseQuadTree::FrameSync()
    {
        // Take a unique lock on the old query lock so that we know no one is use the old nodes anymore.
        // Note that nothing should be locked at this point ot avoid risking a lock inversion deadlock.
        // Note that in other places where we lock this mutex we don't use shared lock ot detect lock inversions. As long
        // as nothing else is locked this is safe. This is why the BroadphaseQuery should be the highest priority lock.
        UniqueLock rootLock(m_queryLocks[m_queryLockIndex ^ 1] NES_IF_ASSERTS_ENABLED(, m_lockContext, EPhysicsLockTypes::BroadPhaseQuery));

        for (BroadPhaseLayer::Type i = 0; i < static_cast<BroadPhaseLayer::Type>(m_numLayers); ++i)
        {
            m_layers[i].DiscardOldTree();
        }
    }

    void BroadPhaseQuadTree::LockModifications()
    {
        // From this point on, prevent modifications to the tree.
        PhysicsLock::Lock(m_updateMutex NES_IF_ASSERTS_ENABLED(, m_lockContext, EPhysicsLockTypes::BroadPhaseUpdate));
    }

    void BroadPhaseQuadTree::UnlockModifications()
    {
        // From this point on, we allow modifications to the tree again.
        PhysicsLock::Unlock(m_updateMutex NES_IF_ASSERTS_ENABLED(, m_lockContext, EPhysicsLockTypes::BroadPhaseUpdate));
    }

    BroadPhase::UpdateState BroadPhaseQuadTree::UpdatePrepare()
    {
        NES_ASSERT(m_updateMutex.is_locked());

        // Create the update state
        UpdateState updateState;
        UpdateStateImpl* pUpdateStateImpl = reinterpret_cast<UpdateStateImpl*>(&updateState);

        // Loop until we've seen all layers
        for (uint32_t i = 0; i < m_numLayers; ++i)
        {
            QuadTree& tree = m_layers[m_nextLayerToUpdate];
            m_nextLayerToUpdate = (m_nextLayerToUpdate + 1) % m_numLayers;

            // If it is dirty, then we update it and return.
            if (tree.HasBodies() || tree.IsDirty() || tree.CanBeUpdated())
            {
                pUpdateStateImpl->m_pTree = &tree;
                tree.UpdatePrepare(m_pBodyManager->GetBodies(), m_trackers, pUpdateStateImpl->m_updateState, false);
                return updateState;
            }
        }

        // Nothing to update:
        pUpdateStateImpl->m_pTree = nullptr;
        return updateState;
    }

    void BroadPhaseQuadTree::UpdateFinalize(const UpdateState& updateState)
    {
        NES_ASSERT(m_updateMutex.is_locked());

        // Check if a tree has been updated.
        const UpdateStateImpl* pUpdateStateImpl = reinterpret_cast<const UpdateStateImpl*>(&updateState);
        if (pUpdateStateImpl->m_pTree == nullptr)
            return;

        pUpdateStateImpl->m_pTree->UpdateFinalize(m_pBodyManager->GetBodies(), m_trackers, pUpdateStateImpl->m_updateState);

        // Make all queries from now on use the new lock.
        m_queryLockIndex = m_queryLockIndex ^ 1;
    }

    BroadPhase::AddState BroadPhaseQuadTree::AddBodiesPrepare(BodyID* pBodies, int number)
    {
        if (number <= 0)
            return nullptr;

        const BodyVector& bodies = m_pBodyManager->GetBodies();
        NES_ASSERT(m_maxBodies == m_pBodyManager->GetMaxNumBodies());

        // 'New' is overriden in layer state.
        LayerState* pLayerStates = new LayerState[m_numLayers];

        // Sort the bodies by BroadPhaseLayer.
        const Body* const* pBodiesPtr = bodies.data();
        QuickSort(pBodies, pBodies + number, [pBodiesPtr](BodyID left, BodyID right)
        {
            return pBodiesPtr[left.GetIndex()]->GetBroadPhaseLayer() < pBodiesPtr[right.GetIndex()]->GetBroadPhaseLayer();
        });

        // Insert the Bodies into the appropriate layer:
        BodyID* pStart = pBodies;
        BodyID* pEnd = pBodies + number;
        while (pStart < pEnd)
        {
            // Get the broadphase layer that the body is in.
            const BroadPhaseLayer::Type broadPhaseLayer = static_cast<BroadPhaseLayer::Type>(pBodiesPtr[pStart->GetIndex()]->GetBroadPhaseLayer());
            NES_ASSERT(broadPhaseLayer < m_numLayers);

            // Get the first body with a different layer
            BodyID* pMid = std::upper_bound(pStart, pEnd, broadPhaseLayer, [pBodiesPtr](BroadPhaseLayer::Type layer, BodyID bodyID)
            {
                return layer < static_cast<BroadPhaseLayer::Type>(pBodiesPtr[bodyID.GetIndex()]->GetBroadPhaseLayer());
            });

            // Keep track of the state for this layer.
            LayerState& layerState = pLayerStates[broadPhaseLayer];
            layerState.m_pBodyStart = pStart;
            layerState.m_pBodyEnd = pEnd;

            // Insert all Bodies of the same layer
            m_layers[broadPhaseLayer].AddBodiesPrepare(bodies, m_trackers, pStart, static_cast<int>(pMid - pStart), layerState.m_addState);

            // Keep track in which tree we placed the object:
            for (const BodyID* pBodyID = pStart; pBodyID < pMid; ++pBodyID)
            {
                const uint32_t index = pBodyID->GetIndex();
                NES_ASSERT(bodies[index]->GetID() == *pBodyID);
                NES_ASSERT(!bodies[index]->IsInBroadPhase());

                // Update the Tracker info.
                // At this point, the data should be invalid.
                Tracker& tracker = m_trackers[index];
                NES_ASSERT(tracker.m_broadPhaseLayer == static_cast<BroadPhaseLayer::Type>(kInvalidBroadPhaseLayer));
                tracker.m_broadPhaseLayer = broadPhaseLayer;
                NES_ASSERT(tracker.m_collisionLayer == kInvalidCollisionLayer);
                tracker.m_collisionLayer = bodies[index]->GetCollisionLayer();
            }

            // Repeat for the next layer:
            pStart = pMid;
        }

        return pLayerStates;
    }

    void BroadPhaseQuadTree::AddBodiesFinalize([[maybe_unused]] BodyID* pBodies, const int number, AddState addState)
    {
        if (number <= 0)
        {
            NES_ASSERT(addState == nullptr);
            return;
        }

        // This cannot run concurrently with UpdatePrepare()/UpdateFinalize().
        SharedLock lock(m_updateMutex NES_IF_ASSERTS_ENABLED(, m_lockContext, EPhysicsLockTypes::BroadPhaseUpdate));

        BodyVector& bodies = m_pBodyManager->GetBodies();
        NES_ASSERT(m_maxBodies == m_pBodyManager->GetMaxNumBodies());

        LayerState* pLayerStates = static_cast<LayerState*>(addState);
        NES_ASSERT(pLayerStates != nullptr);

        for (BroadPhaseLayer::Type broadPhaseLayer = 0; broadPhaseLayer < static_cast<BroadPhaseLayer::Type>(m_numLayers); ++broadPhaseLayer)
        {
            const LayerState& layerState = pLayerStates[broadPhaseLayer];
            if (layerState.m_pBodyStart != nullptr)
            {
                // Insert all bodies of the same layer:
                m_layers[broadPhaseLayer].AddBodiesFinalize(m_trackers, static_cast<int>(layerState.m_pBodyEnd - layerState.m_pBodyStart), layerState.m_addState);

                // Mark Added to the Broadphase
                for (const BodyID* pBodyID = layerState.m_pBodyStart; pBodyID < layerState.m_pBodyEnd; ++pBodyID)
                {
                    const uint32_t index = pBodyID->GetIndex();
                    NES_ASSERT(bodies[index]->GetID() == *pBodyID);
                    NES_ASSERT(m_trackers[index].m_broadPhaseLayer == broadPhaseLayer);
                    NES_ASSERT(m_trackers[index].m_collisionLayer == bodies[index]->GetCollisionLayer());
                    NES_ASSERT(!bodies[index]->IsInBroadPhase()); // They shouldn't have this flag set yet.
                    bodies[index]->Internal_SetInBroadPhase(true);
                }
            }
        }

        // Clean up the Layer States:
        delete [] pLayerStates;
    }

    void BroadPhaseQuadTree::AddBodiesAbort([[maybe_unused]] BodyID* pBodies, const int number, AddState addState)
    {
        if (number <= 0)
        {
            NES_ASSERT(addState == nullptr);
            return;
        }

        NES_IF_ASSERTS_ENABLED(const BodyVector& bodies = m_pBodyManager->GetBodies());
        NES_ASSERT(m_maxBodies == m_pBodyManager->GetMaxNumBodies());

        LayerState* pLayerStates = static_cast<LayerState*>(addState);
        NES_ASSERT(pLayerStates != nullptr);

        for (BroadPhaseLayer::Type broadPhaseLayer = 0; broadPhaseLayer < static_cast<BroadPhaseLayer::Type>(m_numLayers); ++broadPhaseLayer)
        {
            const LayerState& layerState = pLayerStates[broadPhaseLayer];
            if (layerState.m_pBodyStart != nullptr)
            {
                // Abort the add operation on the Layer:
                m_layers[broadPhaseLayer].AddBodiesAbort(m_trackers, layerState.m_addState);

                // Reset the tracking info for each Body:
                for (const BodyID* pBodyID = layerState.m_pBodyStart; pBodyID < layerState.m_pBodyEnd; ++pBodyID)
                {
                    const uint32_t index = pBodyID->GetIndex();
                    NES_ASSERT(bodies[index]->GetID() == *pBodyID);
                    NES_ASSERT(!bodies[index]->IsInBroadPhase()); // They shouldn't have this flag set yet.

                    // Reset the Tracker:
                    Tracker& tracker = m_trackers[index];
                    NES_ASSERT(tracker.m_broadPhaseLayer == broadPhaseLayer);
                    tracker.m_broadPhaseLayer = static_cast<BroadPhaseLayer::Type>(kInvalidBroadPhaseLayer);
                    tracker.m_collisionLayer = kInvalidCollisionLayer;
                }
            }
        }

        // Clean up the Layer States:
        delete [] pLayerStates;
    }

    void BroadPhaseQuadTree::RemoveBodies(BodyID* pBodies, int number)
    {
        if (number <= 0)
            return;

        // This cannot run concurrently with UpdatePrepare()/UpdateFinalize().
        SharedLock lock(m_updateMutex NES_IF_ASSERTS_ENABLED(, m_lockContext, EPhysicsLockTypes::BroadPhaseUpdate));

        const BodyVector& bodies = m_pBodyManager->GetBodies();
        NES_ASSERT(m_maxBodies == m_pBodyManager->GetMaxNumBodies());
        
        // Sort the bodies by BroadPhaseLayer.
        const Body* const* pBodiesPtr = bodies.data();
        QuickSort(pBodies, pBodies + number, [pBodiesPtr](BodyID left, BodyID right)
        {
            return pBodiesPtr[left.GetIndex()]->GetBroadPhaseLayer() < pBodiesPtr[right.GetIndex()]->GetBroadPhaseLayer();
        });

        // Remove the Bodies from the appropriate layer:
        BodyID* pStart = pBodies;
        BodyID* pEnd = pBodies + number;
        while (pStart < pEnd)
        {
            // Get the broadphase layer that the body is in.
            const BroadPhaseLayer::Type broadPhaseLayer = static_cast<BroadPhaseLayer::Type>(pBodiesPtr[pStart->GetIndex()]->GetBroadPhaseLayer());
            NES_ASSERT(broadPhaseLayer < m_numLayers);

            // Get the first body with a different layer
            BodyID* pMid = std::upper_bound(pStart, pEnd, broadPhaseLayer, [pBodiesPtr](BroadPhaseLayer::Type layer, BodyID bodyID)
            {
                return layer < static_cast<BroadPhaseLayer::Type>(pBodiesPtr[bodyID.GetIndex()]->GetBroadPhaseLayer());
            });

            m_layers[broadPhaseLayer].RemoveBodies(bodies, m_trackers, pStart, static_cast<int>(pMid - pStart));

            // Reset our tracking information
            for (const BodyID* pBodyID = pStart; pBodyID < pMid; ++pBodyID)
            {
                const uint32_t index = pBodyID->GetIndex();

                // Reset the tracker info:
                Tracker& tracker = m_trackers[index];
                tracker.m_broadPhaseLayer = static_cast<BroadPhaseLayer::Type>(kInvalidBroadPhaseLayer);
                tracker.m_collisionLayer = kInvalidCollisionLayer;

                // Mark removed from the BroadPhase
                NES_ASSERT(bodies[index]->IsInBroadPhase());
                bodies[index]->Internal_SetInBroadPhase(false);
            }

            // Repeat for the next layer:
            pStart = pMid;
        }
    }

    void BroadPhaseQuadTree::NotifyBodiesAABBChanged(BodyID* pBodies, int number, bool takeLock)
    {
        if (number <= 0)
            return;

        // This cannot run concurrently with UpdatePrepare()/UpdateFinalize().
        if (takeLock)
            PhysicsLock::LockShared(m_updateMutex NES_IF_ASSERTS_ENABLED(, m_lockContext, EPhysicsLockTypes::BroadPhaseUpdate));
        else
            NES_ASSERT(m_updateMutex.is_locked());

        const BodyVector& bodies = m_pBodyManager->GetBodies();
        NES_ASSERT(m_maxBodies == m_pBodyManager->GetMaxNumBodies());

        const Tracker* pTrackers = m_trackers.data();
        QuickSort(pBodies, pBodies + number, [pTrackers](BodyID left, BodyID right)
        {
            return pTrackers[left.GetIndex()].m_broadPhaseLayer < pTrackers[right.GetIndex()].m_broadPhaseLayer;
        });

        BodyID* pStart = pBodies;
        BodyID* pEnd = pBodies + number;
        while (pStart < pEnd)
        {
            // Get the broadphase layer that the body is in.
            const BroadPhaseLayer::Type broadPhaseLayer = pTrackers[pStart->GetIndex()].m_broadPhaseLayer;
            NES_ASSERT(broadPhaseLayer < m_numLayers);

            // Get the first body with a different layer
            BodyID* pMid = std::upper_bound(pStart, pEnd, broadPhaseLayer, [pTrackers](BroadPhaseLayer::Type layer, BodyID bodyID)
            {
                return layer < static_cast<BroadPhaseLayer::Type>(pTrackers[bodyID.GetIndex()].m_broadPhaseLayer);
            });

            m_layers[broadPhaseLayer].NotifyBodiesAABBChanged(bodies, m_trackers, pStart, static_cast<int>(pMid - pStart));

            // Repeat for the next layer
            pStart = pMid;
        }

        // Unlock if necessary:
        if (takeLock)
            PhysicsLock::UnlockShared(m_updateMutex NES_IF_ASSERTS_ENABLED(, m_lockContext, EPhysicsLockTypes::BroadPhaseUpdate));
    }

    void BroadPhaseQuadTree::NotifyBodiesLayerChanged(BodyID* pBodies, int number)
    {
        if (number <= 0)
            return;

        // First sort the bodies that actually changed layer to the beginning of the array.
        const BodyVector& bodies = m_pBodyManager->GetBodies();
        NES_ASSERT(m_maxBodies == m_pBodyManager->GetMaxNumBodies());

        for (BodyID* pBodyID = pBodies + number - 1; pBodyID >= pBodies; --pBodyID)
        {
            const uint32_t index = pBodyID->GetIndex();
            NES_ASSERT(bodies[index]->GetID() == *pBodyID);
            
            const Body* pBody = bodies[index];
            BroadPhaseLayer::Type broadphaseLayer = static_cast<BroadPhaseLayer::Type>(pBody->GetBroadPhaseLayer());
            NES_ASSERT(broadphaseLayer < m_numLayers);

            // If this body didn't actually change, then swap to the end
            // and reduce our count.
            if (m_trackers[index].m_broadPhaseLayer == broadphaseLayer)
            {
                // Update the tracking information:
                m_trackers[index].m_collisionLayer = pBody->GetCollisionLayer();

                // Swap to the end, the layer didn't change:
                std::swap(*pBodyID, pBodies[number - 1]);
                --number;
            }
        }
        
        if (number > 0)
        {
            // Changing the layer requires us to remove form one tree and add to another, so this is
            // equivalent to removing all bodies first then adding them again.
            RemoveBodies(pBodies, number);
            const AddState state = AddBodiesPrepare(pBodies, number);
            AddBodiesFinalize(pBodies, number, state);
        }
    }

    void BroadPhaseQuadTree::CastRay(const RayCast& ray, RayCastBodyCollector& collector,
        const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter) const
    {
        NES_ASSERT(m_maxBodies == m_pBodyManager->GetMaxNumBodies());

        // Prevent this from running in parallel with node deletion in FrameSync() - see notes there.
        std::shared_lock lock(m_queryLocks[m_queryLockIndex]);

        // Loop over all layers and test the ones that could hit.
        for (BroadPhaseLayer::Type i = 0; i < static_cast<BroadPhaseLayer::Type>(m_numLayers); ++i)
        {
            const QuadTree& tree = m_layers[i];
            if (tree.HasBodies() && broadPhaseLayerFilter.ShouldCollide(BroadPhaseLayer(i)))
            {
                tree.CastRay(ray, collector, collisionLayerFilter, m_trackers);
                if (collector.ShouldEarlyOut())
                    break;
            }
        }
    }

    void BroadPhaseQuadTree::CastAABox(const AABoxCast& box, CastShapeBodyCollector& collector,
        const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter) const
    {
        // Prevent this from running in parallel with node deletion in FrameSync() - see notes there.
        std::shared_lock lock(m_queryLocks[m_queryLockIndex]);
        
        CastAABoxNoLock(box, collector, broadPhaseLayerFilter, collisionLayerFilter);
    }

    void BroadPhaseQuadTree::CastAABoxNoLock(const AABoxCast& box, CastShapeBodyCollector& collector,
        const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter) const
    {
        NES_ASSERT(m_maxBodies == m_pBodyManager->GetMaxNumBodies());
        
        // Loop over all layers and test the ones that could hit.
        for (BroadPhaseLayer::Type i = 0; i < static_cast<BroadPhaseLayer::Type>(m_numLayers); ++i)
        {
            const QuadTree& tree = m_layers[i];
            if (tree.HasBodies() && broadPhaseLayerFilter.ShouldCollide(BroadPhaseLayer(i)))
            {
                tree.CastAABox(box, collector, collisionLayerFilter, m_trackers);
                if (collector.ShouldEarlyOut())
                    break;
            }
        }
    }

    void BroadPhaseQuadTree::CollideAABox(const AABox& box, CollideShapeBodyCollector& collector,
                                          const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter) const
    {
        NES_ASSERT(m_maxBodies == m_pBodyManager->GetMaxNumBodies());

        // Prevent this from running in parallel with node deletion in FrameSync() - see notes there.
        std::shared_lock lock(m_queryLocks[m_queryLockIndex]);

        // Loop over all layers and test the ones that could hit.
        for (BroadPhaseLayer::Type i = 0; i < static_cast<BroadPhaseLayer::Type>(m_numLayers); ++i)
        {
            const QuadTree& tree = m_layers[i];
            if (tree.HasBodies() && broadPhaseLayerFilter.ShouldCollide(BroadPhaseLayer(i)))
            {
                tree.CollideAABox(box, collector, collisionLayerFilter, m_trackers);
                if (collector.ShouldEarlyOut())
                    break;
            }
        }
    }

    void BroadPhaseQuadTree::CollideSphere(const Vec3& center, const float radius,
        CollideShapeBodyCollector& collector, const BroadPhaseLayerFilter& broadPhaseLayerFilter,
        const CollisionLayerFilter& collisionLayerFilter) const
    {
        NES_ASSERT(m_maxBodies == m_pBodyManager->GetMaxNumBodies());

        // Prevent this from running in parallel with node deletion in FrameSync() - see notes there.
        std::shared_lock lock(m_queryLocks[m_queryLockIndex]);

        // Loop over all layers and test the ones that could hit.
        for (BroadPhaseLayer::Type i = 0; i < static_cast<BroadPhaseLayer::Type>(m_numLayers); ++i)
        {
            const QuadTree& tree = m_layers[i];
            if (tree.HasBodies() && broadPhaseLayerFilter.ShouldCollide(BroadPhaseLayer(i)))
            {
                tree.CollideSphere(center, radius, collector, collisionLayerFilter, m_trackers);
                if (collector.ShouldEarlyOut())
                    break;
            }
        }
    }

    void BroadPhaseQuadTree::CollidePoint(const Vec3& point, CollideShapeBodyCollector& collector,
        const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter) const
    {
        NES_ASSERT(m_maxBodies == m_pBodyManager->GetMaxNumBodies());

        // Prevent this from running in parallel with node deletion in FrameSync() - see notes there.
        std::shared_lock lock(m_queryLocks[m_queryLockIndex]);

        // Loop over all layers and test the ones that could hit.
        for (BroadPhaseLayer::Type i = 0; i < static_cast<BroadPhaseLayer::Type>(m_numLayers); ++i)
        {
            const QuadTree& tree = m_layers[i];
            if (tree.HasBodies() && broadPhaseLayerFilter.ShouldCollide(BroadPhaseLayer(i)))
            {
                tree.CollidePoint(point, collector, collisionLayerFilter, m_trackers);
                if (collector.ShouldEarlyOut())
                    break;
            }
        }
    }

    void BroadPhaseQuadTree::CollideOrientedBox(const OrientedBox& box, CollideShapeBodyCollector& collector,
        const BroadPhaseLayerFilter& broadPhaseLayerFilter, const CollisionLayerFilter& collisionLayerFilter) const
    {
        NES_ASSERT(m_maxBodies == m_pBodyManager->GetMaxNumBodies());

        // Prevent this from running in parallel with node deletion in FrameSync() - see notes there.
        std::shared_lock lock(m_queryLocks[m_queryLockIndex]);

        // Loop over all layers and test the ones that could hit.
        for (BroadPhaseLayer::Type i = 0; i < static_cast<BroadPhaseLayer::Type>(m_numLayers); ++i)
        {
            const QuadTree& tree = m_layers[i];
            if (tree.HasBodies() && broadPhaseLayerFilter.ShouldCollide(BroadPhaseLayer(i)))
            {
                tree.CollideOrientedBox(box, collector, collisionLayerFilter, m_trackers);
                if (collector.ShouldEarlyOut())
                    break;
            }
        }
    }

    void BroadPhaseQuadTree::FindCollidingPairs(BodyID* pActiveBodies, const int numActiveBodies,
        float speculativeContactDistance, const CollisionVsBroadPhaseLayerFilter& collisionVsBroadPhaseLayerFilter,
        const CollisionLayerPairFilter& collisionLayerPairFilter, BodyPairCollector& pairCollector) const
    {
        const BodyVector& bodies = m_pBodyManager->GetBodies();
        NES_ASSERT(m_maxBodies == m_pBodyManager->GetMaxNumBodies());

        // Note that we don't take any locks at this point. We know that the tree is not going to be swapped or deleted
        // while finding collision pairs due to the way the jobs are scheduled in PhysicsScene::Update.

        // Sort the Bodies based on CollisionLayer
        const Tracker* pTrackers = m_trackers.data();
        QuickSort(pActiveBodies, pActiveBodies + numActiveBodies, [pTrackers](BodyID left, BodyID right)
        {
            return pTrackers[left.GetIndex()].m_collisionLayer < pTrackers[right.GetIndex()].m_collisionLayer;
        });

        BodyID* pStart = pActiveBodies;
        BodyID* pEnd = pActiveBodies + numActiveBodies;
        while (pStart < pEnd)
        {
            // Get the Collision Layer:
            const CollisionLayer collisionLayer = pTrackers[pStart->GetIndex()].m_collisionLayer;
            NES_ASSERT(collisionLayer != kInvalidCollisionLayer);

            // Find the first Body with a different layer:
            BodyID* pMid = std::upper_bound(pStart, pEnd, collisionLayer, [pTrackers](CollisionLayer layer, BodyID bodyID)
            {
                return layer < static_cast<BroadPhaseLayer::Type>(pTrackers[bodyID.GetIndex()].m_collisionLayer);
            });

            // Loop over all broadphase layers and test the ones that we could hit:
            for (BroadPhaseLayer::Type i = 0; i < static_cast<BroadPhaseLayer::Type>(m_numLayers); ++i)
            {
                const QuadTree& tree = m_layers[i];
                if (tree.HasBodies() && collisionVsBroadPhaseLayerFilter.ShouldCollide(collisionLayer, BroadPhaseLayer(i)))
                {
                    tree.FindCollidingPairs(bodies, pStart, static_cast<int>(pMid - pStart), speculativeContactDistance, pairCollector, collisionLayerPairFilter);
                }
            }

            // Repeat for the next Collision Layer
            pStart = pMid;
        }
    }

    AABox BroadPhaseQuadTree::GetBounds() const
    {
        // Prevent this from running in parallel with node deletion in FrameSync(), see notes there.
        std::shared_lock lock(m_queryLocks[m_queryLockIndex]);

        AABox bounds;
        for (BroadPhaseLayer::Type i = 0; i < static_cast<BroadPhaseLayer::Type>(m_numLayers); ++i)
        {
            bounds.Encapsulate(m_layers[i].GetBounds());
        }
        return bounds;
    }
}
