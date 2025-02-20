// IntersectionQueries3.h
#pragma once

#include "Plane.h"
#include "Segment.h"
#include "Triangle.h"

namespace nes::geo
{
    template <FloatingPointType Type>
    [[nodiscard]] bool IntersectsSegmentPlane(const TSegment2<Type>& segment, const TPlane<Type>& plane, TVector3<Type>& intersectionPoint);
    
    template <FloatingPointType Type>
    [[nodiscard]] bool IntersectsSegmentTriangle(const TSegment3<Type>& segment, const TTriangle3<Type>& triangle, TVector3<Type>& intersectionPoint);
}

// Inline implementation:
namespace nes::geo
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Test whether a Segment intersects a Plane. If this returns false, the point of intersection
    ///             will be invalid.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool IntersectsSegmentPlane(const TSegment2<Type>& segment, const TPlane<Type>& plane,
        TVector3<Type>& intersectionPoint)
    {
        const Type projStart = TVector3<Type>::Dot(plane.m_normal, segment.m_start);
        const Type projEnd = TVector3<Type>::Dot(plane.m_normal, segment.m_end);

        // If the segments end points are on the same side of the plane, then there
        // is no intersection:
        if (math::SameSign(projStart, projEnd))
        {
            return false;
        }

        intersectionPoint = segment.m_start + ((segment.m_end - segment.m_start) * projStart / segment.Length());
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Test whether a line segment intersects a triangle. If this returns false, then the
    ///             intersection point is invalid.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool IntersectsSegmentTriangle(const TSegment3<Type>& segment, const TTriangle3<Type>& triangle,
        TVector3<Type>& intersectionPoint)
    {
        TVector3<Type> ab = triangle[1] - triangle[0];
        ab.Normalize();
        TVector3<Type> ac = triangle[2] - triangle[0];
        ac.Normalize();

        // Construct a plane from the triangle normal:
        TVector3<Type> triangleNormal = ab.Cross(ac);
        TPlane<Type> plane(triangleNormal, triangle[0].Magnitude());

        // If the segment does not intersect the place of the triangle, then no intersection occurs.
        if (!IntersectsSegmentPlane(segment, plane, intersectionPoint))
        {
            return false;
        }

        Type bary0;
        Type bary1;
        Type bary2;
        triangle.CalculateBarycentricCoordinate(intersectionPoint, bary0, bary1, bary2);

        // If the barycentric coordinates are within the triangle (all greater than zero) then
        // the segment intersects the plane within the triangle's dimensions, and thus intersects. 
        if (bary0 >= static_cast<Type>(0.f) && bary1 >= static_cast<Type>(0.f) && bary2 >= static_cast<Type>(0.f))
        {
            return true;
        }

        return false;
    }
}
