// IntersectionQueries2.h
#pragma once

#include "Math/Box.h"
#include "Math/Geometry.h"
#include "Math/OrientedBox.h"
#include "Math/Segment.h"
#include "Math/Sphere.h"
#include "Math/Triangle.h"

namespace nes::geo
{
    template <FloatingPointType Type>
    [[nodiscard]] bool SegmentIntersectsSegment2(const TSegment2<Type>& a, const TSegment2<Type>& b, Type& t, TVector2<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool SegmentIntersectsTriangle2(const TSegment2<Type>& segment, const TTriangle2<Type>& triangle, TVector2<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsAABB2(const TSphere2<Type>& sphere, const TBox2<Type>& box);
    
    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsAABB2(const TSphere2<Type>& sphere, const TBox2<Type>& box, TVector2<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsOBB2(const TSphere2<Type>& sphere, const TOrientedBox2<Type>& obb);
    
    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsOBB2(const TSphere2<Type>& sphere, const TOrientedBox2<Type>& obb, TVector2<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsTriangle2(const TSphere2<Type>& sphere, const TTriangle2<Type>& triangle);
    
    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsTriangle2(const TSphere2<Type>& sphere, const TTriangle2<Type>& triangle, TVector2<Type>& intersectionPoint);
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
    bool SegmentIntersectsSegment2(const TSegment2<Type>& a, const TSegment2<Type>& b, Type& t,
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
    bool SegmentIntersectsTriangle2(const TSegment2<Type>& segment, const TTriangle2<Type>& triangle, TVector2<Type>& intersectionPoint)
    {
        // 3 Triangle Edges with vertices "ABC"
        const TVector2<Type> ab = triangle[1] - triangle[0];
        const TVector2<Type> bc = triangle[1] - triangle[2];
        const TVector2<Type> ca = triangle[2] - triangle[0];

        Type t;
        Type smallestT = std::numeric_limits<Type>::max();
        TVector2<Type> closestPoint = TVector2<Type>::GetZeroVector();
        
        if (SegmentIntersectsSegment2(segment, ab, t, closestPoint))
        {
            if (t < smallestT)
            {
                smallestT = t;
                intersectionPoint = closestPoint;
            }
        }
        
        if (SegmentIntersectsSegment2(segment, bc, t, closestPoint))
        {
            if (t < smallestT)
            {
                smallestT = t;
                intersectionPoint = closestPoint;
            }
        }

        if (SegmentIntersectsSegment2(segment, ca, t, closestPoint))
        {
            if (t < smallestT)
            {
                smallestT = t;
                intersectionPoint = closestPoint;
            }
        }
        
        return smallestT < std::numeric_limits<Type>::max(); 
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if a Circle intersects an AABB.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereIntersectsAABB2(const TSphere2<Type>& sphere, const TBox2<Type>& box)
    {
        const Type sqrDist = box.SquaredDistanceToPoint(sphere.m_center);
        return sqrDist <= math::Squared(sphere.m_center);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if a Circle intersects an AABB. If this returns false, the intersection
    ///         point will still represent the closest point on the AABB to the circle.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereIntersectsAABB2(const TSphere2<Type>& sphere, const TBox2<Type>& box, TVector2<Type>& intersectionPoint)
    {
        intersectionPoint = box.ClosestPointToPoint(sphere.m_center);
        const Type sqrDist = (intersectionPoint - sphere.m_center).SquaredMagnitude();
        return sqrDist <= math::Squared(sphere.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if a Circle intersects an OBB.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereIntersectsOBB2(const TSphere2<Type>& sphere, const TOrientedBox2<Type>& obb)
    {
        const Type sqrDist = obb.SquaredDistanceToPoint(sphere.m_center);
        return sqrDist <= math::Squared(sphere.m_center);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if a Circle intersects an OBB. If this returns false, the intersection
    ///         point will still represent the closest point on the OBB to the circle.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereIntersectsOBB2(const TSphere2<Type>& sphere, const TOrientedBox2<Type>& obb,
        TVector2<Type>& intersectionPoint)
    {
        intersectionPoint = obb.ClosestPointToPoint(sphere.m_center);
        const Type sqrDist = (intersectionPoint - sphere.m_center).SquaredMagnitude();
        return sqrDist <= math::Squared(sphere.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if a Circle intersects a Triangle.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereIntersectsTriangle2(const TSphere2<Type>& sphere, const TTriangle2<Type>& triangle)
    {
        const Type sqrDist = triangle.SquaredDistanceToPoint(sphere.m_center);
        return sqrDist <= math::Squared(sphere.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if a Circle intersects a Triangle. If this returns false, the intersection
    ///         point will still represent the closest point on the Triangle to the circle.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereIntersectsTriangle2(const TSphere2<Type>& sphere, const TTriangle2<Type>& triangle,
        TVector2<Type>& intersectionPoint)
    {
        intersectionPoint = triangle.ClosestPointToPoint(sphere.m_center);
        const Type sqrDist = (intersectionPoint - sphere.m_center).SquaredMagnitude();
        return sqrDist <= math::Squared(sphere.m_radius);
    }
}
