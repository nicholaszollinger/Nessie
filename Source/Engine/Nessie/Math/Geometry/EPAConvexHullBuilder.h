// EPAConvexHullBuilder.h
#pragma once
#include <vector>
#include "Core/StaticArray.h"
#include "Math/Vector3.h"

namespace nes
{
    class EPAConvexHullBuilder
    {
    public:
        struct Triangle;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Struct that holds the information of an Edge. 
        //----------------------------------------------------------------------------------------------------
        struct Edge
        {
            Triangle*   m_pNeighborTriangle;    /// Triangle that neighbors this triangle.
            int         m_neighborEdge;        /// Index in m_edge that specifies which Edge this is connected to.
            int         m_startIndex;           /// Vertex index in m_positions that indicates the start vertex of this Edge.
        };

        // Due to the Euler characteristic (https://en.wikipedia.org/wiki/Euler_characteristic) we know that Vertices - Edges + Faces = 2
        // In our case we only have triangles and they are always fully connected, so each edge is shared exactly between 2 faces: Edges = Faces * 3 / 2
        // Substituting: Vertices = Faces / 2 + 2 which is approximately Faces / 2.
        static constexpr int kMaxTriangles = 256;               /// Max triangles in the hull.
        static constexpr int kMaxPoints = 256 / 2;              /// Max number of points in the hull.
        static constexpr int kMaxEdgeLength = 128;              /// Max number of edges in FindEdge()
        static constexpr float kMinTriangleArea = 1.0e-10f;     /// Minimum area of a triangle before, if smaller than this it will not be added to the priority queue.
        static constexpr float kBarycentricEpsilon = 1.0e-3f;   /// Epsilon value used to determine if a point is in the interior of a triangle.
        
        using Edges = StaticArray<Edge, kMaxEdgeLength>; 
        using NewTriangles = StaticArray<Triangle*, kMaxEdgeLength>;

        struct Triangle
        {
            Edge    m_edges[3];
            Vector3 m_normal;
            Vector3 m_centroid;
            float   m_closestLengthSqr = FLT_MAX;
            float   m_lambda[2];
            bool    m_lambdaRelativeTo0;
            bool    m_closestPointInterior = false;
            bool    m_isRemoved = false;
            bool    m_inQueue = false;
            
            Triangle() = default;
            inline Triangle(int index0, int index1, int index2, const Vector3* pPositions);
            Triangle(const Triangle&) = delete;
            Triangle& operator=(const Triangle&) = delete;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Check if a triangle is facing 'position'. 
            //----------------------------------------------------------------------------------------------------
            inline bool IsFacing(const Vector3& position) const
            {
                NES_ASSERT(!m_isRemoved);
                return m_normal.Dot(position - m_centroid) > 0.0f;
            }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Check if the triangle is facing the origin. 
            //----------------------------------------------------------------------------------------------------
            inline bool IsFacingOrigin() const
            {
                NES_ASSERT(!m_isRemoved);
                return m_normal.Dot(m_centroid) > 0.0f;
            }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Get the next edge of edge index.
            //----------------------------------------------------------------------------------------------------
            inline const Edge& GetNextEdge(const int index) const { return m_edges[(index + 1) % 3]; } 
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Factory that creates triangles in a fixed sized buffer. 
        //----------------------------------------------------------------------------------------------------
        class TriangleFactory
        {
            //----------------------------------------------------------------------------------------------------
            /// @brief : Struct that stores both a triangle or a next pointer in case the triangle is unused. 
            //----------------------------------------------------------------------------------------------------
            union alignas(Triangle) Block
            {
                uint8_t m_triangle[sizeof(Triangle)];
                Block*  m_pNextFreeBlock;
            };

            Block   m_triangles[kMaxTriangles]; /// Storage for triangles.
            Block*  m_pNextFree = nullptr;      /// List of free triangles.
            int     m_highWatermark = 0;        /// High water-mark for used triangles. (If m_pNextFree = nullptr, we can take one from here).
        public:
            TriangleFactory() = default;
            TriangleFactory(const TriangleFactory&) = delete;
            TriangleFactory& operator=(const TriangleFactory&) = delete;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Return all triangles to the free pool. 
            //----------------------------------------------------------------------------------------------------
            void Clear();

