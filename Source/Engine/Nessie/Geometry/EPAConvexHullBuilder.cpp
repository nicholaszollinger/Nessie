// EPAConvexHullBuilder.cpp
#include "EPAConvexHullBuilder.h"
#include "Core/BinaryHeap.h"

namespace nes
{
    // The determinant that is calculated in the Triangle constructor is really sensitive
    // to numerical round off, disable the fma instructions to maintain precision.
    NES_PRECISE_MATH_BEGIN
    
    EPAConvexHullBuilder::Triangle::Triangle(int index0, int index1, int index2, const Vec3* pPositions)
    {
        // Fill in indexes
        NES_ASSERT(index0 != index1 && index0 != index2 && index1 != index2);
        m_edges[0].m_startIndex = index0;
        m_edges[1].m_startIndex = index1;
        m_edges[2].m_startIndex = index2;

        // Clear links
        m_edges[0].m_pNeighborTriangle = nullptr;
        m_edges[1].m_pNeighborTriangle = nullptr;
        m_edges[2].m_pNeighborTriangle = nullptr;

        // Get vertex positions
        Vec3 v0 = pPositions[index0];
        Vec3 v1 = pPositions[index1];
        Vec3 v2 = pPositions[index2];

        // Calculate the centroid
        m_centroid = (v0 + v1 + v2) / 3.0f;

        // Calculate edges
        Vec3 e10 = v1 - v0;
        Vec3 e20 = v2 - v0;
        Vec3 e21 = v2 - v1;

        // The most accurate normal is calculated by using the two shortest edges.
        // See: https://box2d.org/posts/2014/01/troublesome-triangle/
        // The difference in normals is most pronounced when one edge is much smaller than the others (in which case the other 2 must have roughly the same length).
        // Therefore, we can suffice by just picking the shortest from 2 edges and use that with the 3rd edge to calculate the normal.
        // We first check which of the edges is shorter.
        float e20Dote20 = e20.Dot(e20);
        float e21Dote21 = e21.Dot(e21);
        if (e20Dote20 < e21Dote21)
        {
            // We select the edges e10 and e20
            m_normal = e10.Cross(e20);

            // Check if the triangle is degenerate
            const float normalLenSqr = m_normal.LengthSqr();
            if (normalLenSqr > kMinTriangleArea)
            {
                // Determine distance between triangle and origin: distance = (centroid - origin) . normal / |normal|
                // Note that this way of calculating the closest point is much more accurate than first calculating barycentric coordinates and then calculating the closest
                // point based on those coordinates. Note that we preserve the sign of the distance to check on which side the origin is.
                const float cDotN = m_centroid.Dot(m_normal);
                m_closestLengthSqr = math::Abs(cDotN) * cDotN / normalLenSqr;

                // Calculate closest point to origin using barycentric coordinates:
                //
                // v = y0 + l0 * (y1 - y0) + l1 * (y2 - y0)
                // v . (y1 - y0) = 0
                // v . (y2 - y0) = 0
                //
                // Written in matrix form:
                //
                // | y10.y10  y20.y10 | | l0 | = | -y0.y10 |
                // | y10.y20  y20.y20 | | l1 |   | -y0.y20 |
                //
                // (y10 = y1 - y0 etc.)
                //
                // Cramers rule to invert matrix:
                const float e10Dote10 = e10.LengthSqr();
                const float e10Dote20 = e10.Dot(e20);
                const float determinant = e10Dote10 * e20Dote20 - e10Dote20 * e10Dote20;

                // If the determinant == 0 then the system is linearly dependent and the triangle is degenerate.
                // Since y10.y10 * y20.y20 > y10.y20^2 it should also be > 0
                if (determinant > 0.f)
                {
                    float v0Dote10 = v0.Dot(e10);
                    float v0Dote20 = v0.Dot(e20);
                    float l0 = (e10Dote20 * v0Dote20 - e20Dote20 * v0Dote10) / determinant;
                    float l1 = (e10Dote20 * v0Dote10 - e10Dote10 * v0Dote20) / determinant;
                    m_lambda[0] = l0;
                    m_lambda[1] = l1;
                    m_lambdaRelativeTo0 = true;

                    // Check if closest point is interior to the triangle. For a convex hull which contains the origin each face must contain the origin, but because
                    // our faces are triangles, we can have multiple coplanar triangles and only 1 will have the origin as an interior point. We want to use this triangle
                    // to calculate the contact points because it gives the most accurate results, so we will only add these triangles to the priority queue.
                    if (l0 > -kBarycentricEpsilon && l1 > -kBarycentricEpsilon && l0 + l1 < 1.0f + kBarycentricEpsilon)
                        m_closestPointInterior = true;
                }
            }
        }
        
        else
        {
            // We select the edges e10 and e21
            m_normal = e10.Cross(e21);

            // Check if triangle is degenerate
            float normalLenSqr = m_normal.LengthSqr();
            if (normalLenSqr > kMinTriangleArea)
            {
                // Again calculate distance between triangle and origin
                const float cDotN = m_centroid.Dot(m_normal);
                m_closestLengthSqr = abs(cDotN) * cDotN / normalLenSqr;

                // Calculate closest point to origin using barycentric coordinates but this time using y1 as the reference vertex
                //
                // v = y1 + l0 * (y0 - y1) + l1 * (y2 - y1)
                // v . (y0 - y1) = 0
                // v . (y2 - y1) = 0
                //
                // Written in matrix form:
                //
                // | y10.y10  -y21.y10 | | l0 | = | y1.y10 |
                // | -y10.y21  y21.y21 | | l1 |   | -y1.y21 |
                //
                // Cramers rule to invert matrix:
                const float e10Dote10 = e10.LengthSqr();
                const float e10Dote21 = e10.Dot(e21);
                const float determinant = e10Dote10 * e21Dote21 - e10Dote21 * e10Dote21;
                if (determinant > 0.0f)
                {
                    const float v1Dote10 = v1.Dot(e10);
                    const float v1Dote21 = v1.Dot(e21);
                    const float l0 = (e21Dote21 * v1Dote10 - e10Dote21 * v1Dote21) / determinant;
                    const float l1 = (e10Dote21 * v1Dote10 - e10Dote10 * v1Dote21) / determinant;
                    m_lambda[0] = l0;
                    m_lambda[1] = l1;
                    m_lambdaRelativeTo0 = false;

                    // Again check if the closest point is inside the triangle
                    if (l0 > -kBarycentricEpsilon && l1 > -kBarycentricEpsilon && l0 + l1 < 1.0f + kBarycentricEpsilon)
                        m_closestPointInterior = true;
                }
            }
        }
    }

