// Segment.h
#pragma once
#include "Vector3.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //      Unreal decided to store the segment as an origin, direction and extent (half-length).
    //      This adds an extra float, but would make certain operations faster, namely the Length & direction.
    //
    ///		@brief : A Line Segment between a start and end point, expressed in 2D coordinates. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TSegment2
    {
        TVector2<Type> m_start{};
        TVector2<Type> m_end{};

        constexpr TSegment2() = default;
        constexpr TSegment2(const TVector2<Type>& start, const TVector2<Type>& end);

        Type Length() const;
        constexpr Type SquaredLength() const;
        constexpr TVector2<Type> Center() const;
        constexpr TVector2<Type> ClosestPointToPoint(const TVector2<Type>& queryPoint);
        Type DistanceToPoint(const TVector2<Type>& queryPoint) const;
        constexpr Type SquaredDistanceToPoint(const TVector2<Type>& queryPoint) const;
    };
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A Line Segment between a start and end point, expressed in 3D coordinates. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TSegment3
    {
        TVector3<Type> m_start{};
        TVector3<Type> m_end{};

        constexpr TSegment3() = default;
        constexpr TSegment3(const TVector3<Type>& start, const TVector3<Type>& end);
        
        Type Length() const;
        constexpr Type SquaredLength() const;
        constexpr TVector3<Type> Center() const;
        constexpr TVector3<Type> ClosestPointToPoint(const TVector3<Type>& queryPoint);
        Type DistanceToPoint(const TVector3<Type>& queryPoint) const;
        constexpr Type SquaredDistanceToPoint(const TVector3<Type>& queryPoint) const;
    };
    
    template <FloatingPointType Type>
    Type ClosestPointsBetweenSegments(const TSegment2<Type>& a, const TSegment2<Type>& b, TVector2<Type>& closestOnA, TVector2<Type>& closestOnB, Type& tA, Type& tB); 

    template <FloatingPointType Type>
    Type ClosestPointsBetweenSegments(const TSegment3<Type>& a, const TSegment3<Type>& b, TVector3<Type>& closestOnA, TVector3<Type>& closestOnB, Type& tA, Type& tB);
    
    using Segment2f = TSegment2<float>;
    using Segment2d = TSegment2<double>;
    using Segment2D = TSegment2<NES_MATH_DEFAULT_REAL_TYPE>;
    
    using Segment3f = TSegment3<float>;
    using Segment3d = TSegment3<double>;
    using Segment = TSegment3<NES_MATH_DEFAULT_REAL_TYPE>;
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a Line Segment between the start and end points. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TSegment2<Type>::TSegment2(const TVector2<Type>& start, const TVector2<Type>& end)
        : m_start(start)
        , m_end(end)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Length of the line segment. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TSegment2<Type>::Length() const
    {
        return (m_end - m_start).Magnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Squared Length of the line segment.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TSegment2<Type>::SquaredLength() const
    {
        return (m_end - m_start).SquaredMagnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the center of the line segment, or midpoint. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TSegment2<Type>::Center() const
    {
        return (m_start + m_end) * static_cast<Type>(0.5f);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return the closest point on the Segment to the query point. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TSegment2<Type>::ClosestPointToPoint(const TVector2<Type>& queryPoint)
    {
        const TVector2<Type> toEnd = (m_end - m_start);

        // If the projection of the point onto the line from start to end is negative, then
        // the closest point is the start.
        const Type projectedDistance = TVector2<Type>::Dot(queryPoint - m_start, toEnd);
        if (projectedDistance < 0)
            return m_start;

        // If the squared projected distance is greater than the squared length of the segment,
        // then the closest point is the end.
        const Type distanceSqr = toEnd.SquaredMagnitude();
        if (math::Squared(projectedDistance) > distanceSqr)
            return m_end;

        // Otherwise, lerp to the closest point on the segment.
        return m_start + (projectedDistance / distanceSqr) * toEnd;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return the Distance from the query point to the nearest point on the line segment.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TSegment2<Type>::DistanceToPoint(const TVector2<Type>& queryPoint) const
    {
        return std::sqrt(SquaredDistanceToPoint(queryPoint));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Squared Distance of the query points to the closest point on the Segment. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TSegment2<Type>::SquaredDistanceToPoint(const TVector2<Type>& queryPoint) const
    {
        TVector2<Type> startToEnd = (m_end - m_start);
        TVector2<Type> startToQuery = (queryPoint - m_start);
        TVector2<Type> endToQuery = (queryPoint - m_end);

        // Case if the query point projects "behind" the start point. 
        const Type projectedDist = TVector2<Type>::Dot(startToQuery, startToEnd);
        if (projectedDist <= 0.f)
        {
            return startToQuery.SquaredMagnitude();
        }

        // Case if the query point projects "past" the end point.
        const Type segmentLengthSqr = startToEnd.SquaredMagnitude();
        if (projectedDist >= segmentLengthSqr)
        {
            return endToQuery.SquaredMagnitude();
        }

        // Returns the distance from the projected point on the segment to the Query point.
        return startToQuery.SquaredMagnitude() - (math::Squared(projectedDist) / segmentLengthSqr);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a line segment between the start and end points. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TSegment3<Type>::TSegment3(const TVector3<Type>& start, const TVector3<Type>& end)
        : m_start(start)
        , m_end(end)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the length of the line segment. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TSegment3<Type>::Length() const
    {
        return (m_end - m_start).Magnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the squared length of the line segment. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TSegment3<Type>::SquaredLength() const
    {
        return (m_end - m_start).SquaredMagnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the center of the line segment, or midpoint. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TSegment3<Type>::Center() const
    {
        return (m_start + m_end) * static_cast<Type>(0.5f);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Distance from the query point to the closest point on the segment. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type TSegment3<Type>::DistanceToPoint(const TVector3<Type>& queryPoint) const
    {
        return std::sqrt(SquaredDistanceToPoint(queryPoint));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the Squared Distance from the query point to the closest point on the segment. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type TSegment3<Type>::SquaredDistanceToPoint(const TVector3<Type>& queryPoint) const
    {
        TVector3<Type> startToEnd = (m_end - m_start);
        TVector3<Type> startToQuery = (queryPoint - m_start);
        TVector3<Type> endToQuery = (queryPoint - m_end);

        // Case if the query point projects "behind" the start point. 
        const Type projectedDist = TVector3<Type>::Dot(startToQuery, startToEnd);
        if (projectedDist <= 0.f)
        {
            return startToQuery.SquaredMagnitude();
        }

        // Case if the query point projects "past" the end point.
        const Type segmentLengthSqr = startToEnd.SquaredMagnitude();
        if (projectedDist >= segmentLengthSqr)
        {
            return endToQuery.SquaredMagnitude();
        }

        // Returns the distance from the projected point on the segment to the Query point.
        return startToQuery.SquaredMagnitude() - (math::Squared(projectedDist) / segmentLengthSqr);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return the closest point on the Segment to the query point. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TSegment3<Type>::ClosestPointToPoint(const TVector3<Type>& queryPoint)
    {
        const TVector3<Type> toEnd = (m_end - m_start);

        // If the projection of the point onto the line from start to end is negative, then
        // the closest point is the start.
        const Type projectedDistance = TVector3<Type>::Dot(queryPoint - m_start, toEnd);
        if (projectedDistance < 0)
            return m_start;

        // If the squared projected distance is greater than the squared length of the segment,
        // then the closest point is the end.
        const Type distanceSqr = toEnd.SquaredMagnitude();
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
    template <FloatingPointType Type>
    Type ClosestPointsBetweenSegments(const TSegment2<Type>& a, const TSegment2<Type>& b, TVector2<Type>& closestOnA,
        TVector2<Type>& closestOnB, Type& tA, Type& tB)
    {
        TVector2<Type> aDir = (a.m_end - a.m_start);
        TVector2<Type> bDir = (b.m_end - b.m_start);
        TVector2<Type> betweenStarts = a.m_start - b.m_start;
        const Type aLengthSqr = a.SquaredLength();
        const Type bLengthSqr = b.SquaredLength();
        const Type projBStart = TVector2<Type>::Dot(bDir, betweenStarts);

        static constexpr Type kTolerance = math::PrecisionDelta();

        // Both Segments degenerate into points:
        if (aLengthSqr <= kTolerance && bLengthSqr <= kTolerance)
        {
            tA = static_cast<Type>(0.f);
            tB = static_cast<Type>(0.f);
            closestOnA = a.m_start;
            closestOnB = b.m_start;
            return TVector2<Type>::Dot(closestOnA - closestOnB, closestOnA - closestOnB);
        }

        // First Segment degenerates into a point:
        if (aLengthSqr <= kTolerance)
        {
            tA = static_cast<Type>(0.f);
            tB = math::ClampNormalized(projBStart / bLengthSqr);
        }

        else
        {
            const Type projAStart = TVector2<Type>::Dot(aDir, betweenStarts);
            
            // Second Segment Degenerates into a point:
            if (bLengthSqr <= kTolerance)
            {
                tB = 0.f;
                tA = math::ClampNormalized(-projAStart / aLengthSqr);
            }

            // Both Segments are valid:
            else
            {
                const Type projDir = TVector2<Type>::Dot(aDir, bDir);
                const Type denom = aLengthSqr * bLengthSqr - math::Squared(projDir);

                // If the segments are not parallel, compute the closest point on LineA and LineB and
                // clamp to the segment A. Else pick an arbitrary tA (here will be 0)
                if (!math::CheckEqualFloats(denom, static_cast<Type>(0.f)))
                    tA = math::ClampNormalized((projDir * projBStart - projAStart * bLengthSqr) / denom);
                else
                    tA = static_cast<Type>(0.f);
                
                const Type tBNom = (b * tA + projBStart);

                // If tBNom is within [0, bLengthSqr], then we can divide by bLengthSqr to get tB.
                // Else, we have to clamp tB and recompute tA for the new value of tB.
                if (tBNom < static_cast<Type>(0.f))
                {
                    tB = static_cast<Type>(0.f);
                    tA = math::ClampNormalized(-projAStart / aLengthSqr);
                }

                else if (tB > bLengthSqr)
                {
                    tB = static_cast<Type>(1.f);
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
        return TVector2<Type>::Dot(closestOnA - closestOnB, closestOnA - closestOnB);
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
    template <FloatingPointType Type>
    Type ClosestPointsBetweenSegments(const TSegment3<Type>& a, const TSegment3<Type>& b, TVector3<Type>& closestOnA,
        TVector3<Type>& closestOnB, Type& tA, Type& tB)
    {
        TVector3<Type> aDir = (a.m_end - a.m_start);
        TVector3<Type> bDir = (b.m_end - b.m_start);
        TVector3<Type> betweenStarts = a.m_start - b.m_start;
        const Type aLengthSqr = a.SquaredLength();
        const Type bLengthSqr = b.SquaredLength();
        const Type projBStart = TVector3<Type>::Dot(bDir, betweenStarts);

        static constexpr Type kTolerance = math::PrecisionDelta();

        // Both Segments degenerate into points:
        if (aLengthSqr <= kTolerance && bLengthSqr <= kTolerance)
        {
            tA = static_cast<Type>(0.f);
            tB = static_cast<Type>(0.f);
            closestOnA = a.m_start;
            closestOnB = b.m_start;
            return TVector3<Type>::Dot(closestOnA - closestOnB, closestOnA - closestOnB);
        }

        // First Segment degenerates into a point:
        if (aLengthSqr <= kTolerance)
        {
            tA = static_cast<Type>(0.f);
            tB = math::ClampNormalized(projBStart / bLengthSqr);
        }

        else
        {
            const Type projAStart = TVector3<Type>::Dot(aDir, betweenStarts);
            
            // Second Segment Degenerates into a point:
            if (bLengthSqr <= kTolerance)
            {
                tB = 0.f;
                tA = math::ClampNormalized(-projAStart / aLengthSqr);
            }

            // Both Segments are valid:
            else
            {
                const Type projDir = TVector3<Type>::Dot(aDir, bDir);
                const Type denom = aLengthSqr * bLengthSqr - math::Squared(projDir);

                // If the segments are not parallel, compute the closest point on LineA and LineB and
                // clamp to the segment A. Else pick an arbitrary tA (here will be 0)
                if (!math::CheckEqualFloats(denom, static_cast<Type>(0.f)))
                    tA = math::ClampNormalized((projDir * projBStart - projAStart * bLengthSqr) / denom);
                else
                    tA = static_cast<Type>(0.f);

                const Type tBNom = (b * tA + projBStart);

                // If tBNom is within [0, bLengthSqr], then we can divide by bLengthSqr to get tB.
                // Else, we have to clamp tB and recompute tA for the new value of tB.
                if (tBNom < static_cast<Type>(0.f))
                {
                    tB = static_cast<Type>(0.f);
                    tA = math::ClampNormalized(-projAStart / aLengthSqr);
                }

                else if (tB > bLengthSqr)
                {
                    tB = static_cast<Type>(1.f);
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
        return TVector3<Type>::Dot(closestOnA - closestOnB, closestOnA - closestOnB);
    }
}