            //----------------------------------------------------------------------------------------------------
            /// @brief : Allocate a new triangle with 3 indices. 
            //----------------------------------------------------------------------------------------------------
            Triangle* CreateTriangle(int index0, int index1, int index2, const Vector3* pPositions);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Free a triangle.
            //----------------------------------------------------------------------------------------------------
            void FreeTriangle(Triangle* pTriangle);
        };
        
        using PointsBase = StaticArray<Vector3, kMaxPoints>;
        using Triangles = StaticArray<Triangle*, kMaxTriangles>;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Specialized points lists that allows direct access to the size. 
        //----------------------------------------------------------------------------------------------------
        class Points : public PointsBase
        {
        public:
            size_type& GetSizeRef()
            {
                return m_size;
            }
        };

        //----------------------------------------------------------------------------------------------------
        //	NOTES:
        //  Can I just use a priority queue and be done with it???
        //		
        /// @brief : Specialized triangles array that keeps them sorted on closest distance to the origin. 
        //----------------------------------------------------------------------------------------------------
        class TriangleQueue : public Triangles
        {
        public:
            //----------------------------------------------------------------------------------------------------
            /// @brief : Function to sort triangles on closest distance to the origin. 
            //----------------------------------------------------------------------------------------------------
            static bool TriangleSorter(const Triangle* pTri1, const Triangle* pTri2);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Add a triangle to the list.
            //----------------------------------------------------------------------------------------------------
            void push_back(Triangle* pTri);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Peek the closest triangle without removing it. 
            //----------------------------------------------------------------------------------------------------
            Triangle* PeekClosest() { return front(); }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Get the next closest triangle. 
            //----------------------------------------------------------------------------------------------------
            Triangle* PopClosest();
        };

    private:
        TriangleFactory m_factory{};    /// Factory to create new triangles and remove old ones.
        const Points&   m_points;       /// List of positions (some of them are part of the hull.
        TriangleQueue   m_queue{};      /// List of triangles that are part of the hull that still need to be checked (if !m_isRemoved).
        
    public:
        explicit EPAConvexHullBuilder(const Points& positions) : m_points(positions) { }
        EPAConvexHullBuilder(const EPAConvexHullBuilder&) = delete;
        EPAConvexHullBuilder& operator=(const EPAConvexHullBuilder&) = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the hull with 3 points.
        //----------------------------------------------------------------------------------------------------
        void        Initialize(const int index1, const int index2, const int index3);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if there's another triangle to process from the queue. 
        //----------------------------------------------------------------------------------------------------
        bool        HasNextTriangle() const                     { return !m_queue.empty(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access to the next closest triangle to the origin (won't remove it from the queue). 
        //----------------------------------------------------------------------------------------------------
        Triangle*   PeekClosestTriangleInQueue()                { return m_queue.PeekClosest(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Access to the next closest triangle to the origin and removes it from the queue. 
        //----------------------------------------------------------------------------------------------------
        Triangle*   PopClosestTriangleFromQueue()               { return m_queue.PopClosest(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Find the triangle on which position is the furthest to the front.
        /// @note : This function works as long as all points have been added with AddPoint(..., FLT_MAX).
        //----------------------------------------------------------------------------------------------------
        Triangle*   FindFacingTriangle(const Vector3& position, float& outBestDistSqr);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a new point to the convex hull.
        //----------------------------------------------------------------------------------------------------
        bool        AddPoint(Triangle* pFacingTriangle, int index, float closestDistSqr, NewTriangles& outTriangles);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free a triangle. 
        //----------------------------------------------------------------------------------------------------
        void        FreeTriangle(Triangle* pTriangle);

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a new triangle using the 3 indices into the m_points array.
        //----------------------------------------------------------------------------------------------------
        Triangle*   CreateTriangle(const int index1, const int index2, const int index3);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Link triangle edge to another triangle edge.
        //----------------------------------------------------------------------------------------------------
        static void LinkTriangle(Triangle* pTri1, int edge1, Triangle* pTri2, int edge2);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Unlink this triangle. 
        //----------------------------------------------------------------------------------------------------
        void        UnlinkTriangle(Triangle* pTri);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Given one triangle that faces 'vertex', find the edges of the triangles that are not facing 'vertex'.
        ///     This will flag all those triangles for removal.
        //----------------------------------------------------------------------------------------------------
        bool        FindEdge(Triangle* pFacingTriangle, const Vector3& vertex, Edges& outEdges);
    };
}
