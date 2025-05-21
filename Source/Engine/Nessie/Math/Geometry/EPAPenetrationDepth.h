// EPAPenetrationDepth.h
#pragma once
#include "GJKClosestPoint.h"
#include "EPAConvexHullBuilder.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Implementation of Expanding Polytope Algorithm as described in:
    ///
    /// Proximity Queries and Penetration Depth Computation on 3D Game Objects - Gino van den Bergen
    ///
    /// The implementation of this algorithm does not completely follow the article, instead of splitting
    /// triangles at each edge as in fig. 7 in the article, we build a convex hull (removing any triangles that
    /// are facing the new point, thereby avoiding the problem of getting really oblong triangles as mentioned in
    /// the article).
    ///
    /// The algorithm roughly works like:
    ///
    /// - Start with a simplex of the Minkowski sum (difference) of two objects that was calculated by GJK
    /// - This simplex should contain the origin (or else GJK would have reported: no collision)
    /// - In cases where the simplex consists of 1 - 3 points, find some extra support points (of the Minkowski sum) to get to at least 4 points
    /// - Convert this into a convex hull with non-zero volume (which includes the origin)
    /// - A: Calculate the closest point to the origin for all triangles of the hull and take the closest one
    /// - Calculate a new support point (of the Minkowski sum) in this direction and add this point to the convex hull
    /// - This will remove all faces that are facing the new point and will create new triangles to fill up the hole
    /// - Loop to A until no closer point found
    /// - The closest point indicates the position / direction of the least penetration
    //----------------------------------------------------------------------------------------------------
    class EPAPenetrationDepth
    {
        static constexpr int kMaxPoints = EPAConvexHullBuilder::kMaxPoints;
        static constexpr int kMaxPointsToIncludeOriginInHull = 32;
        static_assert(kMaxPointsToIncludeOriginInHull < kMaxPoints);

        using Triangle = EPAConvexHullBuilder::Triangle;
        using Points = EPAConvexHullBuilder::Points;

        //----------------------------------------------------------------------------------------------------
        /// @brief : A list of support points for the EPA algorithm. 
        //----------------------------------------------------------------------------------------------------
        struct SupportPoints
        {
            // List of Support Points
            Points  m_y;
            Vector3 m_p[kMaxPoints];
            Vector3 m_q[kMaxPoints];

            template <ValidConvexObjectType A, ValidConvexObjectType B>
            Vector3 Add(const A& inA, const B& inB, const Vector3& direction, int& outIndex)
            {
                // Get the support point of the minkowski sum A - B
                Vector3 p = inA.GetSupport(direction);
                Vector3 q = inB.GetSupport(-direction);
                Vector3 w = p - q;

                // Store the new point
                outIndex = static_cast<int>(m_y.size());
                m_y.push_back(w);
                m_p[outIndex] = p;
                m_q[outIndex] = q;
                return w;
            }
        };

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Return code for GetPenetrationDepthStepGJK() 
        //----------------------------------------------------------------------------------------------------
        enum class Status : uint8_t
        {
            NotColliding,       /// Returned if the objects don't collide, in this cast outPointA/outPointB are invalid. 
            Colliding,          /// Returned if the objects penetrate.
            Indeterminate,      /// Returned if the objects penetrate further than the convex radius. In this case you need to call GetPenetrationDepthStepEPA() to get the actual penetration depth.
        };
        
    private:
        /// The GJK Algorithm, used to start the EPA algorithm
        GJKClosestPoint m_gjk;

#ifdef NES_LOGGING_ENABLED
        /// Tolerance as passed to the GJK algorithm, used for asserting.
        float           m_gjkTolerance = 0.f;
#endif

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates penetration depth between two objects, first step of two (the GJK step)
        ///     Use |outPointB - outPointA| to get the distance of penetration.
        ///	@param inAExcludingConvexRadius : Object A without convex radius.
        ///	@param inBExcludingConvexRadius : Object B without convex radius.
        ///	@param convexRadiusA : Convex radius for A.
        ///	@param convexRadiusB : Convex radius for B.
        ///	@param tolerance :  Minimal distance before A and B are considered colliding.
        ///	@param ioV : Pass in previously returned value or (1, 0, 0). On return this value is changed to direction to move B out of collision along the shortest path (magnitude is meaningless).
        ///	@param outPointA : Position on A that has the least amount of penetration.
        ///	@param outPointB : Position on B that has the least amount of penetration.
        //----------------------------------------------------------------------------------------------------
        template <ValidConvexObjectType A, ValidConvexObjectType B>
        Status          GetPenetrationDepthStepGJK(const A& inAExcludingConvexRadius, float convexRadiusA, const B& inBExcludingConvexRadius, float convexRadiusB, float tolerance, Vector3& ioV, Vector3& outPointA, Vector3& outPointB)
        {
            NES_IF_LOGGING_ENABLED(m_gjkTolerance = tolerance);

            // Don't supply a zero ioV, we only want to get points on the hull of the Minkowski sum and not internal points.
            //
            // Note that if the assert below triggers, it is very likely that you have a MeshShape that contains a degenerate triangle (e.g. a sliver).
            // Go up a couple of levels in the call stack to see if we're indeed testing a triangle and if it is degenerate.
            // If this is the case then fix the triangles you supply to the MeshShape.
            NES_ASSERT(!ioV.IsNearZero());

            // Get the closest points
            const float combinedRadius = convexRadiusA + convexRadiusB;
            const float combinedRadiusSqr = combinedRadius * combinedRadius;
            const float closestPointsDistSqr = m_gjk.GetClosestPoints(inAExcludingConvexRadius, inBExcludingConvexRadius, tolerance, combinedRadiusSqr, ioV, outPointA, outPointB);
            if (closestPointsDistSqr > combinedRadiusSqr)
            {
                // No Collision
                return Status::NotColliding;
            }
            if (closestPointsDistSqr > 0.f)
            {
                // Collision within convex radius, adjust points for convex radius
                float vLength = std::sqrt(closestPointsDistSqr); // GetClosestPoints() function returns |ioV|^2 when return value < FLT_MAX
                outPointA += ioV * (convexRadiusA / vLength);
                outPointB -= ioV * (convexRadiusB / vLength);
                return Status::Colliding;
            }

            return Status::Indeterminate;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates penetration depth between two objects, second step (the EPA step)
        ///     Use |outPointB - outPointA| to get the distance of penetration.
        ///	@param inAIncludingConvexRadius : Object A with convex radius
        ///	@param inBIncludingConvexRadius : Object B with convex radius
        ///	@param tolerance : A factor that determines the accuracy of the result. If the change of the squared distance is less than tolerance * currentPenetrationDepth^2 the algorithm will terminate. Should be bigger or equal to FLT_EPSILON.
        ///	@param outV : Direction to move B out of collision along the shortest path (magnitude is meaningless).
        ///	@param outPointA : Position on A that has the least amount of penetration.
        ///	@param outPointB : Position on B that has the least amount of penetration.
        ///	@returns : False if the objects don't collide, in this case outPointA/outPointB are invalid.
        ///     True if the objects penetrate.
        //----------------------------------------------------------------------------------------------------
        template <ValidConvexObjectType A, ValidConvexObjectType B>
        bool            GetPenetrationDepthStepEPA(const A& inAIncludingConvexRadius, const B& inBIncludingConvexRadius, float tolerance, Vector3& outV, Vector3& outPointA, Vector3& outPointB)
        {
            // Check that the tolerance makes sense (smaller value than this will just result in needless iterations)
            NES_ASSERT(tolerance >= FLT_EPSILON);

            // Fetch the simplex from GJK algorithm
            SupportPoints supportPoints;
            m_gjk.GetClosestPointsSimplex(supportPoints.m_y.data(), supportPoints.m_p, supportPoints.m_q, supportPoints.m_y.GetSizeRef());

            // Fill up the amount of support points to 4
            switch (supportPoints.m_y.size())
            {
                case 1:
                {
                    // 1 vertex, which must be at the origin, which is useless for our purpose
                    NES_ASSERT(supportPoints.m_y[0].IsNearZero(math::Squared(m_gjkTolerance)));
                    supportPoints.m_y.pop_back();

                    // Add support points in 4 directions to form a tetrahedron around the origin
                    int p1, p2, p3, p4;
                    (void)supportPoints.Add(inAIncludingConvexRadius, inBIncludingConvexRadius, Vector3(0.f, 1.f, 0.f), p1);
                    (void)supportPoints.Add(inAIncludingConvexRadius, inBIncludingConvexRadius, Vector3(-1.f, -1.f, -1.f), p2);
                    (void)supportPoints.Add(inAIncludingConvexRadius, inBIncludingConvexRadius, Vector3(1.f, -1.f, -1.f), p3);
                    (void)supportPoints.Add(inAIncludingConvexRadius, inBIncludingConvexRadius, Vector3(0.f, -1.f, 1.f), p4);
                    NES_ASSERT(p1 == 0);
                    NES_ASSERT(p2 == 1);
                    NES_ASSERT(p3 == 2);
                    NES_ASSERT(p4 == 3);
                    break;
                }

                case 2:
                {
                    // Two vertices, create 3 extra by taking perpendicular axis and rotating it around in 120 degree increments.
                    Vector3 axis = (supportPoints.m_y[1] - supportPoints.m_y[0]).Normalized();
                    Mat4 rotation = math::ToMat4(Quat::MakeFromAngleAxis(math::DegreesToRadians() * 120.f, axis));
                    Vector3 dir1 = axis.GetNormalizedPerpendicular();
                    Vector3 dir2 = rotation.TransformPoint(dir1);
                    Vector3 dir3 = rotation.TransformPoint(dir2);
                    
                    int p1, p2, p3;
                    (void)supportPoints.Add(inAIncludingConvexRadius, inBIncludingConvexRadius, dir1, p1);
                    (void)supportPoints.Add(inAIncludingConvexRadius, inBIncludingConvexRadius, dir2, p2);
                    (void)supportPoints.Add(inAIncludingConvexRadius, inBIncludingConvexRadius, dir3, p3);
                    NES_ASSERT(p1 == 2);
                    NES_ASSERT(p2 == 3);
                    NES_ASSERT(p3 == 4);
                    break;
                }

                case 3:
                case 4:
                {
                    // We already have enough points.
                    break;
                }

                default:
                {
                    NES_ASSERT(false);
                    return false;
                }
            }
            
            // Create hull out of the initial points
            NES_ASSERT(supportPoints.m_y.size() >= 3);
            EPAConvexHullBuilder hull(supportPoints.m_y);
            hull.Initialize(0, 1, 2);
            for (Points::size_type i = 3; i < supportPoints.m_y.size(); ++i)
            {
                float distSqr;
                Triangle* pTri = hull.FindFacingTriangle(supportPoints.m_y[i], distSqr);
                if (pTri != nullptr)
                {
                    EPAConvexHullBuilder::NewTriangles newTriangles;
                    if (!hull.AddPoint(pTri, static_cast<int>(i), FLT_MAX, newTriangles))
                    {
                        // We can't recover from a failure to add a point to the hull because the old triangles have been unlinked already.
                        // Assume no collision. This can happen if the shapes touch in 1 point (or place) in while case the hull is degenerate.
                        return false;
                    }
                }
            }

            // Loop until we are sure that the origin is inside the hull
            for (;;)
            {
                // Get the next closest triangle
                Triangle* pTri = hull.PeekClosestTriangleInQueue();

                // Don't process the removed triangles, just free them (Because they're in a heap we don't remove them earlier since we would have to rebuild the
                // sorted heap
                if (pTri->m_isRemoved)
                {
                    hull.PopClosestTriangleFromQueue();

                    // If we run out of triangles, we couldn't include the origin in the hull so there must be very little penetration - we report no collision.
                    if (!hull.HasNextTriangle())
                        return false;

                    hull.FreeTriangle(pTri);
                    continue;
                }

                // If the closest to the triangle is zero or positive, the origin is in the hull and we can proceed to the main algorithm.
                if (pTri->m_closestLengthSqr >= 0.f)
                    break;

                // Remove the triangle from the queue before we start adding new ones (which may result in a new closest triangle at the front of the queue)
                hull.PopClosestTriangleFromQueue();

                // Add a support point to get the origin inside the hull
                int newIndex;
                Vector3 w = supportPoints.Add(inAIncludingConvexRadius, inBIncludingConvexRadius, pTri->m_normal, newIndex);

                // Add the point to the hull, if we fail we terminate and report no collision
                EPAConvexHullBuilder::NewTriangles newTriangles;
                if (!pTri->IsFacing(w) || !hull.AddPoint(pTri, newIndex, FLT_MAX, newTriangles))
                    return false;

                // The triangle is facing the support point "w" and can now be safely removed.
                NES_ASSERT(pTri->m_isRemoved);
                hull.FreeTriangle(pTri);

                // If we run out of triangles, we couldn't include the origin in the hull so there must be very little penetration - we report no collision.
                if (!hull.HasNextTriangle() || supportPoints.m_y.size() >= kMaxPointsToIncludeOriginInHull)
                    return false;
            }

            // Current closest distance to the origin
            float closestDistSqr = FLT_MAX;

            // Remember the last good triangle
            Triangle* pLast = nullptr;

            // If we want to flip the penetration depth
            bool flipVSign = false;

            // Loop until closest point found
            do
            {
                // Get the closest triangle to the origin
                Triangle* pTri = hull.PopClosestTriangleFromQueue();

                // Don't process removed triangles, just free them (because they're in a heap we don't remove them earlier since we would have to rebuild the sorted heap)
                if (pTri->m_isRemoved)
                {
                    hull.FreeTriangle(pTri);
                    continue;
                }

                // Check if next triangle is further away than the closest point, we've found the closest point
                if (pTri->m_closestLengthSqr >= closestDistSqr)
                    break;

                // Replace last good with this triangle
                if (pLast != nullptr)
                    hull.FreeTriangle(pLast);
                pLast = pTri;

                // Add support point in direction of normal of the plane
                // Note that the article uses the closest point between the origin and plane, but this always has the exact same direction as the normal (if the origin is behind the plane)
                // and this way we do less calculations and lose less precision
                int newIndex;
                Vector3 w = supportPoints.Add(inAIncludingConvexRadius, inBIncludingConvexRadius, pTri->m_normal, newIndex);

                // Project w onto the triangle normal
                float dot = pTri->m_normal.Dot(w);

                // Check if we just found a separating axis. This can happen if the shape shrunk by convex radius and then expanded by
                // convex radius is bigger then the original shape due to inaccuracies in the shrinking process.
                if (dot < 0.f)
                    return false;

                // Get the distance squared (Along normal) to the support point
                float distSqr = math::Squared(dot) / pTri->m_normal.SquaredMagnitude();

                // If the error became small enough, we've converged
                if ((distSqr - pTri->m_closestLengthSqr) < (pTri->m_closestLengthSqr * tolerance))
                    break;

                // Keep track of the minimum distance
                closestDistSqr = math::Min(closestDistSqr, distSqr);

                // If the triangle thinks this point is not front facing, we've reached numerical precision and we're done
                if (!pTri->IsFacing(w))
                    break;

                // Add point to hull
                EPAConvexHullBuilder::NewTriangles newTriangles;
                if (!hull.AddPoint(pTri, newIndex, closestDistSqr, newTriangles))
                {
                    break;
                }

                // If the hull is starting to form defects then we're reaching numerical precision and we have to stop
                bool hasDefect = false;
                for (const Triangle* pNewTri : newTriangles)
                {
                    if (pNewTri->IsFacingOrigin())
                    {
                        hasDefect = true;
                        break;
                    }
                }

                if (hasDefect)
                {
                    // When the hull has defects it is possible that the origin has been classified on the wrong side of the triangle
                    // so we do an additional check to see if the penetration in the -triangle normal direction is smaller than
                    // the penetration in the triangle normal direction. If so we must flip the sign of the penetration depth.
                    Vector3 w2 = inAIncludingConvexRadius.GetSupport(-pTri->m_normal) - inBIncludingConvexRadius.GetSupport(pTri->m_normal);
                    float dot2 = -pTri->m_normal.Dot(w2);
                    if (dot2 < dot)
                        flipVSign = true;
                    break;
                }
                
            } while (hull.HasNextTriangle() || supportPoints.m_y.size() < kMaxPoints);
            
            // Determine the closest points, if last == null it means the hull was a plane so there's no penetration
            if (pLast == nullptr)
                return false;

            // Calculate penetration by getting the vector from the origin to the closest point on the triangle:
            // distance = (centroid - origin) . normal / |normal|, closest = origin + distance * normal / |normal|
            outV = (pLast->m_centroid.Dot(pLast->m_normal) / pLast->m_normal.SquaredMagnitude()) * pLast->m_normal;

            // If penetration is near zero, treat this as a non-collision since we cannot find a good normal
            if (outV.IsNearZero())
                return false;

            // Check if we have to flip the size of the penetration depth
            if (flipVSign)
                outV = -outV;

            // Use the barycentric coordinates for the closest point to the origin to find the contact points on A and B
            Vector3 p0 = supportPoints.m_p[pLast->m_edges[0].m_startIndex];
            Vector3 p1 = supportPoints.m_p[pLast->m_edges[1].m_startIndex];
            Vector3 p2 = supportPoints.m_p[pLast->m_edges[2].m_startIndex];

            Vector3 q0 = supportPoints.m_q[pLast->m_edges[0].m_startIndex];
            Vector3 q1 = supportPoints.m_q[pLast->m_edges[1].m_startIndex];
            Vector3 q2 = supportPoints.m_q[pLast->m_edges[2].m_startIndex];

            if (pLast->m_lambdaRelativeTo0)
            {
                // y0 was the reference vertex
                outPointA = p0 + pLast->m_lambda[0] * (p1 - p0) + pLast->m_lambda[1] * (p2 - p0);
                outPointB = q0 + pLast->m_lambda[0] * (q1 - q0) + pLast->m_lambda[1] * (q2 - q0);
            }
            else
            {
                // y1 was the reference vertex
                outPointA = p1 + pLast->m_lambda[0] * (p0 - p1) + pLast->m_lambda[1] * (p2 - p1);
                outPointB = q1 + pLast->m_lambda[0] * (q0 - q1) + pLast->m_lambda[1] * (q2 - q1);
            }

            return true;
        }
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : This function combines the GJK and EPA steps and is provided as a convenience function.
        /// @note : This is less performant because you're providing all support functions in one go.
        /// @note : You need to initialize ioV, see documentation at GetPenetrationDepthStepGJK()!
        //----------------------------------------------------------------------------------------------------
        template <ValidConvexObjectType AE, ValidConvexObjectType AI, ValidConvexObjectType BE, ValidConvexObjectType BI>
        bool            GetPenetrationDepth(const AE& inAExcludingConvexRadius, const AI& inAIncludingConvexRadius, float convexRadiusA, const BE& inBExcludingConvexRadius, const BI& inBIncludingConvexRadius, float convexRadiusB, float collisionToleranceSq, float penetrationTolerance, Vector3& ioV, Vector3& outPointA, Vector3& outPointB)
        {
            switch (GetPenetrationDepthStepGJK(inAExcludingConvexRadius, convexRadiusA, inBExcludingConvexRadius, convexRadiusB, collisionToleranceSq, ioV, outPointA, outPointB))
            {
                case Status::Colliding:
                    return true;
                
                case Status::NotColliding:
                    return false;
                
                case Status::Indeterminate:
                    return GetPenetrationDepthStepEPA(inAIncludingConvexRadius, inBIncludingConvexRadius, penetrationTolerance, ioV, outPointA, outPointB);

                default:
                    NES_ASSERT(false);
            }
            
            return false;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if a cast shape inA moving from start to lambda * start.GetTranslation() + direction
        ///         where lambda e [0, ioLambda> intersects inB.
        ///	@param start : Start position and orientation of the convex object
        ///	@param direction : Direction of the sweep (ioLambda * direction determines length).
        ///	@param collisionTolerance : The minimal distance between A and B before they are considered colliding.
        ///	@param penetrationTolerance : A factor that determines the accuracy of the result. If the change of the squared distance is less than tolerance * currentPenetrationDepth^2 the algorithm will terminate. Should be bigger or equal to FLT_EPSILON.
        ///	@param inA : The convex object A, must support the GetSupport(Vec3) function.
        ///	@param inB : The convex object B, must support the GetSupport(Vec3) function.
        ///	@param convexRadiusA : The convex radius of A, this will be added on all sides to pad A.
        ///	@param convexRadiusB : The convex radius of B, this will be added on all sides to pad B.
        ///	@param returnDeepestPoint : If the shapes are initially intersecting this determines if the EPA algorithm will run to find the deepest point
        ///	@param ioLambda : The max fraction along the sweep, on output updated with the actual collision fraction.
        ///	@param outPointA : The contact point on A.
        ///	@param outPointB : The contact point on B.
        ///	@param outContactNormal : Either the contact normal when the objects are touching or the penetration axis when the objects are penetrating at the start of the sweep (pointing from A to B, length will not be 1)
        //----------------------------------------------------------------------------------------------------
        template <ValidConvexObjectType A, ValidConvexObjectType B>
        bool            CastShape(const Mat4& start, const Vector3& direction, float collisionTolerance, float penetrationTolerance, const A& inA, const B& inB, float convexRadiusA, float convexRadiusB, bool returnDeepestPoint, float& ioLambda, Vector3& outPointA, Vector3& outPointB, Vector3& outContactNormal)
        {
            NES_IF_LOGGING_ENABLED(m_gjkTolerance = collisionTolerance);

            // First determine if there's a collision at all
            if (!m_gjk.CastShape(start, direction, collisionTolerance, inA, inB, convexRadiusA, convexRadiusB, ioLambda, outPointA, outPointB, outContactNormal))
                return false;

            // When our contact normal is too small, we don't have an accurate result.
            bool contactNormalInvalid = outContactNormal.IsNearZero(math::Squared(collisionTolerance));

            if (returnDeepestPoint
                && ioLambda == 0.0f // Only when lambda == 0 can we have the bodies overlap
                && (convexRadiusA + convexRadiusB == 0.0f // When no convex radius was provided, we can never trust contact points at lambda = 0.
                || contactNormalInvalid))
            {
                // If we're initially intersecting, we need to run the EPA algorithm in order to find the deepest contact point
                AddConvexRadius addConvexA(inA, convexRadiusA);
                AddConvexRadius addConvexB(inB, convexRadiusB);
                TransformedConvexObject transformedA(start, addConvexA);
                if (!GetPenetrationDepthStepEPA(transformedA, addConvexB, penetrationTolerance, outContactNormal, outPointA, outPointB))
                    return false;
            }
            else if (contactNormalInvalid)
            {
                // If we weren't able to calculate a contact normal, use the cast direction instead.
                outContactNormal = direction;
            }

            return true;
        }
    };
}