    NES_PRECISE_MATH_END

    inline void EPAConvexHullBuilder::TriangleFactory::Clear()
    {
        m_pNextFree = nullptr;
        m_highWatermark = 0;
    }

    EPAConvexHullBuilder::Triangle* EPAConvexHullBuilder::TriangleFactory::CreateTriangle(const int index0, const int index1, const int index2, const Vec3* pPositions)
    {
        Triangle* pResult;
        if (m_pNextFree != nullptr)
        {
            // Entry available in the free list
            pResult = reinterpret_cast<Triangle*>(&m_pNextFree->m_triangle);
            m_pNextFree = m_pNextFree->m_pNextFree;
        }
        else
        {
            // Allocate from never used before triangle storage
            if (m_highWatermark >= kMaxTriangles)
                return nullptr; // Buffer full.
            
            pResult = reinterpret_cast<Triangle*>(&m_triangles[m_highWatermark].m_triangle);
            ++m_highWatermark;
        }

        // Call constructor
        new (pResult) Triangle(index0, index1, index2, pPositions);
        return pResult;
    }

    void EPAConvexHullBuilder::TriangleFactory::FreeTriangle(Triangle* pTriangle)
    {
        // Destruct triangle
        pTriangle->~Triangle();
#ifdef NES_DEBUG
        memset(pTriangle, 0xcd, sizeof(Triangle));
#endif

        // Add Triangle to the free list
        Block* pBlock = reinterpret_cast<Block*>(pTriangle);
        pBlock->m_pNextFree = m_pNextFree;
        m_pNextFree = pBlock;
    }

    inline bool EPAConvexHullBuilder::TriangleQueue::TriangleSorter(const Triangle* pTri1, const Triangle* pTri2)
    {
        return pTri1->m_closestLengthSqr > pTri2->m_closestLengthSqr;
    }

    void EPAConvexHullBuilder::TriangleQueue::push_back(Triangle* pTri)
    {
        // Add to base
        Triangles::push_back(pTri);

        // Mark in queue
        pTri->m_inQueue = true;

        // Resort Heap
        BinaryHeapPush(begin(), end(), TriangleSorter);
    }

    EPAConvexHullBuilder::Triangle* EPAConvexHullBuilder::TriangleQueue::PopClosest()
    {
        // Move the closest to the end.
        BinaryHeapPop(begin(), end(), TriangleSorter);

        // Remove the last triangle.
        Triangle* pResult = back();
        pop_back();
        return pResult;
    }

