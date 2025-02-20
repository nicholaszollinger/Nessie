// IntersectionQueries2.h
#pragma once

#include "Math/Geometry.h"
#include "Math/Segment.h"
#include "Math/Triangle.h"

namespace nes::geo
{
    template <FloatingPointType Type>
    [[nodiscard]] bool IntersectsSegmentSegment2(const TSegment2<Type>& a, const TSegment2<Type>& b, Type& t, TVector2<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool IntersectsSegmentTriangle2(const TSegment2<Type>& segment, const TTriangle2<Type>& triangle, TVector2<Type>& intersectionPoint);
}

namespace nes::geo
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Determines if the two segments intersect, and if they do, computes the point of intersection
    ///         and the t value along segment A to get the point of intersection. 
    ///		@param a : Segment A.
    ///		@param b : Segment B.
    ///		@param t : Computed t value along segment A to the get the point of intersection. This will be invalid if
    ///                 this returns false.
    ///		@param intersectionPoint : Point of intersection between the two segments. This will be invalid if
    ///                 this returns false.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool IntersectsSegmentSegment2(const TSegment2<Type>& a, const TSegment2<Type>& b, Type& t,
        TVector2<Type>& intersectionPoint)
    {
        // Sign of areas correspond to which side of segment A the points b.Start and b.End are.
        const Type bStartSide = math::Orient2D(a.m_start, a.m_end, b.m_start);  
        const Type bEndSide = math::Orient2D(a.m_start, a.m_end, b.m_end);
        
        // The end points of segment B are on opposite sides of A:
        // if (bStartSide != 0.f && bEndSide != 0.f && bStartSide * bEndSide < 0.f)
        if (!math::SameSign(bStartSide, bEndSide))
        {
            // Compute signs for the end points of A with respect to the segment B.
            const Type aStartSide = math::Orient2D(b.m_start, b.m_end, a.m_start);
            const Type aEndSide = aStartSide + bEndSide - bEndSide;

            // The end points of segment A are also on opposite sides of B, so the segments
            // intersect
            if (!math::SameSign(aStartSide, aEndSide))
            {
                t = aStartSide / (aStartSide - aEndSide);
                intersectionPoint = a.m_start + t * (a.m_end - a.m_start);
                return true;
            }
        }

        return false;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Determines if a segment and a triangle intersect. In the case where the segment goes through
    ///             the triangle in multiple intersections, then the closest intersection point to the
    ///             segment's start point is set as the intersection point.
    ///		@param segment : Segment to test.
    ///		@param triangle : Triangle to test.
    ///		@param intersectionPoint : Point of intersection. If there is no intersection point, then this
    ///             will not be valid.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool IntersectsSegmentTriangle2(const TSegment2<Type>& segment, const TTriangle2<Type>& triangle, TVector2<Type>& intersectionPoint)
    {
        // 3 Triangle Edges with vertices "ABC"
        const TVector2<Type> ab = triangle[1] - triangle[0];
        const TVector2<Type> bc = triangle[1] - triangle[2];
        const TVector2<Type> ca = triangle[2] - triangle[0];

        Type t;
        Type smallestT = std::numeric_limits<Type>::max();
        TVector2<Type> closestPoint = TVector2<Type>::GetZeroVector();
        
        if (IntersectsSegmentSegment2(segment, ab, t, closestPoint))
        {
            if (t < smallestT)
            {
                smallestT = t;
                intersectionPoint = closestPoint;
            }
        }
        
        if (IntersectsSegmentSegment2(segment, bc, t, closestPoint))
        {
            if (t < smallestT)
            {
                smallestT = t;
                intersectionPoint = closestPoint;
            }
        }

        if (IntersectsSegmentSegment2(segment, ca, t, closestPoint))
        {
            if (t < smallestT)
            {
                smallestT = t;
                intersectionPoint = closestPoint;
            }
        }
        
        return smallestT < std::numeric_limits<Type>::max(); 
    }
}
