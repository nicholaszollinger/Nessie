// QuadTree.h
#pragma once
#include "BroadPhase.h"
#include "BroadPhase.h"
#include "Core/Memory/FixedSizedFreeList.h"
#include "Core/Thread/Atomics.h"
#include "Physics/Body/BodyManager.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //		
    /// @brief : Internal tree structure in the Broadphase - a Quad AABB Tree.
    //----------------------------------------------------------------------------------------------------
    class QuadTree
    {
        static constexpr uint32_t kInvalidNodeIndex = std::numeric_limits<uint32_t>::max();
        /// Maximum size of the Stack during a Tree Walk. 
        static constexpr int kStackSize = 128;
        static const AABox kInvalidBounds;
        
        class AtomicNodeID;

        //----------------------------------------------------------------------------------------------------
        /// @brief : ID that either points to a Body or a Node in the Tree. 
        //----------------------------------------------------------------------------------------------------
        class NodeID
        {
            friend class AtomicNodeID;
            
            static constexpr uint32_t kIsNode = BodyID::kBroadPhaseBit;
            uint32_t m_id = kInvalidNodeIndex;

        private:
            explicit constexpr NodeID(uint32_t id) : m_id(id) {}
        
        public:
            constexpr NodeID() = default;
            static constexpr NodeID InvalidID() { return NodeID(kInvalidNodeIndex); }
            static constexpr NodeID FromBodyID(const BodyID id)
            {
                NodeID nodeID(id.GetIndexAndGeneration());
                NES_ASSERT(nodeID.IsBody());
                return nodeID;
            }
            
            static constexpr NodeID FromNodeIndex(const uint32_t index)
            {
                NES_ASSERT((index & kIsNode) == 0);
                return NodeID(index | kIsNode);
            }

            constexpr bool operator==(const BodyID& bodyID) const { return m_id == bodyID.GetIndexAndGeneration(); }
            constexpr bool operator==(const NodeID& nodeID) const { return m_id == nodeID.m_id; }

            constexpr bool IsValid() const    { return m_id != kInvalidNodeIndex; }
            constexpr bool IsBody() const     { return (m_id & kIsNode) == 0; }
            constexpr bool IsNode() const     { return (m_id & kIsNode) != 0; }

            constexpr BodyID GetBodyID() const        { NES_ASSERT(IsBody()); return BodyID(m_id); }
            constexpr uint32_t GetNodeIndex() const   { NES_ASSERT(IsNode()); return m_id & ~kIsNode; }
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : A NodeID that uses atomics to store its value.
        //----------------------------------------------------------------------------------------------------
        class AtomicNodeID
        {
            std::atomic<uint32_t> m_id;

        public:
            AtomicNodeID() = default;
            explicit AtomicNodeID(const NodeID id) : m_id(id.m_id) {}
            AtomicNodeID& operator=(const NodeID& id);

            operator NodeID() const { return NodeID(m_id); }
            bool operator==(const BodyID& bodyID) const     { return m_id == bodyID.GetIndexAndGeneration(); }
            bool operator==(const NodeID& nodeID) const     { return m_id == nodeID.m_id; }
            bool IsValid() const                            { return m_id != kInvalidNodeIndex; }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Atomically compare and swap value. Expects the current value to be equal to the oldID, and
            ///         if the same, it will replace with the new ID. Otherwise, this will return false. This is
            ///         to dismiss a change if the ID has been changed by another thread first. 
            //----------------------------------------------------------------------------------------------------
            bool CompareExchange(NodeID oldID, NodeID newID) { return m_id.compare_exchange_strong(oldID.m_id, newID.m_id); }
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Class that represents a single node in the Tree. 
        //----------------------------------------------------------------------------------------------------
        struct Node
        {
            /// The Bounding Box values for all child nodes or bodies. These are all initialized to invalid values
            /// so that no collision test will ever traverse to the leaf.
            std::atomic<float> m_minX[4];
            std::atomic<float> m_minY[4];
            std::atomic<float> m_minZ[4];
            std::atomic<float> m_maxX[4];
            std::atomic<float> m_maxY[4];
            std::atomic<float> m_maxZ[4];

            /// Indices of Child Nodes or Body IDs.
            AtomicNodeID m_childNodeIDs[4];

            /// Index of the Parent Node.
            /// This can be unreliable during the UpdatePrepare/Finalize() functions as a Node may be
            /// relinked to the newly built tree.
            std::atomic<uint32_t> m_parentNodeIndex = kInvalidNodeIndex;
            std::atomic<uint32_t> m_isChanged;

            /// Padding to align to 124 bytes
            uint32_t m_padding = 0;
            
            explicit Node(bool isChanged);
            void GetNodeBounds(AABox& outBounds) const;
            void GetChildBounds(int childIndex, AABox& outBounds) const;
            void SetChildBounds(int childIndex, const AABox& bounds);
            void InvalidateChildBounds(int childIndex);
            bool EncapsulateChildBounds(int childIndex, const AABox& bounds);
        };

    public:
        /// Class that allocates Tree Nodes - this can be shared among multiple trees.
        using Allocator = FixedSizeFreeList<Node>;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Data to Track a Body in the tree. 
        //----------------------------------------------------------------------------------------------------
        struct BodyTracker
        {
            static constexpr uint32_t kInvalidBodyLocation = std::numeric_limits<uint32_t>::max();
            static constexpr uint32_t kBodyIndexMask = 0x3fffffff;
            static constexpr uint32_t kChildIndexShift = 30;
            
            std::atomic<BroadPhaseLayer::Type> m_broadPhaseLayer = static_cast<BroadPhaseLayer::Type>(kInvalidBroadPhaseLayer);
            std::atomic<CollisionLayer> m_collisionLayer = kInvalidCollisionLayer;
            std::atomic<uint32_t> m_bodyLocation{ kInvalidBodyLocation }; // Location of the Body in the Quadtree.
            
            BodyTracker() = default;
            BodyTracker(const BodyTracker& other);
        };

        using BodyTrackerArray = std::vector<BodyTracker>;

        struct UpdateState
        {
            NodeID m_rootNodeID; /// This will be the new tree's root node ID.  
        };

        struct AddState
        {
            NodeID  m_leafID = NodeID::InvalidID();
            AABox   m_leafBounds{};
        };

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Root Node of the Tree. The index will always point to a Node, it will never point to a body.
        ///     The QuadTree maintains two RootNodes, meaning two trees, in order to let collision queries complete
        ///     in parallel to adding/removing Bodies/Nodes to the tree. 
        //----------------------------------------------------------------------------------------------------
        struct RootNode
        {
            NodeID GetNodeID() const { return NodeID::FromNodeIndex(m_index); }
            std::atomic<uint32_t> m_index{ kInvalidNodeIndex };
        };

        /// Allocator that controls adding and freeing nodes.
        Allocator* m_pAllocator = nullptr;

        /// This is a list of Nodes that must be deleted after the trees are swapped and the old tree is no
        /// longer in use.
        Allocator::Batch m_freeNodeBatch;

        /// Number of Bodies currently in the Tree.
        /// This is aligned to be in a different cache line from the 'Allocator' pointer to prevent cross
        /// thread syncs when reading nodes.
        alignas (NES_CACHE_LINE_SIZE) std::atomic<uint32_t> m_numBodies;
        
        /// Roots of the two internal tree structures. When updating, we activate the new tree and keep
        /// the old tree alive for queries that are in progress until the next time that DiscardOldTree() is
        /// called.
        RootNode m_rootNodes[2];
        std::atomic<uint32_t> m_rootNodeIndex{ 0 };

        /// Flag to keep track of changes to the broadphase. If False, we don't need to UpdatePrepare/Finalize().
        std::atomic<bool> m_isDirty = false;

        // [TODO]: Stats:
        //struct Stats{};
        
    public:
        QuadTree() = default;
        QuadTree(const QuadTree&) = delete;
        QuadTree& operator=(const QuadTree&) = delete;
        ~QuadTree();
        
        void Init(Allocator& allocator);
        void DiscardOldTree();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Update the Broadphase. Needs to be called regularly to achieve a tight fit of the tree when
        ///     Bodies have been modified. UpdatePrepare() will build the tree, and UpdateFinalize() will lock the
        ///     root of the tree shortly and swap the trees, then afterward clean up temporary data structures. 
        //----------------------------------------------------------------------------------------------------
        void UpdatePrepare(const BodyArray& bodies, BodyTrackerArray& outTrackers, UpdateState& outState, bool doFullRebuild);
        void UpdateFinalize(const BodyArray& bodies, const BodyTrackerArray& trackers, const UpdateState& state);

        void AddBodiesPrepare(const BodyArray& bodies, BodyTrackerArray& trackers, BodyID* bodyIDArray, const int number, AddState& outState);
        void AddBodiesFinalize(BodyTrackerArray& trackers, int numBodies, const AddState& state);
        void AddBodiesAbort(BodyTrackerArray& trackers, const AddState& state);
        void RemoveBodies(const BodyArray& bodies, BodyTrackerArray& trackers, const BodyID* bodyIDArray, const int number);
        void NotifyBodiesAABBChanged(const BodyArray& bodies, const BodyTrackerArray& trackers, const BodyID* bodyIDArray, int number);
        
        void CastRay(const RayCast& ray, RayCastBodyCollector& collector, const CollisionLayerFilter& layerFilter, const BodyTrackerArray& trackers) const;
        void CastAABox(const AABoxCast& box, CastShapeBodyCollector& collector, const CollisionLayerFilter& layerFilter, const BodyTrackerArray& trackers) const;
        void CollideAABox(const AABox& box, CollideShapeBodyCollector& collector, const CollisionLayerFilter& layerFilter, const BodyTrackerArray& trackers) const;
        void CollideSphere(const Vector3& center, const float radius, CollideShapeBodyCollector& collector, const CollisionLayerFilter& layerFilter, const BodyTrackerArray& trackers) const;
        void CollidePoint(const Vector3& point, CollideShapeBodyCollector& collector, const CollisionLayerFilter& layerFilter, const BodyTrackerArray& trackers) const;
        void CollideOrientedBox(const OrientedBox& box, CollideShapeBodyCollector& collector, const CollisionLayerFilter& layerFilter, const BodyTrackerArray& trackers) const;
        void FindCollidingPairs(const BodyArray& bodies, const BodyID* activeBodiesArray, const int numActiveBodies, float speculativeContactDistance, BodyPairCollector& collector, const CollisionLayerPairFilter& layerFilter) const;
        
        AABox GetBounds() const;
        
        /// Check to see if there are any Bodies in the Tree.
        bool HasBodies() const      { return m_numBodies != 0; }
        bool IsDirty() const        { return m_isDirty; }
        bool CanBeUpdated() const	{ return m_freeNodeBatch.m_numObjects == 0; }
    
    private:
        void GetBodyLocation(const BodyTrackerArray& trackers, BodyID bodyID, uint32_t& outNodeIndex, uint32_t& outChildIndex) const;
        void SetBodyLocation(BodyTrackerArray& trackers, BodyID bodyID, uint32_t nodeIndex, uint32_t childIndex) const;
        static void InvalidateBodyLocation(BodyTrackerArray& trackers, BodyID bodyID);

        /// Get the current root of the Tree. We manage two trees, with one being readonly. This returns the
        /// writable Root.
        const RootNode& GetCurrentRoot() const { return m_rootNodes[m_rootNodeIndex]; }
        RootNode&       GetCurrentRoot()       { return m_rootNodes[m_rootNodeIndex]; }

        AABox GetNodeOrBodyBounds(const BodyArray& bodies, NodeID nodeID) const;
        void MarkNodeAndParentsChanged(uint32_t nodeIndex);
        void WidenAndMarkNodeAndParentsChanged(uint32_t nodeIndex, const AABox& newBounds);

        uint32_t    AllocateNode(bool isChanged);
        bool        TryInsertLeaf(BodyTrackerArray& trackers, int nodeIndex, NodeID leafID, const AABox& leafBounds, int numLeafBodies);
        bool        TryCreateNewRoot(BodyTrackerArray& trackers, std::atomic<uint32_t>& rootNodeIndex, NodeID leafID, const AABox& leafBounds, int numLeafBodies);
        NodeID      BuildTree(const BodyArray& bodies, BodyTrackerArray& trackers, NodeID* nodeIDArray, int number, unsigned int maxDepthMarkChanged, AABox& outBounds);
        
        static void Partition(NodeID* nodeIDs, Vector3* nodeCenters, int number, int& outMidPoint);
        static void Partition4(NodeID* nodeIDs, Vector3* nodeCenters, int begin, int end, int* outSplitIndices);
        
        uint32_t GetMaxTreeDepth(const NodeID nodeID) const;

        template <typename Visitor>
        NES_INLINE void WalkTree(const CollisionLayerFilter& layerFilter, const BodyTrackerArray& trackers, Visitor& visitor) const;

#ifdef NES_DEBUG
        void ValidateTree(const BodyArray& bodies, const BodyTrackerArray& trackers, uint32_t nodeIndex, uint32_t numExpectedBodies) const;
#endif
    };
}