    void EPAConvexHullBuilder::Initialize(const int index1, const int index2, const int index3)
    {
        // Release triangles
        m_factory.Clear();

        // Create triangles, back to back
        Triangle* pTri1 = CreateTriangle(index1, index2, index3);
        Triangle* pTri2 = CreateTriangle(index1, index3, index2);

        // Link Triangle edges
        LinkTriangle(pTri1, 0, pTri2, 2);
        LinkTriangle(pTri1, 1, pTri2, 1);
        LinkTriangle(pTri1, 2, pTri2, 0);

        // Always add both triangles to the priority queue
        m_queue.push_back(pTri1);
        m_queue.push_back(pTri2);
    }

    EPAConvexHullBuilder::Triangle* EPAConvexHullBuilder::FindFacingTriangle(const Vec3& position,
        float& outBestDistSqr)
    {
        Triangle* pBest = nullptr;
        float bestDistSqr = 0.f;

        for (Triangle* pTri : m_queue)
        {
            if (!pTri->m_isRemoved)
            {
                const float dot = pTri->m_normal.Dot(position - pTri->m_centroid);
                if (dot > 0.f)
                {
                    const float distSqr = dot * dot / pTri->m_normal.LengthSqr();
                    if (distSqr > bestDistSqr)
                    {
                        pBest = pTri;
                        bestDistSqr = distSqr;
                    }
                }
            }
        }
        
        outBestDistSqr = bestDistSqr;
        return pBest;
    }

    bool EPAConvexHullBuilder::AddPoint(Triangle* pFacingTriangle, int index, float closestDistSqr, NewTriangles& outTriangles)
    {
        NES_ASSERT(static_cast<size_t>(index) < m_points.capacity());
        Vec3 position = m_points[index];

        // Find edge of convex hull of triangles that are not facing the new vertex w
        Edges edges;
        if (!FindEdge(pFacingTriangle, position, edges))
            return false;

        // Create new Triangles
        size_t numEdges = edges.size();
        for (size_t i = 0; i < numEdges; ++i)
        {
            // Create new triangle
            Triangle* pNew = CreateTriangle(edges[i].m_startIndex, edges[(i + 1) % numEdges].m_startIndex, index);
            if (pNew == nullptr)
                return false;
            outTriangles.push_back(pNew);

            // Check if we need to put this triangle in the priority queue.
            if ((pNew->m_closestPointInterior && pNew->m_closestLengthSqr < closestDistSqr) // For the main algorithm.
                || pNew->m_closestLengthSqr < 0.f)                                          // For when the origin is not inside the hull yet.
            {
                m_queue.push_back(pNew);
            }
        }

        // Link the edges
        for (size_t i = 0; i < numEdges; ++i)
        {
            LinkTriangle(outTriangles[i], 0, edges[i].m_pNeighborTriangle, edges[i].m_neighborEdge);
            LinkTriangle(outTriangles[i], 1, outTriangles[(i + 1) % numEdges], 2);
        }

        return true;
    }

    void EPAConvexHullBuilder::FreeTriangle(Triangle* pTriangle)
    {
#if NES_ASSERTS_ENABLED
        NES_ASSERT(pTriangle->m_isRemoved);
        for (const Edge& edge : pTriangle->m_edges)
        {
            NES_ASSERT(edge.m_pNeighborTriangle == nullptr);
        }
#endif

        m_factory.FreeTriangle(pTriangle);
    }

    EPAConvexHullBuilder::Triangle* EPAConvexHullBuilder::CreateTriangle(const int index1, const int index2, const int index3)
    {
        Triangle* pResult = m_factory.CreateTriangle(index1, index2, index3, m_points.data());
        if (pResult == nullptr)
            return nullptr;

        return pResult;
    }

    void EPAConvexHullBuilder::LinkTriangle(Triangle* pTri1, int edge1, Triangle* pTri2, int edge2)
    {
        NES_ASSERT(edge1 >= 0 && edge1 < 3);
        NES_ASSERT(edge2 >= 0 && edge2 < 3);
        Edge& e1 = pTri1->m_edges[edge1];
        Edge& e2 = pTri2->m_edges[edge2];

        // Check not connected yet
        NES_ASSERT(e1.m_pNeighborTriangle == nullptr);
        NES_ASSERT(e2.m_pNeighborTriangle == nullptr);

        // Check vertices match
        NES_ASSERT(e1.m_startIndex == pTri2->GetNextEdge(edge2).m_startIndex);
        NES_ASSERT(e2.m_startIndex == pTri1->GetNextEdge(edge1).m_startIndex);

        // Link
        e1.m_pNeighborTriangle = pTri2;
        e1.m_neighborEdge = edge2;
        e2.m_pNeighborTriangle = pTri1;
        e2.m_neighborEdge = edge1;
    }

