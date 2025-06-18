// IntersectionQueries2.h
#pragma once

#include "AABox2.h"
#include "Geometry.h"
#include "OrientedBox2.h"
#include "Segment.h"
#include "Circle.h"
#include "Triangle.h"

namespace nes::geo
{
    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the two segments intersect, and if they do, computes the point of intersection
    ///     and the t value along segment A to get the point of intersection. 
    ///	@param a : Segment A.
    ///	@param b : Segment B.
    /// @param t : Computed t value along segment A to the get the point of intersection. This will be invalid if
    ///                 this returns false.
    ///	@param intersectionPoint : Point of intersection between the two segments. This will be invalid if
    ///     this returns false.
    //----------------------------------------------------------------------------------------------------
    inline bool SegmentIntersectsSegment2(const Segment2& a, const Segment2& b, float& t, Vec2& intersectionPoint)
    {
        // Sign of areas that correspond to which side of segment A the points b.Start and b.End are.
        const float bStartSide = math::Orient2D(a.m_start, a.m_end, b.m_start);  
        const float bEndSide = math::Orient2D(a.m_start, a.m_end, b.m_end);
        
        // The end points of segment B are on opposite sides of A:
        // if (bStartSide != 0.f && bEndSide != 0.f && bStartSide * bEndSide < 0.f)
        if (!math::SameSign(bStartSide, bEndSide))
        {
            // Compute signs for the end points of A with respect to the segment B.
            const float aStartSide = math::Orient2D(b.m_start, b.m_end, a.m_start);
            const float aEndSide = aStartSide + bEndSide - bEndSide;

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
    ///	@brief : Determines if a segment and a triangle intersect. In the case where the segment goes through
    ///     the triangle in multiple intersections, then the closest intersection point to the
    ///     segment's start point is set as the intersection point.
    ///	@param segment : Segment to test.
    ///	@param triangle : Triangle to test.
    ///	@param intersectionPoint : Point of intersection. If there is no intersection point, then this
    ///     will not be valid.
    //----------------------------------------------------------------------------------------------------
    inline bool SegmentIntersectsTriangle2(const Segment2& segment, const Triangle2& triangle, Vec2& intersectionPoint)
    {
        // 3 Triangle Edges with vertices "ABC"
        const Segment2 ab = Segment2(Vec2(triangle[0]), Vec2(triangle[1]));
        const Segment2 bc = Segment2(Vec2(triangle[1]), Vec2(triangle[2]));
        const Segment2 ca = Segment2(Vec2(triangle[2]), Vec2(triangle[0]));

        float t;
        float smallestT = std::numeric_limits<float>::max();
        Vec2 closestPoint = Vec2::Zero();
        
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
        
        return smallestT < std::numeric_limits<float>::max(); 
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if a Circle intersects an AABB.
    //----------------------------------------------------------------------------------------------------
    inline bool SphereIntersectsAABB2(const Circle& circle, const AABox2& box)
    {
        const float sqrDist = box.DistanceSqrTo(circle.m_center);
        return sqrDist <= math::Squared(circle.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if a Circle intersects an AABB. If this returns false, the intersection
    ///         point will still represent the closest point on the AABB to the circle.
    //----------------------------------------------------------------------------------------------------
    inline bool SphereIntersectsAABB2(const Circle& circle, const AABox2& box, Vec2& intersectionPoint)
    {
        intersectionPoint = box.GetClosestPoint(circle.m_center);
        const float sqrDist = (intersectionPoint - circle.m_center).LengthSqr();
        return sqrDist <= math::Squared(circle.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if a Circle intersects an OBB.
    //----------------------------------------------------------------------------------------------------
    inline bool SphereIntersectsOBB2(const Circle& circle, const OrientedBox2& obb)
    {
        const float sqrDist = obb.DistanceSqrToPoint(circle.m_center);
        return sqrDist <= math::Squared(circle.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if a Circle intersects an OBB. If this returns false, the intersection
    ///     point will still represent the closest point on the OBB to the circle.
    //----------------------------------------------------------------------------------------------------
    inline bool SphereIntersectsOBB2(const Circle& circle, const OrientedBox2& obb, Vec2& intersectionPoint)
    {
        intersectionPoint = obb.ClosestPoint(circle.m_center);
        const float sqrDist = (intersectionPoint - circle.m_center).LengthSqr();
        return sqrDist <= math::Squared(circle.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if a Circle intersects a Triangle.
    //----------------------------------------------------------------------------------------------------
    inline bool SphereIntersectsTriangle2(const Circle& sphere, const Triangle2& triangle)
    {
        const float sqrDist = triangle.DistanceSqrTo(sphere.m_center);
        return sqrDist <= math::Squared(sphere.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if a Circle intersects a Triangle. If this returns false, the intersection
    ///         point will still represent the closest point on the Triangle to the circle.
    //----------------------------------------------------------------------------------------------------
    inline bool SphereIntersectsTriangle2(const Circle& circle, const Triangle2& triangle, Vec2& intersectionPoint)
    {
        intersectionPoint = triangle.ClosestPointTo(circle.m_center);
        const float sqrDist = (intersectionPoint - circle.m_center).LengthSqr();
        return sqrDist <= math::Squared(circle.m_radius);
    }
}
