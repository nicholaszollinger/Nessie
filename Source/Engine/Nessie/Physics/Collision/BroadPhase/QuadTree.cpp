// QuadTree.cpp
#include "QuadTree.h"

#include "Nessie/Core/Memory/STLLocalAllocator.h"
#include "Nessie/Geometry/AABoxSIMD.h"
#include "Nessie/Geometry/OrientedBox.h"
#include "Nessie/Geometry/RayAABox.h"
#include "Nessie/Physics/Collision/RayCast.h"
#include "Nessie/Physics/Body/BodyPair.h"
#include "Nessie/Physics/Collision/AABoxCast.h"
#include "Nessie/Physics/Collision/SortReverseAndStore.h"
#include "Nessie/Physics/Collision/CastResult.h"
#include "Nessie/Physics/PhysicsLock.h"

namespace nes
{
    QuadTree::Node::Node(const bool isChanged)
        : m_isChanged(isChanged)
    {
        // Initialize the Node bounds to have the min and max positions
        // switched, ensuring that no collision can occur with this Node.
        Vec4Reg val = Vec4::Replicate(math::kLargeFloat);
        val.StoreFloat4(reinterpret_cast<Float4*>(&m_minX));
        val.StoreFloat4(reinterpret_cast<Float4*>(&m_minY));
        val.StoreFloat4(reinterpret_cast<Float4*>(&m_minZ));
        
        val = Vec4Reg::Replicate(-math::kLargeFloat);
        val.StoreFloat4(reinterpret_cast<Float4*>(&m_maxX));
        val.StoreFloat4(reinterpret_cast<Float4*>(&m_maxY));
        val.StoreFloat4(reinterpret_cast<Float4*>(&m_maxZ));
        
        // Reset child NodeIDs.
        m_childNodeIDs[0] = NodeID::InvalidID();
        m_childNodeIDs[1] = NodeID::InvalidID();
        m_childNodeIDs[2] = NodeID::InvalidID();
        m_childNodeIDs[3] = NodeID::InvalidID();
    }

    void QuadTree::Node::GetNodeBounds(AABox& outBounds) const
    {
        // Get the first child bounds.
        GetChildBounds(0, outBounds);

        // Grow to encapsulate the other children.
        for (int i = 1; i < 4; ++i)
        {
            AABox temp;
            GetChildBounds(i, temp);
            outBounds.Encapsulate(temp);
        }
    }

    void QuadTree::Node::GetChildBounds(const int childIndex, AABox& outBounds) const
    {
        NES_ASSERT(childIndex >= 0 && childIndex < 4);

        outBounds.m_min = Vec3(m_minX[childIndex], m_minY[childIndex], m_minZ[childIndex]);
        outBounds.m_max = Vec3(m_maxX[childIndex], m_maxY[childIndex], m_maxZ[childIndex]);
    }

    void QuadTree::Node::SetChildBounds(const int childIndex, const AABox& bounds)
    {
        NES_ASSERT(childIndex >= 0 && childIndex < 4);
        NES_ASSERT(bounds.IsValid());

        // Set max first (this keeps the bounding box invalid for reading threads)
        m_maxZ[childIndex] = bounds.m_max.z;
        m_maxY[childIndex] = bounds.m_max.y;
        m_maxX[childIndex] = bounds.m_max.x;

        // Then set min (which will make the box valid).
        m_minZ[childIndex] = bounds.m_min.z;
        m_minY[childIndex] = bounds.m_min.y;
        m_minX[childIndex] = bounds.m_min.x;
    }

    void QuadTree::Node::InvalidateChildBounds(const int childIndex)
    {
        NES_ASSERT(childIndex >= 0 && childIndex < 4);
        
        // First we make the box invalid by setting the min to cLargeFloat
        m_minX[childIndex] = math::kLargeFloat; // Min X becomes invalid first
        m_minY[childIndex] = math::kLargeFloat;
        m_minZ[childIndex] = math::kLargeFloat;

        // Then we reset the max values too
        m_maxX[childIndex] = -math::kLargeFloat;
        m_maxY[childIndex] = -math::kLargeFloat;
        m_maxZ[childIndex] = -math::kLargeFloat;
    }

    bool QuadTree::Node::EncapsulateChildBounds(int childIndex, const AABox& bounds)
    {
        NES_ASSERT(childIndex >= 0 && childIndex < 4);
        
        bool wasChanged = AtomicMin(m_minX[childIndex], bounds.m_min.x);
        wasChanged |= AtomicMin(m_minY[childIndex], bounds.m_min.y);
        wasChanged |= AtomicMin(m_minZ[childIndex], bounds.m_min.z);
        wasChanged |= AtomicMax(m_maxX[childIndex], bounds.m_max.x);
        wasChanged |= AtomicMax(m_maxY[childIndex], bounds.m_max.y);
        wasChanged |= AtomicMax(m_maxZ[childIndex], bounds.m_max.z);
        return wasChanged;
    }

    QuadTree::BodyTracker::BodyTracker(const BodyTracker& other)
        : m_broadPhaseLayer(other.m_broadPhaseLayer.load())
        , m_collisionLayer(other.m_collisionLayer.load())
        , m_bodyLocation(other.m_bodyLocation.load())
    {
        //
    }

    const AABox QuadTree::kInvalidBounds(Vec3(math::kLargeFloat), Vec3(-math::kLargeFloat));

    static inline void QuadTreePerformanceWarning()
    {
    #if NES_ASSERTS_ENABLED
        static std::atomic<bool> triggeredReport { false };
        bool expected = false;
        if (triggeredReport.compare_exchange_strong(expected, true))
        {
            NES_WARN("QuadTree: Performance warning: Stack full!\n"
                "This must be a very deep tree. Are you batch adding bodies through BodyInterface::AddBodiesPrepare/AddBodiesFinalize?\n"
                "If you add lots of bodies through BodyInterface::AddBody you may need to call PhysicsScene::OptimizeBroadPhase to rebuild the tree.");
        }
    #endif
    }