    void EPAConvexHullBuilder::UnlinkTriangle(Triangle* pTri)
    {
        // Unlink from neighbors
        for (int i = 0; i < 3; ++i)
        {
            Edge& edge = pTri->m_edges[i];
            if (edge.m_pNeighborTriangle != nullptr)
            {
                Edge& neighborEdge = edge.m_pNeighborTriangle->m_edges[edge.m_neighborEdge];

                // Validate that the neighbor points to us
                NES_ASSERT(neighborEdge.m_pNeighborTriangle == pTri);
                NES_ASSERT(neighborEdge.m_neighborEdge == i);

                // Unlink
                neighborEdge.m_pNeighborTriangle = nullptr;
                edge.m_pNeighborTriangle = nullptr;
                
            }
        }

        // If this triangle is not in the priority queue, we can delete it now
        if (!pTri->m_inQueue)
            FreeTriangle(pTri);
    }

    bool EPAConvexHullBuilder::FindEdge(Triangle* pFacingTriangle, const Vec3& vertex, Edges& outEdges)
    {
        // Assert we were given an empty array.
        NES_ASSERT(outEdges.empty());

        // Should start with a facing triangle
        NES_ASSERT(pFacingTriangle->IsFacing(vertex));

        // Flag as removed
        pFacingTriangle->m_isRemoved = true;

        // Instead of recursing, we build our own stack with the info we need
        struct StackEntry
        {
            Triangle*   m_pTriangle;
            int         m_edge;
            int         m_iter;
        };
        StackEntry stack[kMaxEdgeLength];
        int currentStackPos = 0;

        // Start with the triangle / edge provided
        stack[0].m_pTriangle = pFacingTriangle;
        stack[0].m_edge = 0;
        stack[0].m_iter = -1; // Start with edge 0 (is incremented below before use).

        // Next index that we expect to find, if we don't then there are 'islands'.
        int nextExpectedStartIndex = -1;

        for (;;)
        {
            StackEntry& currentEntry = stack[currentStackPos];

            // Next iteration
            if (++currentEntry.m_iter >= 3)
            {
                // This triangle needs to be removed, unlink for now
                UnlinkTriangle(currentEntry.m_pTriangle);

                // Pop from the stack
                if (--currentStackPos < 0)
                    break;
            }
            else
            {
                // Visit neighbor
                Edge& edge = currentEntry.m_pTriangle->m_edges[(currentEntry.m_edge + currentEntry.m_iter) % 3];
                Triangle* pNeighbor = edge.m_pNeighborTriangle;
                if (pNeighbor != nullptr && !pNeighbor->m_isRemoved)
                {
                    // Check if the vertex is on the front side of this triangle
                    if (pNeighbor->IsFacing(vertex))
                    {
                        // Vertex on front, this triangle needs to be removed
                        pNeighbor->m_isRemoved = true;

                        // Add the element to the stack of elements to visit
                        ++currentStackPos;
                        NES_ASSERT(currentStackPos < kMaxEdgeLength);
                        StackEntry& newEntry = stack[currentStackPos];
                        newEntry.m_pTriangle = pNeighbor;
                        newEntry.m_edge = edge.m_neighborEdge;
                        newEntry.m_iter = 0; // Is incremented before use, we don't need to test this edge again since we came from it.
                    }
                    else
                    {
                        // Detect if edge doesn't connect to previous edge, if this happens we have found and 'island' which means
                        // the newly added point is so close to the triangles of the hull that we classified some (nearly) coplanar
                        // triangles as before and some behind the point. At this point we just abort adding the point because
                        // we've reached numerical precision.
                        // Note that we do not need to test if the first and last edge connect, since when there are islands
                        // there should be at least 2 disconnects.
                        if (edge.m_startIndex != nextExpectedStartIndex && nextExpectedStartIndex != -1)
                            return false;

                        // The next expected index is the start index of our neighbor's edge
                        nextExpectedStartIndex = pNeighbor->m_edges[edge.m_neighborEdge].m_startIndex;

                        // Vertex behind, keep edge
                        outEdges.push_back(edge);
                    }
                }
            }
        }

        // Assert that we have a fully connected loop
        NES_ASSERT(outEdges.empty() || outEdges[0].m_startIndex == nextExpectedStartIndex);

        // When we start with two triangles facing away from each other and adding a point that is on the plane,
        // we sometimes consider the point in front of both causing both triangles to be removed This results in an empty edge-list.
        // In this case, we fail to add the point which will result in no collision reported (the shapes are contacting in 1 point so there's 0 penetration).
        return outEdges.size() >= 3;
    }
}
