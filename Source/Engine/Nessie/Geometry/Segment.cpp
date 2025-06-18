// Segment.cpp
#include "Segment.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return the closest point on the Segment to the query point. 
    //----------------------------------------------------------------------------------------------------
    Vec2 Segment2::ClosestPoint(const Vec2 queryPoint) const
    {
        const Vec2 toEnd = (m_end - m_start);

        // If the projection of the point onto the line from start to end is negative, then
        // the closest point is the start.
        const float projectedDistance = Vec2::Dot(queryPoint - m_start, toEnd);
        if (projectedDistance < 0)
            return m_start;

        // If the squared projected distance is greater than the squared length of the segment,
        // then the closest point is the end.
        const float distanceSqr = toEnd.LengthSqr();
        if (math::Squared(projectedDistance) > distanceSqr)
            return m_end;

        // Otherwise, lerp to the closest point on the segment.
        return m_start + (projectedDistance / distanceSqr) * toEnd;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Squared Distance of the query points to the closest point on the Segment. 
    //----------------------------------------------------------------------------------------------------
    float Segment2::DistanceSqr(const Vec2 queryPoint) const
    {
        Vec2 startToEnd = (m_end - m_start);
        Vec2 startToQuery = (queryPoint - m_start);
        Vec2 endToQuery = (queryPoint - m_end);

        // Case if the query point projects "behind" the start point. 
        const float projectedDist = Vec2::Dot(startToQuery, startToEnd);
        if (projectedDist <= 0.f)
        {
            return startToQuery.LengthSqr();
        }

        // Case if the query point projects "past" the end point.
        const float segmentLengthSqr = startToEnd.LengthSqr();
        if (projectedDist >= segmentLengthSqr)
        {
            return endToQuery.LengthSqr();
        }

        // Returns the distance from the projected point on the segment to the Query point.
        return startToQuery.LengthSqr() - (math::Squared(projectedDist) / segmentLengthSqr);
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Squared Distance from the query point to the closest point on the segment. 
    //----------------------------------------------------------------------------------------------------
    float Segment::DistanceSqr(const Vec3 queryPoint) const
    {
        Vec3 startToEnd = (m_end - m_start);
        Vec3 startToQuery = (queryPoint - m_start);
        Vec3 endToQuery = (queryPoint - m_end);

        // Case if the query point projects "behind" the start point. 
        const float projectedDist = Vec3::Dot(startToQuery, startToEnd);
        if (projectedDist <= 0.f)
        {
            return startToQuery.LengthSqr();
        }

        // Case if the query point projects "past" the end point.
        const float segmentLengthSqr = startToEnd.LengthSqr();
        if (projectedDist >= segmentLengthSqr)
        {
            return endToQuery.LengthSqr();
        }

        // Returns the distance from the projected point on the segment to the Query point.
        return startToQuery.LengthSqr() - (math::Squared(projectedDist) / segmentLengthSqr);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return the closest point on the Segment to the query point. 
    //----------------------------------------------------------------------------------------------------
    Vec3 Segment::ClosestPoint(const Vec3 queryPoint) const
    {
        const Vec3 toEnd = (m_end - m_start);

        // If the projection of the point onto the line from start to end is negative, then
        // the closest point is the start.
        const float projectedDistance = Vec3::Dot(queryPoint - m_start, toEnd);
        if (projectedDistance < 0)
            return m_start;

        // If the squared projected distance is greater than the squared length of the segment,
        // then the closest point is the end.
        const float distanceSqr = toEnd.LengthSqr();
        if (math::Squared(projectedDistance) > distanceSqr)
            return m_end;

        // Otherwise, lerp to the closest point on the segment.
        return m_start + (projectedDistance / distanceSqr) * toEnd;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Computes the closest points "closestOnA" & "closestOnB" between the segments, the normalized
    ///         parameters "tA" & "tB" that represent the position of the closest point to the respective segment where
    ///         0 == the start point and 1 == the end point. The return value is the squared distance between the
    ///         two closest points.
    ///             
    ///		@param a : Segment A.
    ///		@param b : Segment B.
    ///		@param closestOnA : The closest point on Segment A to the Segment B.
    ///		@param closestOnB : The closest point on Segment B to the Segment A.
    ///     @param tA : t value along Segment A that represents the position of the closest point from the
    ///                     start point (0) to the end (1) of segment A.  
    ///     @param tB : t value along Segment B that represents the position of the closest point from the
    ///                     start point (0) to the end (1) of segment B.  
    ///		@returns : The Squared Distance between the two closest points.
    //----------------------------------------------------------------------------------------------------
    float ClosestPointsBetweenSegments(const Segment2& a, const Segment2& b, Vec2& closestOnA, Vec2& closestOnB, float& tA, float& tB)
    {
        Vec2 aDir = (a.m_end - a.m_start);
        Vec2 bDir = (b.m_end - b.m_start);
        Vec2 betweenStarts = a.m_start - b.m_start;
        const float aLengthSqr = a.LengthSqr();
        const float bLengthSqr = b.LengthSqr();
        const float projBStart = Vec2::Dot(bDir, betweenStarts);

        static constexpr float kTolerance = math::PrecisionDelta();

        // Both Segments degenerate into points:
        if (aLengthSqr <= kTolerance && bLengthSqr <= kTolerance)
        {
            tA = 0.f;
            tB = 0.f;
            closestOnA = a.m_start;
            closestOnB = b.m_start;
            return Vec2::Dot(closestOnA - closestOnB, closestOnA - closestOnB);
        }

        // First Segment degenerates into a point:
        if (aLengthSqr <= kTolerance)
        {
            tA = 0.f;
            tB = math::ClampNormalized(projBStart / bLengthSqr);
        }

        else
        {
            const float projAStart = Vec2::Dot(aDir, betweenStarts);
            
            // Second Segment Degenerates into a point:
            if (bLengthSqr <= kTolerance)
            {
                tB = 0.f;
                tA = math::ClampNormalized(-projAStart / aLengthSqr);
            }

            // Both Segments are valid:
            else
            {
                const float projDir = Vec2::Dot(aDir, bDir);
                const float denom = aLengthSqr * bLengthSqr - math::Squared(projDir);

                // If the segments are not parallel, compute the closest point on LineA and LineB and
                // clamp to the segment A. Else pick an arbitrary tA (here will be 0)
                if (!math::CheckEqualFloats(denom, 0.f))
                    tA = math::ClampNormalized((projDir * projBStart - projAStart * bLengthSqr) / denom);
                else
                    tA = 0.f;
                
                const float tBNom = (projDir * tA + projBStart);

                // If tBNom is within [0, bLengthSqr], then we can divide by bLengthSqr to get tB.
                // Else, we have to clamp tB and recompute tA for the new value of tB.
                if (tBNom < 0.f)
                {
                    tB = 0.f;
                    tA = math::ClampNormalized(-projAStart / aLengthSqr);
                }

                else if (tB > bLengthSqr)
                {
                    tB = 1.f;
                    tA = math::ClampNormalized((projDir - projAStart) / aLengthSqr);
                }

                else
                {
                    tB = tBNom / bLengthSqr;
                }
            }
        }

        closestOnA = a.m_start + (aDir * tA);
        closestOnB = b.m_start + (bDir * tB);
        return Vec2::Dot(closestOnA - closestOnB, closestOnA - closestOnB);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Computes the closest points "closestOnA" & "closestOnB" between the segments, the normalized
    ///         parameters "tA" & "tB" that represent the position of the closest point to the respective segment where
    ///         0 == the start point and 1 == the end point. The return value is the squared distance between the
    ///         two closest points.
    ///             
    ///		@param a : Segment A.
    ///		@param b : Segment B.
    ///		@param closestOnA : The closest point on Segment A to the Segment B.
    ///		@param closestOnB : The closest point on Segment B to the Segment A.
    ///     @param tA : t value along Segment A that represents the position of the closest point from the
    ///                     start point (0) to the end (1) of segment A.  
    ///     @param tB : t value along Segment B that represents the position of the closest point from the
    ///                     start point (0) to the end (1) of segment B.  
    ///		@returns : The Squared Distance between the two closest points.
    //----------------------------------------------------------------------------------------------------
    float ClosestPointsBetweenSegments(const Segment& a, const Segment& b, Vec3& closestOnA, Vec3& closestOnB, float& tA, float& tB)
    {
        Vec3 aDir = (a.m_end - a.m_start);
        Vec3 bDir = (b.m_end - b.m_start);
        Vec3 betweenStarts = a.m_start - b.m_start;
        const float aLengthSqr = a.LengthSqr();
        const float bLengthSqr = b.LengthSqr();
        const float projBStart = Vec3::Dot(bDir, betweenStarts);

        static constexpr float kTolerance = math::PrecisionDelta();

        // Both Segments degenerate into points:
        if (aLengthSqr <= kTolerance && bLengthSqr <= kTolerance)
        {
            tA = 0.f;
            tB = 0.f;
            closestOnA = a.m_start;
            closestOnB = b.m_start;
            return Vec3::Dot(closestOnA - closestOnB, closestOnA - closestOnB);
        }

        // First Segment degenerates into a point:
        if (aLengthSqr <= kTolerance)
        {
            tA = 0.f;
            tB = math::ClampNormalized(projBStart / bLengthSqr);
        }

        else
        {
            const float projAStart = Vec3::Dot(aDir, betweenStarts);
            
            // Second Segment Degenerates into a point:
            if (bLengthSqr <= kTolerance)
            {
                tB = 0.f;
                tA = math::ClampNormalized(-projAStart / aLengthSqr);
            }

            // Both Segments are valid:
            else
            {
                const float projDir = Vec3::Dot(aDir, bDir);
                const float denom = aLengthSqr * bLengthSqr - math::Squared(projDir);

                // If the segments are not parallel, compute the closest point on LineA and LineB and
                // clamp to the segment A. Else pick an arbitrary tA (here will be 0)
                if (!math::CheckEqualFloats(denom, 0.f))
                    tA = math::ClampNormalized((projDir * projBStart - projAStart * bLengthSqr) / denom);
                else
                    tA = 0.f;

                const float tBNom = (projDir * tA + projBStart);

                // If tBNom is within [0, bLengthSqr], then we can divide by bLengthSqr to get tB.
                // Else, we have to clamp tB and recompute tA for the new value of tB.
                if (tBNom < 0.f)
                {
                    tB = 0.f;
                    tA = math::ClampNormalized(-projAStart / aLengthSqr);
                }

                else if (tB > bLengthSqr)
                {
                    tB = 1.f;
                    tA = math::ClampNormalized((projDir - projAStart) / aLengthSqr);
                }

                else
                {
                    tB = tBNom / bLengthSqr;
                }
            }
        }

        closestOnA = a.m_start + (aDir * tA);
        closestOnB = b.m_start + (bDir * tB);
        return Vec3::Dot(closestOnA - closestOnB, closestOnA - closestOnB);
    }
}