    QuadTree::~QuadTree()
    {
        DiscardOldTree();

        const RootNode& rootNode = GetCurrentRoot();

        // Collect all Nodes:
        Allocator::Batch freeBatch;
        std::vector<NodeID, STLLocalAllocator<NodeID, kStackSize>> nodeStack;
        nodeStack.reserve(kStackSize);
        nodeStack.push_back(rootNode.GetNodeID());
        NES_ASSERT(nodeStack.front().IsValid());
        if (nodeStack.front().IsNode())
        {
            do
            {
                NodeID nodeID = nodeStack.back();
                nodeStack.pop_back();
                NES_ASSERT(!nodeID.IsBody());

                const uint32 nodeIndex = nodeID.GetNodeIndex();
                const Node& node = m_pAllocator->Get(nodeIndex);

                // Recurse and get all child nodes:
                for (NodeID childNodeID : node.m_childNodeIDs)
                {
                    if (childNodeID.IsValid() && childNodeID.IsNode())
                    {
                        nodeStack.push_back(childNodeID);
                    }
                }

                // Mark Node to be Freed
                m_pAllocator->AddObjectToBatch(freeBatch, nodeIndex);
            }
            while (!nodeStack.empty());
        }

        // Free all the Nodes:
        m_pAllocator->DestructBatch(freeBatch);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Initialize the Quadtree. 
    //----------------------------------------------------------------------------------------------------
    void QuadTree::Init(Allocator& allocator)
    {
        m_pAllocator = &allocator;
        m_rootNodes[m_rootNodeIndex].m_index = AllocateNode(false);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Throws away the previous frame's Nodes so that a new Tree can be built in the background. 
    //----------------------------------------------------------------------------------------------------
    void QuadTree::DiscardOldTree()
    {
        // Check if there is an old tree:
        RootNode& oldRoot = m_rootNodes[m_rootNodeIndex ^ 1];
        if (oldRoot.m_index != kInvalidNodeIndex)
        {
            // Clear the Root:
            oldRoot.m_index = kInvalidNodeIndex;
            
            // Free all old Nodes:
            m_pAllocator->DestructBatch(m_freeNodeBatch);
            
            // Clear the Batch:
            m_freeNodeBatch = Allocator::Batch();
        }
    }

    void QuadTree::UpdatePrepare(const BodyVector& bodies, BodyTrackerArray& outTrackers, UpdateState& outState, const bool doFullRebuild)
    {
#if NES_ASSERTS_ENABLED
        // We only read positions.
        BodyAccess::GrantScope scope (BodyAccess::EAccess::None, BodyAccess::EAccess::Read);
#endif
        
        // Assert we have no nodes pending deletion, this means DiscardOldTree wasn't called yet
        NES_ASSERT(m_freeNodeBatch.m_numObjects == 0);
        m_isDirty = false;

        const RootNode& rootNode = GetCurrentRoot();
        
#ifdef NES_DEBUG
        ValidateTree(bodies, outTrackers, rootNode.m_index, m_numBodies);
#endif

        // Create space for all BodyIDs.
        NodeID* pNodeIDs = new NodeID[m_numBodies];
        NodeID* pCurrentNodeID = pNodeIDs;

        // Collect all Bodies
        std::vector<NodeID, STLLocalAllocator<NodeID, kStackSize>> nodeStack;
        nodeStack.reserve(kStackSize);
        nodeStack.push_back(rootNode.GetNodeID());
        NES_ASSERT(nodeStack.front().IsValid());
        do
        {
            // Pop node from the stack.
            NodeID nodeID = nodeStack.back();
            nodeStack.pop_back();

            // Check if the node is a body:
            if (nodeID.IsBody())
            {
                // Validate that we're still in the right layer.
            #if NES_ASSERTS_ENABLED
                const uint32 bodyIndex = nodeID.GetBodyID().GetIndex();
                NES_ASSERT(outTrackers[bodyIndex].m_collisionLayer == bodies[bodyIndex]->GetCollisionLayer());
            #endif

                // Store the Body
                *pCurrentNodeID = nodeID;
                ++pCurrentNodeID;
            }
            else
            {
                // Process normal Node.
                const uint32 nodeIndex = nodeID.GetNodeIndex();
                const Node& node = m_pAllocator->Get(nodeIndex);

                if (!node.m_isChanged && !doFullRebuild)
                {
                    // Node is unchanged, treat it as a whole.
                    *pCurrentNodeID = nodeID;
                    ++pCurrentNodeID;
                }
                else
                {
                    // Node is changed, recurse and get all children
                    for (NodeID childNodeID : node.m_childNodeIDs)
                    {
                        if (childNodeID.IsValid())
                            nodeStack.push_back(childNodeID);
                    }

                    m_pAllocator->AddObjectToBatch(m_freeNodeBatch, nodeIndex);
                }
            }
        } while (!nodeStack.empty());

        // Check that our bookkeeping matches.
        const uint32 numNodeIDs = static_cast<uint32>(pCurrentNodeID - pNodeIDs);
        NES_ASSERT(doFullRebuild? numNodeIDs == m_numBodies : numNodeIDs <= m_numBodies);

        // This will be the new Root NodeID.
        NodeID rootNodeID;
        if (numNodeIDs > 0)
        {
            // We mark the first 5 levels (max 1024 nodes) of the newly built tree as 'changed' so that
            // those nodes get recreated every time when we rebuild the tree. This balances the amount of
            // time we spend on rebuilding the tree ('unchanged' nodes will be put in the new tree as a whole)
            // vs the quality of the built tree.
            constexpr uint kMaxDepthMarkChanged = 5;

            // Build the new Tree:
            AABox rootBounds;
            rootNodeID = BuildTree(bodies, outTrackers, pNodeIDs, static_cast<int>(numNodeIDs), kMaxDepthMarkChanged, rootBounds);

            // For a single Body, we allocate a new Root Node.
            if (rootNodeID.IsBody())
            {
                const uint32 rootIndex = AllocateNode(false);
                Node& root = m_pAllocator->Get(rootIndex);
                root.SetChildBounds(0, rootBounds);
                root.m_childNodeIDs[0] = rootNodeID;
                SetBodyLocation(outTrackers, rootNodeID.GetBodyID(), rootIndex, 0);
                rootNodeID = NodeID::FromNodeIndex(rootIndex);
            }
        }
        else
        {
            // Empty tree, create the Root Node
            const uint32 rootIndex = AllocateNode(false);
            rootNodeID = NodeID::FromNodeIndex(rootIndex);
        }
        
        delete[] pNodeIDs;
        outState.m_rootNodeID = rootNodeID;
    }

    void QuadTree::UpdateFinalize([[maybe_unused]] const BodyVector& bodies, [[maybe_unused]] const BodyTrackerArray& trackers, const UpdateState& state)
    {
        // Tree Building is complete, now we switch the old with the new tree.
        uint32 newRootIndex = m_rootNodeIndex ^ 1;
        RootNode& newRootNode = m_rootNodes[newRootIndex];
        {
            // Note: We don't need to lock here as the old tree stays available so any queries
            // that use it can continue using it until DiscardOldTree is called. This slot
            // should be empty and unused at this moment.
            NES_ASSERT(newRootNode.m_index == kInvalidNodeIndex);
            newRootNode.m_index = state.m_rootNodeID.GetNodeIndex(); 
        }

        // All queries that start from now on will use this new tree
        m_rootNodeIndex = newRootIndex;

#if NES_LOGGING_ENABLED
        ValidateTree(bodies, trackers, newRootNode.m_index, m_numBodies);
#endif
    }

    void QuadTree::AddBodiesPrepare(const BodyVector& bodies, BodyTrackerArray& trackers, BodyID* bodyIDArray,
        const int number, AddState& outState)
    {
        // Assert sane input
        NES_ASSERT(bodyIDArray != nullptr);
        NES_ASSERT(number > 0);

#if NES_LOGGING_ENABLED
        // Check that casting to NodeID is valid for every BodyID.
        for (const BodyID* pCurrentBodyID = bodyIDArray, * pBodyIDEnd = bodyIDArray + number; pCurrentBodyID < pBodyIDEnd; ++pCurrentBodyID)
        {
            NodeID::FromBodyID(*pCurrentBodyID);
        }
#endif

        // Build a subtree for the new Bodies. Note that we make all nodes as 'not changed'
        // so that they will stay together as a batch and will make the tree rebuild cheaper.
        outState.m_leafID = BuildTree(bodies, trackers, reinterpret_cast<NodeID*>(bodyIDArray), number, 0, outState.m_leafBounds);

#ifdef NES_DEBUG
        if (outState.m_leafID.IsNode())
            ValidateTree(bodies, trackers, outState.m_leafID.GetNodeIndex(), number);
#endif
    }

    void QuadTree::AddBodiesFinalize(BodyTrackerArray& trackers, int numBodies, const AddState& state)
    {
        NES_ASSERT(numBodies > 0);

        // Mark the tree dirty:
        m_isDirty = true;

        RootNode& rootNode = GetCurrentRoot();

        for (;;)
        {
            // Check if we can insert the body in the root:
            if (TryInsertLeaf(trackers, static_cast<int>(rootNode.m_index), state.m_leafID, state.m_leafBounds, numBodies))
                return;

            // Check if we can create a new root:
            if (TryCreateNewRoot(trackers, rootNode.m_index, state.m_leafID, state.m_leafBounds, numBodies))
                return;
        }
    }

    void QuadTree::AddBodiesAbort(BodyTrackerArray& trackers, const AddState& state)
    {
        // Collect all bodies:
        Allocator::Batch freeBatch;
        NodeID nodeStack[kStackSize];
        nodeStack[0] = state.m_leafID;
        NES_ASSERT(nodeStack[0].IsValid());
        int top = 0;
        do
        {
            const NodeID childNodeID = nodeStack[top];
            if (childNodeID.IsBody())
            {
                // Reset the location of a Body:
                InvalidateBodyLocation(trackers, childNodeID.GetBodyID());
            }
            else
            {
                // Process normal node
                const uint32 nodeIndex = childNodeID.GetNodeIndex();
                const Node& node = m_pAllocator->Get(nodeIndex);
                for (NodeID subChildNodeID : node.m_childNodeIDs)
                {
                    if (subChildNodeID.IsValid())
                    {
                        NES_ASSERT(top < kStackSize);
                        nodeStack[top] = subChildNodeID;
                        ++top;
                    }
                }

                m_pAllocator->AddObjectToBatch(freeBatch, nodeIndex);
            }
            
            --top;
        }
        while (top >= 0);

        // Now free all nodes in a single batch.
        m_pAllocator->DestructBatch(freeBatch);
    }

    void QuadTree::RemoveBodies([[maybe_unused]] const BodyVector& bodies, BodyTrackerArray& trackers, const BodyID* bodyIDArray,
        const int number)
    {
        NES_ASSERT(bodyIDArray != nullptr);
        NES_ASSERT(number > 0);

        // Mark the tree dirty
        m_isDirty = true;

        for (const BodyID* pCurrent = bodyIDArray, *pEnd = bodyIDArray + number; pCurrent < pEnd; ++pCurrent)
        {
            NES_ASSERT(bodies[pCurrent->GetIndex()]->GetID() == *pCurrent, "Provided BodyID doesn't match the BodyID in the BodyManager!");

            // Get the Location of the Body
            uint32 nodeIndex;
            uint32 childNodeIndex;
            GetBodyLocation(trackers, *pCurrent, nodeIndex, childNodeIndex);

            // First we reset our internal bookkeeping
            InvalidateBodyLocation(trackers, *pCurrent);

            // Then we make the bounding box invalid - no queries can find this Node anymore.
            Node& node = m_pAllocator->Get(nodeIndex);
            node.InvalidateChildBounds(static_cast<int>(childNodeIndex));

            // Finally, we reset the child ID, this makes the Node available for adds again.
            node.m_childNodeIDs[childNodeIndex] = NodeID::InvalidID();

            // We don't need to bubble up our bounding box changes to our parents since we never make volumes smaller, only bigger.
            // But, we do need to mark the nodes as changed so that the tree can be rebuilt.
            MarkNodeAndParentsChanged(nodeIndex);
        }

        m_numBodies -= number;
    }

    void QuadTree::NotifyBodiesAABBChanged(const BodyVector& bodies, const BodyTrackerArray& trackers,
        const BodyID* bodyIDArray, const int number)
    {
        NES_ASSERT(bodyIDArray != nullptr);
        NES_ASSERT(number > 0);

        for (const BodyID* pCurrent = bodyIDArray, *pEnd = bodyIDArray + number; pCurrent < pEnd; ++pCurrent)
        {
            // Check if the BodyID is correct
            const Body* pBody = bodies[pCurrent->GetIndex()];
            NES_ASSERT(pBody->GetID() == *pCurrent, "Provided BodyID doesn't match the BodyID in the BodyManager!");

            // Get the new bounding box
            const AABox& newBounds = pBody->GetWorldSpaceBounds();

            // Get the Location of the Body
            uint32 nodeIndex;
            uint32 childNodeIndex;
            GetBodyLocation(trackers, *pCurrent, nodeIndex, childNodeIndex);

            // Widen the bounds for the Node
            Node& node = m_pAllocator->Get(nodeIndex);
            if (node.EncapsulateChildBounds(static_cast<int>(childNodeIndex), newBounds))
            {
                // If changed, our tree needs to be updated, and we need to walk up the
                // tree and widen all parents.
                m_isDirty = true;
                WidenAndMarkNodeAndParentsChanged(nodeIndex, newBounds);
            }
            
        }
    }

    void QuadTree::CastRay(const RayCast& ray, RayCastBodyCollector& collector, const CollisionLayerFilter& layerFilter, const BodyTrackerArray& trackers) const
    {
        class Visitor
        {
        public:
            NES_INLINE Visitor(const RayCast& ray, RayCastBodyCollector& collector)
                : m_origin(ray.m_origin)
                , m_invDirection(ray.m_direction)
                , m_collector(collector)
            {
                m_fractionStack.resize(kStackSize);
                m_fractionStack[0] = -1.f;
            }

            //----------------------------------------------------------------------------------------------------
            ///	@brief : Returns true if further processing of the tree should be aborted.   
            //----------------------------------------------------------------------------------------------------
            NES_INLINE bool         ShouldAbort() const { return m_collector.ShouldEarlyOut(); }

            //----------------------------------------------------------------------------------------------------
            ///	@brief : Returns true if this node / body should be visited, false if no hit can be generated.   
            //----------------------------------------------------------------------------------------------------
            NES_INLINE bool         ShouldVisitNode(const int stackTop) const { return m_fractionStack[stackTop] < m_collector.GetEarlyOutFraction(); }

            //----------------------------------------------------------------------------------------------------
            ///	@brief : Visit nodes, returns the number of hits found and sorts childNodeIDs so that they are at the
            ///     beginning of the vector.
            //----------------------------------------------------------------------------------------------------
            NES_INLINE int          VisitNodes(const Vec4Reg& boundsMinX, const Vec4Reg& boundsMinY, const Vec4Reg& boundsMinZ, const Vec4Reg& boundsMaxX, const Vec4Reg& boundsMaxY, const Vec4Reg& boundsMaxZ, UVec4Reg& childNodeIDs, const int stackTop)
            {
                // Test the ray against 4 bounding boxes.
                Vec4Reg fraction = RayAABox4(m_origin, m_invDirection, boundsMinX, boundsMinY, boundsMinZ, boundsMaxX, boundsMaxY, boundsMaxZ);

                // Sort so that the highest values are first (we want to first process closer hits, and we process the stack from top to bottom).
                return SortReverseAndStore(fraction, m_collector.GetEarlyOutFraction(), childNodeIDs, &m_fractionStack[stackTop]);
            }

            //----------------------------------------------------------------------------------------------------
            ///	@brief : Visit a body. Returns false if the algorithm should terminate because no hits can be generated anymore.
            //----------------------------------------------------------------------------------------------------
            NES_INLINE void         VisitBody(const BodyID& bodyID, const int stackTop)
            {
                // Store potential hit with Body
                BroadPhaseCastResult result { bodyID, m_fractionStack[stackTop] };
                m_collector.AddHit(result);
            }

            //----------------------------------------------------------------------------------------------------
            ///	@brief : Called when the stack is resized. This allows us to resize the fraction stack to match the
            ///     new stack size.
            //----------------------------------------------------------------------------------------------------
            NES_INLINE void         OnStackResized(const size_t newStackSize)
            {
                m_fractionStack.resize(newStackSize);
            }

        private:
            Vec3                    m_origin;
            RayInvDirection         m_invDirection;
            RayCastBodyCollector&   m_collector;
            std::vector<float, STLLocalAllocator<float, kStackSize>> m_fractionStack;
        };

        Visitor visitor(ray, collector);
        WalkTree(layerFilter, trackers, visitor);
    }

    void QuadTree::CastAABox(const AABoxCast& box, CastShapeBodyCollector& collector, const CollisionLayerFilter& layerFilter, const BodyTrackerArray& trackers) const
    {
        class Visitor
        {
        public:
            NES_INLINE Visitor(const AABoxCast& box, CastShapeBodyCollector& collector)
                : m_origin(box.m_box.Center())
                , m_extent(box.m_box.Extent())
                , m_invDirection(box.m_direction)
                , m_collector(collector)
            {
                m_fractionStack.resize(kStackSize);
                m_fractionStack[0] = -1.f;
            }

            //----------------------------------------------------------------------------------------------------
            ///	@brief : Returns true if further processing of the tree should be aborted.   
            //----------------------------------------------------------------------------------------------------
            NES_INLINE bool         ShouldAbort() const { return m_collector.ShouldEarlyOut(); }

            //----------------------------------------------------------------------------------------------------
            ///	@brief : Returns true if this node / body should be visited, false if no hit can be generated.   
            //----------------------------------------------------------------------------------------------------
            NES_INLINE bool         ShouldVisitNode(const int stackTop) const { return m_fractionStack[stackTop] < m_collector.GetPositiveEarlyOutFraction(); }

            //----------------------------------------------------------------------------------------------------
            ///	@brief : Visit nodes, returns the number of hits found and sorts childNodeIDs so that they are at the
            ///     beginning of the vector.
            //----------------------------------------------------------------------------------------------------
            NES_INLINE int          VisitNodes(const Vec4Reg& inBoundsMinX, const Vec4Reg& inBoundsMinY, const Vec4Reg& inBoundsMinZ, const Vec4Reg& inBoundsMaxX, const Vec4Reg& inBoundsMaxY, const Vec4Reg& inBoundsMaxZ, UVec4Reg& childNodeIDs, const int stackTop)
            {
                // Enlarge them by the casted AABox extents
                Vec4Reg boundsMinX = inBoundsMinX; 
                Vec4Reg boundsMinY = inBoundsMinY; 
                Vec4Reg boundsMinZ = inBoundsMinZ;
                Vec4Reg boundsMaxX = inBoundsMaxX; 
                Vec4Reg boundsMaxY = inBoundsMaxY; 
                Vec4Reg boundsMaxZ = inBoundsMaxZ; 
                math::AABox4EnlargeWithExtent(m_extent, boundsMinX, boundsMinY, boundsMinZ, boundsMaxX, boundsMaxY, boundsMaxZ);
                
                // Test the ray against 4 bounding boxes.
                Vec4Reg fraction = RayAABox4(m_origin, m_invDirection, boundsMinX, boundsMinY, boundsMinZ, boundsMaxX, boundsMaxY, boundsMaxZ);

                // Sort so that the highest values are first (we want to first process closer hits, and we process the stack from top to bottom).
                return SortReverseAndStore(fraction, m_collector.GetPositiveEarlyOutFraction(), childNodeIDs, &m_fractionStack[stackTop]);
            }

            //----------------------------------------------------------------------------------------------------
            ///	@brief : Visit a body. Returns false if the algorithm should terminate because no hits can be generated anymore.
            //----------------------------------------------------------------------------------------------------
            NES_INLINE void         VisitBody(const BodyID& bodyID, const int stackTop)
            {
                // Store potential hit with Body
                BroadPhaseCastResult result { bodyID, m_fractionStack[stackTop] };
                m_collector.AddHit(result);
            }

            //----------------------------------------------------------------------------------------------------
            ///	@brief : Called when the stack is resized. This allows us to resize the fraction stack to match the
            ///     new stack size.
            //----------------------------------------------------------------------------------------------------
            NES_INLINE void         OnStackResized(const size_t newStackSize)
            {
                m_fractionStack.resize(newStackSize);
            }

        private:
            Vec3                    m_origin;
            Vec3                    m_extent;
            RayInvDirection         m_invDirection;
            CastShapeBodyCollector& m_collector;
            std::vector<float, STLLocalAllocator<float, kStackSize>> m_fractionStack;
        };

        Visitor visitor(box, collector);
        WalkTree(layerFilter, trackers, visitor);
    }

    void QuadTree::CollideAABox(const AABox& box, CollideShapeBodyCollector& collector, const CollisionLayerFilter& layerFilter, const BodyTrackerArray& trackers) const
    {
        class Visitor
        {
        public:
            NES_INLINE explicit         Visitor(const AABox& box, CollideShapeBodyCollector& collector) : m_box(box), m_collector(collector) {}

            NES_INLINE bool             ShouldAbort() const { return m_collector.ShouldEarlyOut(); }

            /// Returns true if this node / body should be visited, false if no hit can be generated.
            NES_INLINE bool             ShouldVisitNode([[maybe_unused]] int stackTop) const
            {
                return true;
            }

            NES_INLINE int              VisitNodes(const Vec4Reg& boundsMinX, const Vec4Reg& boundsMinY, const Vec4Reg& boundsMinZ, const Vec4Reg& boundsMaxX, const Vec4Reg& boundsMaxY, const Vec4Reg& boundsMaxZ, UVec4Reg& childNodeIDs, [[maybe_unused]] const int stackTop) const
            {
                const UVec4Reg hitting = math::AABox4VsAABox(m_box, boundsMinX, boundsMinY, boundsMinZ, boundsMaxX, boundsMaxY, boundsMaxZ);
                return CountAndSortTrues(hitting, childNodeIDs);
            }

            NES_INLINE void             VisitBody(const BodyID& id, [[maybe_unused]] const int stackTop)
            {
                // Store the potential hit with the body.
                m_collector.AddHit(id);
            }

            NES_INLINE void             OnStackResized([[maybe_unused]] const size_t newStackSize) const { /* Nothing to do. */ }

        private:
            const AABox&                m_box;
            CollideShapeBodyCollector&  m_collector;
        };
        
        Visitor visitor(box, collector);
        WalkTree(layerFilter, trackers, visitor);
    }

    void QuadTree::CollideSphere(const Vec3& center, const float radius, CollideShapeBodyCollector& collector, const CollisionLayerFilter& layerFilter, const BodyTrackerArray& trackers) const
    {
        class Visitor
        {
        public:
            NES_INLINE Visitor(const Vec3 center, const float radius, CollideShapeBodyCollector& collector)
                : m_centerX(center.SplatX())
                , m_centerY(center.SplatY())
                , m_centerZ(center.SplatZ())
                , m_radiusSqr(Vec4Reg::Replicate(math::Squared(radius)))
                , m_collector(collector)
            {
                //
            }

            NES_INLINE bool         ShouldAbort() const { return m_collector.ShouldEarlyOut(); }

            /// Returns true if this node / body should be visited, false if no hit can be generated.
            NES_INLINE bool             ShouldVisitNode([[maybe_unused]] int stackTop) const
            {
                return true;
            }

            NES_INLINE int              VisitNodes(const Vec4Reg& boundsMinX, const Vec4Reg& boundsMinY, const Vec4Reg& boundsMinZ, const Vec4Reg& boundsMaxX, const Vec4Reg& boundsMaxY, const Vec4Reg& boundsMaxZ, UVec4Reg& childNodeIDs, [[maybe_unused]] const int stackTop) const
            {
                const UVec4Reg hitting = math::AABox4VsSphere(m_centerX, m_centerY, m_centerZ, m_radiusSqr, boundsMinX, boundsMinY, boundsMinZ, boundsMaxX, boundsMaxY, boundsMaxZ);
                return CountAndSortTrues(hitting, childNodeIDs);
            }

            NES_INLINE void             VisitBody(const BodyID& id, [[maybe_unused]] const int stackTop)
            {
                // Store the potential hit with the body.
                m_collector.AddHit(id);
            }

            NES_INLINE void             OnStackResized([[maybe_unused]] const size_t newStackSize) const { /* Nothing to do. */ }

        private:
            Vec4Reg                     m_centerX;
            Vec4Reg                     m_centerY;
            Vec4Reg                     m_centerZ;
            Vec4Reg                     m_radiusSqr;
            CollideShapeBodyCollector&  m_collector;
        };
        
        Visitor visitor(center, radius, collector);
        WalkTree(layerFilter, trackers, visitor);
    }

    void QuadTree::CollidePoint(const Vec3& point, CollideShapeBodyCollector& collector, const CollisionLayerFilter& layerFilter, const BodyTrackerArray& trackers) const
    {
        class Visitor
        {
        public:
            NES_INLINE Visitor(const Vec3 point, CollideShapeBodyCollector& collector)
                : m_point(point)
                , m_collector(collector)
            {
                //
            }

            NES_INLINE bool         ShouldAbort() const { return m_collector.ShouldEarlyOut(); }

            /// Returns true if this node / body should be visited, false if no hit can be generated.
            NES_INLINE bool             ShouldVisitNode([[maybe_unused]] int stackTop) const
            {
                return true;
            }

            NES_INLINE int              VisitNodes(const Vec4Reg& boundsMinX, const Vec4Reg& boundsMinY, const Vec4Reg& boundsMinZ, const Vec4Reg& boundsMaxX, const Vec4Reg& boundsMaxY, const Vec4Reg& boundsMaxZ, UVec4Reg& childNodeIDs, [[maybe_unused]] const int stackTop) const
            {
                const UVec4Reg hitting = math::AABox4VsPoint(m_point, boundsMinX, boundsMinY, boundsMinZ, boundsMaxX, boundsMaxY, boundsMaxZ);
                return CountAndSortTrues(hitting, childNodeIDs);
            }

            NES_INLINE void             VisitBody(const BodyID& id, [[maybe_unused]] const int stackTop)
            {
                // Store the potential hit with the body.
                m_collector.AddHit(id);
            }

            NES_INLINE void             OnStackResized([[maybe_unused]] const size_t newStackSize) const { /* Nothing to do. */ }

        private:
            Vec3                        m_point;
            CollideShapeBodyCollector&  m_collector;
        };

        Visitor visitor(point, collector);
        WalkTree(layerFilter, trackers, visitor);
    }

    void QuadTree::CollideOrientedBox(const OrientedBox& box, CollideShapeBodyCollector& collector, const CollisionLayerFilter& layerFilter, const BodyTrackerArray& trackers) const
    {
        class Visitor
        {
        public:
            NES_INLINE Visitor(const OrientedBox& box, CollideShapeBodyCollector& collector)
                : m_box(box)
                , m_collector(collector)
            {
                //
            }

            NES_INLINE bool         ShouldAbort() const
            {
                return m_collector.ShouldEarlyOut();
            }

            /// Returns true if this node / body should be visited, false if no hit can be generated.
            NES_INLINE bool             ShouldVisitNode([[maybe_unused]] int stackTop) const
            {
                return true;
            }

            NES_INLINE int              VisitNodes(const Vec4Reg& boundsMinX, const Vec4Reg& boundsMinY, const Vec4Reg& boundsMinZ, const Vec4Reg& boundsMaxX, const Vec4Reg& boundsMaxY, const Vec4Reg& boundsMaxZ, UVec4Reg& childNodeIDs, [[maybe_unused]] const int stackTop) const
            {
                const UVec4Reg hitting = math::AABox4VsBox(m_box, boundsMinX, boundsMinY, boundsMinZ, boundsMaxX, boundsMaxY, boundsMaxZ);
                return CountAndSortTrues(hitting, childNodeIDs);
            }

            NES_INLINE void             VisitBody(const BodyID& id, [[maybe_unused]] const int stackTop)
            {
                // Store the potential hit with the body.
                m_collector.AddHit(id);
            }

            NES_INLINE void             OnStackResized([[maybe_unused]] const size_t newStackSize) const
            {
                /* Nothing to do. */
            }

        private:
            OrientedBox                 m_box;
            CollideShapeBodyCollector&  m_collector;
        };

        Visitor visitor(box, collector);
        WalkTree(layerFilter, trackers, visitor); 
    }

    void QuadTree::FindCollidingPairs(const BodyVector& bodies, const BodyID* activeBodiesArray, const int numActiveBodies, float speculativeContactDistance, BodyPairCollector& collector, const CollisionLayerPairFilter& layerFilter) const
    {
        // Note that we don't lock the tree at this point. We know that the tree is not going to be swapped or deleted while finding collision pairs due to the way the jobs are scheduled in
        // the PhysicsScene::Update. We double-check this assumption at the end of the function.
        const RootNode& rootNode = GetCurrentRoot();
        NES_ASSERT(rootNode.m_index != kInvalidNodeIndex);

        // Assert sane input
        NES_ASSERT(activeBodiesArray != nullptr);
        NES_ASSERT(numActiveBodies > 0);

        std::vector<NodeID, STLLocalAllocator<NodeID, kStackSize>> nodeStackArray;
        nodeStackArray.resize(kStackSize);
        NodeID* pNodeStack = nodeStackArray.data();

        // Loop over all active bodies
        for (int b1 = 0; b1 < numActiveBodies; ++b1)
        {
            BodyID body1ID = activeBodiesArray[b1];
            const Body& body1 = *bodies[body1ID.GetIndex()];
            NES_ASSERT(!body1.IsStatic());

            // Expand the bounding box by the speculative contact distance.
            AABox bounds1 = body1.GetWorldSpaceBounds();
            bounds1.ExpandBy(Vec3::Replicate(speculativeContactDistance));

            // Test each body with the tree
            pNodeStack[0] = rootNode.GetNodeID();
            int top = 0;
            do
            {
                // Check if node is a body
                NodeID childNodeID = pNodeStack[top];
                if (childNodeID.IsBody())
                {
                    // Don't collide with self.
                    const BodyID body2ID = childNodeID.GetBodyID();
                    if (body1ID != body2ID)
                    {
                        // Collisions between dynamic pairs need to be picked up only once.
                        const Body& body2 = *bodies[body2ID.GetIndex()];
                        if (layerFilter.ShouldCollide(body1.GetCollisionLayer(), body2.GetCollisionLayer())
                            && Body::Internal_FindCollidingPairsCanCollide(body1, body2)
                            && bounds1.Overlaps(body2.GetWorldSpaceBounds())) // In the broadphase we widen the bounding box when a body moves - do a final check to see if the bounding boxes actually overlap.
                        {
                            collector.AddHit({ body1ID, body2ID}); 
                        }
                    }
                }
                else if (childNodeID.IsValid())
                {
                    // Process normal node
                    const Node& node = m_pAllocator->Get(childNodeID.GetNodeIndex());
                    NES_ASSERT(math::IsAligned(&node, NES_CACHE_LINE_SIZE));

                    // Get the bounds of the 4 children
                    const Vec4Reg boundsMinX = Vec4Reg::LoadFloat4Aligned(reinterpret_cast<const Float4*>(&node.m_minX));
                    const Vec4Reg boundsMinY = Vec4Reg::LoadFloat4Aligned(reinterpret_cast<const Float4*>(&node.m_minY));
                    const Vec4Reg boundsMinZ = Vec4Reg::LoadFloat4Aligned(reinterpret_cast<const Float4*>(&node.m_minZ));
                    const Vec4Reg boundsMaxX = Vec4Reg::LoadFloat4Aligned(reinterpret_cast<const Float4*>(&node.m_maxX));
                    const Vec4Reg boundsMaxY = Vec4Reg::LoadFloat4Aligned(reinterpret_cast<const Float4*>(&node.m_maxY));
                    const Vec4Reg boundsMaxZ = Vec4Reg::LoadFloat4Aligned(reinterpret_cast<const Float4*>(&node.m_maxZ));

                    // Test overlap
                    const UVec4Reg overlap = math::AABox4VsAABox(bounds1, boundsMinX, boundsMinY, boundsMinZ, boundsMaxX, boundsMaxY, boundsMaxZ);
                    const int numResults = overlap.CountTrues();
                    if (numResults > 0)
                    {
                        // Load the ids for the 4 children
                        UVec4Reg childIDs = UVec4Reg::LoadInt4Aligned(reinterpret_cast<const uint32*>(&node.m_childNodeIDs[0]));

                        // Sort so that overlaps are first.
                        childIDs = UVec4Reg::Sort4True(overlap, childIDs);

                        // Ensure there is space on the stack (falls back to the heap if there isn't).
                        if (top + 4 >= static_cast<int>(nodeStackArray.size()))
                        {
                            QuadTreePerformanceWarning();
                            nodeStackArray.resize(nodeStackArray.size() << 1);
                            pNodeStack = nodeStackArray.data();
                        }

                        // Push them onto the stack
                        childIDs.StoreInt4(reinterpret_cast<uint32*>(&pNodeStack[top]));
                        top += numResults;
                    }
                }

                --top;
            }
            while (top >= 0);
        }

        // Test that the root node was not swapped while finding colliding pairs.
        // This would mean that UpdateFinalize/DiscardOldTree ran during collision detection, which should not be possible with the way the jobs are scheduled.
        NES_ASSERT(rootNode.m_index != kInvalidNodeIndex);
        NES_ASSERT(&rootNode == &GetCurrentRoot());
    }

    AABox QuadTree::GetBounds() const
    {
        const uint32 nodeIndex = GetCurrentRoot().m_index;
        NES_ASSERT(nodeIndex != kInvalidNodeIndex);
        const Node& node = m_pAllocator->Get(nodeIndex);

        AABox bounds;
        node.GetNodeBounds(bounds);
        return bounds;
    }

    void QuadTree::GetBodyLocation(const BodyTrackerArray& trackers, const BodyID bodyID, uint32& outNodeIndex, uint32& outChildIndex) const
    {
        const uint32 bodyLocation = trackers[bodyID.GetIndex()].m_bodyLocation;
        NES_ASSERT(bodyLocation != BodyTracker::kInvalidBodyLocation);
        outNodeIndex = bodyLocation & BodyTracker::kBodyIndexMask;
        outChildIndex = bodyLocation >> BodyTracker::kChildIndexShift;
        NES_ASSERT(m_pAllocator->Get(outNodeIndex).m_childNodeIDs[outChildIndex] == bodyID, "Make sure that the body is in the node where it should be!");
    }

    void QuadTree::SetBodyLocation(BodyTrackerArray& trackers, const BodyID bodyID, const uint32 nodeIndex, const uint32 childIndex) const
    {
        NES_ASSERT(nodeIndex <= BodyTracker::kBodyIndexMask);
        NES_ASSERT(childIndex < 4);
        NES_ASSERT(m_pAllocator->Get(nodeIndex).m_childNodeIDs[childIndex] == bodyID, "Make sure that the body is in the node where it should be!");
        trackers[bodyID.GetIndex()].m_bodyLocation = nodeIndex + (childIndex << BodyTracker::kChildIndexShift);

#if NES_LOGGING_ENABLED
        // Validate GetBodyLocation
        uint32 vNodeIndex;
        uint32 vChildIndex;
        GetBodyLocation(trackers, bodyID, vNodeIndex, vChildIndex);
        NES_ASSERT(vNodeIndex == nodeIndex);
        NES_ASSERT(vChildIndex == childIndex);
#endif
    }

    void QuadTree::InvalidateBodyLocation(BodyTrackerArray& trackers, const BodyID bodyID)
    {
        trackers[bodyID.GetIndex()].m_bodyLocation = BodyTracker::kInvalidBodyLocation;
    }

    AABox QuadTree::GetNodeOrBodyBounds(const BodyVector& bodies, NodeID nodeID) const
    {
        if (nodeID.IsNode())
        {
            // Node:
            const uint32 nodeIndex = nodeID.GetNodeIndex();
            const Node& node = m_pAllocator->Get(nodeIndex);

            AABox bounds;
            node.GetNodeBounds(bounds);
            return bounds;
        }
        
        // Otherwise it is a Body:
        return bodies[nodeID.GetBodyID().GetIndex()]->GetWorldSpaceBounds();   
    }

    void QuadTree::MarkNodeAndParentsChanged(uint32 nodeIndex)
    {
        uint32 currentIndex = nodeIndex;

        do
        {
            // If the node is already Marked as changed, then the parent will
            // be too.
            Node& node = m_pAllocator->Get(currentIndex);
            if (node.m_isChanged)
                break;

            // Mark node as changed
            node.m_isChanged = true;

            // Get the parent to continue
            currentIndex = node.m_parentNodeIndex;
        }
        while (currentIndex != kInvalidNodeIndex);
    }

    void QuadTree::WidenAndMarkNodeAndParentsChanged(const uint32 nodeIndex, const AABox& newBounds)
    {
        uint32 currentIndex = nodeIndex;

        for (;;)
        {
            // Mark the Node as changed:
            Node& node = m_pAllocator->Get(currentIndex);
            node.m_isChanged = true;

            // Get the parent
            uint32 parentNodeIndex = node.m_parentNodeIndex;
            if (parentNodeIndex == kInvalidNodeIndex)
                break;

            // Find out which child of the parent that this node is in.
            Node& parentNode = m_pAllocator->Get(parentNodeIndex);
            NodeID nodeID = NodeID::FromNodeIndex(currentIndex);
            int childIndex = -1;
            for (int i = 0; i < 4; ++i)
            {
                if (parentNode.m_childNodeIDs[i] == nodeID)
                {
                    // Found it. Set the node and child index and update the bounding box too.
                    childIndex = i;
                    break;
                }
            }
            NES_ASSERT(childIndex != -1, "Nodes should not get removed from the tree, we should have found it.");

            // To avoid any race conditions with other threads we only enlarge bounding boxes.
            if (!parentNode.EncapsulateChildBounds(childIndex, newBounds))
            {
                // No changes to the bounding box, only marking as changed needs to be done.
                if (!parentNode.m_isChanged)
                    MarkNodeAndParentsChanged(nodeIndex);
                
                break;
            }

            currentIndex = parentNode.m_parentNodeIndex;
        }
    }

    uint32 QuadTree::AllocateNode(bool isChanged)
    {
        const uint32 index = m_pAllocator->ConstructObject(isChanged);
        if (index == Allocator::kInvalidObjectIndex)
        {
            // If you're running out of nodes, you're most likely adding too many individual bodies to the tree.
            // Because of the lock free nature of this tree, any individual body is added to the root of the tree.
            // This means that if you add a lot of bodies individually, you will end up with a very deep tree and you'll be
            // using a lot more nodes than you would if you added them in batches.
            //
            // The system keeps track of a previous and a current tree, this allows for queries to continue using the old tree
            // while the new tree is being built. If you completely clean the PhysicsSystem and rebuild it from scratch, you may
            // want to call PhysicsSystem::OptimizeBroadPhase two times after clearing to completely get rid of any lingering nodes.
            //
            // The number of nodes that is allocated is related to the max number of bodies that is passed in PhysicsSystem::Init.
            // For normal situations there are plenty of nodes available. If all else fails, you can increase the number of nodes
            // by increasing the maximum number of bodies.
            
            NES_FATAL("QuadTree: Out of Nodes!");
        }
        return index;
    }

    bool QuadTree::TryInsertLeaf(BodyTrackerArray& trackers, const int nodeIndex, const NodeID leafID, const AABox& leafBounds, const int numLeafBodies)
    {
        // Tentatively assign the node as the parent.
        const bool leafIsNode = leafID.IsNode();
        if (leafIsNode)
        {
            uint32 leafIndex = leafID.GetNodeIndex();
            m_pAllocator->Get(leafIndex).m_parentNodeIndex = nodeIndex;
        }

        // Get the node that we're adding to.
        Node& node = m_pAllocator->Get(nodeIndex);

        // Find an empty child node
        for (uint32 childIndex = 0; childIndex < 4; ++childIndex)
        {
            // Check if we can claim the Child Node
            if (node.m_childNodeIDs[childIndex].CompareExchange(NodeID::InvalidID(), leafID))
            {
                // We were able to claim it!

                // If the Leaf was a Body, then we need to update the bookkeeping
                if (!leafIsNode)
                    SetBodyLocation(trackers, leafID.GetBodyID(), nodeIndex, childIndex);

                // Now set the bounding box making the child valid for queries
                node.SetChildBounds(static_cast<int>(childIndex), leafBounds);

                // Widen the bounds for our parents too
                WidenAndMarkNodeAndParentsChanged(nodeIndex, leafBounds);

                // Update the body count
                m_numBodies += numLeafBodies;
                
                return true;
            }
        }

        // No Child index was available for this node.
        return false;
    }

    bool QuadTree::TryCreateNewRoot(BodyTrackerArray& trackers, std::atomic<uint32>& rootNodeIndex, NodeID leafID, const AABox& leafBounds, int numLeafBodies)
    {
        // Grab the old root
        uint32 rootIndex = rootNodeIndex;
        Node& root = m_pAllocator->Get(rootIndex);

        // Create the new root, marking it as changed as we're not creating a very efficient tree at this point.
        const uint32 newRootIndex = AllocateNode(true);
        Node& newRoot = m_pAllocator->Get(newRootIndex);
        
        // First child is the current root, not that since the tree may be modified concurrently we cannot assume that the bounds of our child will
        // be correct so we set a very large bounding box.
        newRoot.m_childNodeIDs[0] = NodeID::FromNodeIndex(rootIndex);
        newRoot.SetChildBounds(0, AABox(Vec3(-math::kLargeFloat), Vec3(math::kLargeFloat)));

        // Second child is a new leaf.
        newRoot.m_childNodeIDs[1] = leafID;
        newRoot.SetChildBounds(1, leafBounds);

        // Tentatively assign new root as the parent.
        const bool leafIsNode = leafID.IsNode();
        if (leafIsNode)
        {
            const uint32 leafIndex = leafID.GetNodeIndex();
            m_pAllocator->Get(leafIndex).m_parentNodeIndex = newRootIndex;
        }

        // Try to swap it.
        if (rootNodeIndex.compare_exchange_strong(rootIndex, newRootIndex))
        {
            // We managed to set the new root.

            // If the leaf was a body, update its bookkeeping
            if (!leafIsNode)
                SetBodyLocation(trackers, leafID.GetBodyID(), newRootIndex, 1);

            // Store the parent node for old root.
            root.m_parentNodeIndex = newRootIndex;

            // Update the body count
            m_numBodies += numLeafBodies;

            return true;
        }

        // Failed to swap, someone else must have created a new root. Need to try again.
        m_pAllocator->DestructObject(newRootIndex);
        return false;
    }

    QuadTree::NodeID QuadTree::BuildTree(const BodyVector& bodies, BodyTrackerArray& trackers, NodeID* pNodeIDs, int number, uint maxDepthMarkChanged, AABox& outBounds)
    {
        // Trivial case: No Bodies in the tree
        if (number == 0)
        {
            outBounds = kInvalidBounds;
            return NodeID::InvalidID();
        }

        // Trivial case: When we have 1 body or node, return it.
        if (number == 1)
        {
            if (pNodeIDs->IsNode())
            {
                // When returning an existing node as root, ensure that no parent has been set
                Node& node = m_pAllocator->Get(pNodeIDs->GetNodeIndex());
                node.m_parentNodeIndex = kInvalidNodeIndex;
            }

            outBounds = GetNodeOrBodyBounds(bodies, *pNodeIDs);
            return *pNodeIDs;
        }

        // Calculate the centers of all bodies that are to be inserted.
        Vec3* pCenters = NES_NEW_ARRAY(Vec3, number);
        NES_ASSERT(math::IsAligned(pCenters, NES_VECTOR_ALIGNMENT));
        Vec3* pCurrent = pCenters;
        for (const NodeID* pValue = pNodeIDs, *pEnd = pNodeIDs + number; pValue < pEnd; ++pValue, ++pCurrent)
        {
            *pCurrent = GetNodeOrBodyBounds(bodies, *pValue).Center();
        }

        // The algorithm is a recursive tree build, but to avoid the call overhead, we keep track of a stack here.
        struct StackEntry
        {
            uint32  m_nodeIndex;       /// Node index of the Node that is generated.
            int     m_childIndex;      /// Index of the child that we are currently processing.
            int     m_splitIndices[5]; /// Indices where the node ID's have been split to form 4 partitions.
            uint32  m_depth;           /// Depth of this node in the tree.
            Vec3    m_boundsMin;       /// Bounding box min, accumulated while iterating over children.
            Vec3    m_boundsMax;       /// Bounding box max, accumulated while iterating over children.
        };
        static_assert(sizeof(StackEntry) == 64);
        StackEntry stack[kStackSize / 4]; // We don't process 4 at a time in this loop but 1, so the stack can be 4x as small.
        int top = 0;

        // Create the root Node
        stack[0].m_nodeIndex = AllocateNode(maxDepthMarkChanged > 0);
        stack[0].m_childIndex = -1;
        stack[0].m_depth = 0;
        stack[0].m_boundsMin = Vec3(math::kLargeFloat);
        stack[0].m_boundsMax = Vec3(-math::kLargeFloat);
        Partition4(pNodeIDs, pCenters, 0, number, stack[0].m_splitIndices);

        for (;;)
        {
            StackEntry& current = stack[top];

            // Next child:
            ++current.m_childIndex;

            // Check if all children processed:
            if (current.m_childIndex >= 4)
            {
                // Terminate if there's nothing left to pop
                if (top <= 0)
                    break;

                // Add our bounds to our parent bounds.
                StackEntry& previous = stack[top - 1];
                previous.m_boundsMin = Vec3::Min(previous.m_boundsMin, current.m_boundsMin);
                previous.m_boundsMax = Vec3::Max(previous.m_boundsMax, current.m_boundsMax);

                // Store parent node
                Node& node = m_pAllocator->Get(current.m_nodeIndex);
                node.m_parentNodeIndex = previous.m_nodeIndex;

                // Store this node's properties in the parent node
                Node& parentNode = m_pAllocator->Get(previous.m_nodeIndex);
                parentNode.m_childNodeIDs[previous.m_childIndex] = NodeID::FromNodeIndex(current.m_nodeIndex);
                parentNode.SetChildBounds(previous.m_childIndex, AABox(current.m_boundsMin, current.m_boundsMax));

                // Pop the entry from the stack.
                --top;
            }

            else
            {
                // Get the low and high index to bodies to process.
                const int low = current.m_splitIndices[current.m_childIndex];
                const int high = current.m_splitIndices[current.m_childIndex + 1];
                const int numBodies = high - low;

                if (numBodies == 1)
                {
                    // Get the Body Info
                    const NodeID childNodeID = pNodeIDs[low];
                    const AABox bounds = GetNodeOrBodyBounds(bodies, childNodeID);

                    // Update the current Node
                    Node& node = m_pAllocator->Get(current.m_nodeIndex);
                    node.m_childNodeIDs[current.m_childIndex] = childNodeID;
                    node.SetChildBounds(current.m_childIndex, bounds);

                    if (childNodeID.IsNode())
                    {
                        // Set the child's parent to the current.
                        Node& childNode = m_pAllocator->Get(childNodeID.GetNodeIndex());
                        childNode.m_parentNodeIndex = current.m_nodeIndex;
                    }
                    else
                    {
                        // Set Body location in tracking
                        SetBodyLocation(trackers, childNodeID.GetBodyID(), current.m_nodeIndex, current.m_childIndex);
                    }

                    // Encapsulate the bounding box in parent
                    current.m_boundsMin = Vec3::Min(current.m_boundsMin, bounds.m_min);
                    current.m_boundsMax = Vec3::Max(current.m_boundsMax, bounds.m_max);
                }
                else if (numBodies > 1)
                {
                    // Allocate a new Node
                    ++top;
                    StackEntry& newStack = stack[top];
                    NES_ASSERT(top < (kStackSize / 4));
                    const uint32 nextDepth = current.m_depth + 1;
                    newStack.m_nodeIndex = AllocateNode(maxDepthMarkChanged > nextDepth);
                    newStack.m_childIndex = -1;
                    newStack.m_depth = nextDepth;
                    newStack.m_boundsMin = Vec3(math::kLargeFloat);
                    newStack.m_boundsMax = Vec3(-math::kLargeFloat);
                    Partition4(pNodeIDs, pCenters, low, high, newStack.m_splitIndices);
                }
            }
        }

        // Delete the centers array
        NES_DELETE_ARRAY(pCenters);

        // Store the bounding box of the Root
        outBounds.m_min = stack[0].m_boundsMin;
        outBounds.m_max = stack[0].m_boundsMax;

        // Return the Root
        return NodeID::FromNodeIndex(stack[0].m_nodeIndex);
    }

    void QuadTree::Partition(NodeID* nodeIDs, Vec3* nodeCenters, int number, int& outMidPoint)
    {
        // Handle trivial case
        if (number <= 4)
        {
            outMidPoint = number / 2;
            return;
        }

        // Calculate the Bounding box of Box Centers
        Vec3 centerMin = Vec3(math::kLargeFloat);
        Vec3 centerMax = Vec3(-math::kLargeFloat);
        for (const Vec3* pCenter = nodeCenters, * pEnd = nodeCenters + number; pCenter < pEnd; ++pCenter)
        {
            Vec3 center = *pCenter;
            centerMin = Vec3::Min(centerMin, center);
            centerMax = Vec3::Max(centerMax, center);
        }

        // Calculate the split plane along the largest distance dimension.
        const int dimension = (centerMax - centerMin).MaxComponentIndex();
        const float split = 0.5f * (centerMin + centerMax)[dimension];

        // Divide the Bodies on the split plane
        int start = 0;
        int end = number;
        while (start < end)
        {
            // Search for the first element that is on the right hand side of the split plane
            while (start < end && nodeCenters[start][dimension] < split)
                ++start;

            // Search for the first element that is on the left hand side of the split plane
            while (start < end && nodeCenters[end - 1][dimension] >= split)
                --end;

            if (start < end)
            {
                // Swap the two elements
                std::swap(nodeIDs[start], nodeIDs[end - 1]);
                std::swap(nodeCenters[start], nodeCenters[end - 1]);
                ++start;
                --end;
            }
        }

        NES_ASSERT(start == end);

        if (start > 0 && start < number)
        {
            // Success!
            outMidPoint = start;
        }

        else
        {
            // Failed to divide the Bodies.
            outMidPoint = number / 2;
        }
    }

    void QuadTree::Partition4(NodeID* nodeIDs, Vec3* nodeCenters, const int begin, const int end, int* outSplitIndices)
    {
        NodeID* subIDs = nodeIDs + begin;
        Vec3* subCenters = nodeCenters + begin;
        const int number = end - begin;

        // Partition the entire range: 
        Partition(subIDs, subCenters, number, outSplitIndices[2]);

        // Partition the lower half:
        Partition(subIDs, subCenters, outSplitIndices[2], outSplitIndices[1]);
        
        // Partition the upper half:
        Partition(subIDs + outSplitIndices[2], subCenters + outSplitIndices[2], number - outSplitIndices[2], outSplitIndices[3]);

        // Convert to proper range: [begin, end]
        outSplitIndices[0] = begin;
        outSplitIndices[1] += begin;
        outSplitIndices[2] += begin;
        outSplitIndices[3] += outSplitIndices[2];
        outSplitIndices[4] = end;
    }

    uint32 QuadTree::GetMaxTreeDepth(const NodeID nodeID) const
    {
        // Reached a leaf:
        if (!nodeID.IsValid() || nodeID.IsBody())
            return 0;

        uint maxDepth = 0;
        const Node& node = m_pAllocator->Get(nodeID.GetNodeIndex());
        for (const NodeID childNodeID : node.m_childNodeIDs)
        {
            maxDepth = math::Max(maxDepth, GetMaxTreeDepth(childNodeID));
        }
        
        return maxDepth + 1;
    }

    template <typename Visitor>
    void QuadTree::WalkTree(const CollisionLayerFilter& layerFilter, const BodyTrackerArray& trackers, Visitor& visitor) const
    {
        const RootNode& rootNode = GetCurrentRoot();

        std::vector<NodeID, STLLocalAllocator<NodeID, kStackSize>> nodeStackArray;
        nodeStackArray.resize(kStackSize);;
        NodeID* pNodeStack = nodeStackArray.data();
        pNodeStack[0] = rootNode.GetNodeID();
        int top = 0;
        do
        {
            // Check if the Node is a Body:
            NodeID childNodeID = pNodeStack[top];
            if (childNodeID.IsBody())
            {
                const BodyID bodyID = childNodeID.GetBodyID();
                const CollisionLayer layer = trackers[bodyID.GetIndex()].m_collisionLayer;
                if (layer != kInvalidCollisionLayer && layerFilter.ShouldCollide(layer))
                {
                    // [TODO]: Stat tracking.
                    // Visit Body:
                    visitor.VisitBody(bodyID, top);
                    if (visitor.ShouldAbort())
                        break;
                }
            }

            else if (childNodeID.IsValid())
            {
                if (top + 4 >= static_cast<int>(nodeStackArray.size()))
                {
                    QuadTreePerformanceWarning();
                    nodeStackArray.resize(static_cast<int>(nodeStackArray.size() << 1));
                    pNodeStack = nodeStackArray.data();
                    visitor.OnStackResized(nodeStackArray.size());
                }
                
                const Node& node = m_pAllocator->Get(childNodeID.GetNodeIndex());
                NES_ASSERT(math::IsAligned(&node, NES_CACHE_LINE_SIZE));
                
                // Load the bounds of the 4 children:
                Vec4Reg boundsMinX = Vec4Reg::LoadFloat4Aligned(reinterpret_cast<const Float4*>(node.m_minX)); 
                Vec4Reg boundsMinY = Vec4Reg::LoadFloat4Aligned(reinterpret_cast<const Float4*>(node.m_minY)); 
                Vec4Reg boundsMinZ = Vec4Reg::LoadFloat4Aligned(reinterpret_cast<const Float4*>(node.m_minZ));
                Vec4Reg boundsMaxX = Vec4Reg::LoadFloat4Aligned(reinterpret_cast<const Float4*>(node.m_maxX)); 
                Vec4Reg boundsMaxY = Vec4Reg::LoadFloat4Aligned(reinterpret_cast<const Float4*>(node.m_maxY)); 
                Vec4Reg boundsMaxZ = Vec4Reg::LoadFloat4Aligned(reinterpret_cast<const Float4*>(node.m_maxZ));

                // Load the Child IDs.
                UVec4Reg childIDs = UVec4Reg::LoadInt4(reinterpret_cast<const uint32*>(node.m_childNodeIDs));

                const int numResults = visitor.VisitNodes(boundsMinX, boundsMinY, boundsMinZ, boundsMaxX, boundsMaxY, boundsMaxZ, childIDs, top);
                childIDs.StoreInt4(reinterpret_cast<uint32*>(&pNodeStack[top]));
                top += numResults;
            }

            // Fetch the next node until we find one that the visitor wants to see.
            do
            {
                --top;
            } while (top >= 0 && !visitor.ShouldVisitNode(top));
        }
        while (top >= 0);

        // [TODO]: Stat tracking.
    }

#if NES_LOGGING_ENABLED
    void QuadTree::ValidateTree(const BodyVector& bodies, const BodyTrackerArray& trackers, uint32 nodeIndex, uint32 numExpectedBodies) const
    {
        NES_ASSERT(nodeIndex != kInvalidNodeIndex);

        struct StackEntry
        {
            uint32 m_nodeIndex;
            uint32 m_parentNodeIndex;
            
            StackEntry() = default;
            StackEntry(const uint32 nodeIndex, const uint32 parentNodeIndex) : m_nodeIndex(nodeIndex), m_parentNodeIndex(parentNodeIndex) {} 
        };

        std::vector<StackEntry, STLLocalAllocator<StackEntry, kStackSize>> stack;
        stack.reserve(kStackSize);
        stack.emplace_back(nodeIndex, kInvalidNodeIndex);

        uint32 numBodies = 0;

        do
        {
            StackEntry current = stack.back();
            stack.pop_back();

            // Validate Parent
            const Node& node = m_pAllocator->Get(current.m_nodeIndex);
            NES_ASSERT(node.m_parentNodeIndex == current.m_parentNodeIndex);

            // Validate that when a parent is not-changed that all of its children are also not changed.
            NES_ASSERT(current.m_parentNodeIndex == kInvalidNodeIndex || m_pAllocator->Get(current.m_parentNodeIndex).m_isChanged || !node.m_isChanged);

            // Loop childen
            for (int i = 0; i < 4; ++i)
            {
                NodeID childNodeID = node.m_childNodeIDs[i];
                if (childNodeID.IsValid())
                {
                    if (childNodeID.IsNode())
                    {
                        // Child is a node, recurse
                        const uint32 childIndex = childNodeID.GetNodeIndex();
                        stack.emplace_back(childIndex, current.m_nodeIndex);

                        // Validate that the bounding box is bigger or equal to the bounds in the tree
                        // Bounding box could also be invalid if all children of our child were removed
                        AABox childBounds;
                        node.GetChildBounds(i, childBounds);
                        AABox realChildBounds;
                        m_pAllocator->Get(childIndex).GetNodeBounds(realChildBounds);
                        NES_ASSERT(childBounds.Contains(realChildBounds) || !realChildBounds.IsValid());
                    }
                    else
                    {
                        // Increment number of bodies found
                        ++numBodies;

                        // Check if tracker matches position of body
                        uint32 currentNodeIndex;
                        uint32 childIndex;
                        GetBodyLocation(trackers, childNodeID.GetBodyID(), currentNodeIndex, childIndex);
                        NES_ASSERT(currentNodeIndex == current.m_nodeIndex);
                        NES_ASSERT(static_cast<int>(childIndex) == i);

                        // Validate that the body cached bounds still match the actual bounds
                        const Body *body = bodies[childNodeID.GetBodyID().GetIndex()];
                        body->Internal_ValidateCachedBounds();

                        // Validate that the node bounds are bigger or equal to the body bounds
                        AABox bodyBounds;
                        node.GetChildBounds(i, bodyBounds);
                        NES_ASSERT(bodyBounds.Contains(body->GetWorldSpaceBounds()));
                    }
                }
            }
            
        } while (!stack.empty());

        NES_ASSERT(numBodies == numExpectedBodies);
    }
#endif
}
