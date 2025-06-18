// GJKClosestPoint.h
#pragma once
#include "ClosestPoint.h"
#include "ConvexSupport.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //  "Gilbert-Johnson-Keerthi Distance Algorithm"
    //  wiki: https://en.wikipedia.org/wiki/Gilbert%E2%80%93Johnson%E2%80%93Keerthi_distance_algorithm
    //
    /// @brief : Used for Convex vs Convex collision detection. It is a method of determining the minimum
    ///     distance between two convex sets.
    ///
    ///     Based on: "A Fast and Robust GJK Implementation for Collision Detection of Convex Objects"
    ///         - Gino van den Bergen.
    //----------------------------------------------------------------------------------------------------
    class GJKClosestPoint
    {
        Vec3    m_y[4];             /// Support points on A - B
        Vec3    m_p[4];             /// Support point on A
        Vec3    m_q[4];             /// Support point on B
        int     m_numPoints = 0;    /// Number of points in m_y, m_p, and m_q that are valid.
        
    public:
        GJKClosestPoint() = default;
        GJKClosestPoint(const GJKClosestPoint&) = delete;
        GJKClosestPoint& operator=(const GJKClosestPoint&) = delete;

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if A and B intersect.
        ///	@param inA : The convex object A, must support the GetSupport(Vec3) function.
        ///	@param inB : The convex object B, must support the GetSupport(Vec3) function.
        ///	@param tolerance : Minimal distance between two objects when the objects are considered colliding.
        ///	@param separatingAxis : Use as an initial separating axis (provide a zero vector if you don't know yet).
        ///	@returns : True if they intersect (separatingAxis = (0, 0, 0)).
        ///     False if they don't intersect, in which case the separating axis is the axis in the direction from A to B (magnitude is meaningless). 
        //----------------------------------------------------------------------------------------------------
        template <ValidConvexObjectType A, ValidConvexObjectType B>
        bool            Intersects(const A& inA, const B& inB, float tolerance, Vec3& separatingAxis);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the closest points between A and B.
        ///	@param inA : The convex object A, must support the GetSupport(Vec3) function.
        ///	@param inB : The convex object B, must support the GetSupport(Vec3) function.
        ///	@param tolerance : Minimal distance between two objects when the objects are considered colliding.
        ///	@param maxDistSqr : Maximum squared distance between A and B before the objects are considered
        ///     infinitely far away and processing is terminated.
        ///	@param separatingAxis : Initial guess for the separating axis. Start with any non-zero vector if you don't know.
        ///     - If the result value is 0, separatingAxis = (0, 0, 0).
        ///     - If the result value is greater than zero but smaller than FLT_MAX, separatingAxis will be the axis in the direction from A to B and its length the squared distance between A and B.
        ///     - If the result value is FLT_MAX, separatingAxis will be the axis in teh direction from A to B and the magnitude of the vector is meaningless. 
        ///	@param outPointA : If the return value is 0 the point is invalid.
        ///		If the return value is bigger than 0 but smaller than FLT_MAX, then this is the closest point on A.
        ///		If the return value is FLT_MAX the point is invalid.
        /// @param outPointB : If the return value is 0 the point is invalid.
        ///		If the return value is bigger than 0 but smaller than FLT_MAX then this is the closest point on B.
        ///		If the return value is FLT_MAX the point is invalid.
        ///	@returns : The squared distance between A and B or FLT_MAX when they are further away than maxDistSqr.
        //----------------------------------------------------------------------------------------------------
        template <ValidConvexObjectType A, ValidConvexObjectType B>
        float           GetClosestPoints(const A& inA, const B& inB, const float tolerance, const float maxDistSqr, Vec3& separatingAxis, Vec3& outPointA, Vec3& outPointB);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the resulting simplex after the GetClosestPoints algorithm finishes.
        ///     If it returned a squared distance of 0, the origin will be contained in the simplex. 
        //----------------------------------------------------------------------------------------------------
        void            GetClosestPointsSimplex(Vec3* pOutY, Vec3* pOutP, Vec3* pOutQ, size_t& outNumPoints) const;

        //----------------------------------------------------------------------------------------------------
        //	NOTES:
        //  Code based upon: "Ray Casting against General Convex Objects with Application to Continuous Collision Detection" - Gino van den Bergen
        //		
        /// @brief : Test if a ray - rayOrigin + lambda * rayDirection for lambda e [0, ioLambda) - intersects inA.
        ///	@param rayOrigin : Origin of the ray. 
        ///	@param rayDirection : Direction of the ray, including its length! (outLambda * direction determines length).
        ///	@param tolerance : The minimal distance between the ray and A before it is considered colliding.
        ///	@param inA : A convex object that has the GetSupport(Vec3) function.
        ///	@param ioLambda : The max fraction along the ray. On output,this is update with the actual collision
        ///     fraction.
        ///	@returns : True if a hit was found - ioLambda will be the solution lambda for the collision.
        //----------------------------------------------------------------------------------------------------
        template <ValidConvexObjectType A>
        bool            CastRay(const Vec3& rayOrigin, const Vec3& rayDirection, float tolerance, const A& inA, float& ioLambda);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if a cast shape inA moving from start to lambda * start.GetTranslation() + direction where
        ///     lambda e [0, ioLambda) intersects inB.
        ///	@param start : Start position and orientation of the convex object.
        ///	@param direction : Direction of the sweep (ioLambda * direction determines length).
        ///	@param tolerance : The minimal distance between A and B before they are considered colliding.
        ///	@param inA : The convex object A, must support the GetSupport(Vec3) function.
        ///	@param inB : The convex object B, must support the GetSupport(Vec3) function.
        ///	@param ioLambda : The max fraction of the sweep. On output,this is update with the actual collision fraction.
        ///	@returns : True if a hit was found - ioLambda will be the solution lambda for the collision.
        //----------------------------------------------------------------------------------------------------
        template <ValidConvexObjectType A, ValidConvexObjectType B>
        bool            CastShape(const Mat44& start, const Vec3& direction, float tolerance, const A& inA, const B& inB, float& ioLambda);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if a cast shape inA moving from start to lambda * start.GetTranslation() + direction where
        ///     lambda e [0, ioLambda) intersects inB.
        ///	@param start : Start position and orientation of the convex object.
        ///	@param direction : Direction of the sweep (ioLambda * direction determines length).
        ///	@param tolerance : The minimal distance between A and B before they are considered colliding.
        ///	@param inA : The convex object A, must support the GetSupport(Vec3) function.
        ///	@param inB : The convex object B, must support the GetSupport(Vec3) function.
        ///	@param convexRadiusA : The convex radius of A, this will be added on all sides to pad A.
        ///	@param convexRadiusB : The convex radius of B, this will be added on all sides to pad B.
        ///	@param ioLambda : The max fraction of the sweep. On output,this is update with the actual collision fraction.
        ///	@param outPointA : The contact point on A (if outSeparatingAxis is near zero, this may not be not the deepest point).
        ///	@param outPointB : The contact point on B (if outSeparatingAxis is near zero, this may not be not the deepest point).
        ///	@param outSeparatingAxis : On return this will contain a vector that points from A to B along the smallest distance of separation.
        ///     The length of this vector indicates the separation of A and B without their convex radius.
        ///     If it is near zero, the direction may not be accurate as the bodies may overlap when lambda = 0.
        ///	@returns : True if a hit was found, ioLambda will be the solution lambda for the collision, and
        ///     outPoints and outSeparatingAxis will be valid.
        //----------------------------------------------------------------------------------------------------
        template <ValidConvexObjectType A, ValidConvexObjectType B>
        bool            CastShape(const Mat44& start, const Vec3& direction, float tolerance, const A& inA, const B& inB, float convexRadiusA, float convexRadiusB, float& ioLambda, Vec3& outPointA, Vec3& outPointB, Vec3& outSeparatingAxis);

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get new closest point to origin given simplex m_y of m_numPoints points.
        ///	@tparam LastPointPartOfClosestFeature : If true, then the last point added will be assumed to be
        ///     part of the closest feature and the function will do less work.
        ///	@param prevVecLenSqr : Length of |outVec|^2 from the previous iteration, used as a maximum value
        ///     when selecting a new closest point.
        ///	@param outVec : Closest Point.
        ///	@param outVecLenSqr : |outVec|^2
        ///	@param outSet : Set of points that form the new simplex closest to the origin  (bit 1 = m_y[0], bit 2 = m_y[1], ...).
        ///	@returns : True if new closest point was found. False if the function failed. In this case the
        ///     output variables are not modified.
        //----------------------------------------------------------------------------------------------------
        template <bool LastPointPartOfClosestFeature>
        bool            GetClosest(const float prevVecLenSqr, Vec3& outVec, float& outVecLenSqr, uint32_t &outSet) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the max length of valid points in m_y 
        //----------------------------------------------------------------------------------------------------
        float           GetMaxYLengthSqr() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove points that are not in the set. Only updates m_y. 
        //----------------------------------------------------------------------------------------------------
        void            UpdatePointSetY(uint32_t set);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove points that are not in the set. Only updates m_p. 
        //----------------------------------------------------------------------------------------------------
        void            UpdatePointSetP(uint32_t set);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove points that are not in the set. Only updates m_p and m_q. 
        //----------------------------------------------------------------------------------------------------
        void            UpdatePointSetPQ(uint32_t set);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove points that are not in the set. Updates m_y, m_p and m_q. 
        //----------------------------------------------------------------------------------------------------
        void            UpdatePointSetYPQ(uint32_t set);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the closest points on A and B.
        //----------------------------------------------------------------------------------------------------
        void            CalculatePointAAndB(Vec3& outPointA, Vec3& outPointB) const;
    };

    template <ValidConvexObjectType A, ValidConvexObjectType B>
    bool GJKClosestPoint::Intersects(const A& inA, const B& inB, float tolerance, Vec3& separatingAxis)
    {
        // Note: 'separatingAxis' == 'v' for comments in this function, because it is interpreted as just a direction in
        // portions of the algorithm.
        
        const float toleranceSqr = tolerance * tolerance;

        // Reset State:
        m_numPoints = 0;

        // Previous length^2 of v
        float previousLengthSqr = FLT_MAX;

        for (;;)
        {
            // Get the support points for shape A and B in the direction of v.
            Vec3 p = inA.GetSupport(separatingAxis);
            Vec3 q = inB.GetSupport(-separatingAxis);

            // Get support point of the minkowski sum A - B of sv.
            Vec3 w = p - q;

            // If the support point A-B(v) is the opposite direction of v, then we have
            // found a separating axis and there is no intersection.
            if (separatingAxis.Dot(w) < 0.f)
            {
                return false;
            }

            // Store the point for later use.
            m_y[m_numPoints] = w;
            ++m_numPoints;

            // Determine the new closest point
            float vLengthSqr;   // Length^2 of v.
            uint32_t set;       // Set of points that form the new simplex.
            if (!GetClosest<true>(previousLengthSqr, separatingAxis, vLengthSqr, set))
                return false;

            // If there are 4 points, the origin is inside the tetrahedron and we're done.
            if (set == 0xf)
            {
                separatingAxis = Vec3::Zero();
                return true;
            }

            // If v is very close to ero, we consider this a collision.
            if (vLengthSqr <= toleranceSqr)
            {
                separatingAxis = Vec3::Zero();
                return true;
            }

            // If v is very small compared to the length of y, we also consider this a collision
            if (vLengthSqr <= FLT_EPSILON * GetMaxYLengthSqr())
            {
                separatingAxis = Vec3::Zero();
                return true;
            }

            // The next separation axis to test is the negative of the closest point of the Minkowski sum to
            // the origin.
            // Note: This must be done before terminating as converged since the separating axis is -v.
            separatingAxis = -separatingAxis;

            // If the squared length of V is not changing enough, we've converged and there is no collision.
            NES_ASSERT(previousLengthSqr >= vLengthSqr);
            if (previousLengthSqr - vLengthSqr <= FLT_EPSILON * previousLengthSqr)
            {
                // V is a separating axis.
                return false;
            }

            previousLengthSqr = vLengthSqr;

            // Update the points of the simplex.
            UpdatePointSetY(set);
        }
    }

    template <ValidConvexObjectType A, ValidConvexObjectType B>
    float GJKClosestPoint::GetClosestPoints(const A& inA, const B& inB, const float tolerance, const float maxDistSqr,
        Vec3& separatingAxis, Vec3& outPointA, Vec3& outPointB)
    {
        // Note: 'separatingAxis' == 'v' for comments in this function, because it is interpreted as just a direction in
        // portions of the algorithm.
        
        float toleranceSqr = tolerance * tolerance;
        
        // Reset State
        m_numPoints = 0;

        // Length^2 of v
        float vLengthSqr = separatingAxis.LengthSqr();

        // Previous length^2 of v
        float previousLengthSqr = FLT_MAX;

        for (;;)
        {
            // Get support points for shape A and B in the direction of v
            Vec3 p = inA.GetSupport(separatingAxis);
            Vec3 q = inB.GetSupport(-separatingAxis);

            // Get support point of the minkowski sum A - B of v
            Vec3 w = p - q;

            float dot = separatingAxis.Dot(w);

            // Test if we have a separation of more than maxDistSqr, in which case we terminate early.
            if (dot < 0.f && dot * dot > vLengthSqr * maxDistSqr)
            {
            #ifdef NES_DEBUG
                memset(&outPointA, 0xcd, sizeof(outPointA));
                memset(&outPointB, 0xcd, sizeof(outPointB));
            #endif
                
                return FLT_MAX;
            }

            // Store the point for later use:
            m_y[m_numPoints] = w;
            m_p[m_numPoints] = p;
            m_q[m_numPoints] = q;
            ++m_numPoints;

            uint32_t set;
            if (!GetClosest<true>(previousLengthSqr, separatingAxis, vLengthSqr, set))
            {
                --m_numPoints; // Undo the last point.
                break;
            }

            // If there are 4 points, then the origin is inside the tetrahedron and we're done.
            if (set == 0xf)
            {
                separatingAxis = Vec3::Zero();
                vLengthSqr = 0.f;
                break;
            }

            // Update the points of the simplex
            UpdatePointSetYPQ(set);

            // If v is very close to zero, we consider this a collision
            if (vLengthSqr <= toleranceSqr)
            {
                separatingAxis = Vec3::Zero();
                vLengthSqr = 0.f;
                break;
            }

            // If v is very small compared to the length of y, we also consider this a collision.
            if (vLengthSqr <= FLT_EPSILON * GetMaxYLengthSqr())
            {
                separatingAxis = Vec3::Zero();
                vLengthSqr = 0.f;
                break;
            }

            // The next separation axis to test is the negative of the closest point of the Minkowski sum to
            // the origin.
            // Note: This must be done before terminating as converged since the separating axis is -v.
            separatingAxis = -separatingAxis;

            // If the squared length of v is not changing enough, we've converged and there is no collision
            NES_ASSERT(previousLengthSqr >= vLengthSqr);
            if (previousLengthSqr - vLengthSqr <= FLT_EPSILON * previousLengthSqr)
            {
                // v is a separating axis.
                break;
            }
            previousLengthSqr = vLengthSqr;
        }

        CalculatePointAAndB(outPointA, outPointB);

        NES_ASSERT(separatingAxis.LengthSqr() == vLengthSqr);
        return vLengthSqr;
    }

    template <ValidConvexObjectType A>
    bool GJKClosestPoint::CastRay(const Vec3& rayOrigin, const Vec3& rayDirection, const float tolerance, const A& inA, float& ioLambda)
    {
        float toleranceSqr = tolerance * tolerance;

        // Reset State
        m_numPoints = 0;

        float lambda = 0.f;
        Vec3 x = rayOrigin;
        Vec3 v = x - inA.GetSupport(Vec3::Zero());
        float vLengthSqr = v.LengthSqr();
        bool allowRestart = false;

        for (;;)
        {
            // Get new support point
            Vec3 p = inA.GetSupport(v);
            Vec3 w = x - p;

            const float vDotW = v.Dot(w);

            if (vDotW > 0.f)
            {
                // If ray and normal are in the same direction, we've pass A and there's no collision
                const float vDotR = v.Dot(rayDirection);

                // Instead of checking >= 0, check with epsilon as we don't want the division below to overflow to infinity as it can
                // cause a float exception.
                if (vDotR >= -1.0e-18f)
                    return false;

                // Update the lower bound for lambda.
                float delta = vDotW / vDotR;
                const float oldLambda = lambda;
                lambda -= delta;

                // If lambda didn't change, we cannot converge any further and we assume a hit
                if (oldLambda == lambda)
                    break;

                // If lambda is bigger or equal to the max, we don't have a hit
                if (lambda >= ioLambda)
                    return false;

                // Update x to new closest point on the ray
                x = rayOrigin + lambda * rayDirection;

                // We've shifted x, so reset vLengthSqr so that it is not used as early out for GetClosest()
                vLengthSqr = FLT_MAX;

                // We allow rebuilding the simplex once after x changes because the simplex was built
                // for another x and numerical round off builds up as you keep adding points to an
                // existing simplex.
                allowRestart = true;
            }

            // Add p to set P: P = P U {p}
            m_p[m_numPoints] = p;
            ++m_numPoints;

            // Calculate Y = {x} - P
            for (int i = 0; i < m_numPoints; ++i)
            {
                m_y[i] = x - m_p[i];
            }

            // Determine the new closest point from Y to origin
            uint32_t set;
            if (!GetClosest<false>(vLengthSqr, v, vLengthSqr, set))
            {
                // Failed to converge
                
                // Only allow 1 restart, if we still can't get a closest point
                // we're so close that we return this as a hit.
                if (!allowRestart)
                    break;

                allowRestart = false;
                m_p[0] = p;
                m_numPoints = 0;
                v = x - p;
                vLengthSqr = FLT_MAX;
                continue;
            }
            else if (set == 0xf)
            {
                // We're inside the tetrahedron, we have a hit (verify that the length of v is 0).
                NES_ASSERT(vLengthSqr == 0.f);
                break;
            }

            // Update the points P to form the new simplex
            // Note : We're not updating Y as Y will shift with x so we have to calculate it every iteration.
            UpdatePointSetP(set);

            // Check if x is close enough to inA.
            if (vLengthSqr <= toleranceSqr)
            {
                break;
            }
        }

        // Store the hit fraction
        ioLambda = lambda;
        return true;
    }

    template <ValidConvexObjectType A, ValidConvexObjectType B>
    bool GJKClosestPoint::CastShape(const Mat44& start, const Vec3& direction, float tolerance, const A& inA, const B& inB, float& ioLambda)
    {
        // Transform the shape to be cast to the starting position
        TransformedConvexObject transformedA(start, inA);

        // Calculate the Minkowski difference inB - inA
        // inA is moving, so we need to add the back side of inB to the front side on inA.
        MinkowskiDifference difference(inB, transformedA);

        // Do a raycast against the Minkowski difference
        return CastRay(Vec3::Zero(), direction, tolerance, difference, ioLambda);
    }

    template <ValidConvexObjectType A, ValidConvexObjectType B>
    bool GJKClosestPoint::CastShape(const Mat44& start, const Vec3& direction, float tolerance, const A& inA,
        const B& inB, float convexRadiusA, float convexRadiusB, float& ioLambda, Vec3& outPointA, Vec3& outPointB,
        Vec3& outSeparatingAxis)
    {
        float toleranceSqr = tolerance * tolerance;

        // Calculate how close A and B (without their convex radius) need to be to each other in order for us to consider this a collision.
        const float sumConvexRadius = convexRadiusA + convexRadiusB;

        // Transform the shape to be cast to the starting position.
        TransformedConvexObject transformedA(start, inA);

        // Reset state
        m_numPoints = 0;

        float lambda = 0.f;
        Vec3 x = Vec3::Zero(); // Since A is already transformed we can start the cast from zero.

        // See CastRay: v = x - inA.GetSupport(Vec3::Zero()) where inA is the Minkowski difference inB - transformedA (see CastShape above) and x is zero
        Vec3 v = -inB.GetSupport(Vec3::Zero()) + transformedA.GetSupport(Vec3::Zero());

        float vLengthSqr = v.LengthSqr();
        bool allowRestart = false;

        // Keeps track of the separating axis of the previous iteration
        // Initialized to zero as we don't know if our first v is actually a separating axis.
        Vec3 prevV = Vec3::Zero();

        for (;;)
        {
            // Calculate the Minkowski difference inB - inA.
            // inA is moving, so we need to add the back side of inB to the front side of inA.
            // Keep the support points on A and B separate so that in the end we can calculate a contact point.
            Vec3 p = transformedA.GetSupport(-v);
            Vec3 q = inB.GetSupport(v);
            Vec3 w = x - (q - p);

            // Difference from article to this code:
            // We did not include the convex radius in p and q in order to be able to calculate a good separating axis at the end of the algorithm.
            // However, when moving forward along direction we do need to take this into account so that we keep A and B separated by the sum of their convex radii.
            // From p we have to subtract: convexRadiusA * v / |v|
            // To q we have to add: convexRadiusB * v / |v|
            // This means that to w we have to add: -(convexRadiusA + convexRadiusB) * v / |v|
            // So to v . w we have to add: v . (-(convexRadiusA + convexRadiusB) * v / |v|) = -(convexRadiusA + convexRadiusB) * |v|
            float vDotW = v.Dot(w) - sumConvexRadius * v.Length();

            if (vDotW > 0.f)
            {
                // If ray and normal are in the same direction, we've passed A and there's no collision
                float vDotR = v.Dot(direction);
                
                // Instead of checking >= 0, check with epsilon as we don't want the division below to overflow to infinity as it can cause a float exception
                if (vDotR >= -1.0e-18f)
                    return false;

                // Update the lower bound for lambda.
                float delta = vDotW / vDotR;
                float oldLambda = lambda;
                lambda -= delta;

                // If lambda didn't change, we cannot converge any further and we assume a hit.
                if (oldLambda == lambda)
                    break;

                // If lambda is bigger or equal to the max, we don't have a hit.
                if (lambda >= ioLambda)
                    return false;

                // Update x to the new closest point on the ray.
                x = lambda * direction;

                // We've shifted x, so reset vLengthSqr so that it is not used as early out when GetClosest() returns false.
                vLengthSqr = FLT_MAX;

                // Now that we've moved, we know that A and B are not intersecting at lambda = 0, so we can update our tolerance
                // to stop iterating as soon as a and B are convexRadiusA + convexRadiusB apart.
                toleranceSqr = math::Squared(tolerance + sumConvexRadius);

                // We allow rebuilding the simplex once after x changes because the simplex was built
                // for another x and numerical round off builds up as you keep adding points to an
                // existing simplex
                allowRestart = true;
            }

            // Add p to set P, q to set Q: P = P U {p}, Q = Q U {q}
            m_p[m_numPoints] = p;
            m_q[m_numPoints] = q;
            ++m_numPoints;

            // Calculate Y = {x} - (Q - P)
            for (int i = 0; i < m_numPoints; ++i)
            {
                m_y[i] = x - (m_q[i] - m_p[i]);
            }

            // Determine the new closest point form Y to origin.
            uint32_t set; 
            if (!GetClosest<false>(vLengthSqr, v, vLengthSqr, set))
            {
                // Only allow 1 restart, if we still can't get a closest point,
                // we're so close that we return this as a hit.
                if (!allowRestart)
                    break;

                // If we fail to converge, we start again with the last point as simplex
                allowRestart = false;
                m_p[0] = p;
                m_q[0] = q;
                m_numPoints = 1;
                v = x - q;
                vLengthSqr = FLT_MAX;
                continue;
            }
            else if (set == 0xf)
            {
                // We're inside the tetrahedron, we have a hit (verify that the length of v is 0)
                NES_ASSERT(vLengthSqr == 0.f);
                break;
            }

            // Update the points P and Q to form the new simplex
            // Note: We're not updating Y as Y will shift with x so we have to calculate it every iteration
            UpdatePointSetPQ(set);

            // Check if A and B are touching according to our tolerance.
            if (vLengthSqr <= toleranceSqr)
            {
                break;
            }

            // Store our v to return as a separating axis
            prevV = v;
        }

        // Calculate Y = {x} - (Q - P) again so we can calculate the contact points
        for (int i = 0; i < m_numPoints; ++i)
        {
            m_y[i] = x - (m_q[i] - m_p[i]);
        }

        // Calculate the offset we need to apply to A and B to correct for the convex radius
        const Vec3 normalizedV = v.NormalizedOr(Vec3::Zero());
        const Vec3 vecConvexRadiusA = convexRadiusA * normalizedV; 
        const Vec3 vecConvexRadiusB = convexRadiusB * normalizedV;

        // Get the contact point
        // Note that A and B will coincide when lambda > 0. In this case we calculate only B as it is more accurate as it
        // contains less terms.
        switch (m_numPoints)
        {
            case 1:
            {
                outPointB = m_q[0] + vecConvexRadiusB;
                outPointA = lambda > 0.f ? outPointB : m_p[0] - vecConvexRadiusA;
                break;
            }

            case 2:
            {
                float bu, bv;
                ClosestPoint::GetBaryCentricCoordinates(m_y[0], m_y[1], bu, bv);
                outPointB = bu * m_q[0] + bv * m_q[1] + vecConvexRadiusB;
                outPointA = lambda > 0.f ? outPointB : bu * m_p[0] + bv * m_p[1] - vecConvexRadiusA;
                break;
            }

            case 3:
            case 4: // A full simplex, we can't properly determine a contact point! As contact point we take the closest point of the previous iteration.
            {
                float bu, bv, bw;
                ClosestPoint::GetBaryCentricCoordinates(m_y[0], m_y[1], m_y[2], bu, bv, bw);
                outPointB = bu * m_q[0] + bv * m_q[1] + bw * m_q[2] + vecConvexRadiusB;
                outPointA = lambda > 0.0f ? outPointB : bu * m_p[0] + bv * m_p[1] + bw * m_p[2] - vecConvexRadiusA;
                break;
            }
        }

        // Store separating axis, in case we have a convex radius we can just return v,
        // otherwise v will be very small, and we resort to returning previous v as an approximation.
        outSeparatingAxis = sumConvexRadius > 0.f ? -v : -prevV;

        // Store hit fraction
        ioLambda = lambda;
        return true;
    }

    template <bool LastPointPartOfClosestFeature>
    bool GJKClosestPoint::GetClosest(const float prevVecLenSqr, Vec3& outVec, float& outVecLenSqr,
        uint32_t& outSet) const
    {
        uint32_t set;
        Vec3 vec;

        switch (m_numPoints)
        {
            // Single Point
            case 1:
            {
                set = 0b0001;
                vec = m_y[0];
                break;
            }

            // Line Segment
            case 2:
            {
                vec = ClosestPoint::GetClosestPointOnLine(m_y[0], m_y[1], set);
                break;
            }

            // Triangle
            case 3:
            {
                vec = ClosestPoint::GetClosestPointOnTriangle<LastPointPartOfClosestFeature>(m_y[0], m_y[1], m_y[2], set);
                break;
            }

            // Tetrahedron
            case 4:
            {
                vec = ClosestPoint::GetClosestPointOnTetrahedron<LastPointPartOfClosestFeature>(m_y[0], m_y[1], m_y[2], m_y[3], set);
                break;
            }
            
            default:
            {
                NES_ASSERT(false);
                return false;
            }
        }

        const float vecLenSqr = vec.LengthSqr();
        // Note: comparison order is important: if vecLengthSqr is Nan then this expression will be false so we will return false
        if (vecLenSqr < prevVecLenSqr)
        {
            // Return the closest point.
            outVec = vec;
            outVecLenSqr = vecLenSqr;
            outSet = set;
            return true;
        }

        // New closer point is further away, failed to converge.
        return false;
    }
}
