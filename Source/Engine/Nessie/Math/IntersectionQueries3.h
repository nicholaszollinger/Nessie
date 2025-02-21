// IntersectionQueries3.h
#pragma once

#include "OrientedBox.h"
#include "Plane.h"
#include "Segment.h"
#include "Sphere.h"
#include "Triangle.h"

namespace nes::geo
{
    template <FloatingPointType Type>
    [[nodiscard]] bool SegmentIntersectsPlane(const TSegment2<Type>& segment, const TPlane<Type>& plane, TVector3<Type>& intersectionPoint);
    
    template <FloatingPointType Type>
    [[nodiscard]] bool SegmentIntersectsTriangle(const TSegment3<Type>& segment, const TTriangle3<Type>& triangle, TVector3<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsPlane(const TSphere3<Type>& sphere, const TPlane<Type>& plane);

    template <FloatingPointType Type>
    [[nodiscard]] bool SphereInsidePlane(const TSphere3<Type>& sphere, const TPlane<Type>& plane);

    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsHalfspace(const TSphere3<Type>& sphere, const TPlane<Type>& plane);

    template <FloatingPointType Type>
    [[nodiscard]] bool OBBIntersectsPlane(const TOrientedBox3<Type>& obb, TPlane<Type>& plane);

    template <FloatingPointType Type>
    [[nodiscard]] bool OBBInsidePlane(const TOrientedBox3<Type>& obb, TPlane<Type>& plane);

    template <FloatingPointType Type>
    [[nodiscard]] bool OBBIntersectsHalfspace(const TOrientedBox3<Type>& obb, TPlane<Type>& plane);

    template <FloatingPointType Type>
    [[nodiscard]] bool AABBIntersectsPlane(const TBox3<Type>& box, const TPlane<Type>& plane);

    template <FloatingPointType Type>
    [[nodiscard]] bool AABBInsidePlane(const TBox3<Type>& box, const TPlane<Type>& plane);

    template <FloatingPointType Type>
    [[nodiscard]] bool AABBIntersectsHalfspace(const TBox3<Type>& box, const TPlane<Type>& plane);

    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsAABB(const TSphere3<Type>& sphere, const TBox3<Type>& box);
    
    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsAABB(const TSphere3<Type>& sphere, const TBox3<Type>& box, TVector3<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsOBB(const TSphere3<Type>& sphere, const TOrientedBox3<Type>& obb);
    
    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsOBB(const TSphere3<Type>& sphere, const TOrientedBox3<Type>& obb, TVector3<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsTriangle(const TSphere3<Type>& sphere, const TTriangle3<Type>& triangle);
    
    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsTriangle(const TSphere3<Type>& sphere, const TTriangle3<Type>& triangle, TVector3<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool AABBIntersectsTriangle(const TBox3<Type>& box, const TTriangle3<Type>& triangle);

    template <FloatingPointType Type>
    [[nodiscard]] bool OBBIntersectsTriangle(const TOrientedBox3<Type>& obb, const TTriangle3<Type>& triangle);
}

// Inline implementation:
namespace nes::geo
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Test whether a Segment intersects a Plane. If this returns false, the point of intersection
    ///             will be invalid.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SegmentIntersectsPlane(const TSegment2<Type>& segment, const TPlane<Type>& plane,
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
    bool SegmentIntersectsTriangle(const TSegment3<Type>& segment, const TTriangle3<Type>& triangle,
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
        if (!SegmentIntersectsPlane(segment, plane, intersectionPoint))
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

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the Sphere intersects the plane.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereIntersectsPlane(const TSphere3<Type>& sphere, const TPlane<Type>& plane)
    {
        const Type signedDistance = plane.SignedDistanceToPoint(sphere.m_center);
        
        // If the total distance is less than the radius, then the Sphere intersects.
        return math::Abs(signedDistance) <= sphere.m_radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the Sphere is fully behind (in the negative halfspace of) the plane. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereInsidePlane(const TSphere3<Type>& sphere, const TPlane<Type>& plane)
    {
        const Type signedDistance = plane.SignedDistanceToPoint(sphere.m_center);
        return signedDistance <= -sphere.m_radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the Sphere intersects the negative halfspace of the Plane. In other words,
    ///             this test treats anything behind the plane as solid; so if the Sphere is intersecting or
    ///             fully behind the plane, this will return true.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereIntersectsHalfspace(const TSphere3<Type>& sphere, const TPlane<Type>& plane)
    {
        const Type signedDistance = plane.SignedDistanceToPoint(sphere.m_center);
        return signedDistance <= sphere.m_radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the OBB intersects the plane. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool OBBIntersectsPlane(const TOrientedBox3<Type>& obb, TPlane<Type>& plane)
    { 
        // Compute the projection interval radius of the OBB onto the Line(t) = obb.center + t * plane.normal
        const Type radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(plane.m_normal, obb.m_localOrientation.GetAxis(0)))
                            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(plane.m_normal, obb.m_localOrientation.GetAxis(1)))
                            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(plane.m_normal, obb.m_localOrientation.GetAxis(2)));

        const Type signedDistance = plane.SignedDistanceToPoint(obb.m_center);
        
        // Intersection occurs when the signedDistance falls within the [-radius, +radius] interval.
        return math::Abs(signedDistance) <= radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the OBB is fully behind (in the negative halfspace of) the plane.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool OBBInsidePlane(const TOrientedBox3<Type>& obb, TPlane<Type>& plane)
    {
        // Compute the projection interval radius of the OBB onto the Line(t) = obb.center + t * plane.normal
        const Type radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(plane.m_normal, obb.m_localOrientation.GetAxis(0)))
                            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(plane.m_normal, obb.m_localOrientation.GetAxis(1)))
                            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(plane.m_normal, obb.m_localOrientation.GetAxis(2)));

        const Type signedDistance = plane.SignedDistanceToPoint(obb.m_center);
        return signedDistance <= -radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the OBB intersects the negative halfspace of the Plane. In other words,
    ///             this test treats anything behind the plane as solid; so if the OBB is intersecting or
    ///             fully behind the plane, this will return true.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool OBBIntersectsHalfspace(const TOrientedBox3<Type>& obb, TPlane<Type>& plane)
    {
        // Compute the projection interval radius of the OBB onto the Line(t) = obb.center + t * plane.normal
        const Type radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(plane.m_normal, obb.m_localOrientation.GetAxis(0)))
                            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(plane.m_normal, obb.m_localOrientation.GetAxis(1)))
                            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(plane.m_normal, obb.m_localOrientation.GetAxis(2)));

        const Type signedDistance = plane.SignedDistanceToPoint(obb.m_center);
        return signedDistance <= radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the AABB intersects the plane. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool AABBIntersectsPlane(const TBox3<Type>& box, const TPlane<Type>& plane)
    {
        // Compute the projection interval radius of the OBB onto the Line(t) = obb.center + t * plane.normal
        const Type radius = box.m_halfExtents[0] * math::Abs(plane.m_normal[0])
            + box.m_halfExtents[1] * math::Abs(plane.m_normal[1])
            + box.m_halfExtents[2] * math::Abs(plane.m_normal[2]);

        const Type signedDistance = plane.SignedDistanceToPoint(box.m_center);
        
        // Intersection occurs when the signedDistance falls within the [-radius, +radius] interval.
        return math::Abs(signedDistance) <= radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the AABB is fully behind (in the negative halfspace of) the plane.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool AABBInsidePlane(const TBox3<Type>& box, const TPlane<Type>& plane)
    {
        // Compute the projection interval radius of the OBB onto the Line(t) = obb.center + t * plane.normal
        const Type radius = box.m_halfExtents[0] * math::Abs(plane.m_normal[0])
            + box.m_halfExtents[1] * math::Abs(plane.m_normal[1])
            + box.m_halfExtents[2] * math::Abs(plane.m_normal[2]);

        const Type signedDistance = plane.SignedDistanceToPoint(box.m_center);
        return signedDistance <= -radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the AABB intersects the negative halfspace of the Plane. In other words,
    ///             this test treats anything behind the plane as solid; so if the AABB is intersecting or
    ///             fully behind the plane, this will return true.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool AABBIntersectsHalfspace(const TBox3<Type>& box, const TPlane<Type>& plane)
    {
        // Compute the projection interval radius of the OBB onto the Line(t) = obb.center + t * plane.normal
        const Type radius = box.m_halfExtents[0] * math::Abs(plane.m_normal[0])
            + box.m_halfExtents[1] * math::Abs(plane.m_normal[1])
            + box.m_halfExtents[2] * math::Abs(plane.m_normal[2]);

        const Type signedDistance = plane.SignedDistanceToPoint(box.m_center);
        return signedDistance <= radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if a Sphere intersects an AABB. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereIntersectsAABB(const TSphere3<Type>& sphere, const TBox3<Type>& box)
    {
        const Type sqrDist = box.SquaredDistanceToPoint(sphere.m_center);
        return sqrDist <= math::Squared(sphere.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if a Sphere intersects an AABB, and returns the point of intersection. If this
    ///             function returns false, they do not intersect, and the point will still be the closest point on the AABB
    ///             to the sphere.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereIntersectsAABB(const TSphere3<Type>& sphere, const TBox3<Type>& box, TVector3<Type>& intersectionPoint)
    {
        intersectionPoint = box.ClosestPointToPoint(sphere.m_center);
        const Type sqrDist = (intersectionPoint - sphere.m_center).SquaredMagnitude();
        return sqrDist <= math::Squared(sphere.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if a Sphere intersects an OBB. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereIntersectsOBB(const TSphere3<Type>& sphere, const TOrientedBox3<Type>& obb)
    {
        const Type sqrDist = obb.SquaredDistanceToPoint(sphere.m_center);
        return sqrDist <= math::Squared(sphere.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if a Sphere intersects an OBB, and returns the point of intersection. If this
    ///             function returns false, they do not intersect, and the point will still be the closest point on the OBB
    ///             to the sphere.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereIntersectsOBB(const TSphere3<Type>& sphere, const TOrientedBox3<Type>& obb,
        TVector3<Type>& intersectionPoint)
    {
        intersectionPoint = obb.ClosestPointToPoint(sphere.m_center);
        const Type sqrDist = (intersectionPoint - sphere.m_center).SquaredMagnitude();
        return sqrDist <= math::Squared(sphere.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if a Sphere intersects an Triangle. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereIntersectsTriangle(const TSphere3<Type>& sphere, const TTriangle3<Type>& triangle)
    {
        const Type sqrDist = triangle.SquaredDistanceToPoint(sphere.m_center);
        return sqrDist <= math::Squared(sphere.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if a Sphere intersects a triangle, and returns the point of intersection. If this
    ///             function returns false, they do not intersect, and the point will still be the closest point on the
    ///             triangle to the sphere.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereIntersectsTriangle(const TSphere3<Type>& sphere, const TTriangle3<Type>& triangle,
        TVector3<Type>& intersectionPoint)
    {
        intersectionPoint = triangle.ClosestPointToPoint(sphere.m_center);
        const Type sqrDist = (intersectionPoint - sphere.m_center).SquaredMagnitude();
        return sqrDist <= math::Squared(sphere.m_radius);
    }

    //----------------------------------------------------------------------------------------------------
    //      NOTES:
    //      pg 170-172 of "Real-Time Collision Detection".
    //      - This uses the separating axis theorem (SAT) to test the 13 axes required to check for intersection.
    //          1. Three face normals of the AABB
    //          2. One face normal of the Triangle
    //          3. Nine axes give by the cross products of combination of edges from both.
    //      Since the box axes are the basis vectors, much of the implementation can be simplified. The OBB
    //      Triangle intersection method is the same, but without the shortcuts.
    //
    //      [TODO] : To make this more robust, I need to check for a degenerate or over-sized triangle
    //               and check for parallel edges with the 9 test axes. The solution to these are on
    //              pg 159
    //
    ///		@brief : Determine if an AABB intersects with a Triangle.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool AABBIntersectsTriangle(const TBox3<Type>& box, const TTriangle3<Type>& triangle)
    {
        // Translate the triangle's vertices as conceptually moving the AABB to the origin.
        TVector3<Type> vertices[3]
        {
            triangle.m_vertices[0] - box.m_center,  
            triangle.m_vertices[1] - box.m_center,  
            triangle.m_vertices[2] - box.m_center,  
        };

        // Compute the Edge Vectors for the Triangle:
        TVector3<Type> triEdges[3]
        {
            vertices[1] - vertices[0],  
            vertices[2] - vertices[1],  
            vertices[0] - vertices[2],  
        };

        // p0, p1, p2 represent the distances from the origin to the projections of the triangle
        // vertices onto the axis we are testing.
        Type p0{};
        Type p1{};
        Type p2{};
        Type radius{};
        
        // Test the 9 axes pertaining to the cross product of the box axes and the triangle edges.
        // If the projection intervals [-radius, radius] and [Min(p0, p1, p2), Max(p0, p1, p2)] are
        // disjoint, then there is a separating axis and the Triangle and AABB do not overlap.
        
        // axis = (1, 0, 0) x TriEdge[0] = (0, -TriEdge[0].z, TriEdge[0].y)
        p0 = vertices[0].z * vertices[1].y - vertices[0].y * vertices[1].z;
        // p1 == p0
        p2 = vertices[2].y * -triEdges[0].z + vertices[2].z * triEdges[0].y;
        radius = box.m_halfExtents[1] * math::Abs(triEdges[0].z) + box.m_halfExtents[2] * math::Abs(triEdges[0].y);
        if (math::Max(-math::Max(p0, p2), math::Min(p0, p2)) > radius)
        {
            return false;
        }

        // axis = (1, 0, 0) x TriEdge[1] = (0, -TriEdge[1].z, TriEdge[1].y)
        p0 = vertices[0].y * -triEdges[1].z + vertices[0].z * triEdges[1].y;
        p1 = vertices[1].y * -triEdges[1].z + vertices[1].z * triEdges[1].y;
        // p2 == p1
        radius = box.m_halfExtents[1] * math::Abs(triEdges[1].z) + box.m_halfExtents[2] * math::Abs(triEdges[1].y);
        if (math::Max(-math::Max(p0, p1), math::Min(p0, p1)) > radius)
        {
            return false;
        }
        
        // axis = (1, 0, 0) x TriEdge[2] = (0, -TriEdge[2].z, TriEdge[2].y)
        p0 = vertices[0].y * vertices[2].z - vertices[0].z * vertices[2].y;
        p1 = vertices[1].y * -triEdges[2].z + vertices[1].z * triEdges[2].y;
        // p2 == p0
        radius = box.m_halfExtents[1] * math::Abs(triEdges[2].z) + box.m_halfExtents[2] * math::Abs(triEdges[2].y);
        if (math::Max(-math::Max(p0, p1), math::Min(p0, p1)) > radius)
        {
            return false;
        }

        // axis = (0, 1, 0) x TriEdge[0] = (TriEdge[0].z, 0, -TriEdge[0].x)
        p0 = vertices[0].x * vertices[1].z - vertices[0].z * vertices[1].x;
        // p1 == p0
        p2 = vertices[2].x * triEdges[0].z - vertices[2].z * triEdges[0].x;
        radius = box.m_halfExtents[0] * math::Abs(triEdges[0].z) + box.m_halfExtents[2] * math::Abs(triEdges[0].x);
        if (math::Max(-math::Max(p0, p2), math::Min(p0, p2)) > radius)
        {
            return false;
        }
        
        // axis = (0, 1, 0) x TriEdge[1] = (TriEdge[1].z, 0, -TriEdge[1].x)
        p0 = vertices[0].x * triEdges[1].z - vertices[0].z * triEdges[1].x;
        p1 = vertices[1].x * vertices[2].z - vertices[1].z * vertices[2].x;
        // p2 == p1
        radius = box.m_halfExtents[0] * math::Abs(triEdges[1].z) + box.m_halfExtents[2] * math::Abs(triEdges[1].x);
        if (math::Max(-math::Max(p0, p1), math::Min(p0, p1)) > radius)
        {
            return false;
        }
        
        // axis = (0, 1, 0) x TriEdge[2] = (TriEdge[2].z, 0, -TriEdge[2].x)
        p0 = vertices[0].z * vertices[2].x - vertices[0].x * vertices[2].z;
        p1 = vertices[1].x * triEdges[2].z - vertices[1].z * triEdges[2].x;
        // p2 == p0
        radius = box.m_halfExtents[0] * math::Abs(triEdges[2].z) + box.m_halfExtents[2] * math::Abs(triEdges[2].x);
        if (math::Max(-math::Max(p0, p1), math::Min(p0, p1)) > radius)
        {
            return false;
        }

        // axis = (0, 0, 1) x TriEdge[0] = (-TriEdge[0].y, TriEdge[0].x, 0)
        p0 = vertices[0].y * vertices[1].x - vertices[0].x * vertices[1].y;
        // p1 == p0
        p2 = vertices[2].y * triEdges[0].x - vertices[2].x * triEdges[0].y;
        radius = box.m_halfExtents[0] * math::Abs(triEdges[0].y) + box.m_halfExtents[1] * math::Abs(triEdges[0].x);
        if (math::Max(-math::Max(p0, p2), math::Min(p0, p2)) > radius)
        {
            return false;
        }
        
        // axis = (0, 0, 1) x TriEdge[1] = (-TriEdge[1].y, TriEdge[1].x, 0)
        p0 = vertices[0].y * triEdges[1].x - vertices[0].x * triEdges[1].y;
        p1 = vertices[1].y * vertices[2].x - vertices[1].x * vertices[2].y;
        // p2 == p1
        radius = box.m_halfExtents[0] * math::Abs(triEdges[1].y) + box.m_halfExtents[1] * math::Abs(triEdges[1].x);
        if (math::Max(-math::Max(p0, p1), math::Min(p0, p1)) > radius)
        {
            return false;
        }
        
        // axis = (0, 0, 1) x TriEdge[2] = (-TriEdge[2].y, TriEdge[2].x, 0)
        p0 = vertices[0].x * vertices[2].y - vertices[0].y * vertices[2].x;
        p1 = vertices[1].y * triEdges[2].x - vertices[1].x * triEdges[2].y;
        // p2 == p0
        radius = box.m_halfExtents[0] * math::Abs(triEdges[2].y) + box.m_halfExtents[1] * math::Abs(triEdges[2].x);
        if (math::Max(-math::Max(p0, p1), math::Min(p0, p1)) > radius)
        {
            return false;
        }
        
        // Test the 3 axes corresponding to the face normals of the box.

        // X
        if (math::Max(vertices[0].x, vertices[1].x, vertices[2].x) < -box.m_halfExtents.x
            || math::Min(vertices[0].x, vertices[1].x, vertices[2].x) > box.m_halfExtents.x)
        {
            return false;
        }

        // Y
        if (math::Max(vertices[0].y, vertices[1].y, vertices[2].y) < -box.m_halfExtents.y
            || math::Min(vertices[0].y, vertices[1].y, vertices[2].y) > box.m_halfExtents.y)
        {
            return false;
        }

        // Z
        if (math::Max(vertices[0].z, vertices[1].z, vertices[2].z) < -box.m_halfExtents.z
            || math::Min(vertices[0].z, vertices[1].z, vertices[2].z) > box.m_halfExtents.z)
        {
            return false;
        }

        // Test the separating axis corresponding to the triangle face normal.
        TPlane<Type> plane{};
        plane.m_normal = triangle.Normal();
        plane.m_distance = TVector3<Type>::Dot(plane.m_normal, vertices[0]);
        return AABBIntersectsPlane(box, plane);
    }

    //----------------------------------------------------------------------------------------------------
    //      NOTES:
    //      pg 170-172 of "Real-Time Collision Detection".
    //      - This uses the separating axis theorem (SAT) to test the 13 axes required to check for intersection.
    //          1. Three face normals of the OBB
    //          2. One face normal of the Triangle
    //          3. Nine axes give by the cross products of combination of edges from both.
    //
    //      [TODO] : To make this more robust, I need to check for a degenerate or over-sized triangle
    //               and check for parallel edges with the 9 test axes. The solution to these are on
    //              pg 159
    //
    ///		@brief : Determine if an OBB intersects with a Triangle.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool OBBIntersectsTriangle(const TOrientedBox3<Type>& obb, const TTriangle3<Type>& triangle)
    {
        // Translate the triangle's vertices as conceptually moving the OBB to the origin.
        TVector3<Type> vertices[3]
        {
            triangle.m_vertices[0] - obb.m_center,  
            triangle.m_vertices[1] - obb.m_center,  
            triangle.m_vertices[2] - obb.m_center,  
        };

        // Compute the Edge Vectors for the Triangle:
        TVector3<Type> triEdges[3]
        {
            vertices[1] - vertices[0],  
            vertices[2] - vertices[1],  
            vertices[0] - vertices[2],  
        };

        // p0, p1, p2 represent the distances from the origin to the projections of the triangle
        // vertices onto the axis we are testing.
        Type p0;
        Type p1;
        Type p2;
        Type radius;

        // Test the 9 axes pertaining to the cross product of the box axes and the triangle edges.
        // If the projection intervals [-radius, radius] and [Min(p0, p1, p2), Max(p0, p1, p2)] are
        // disjoint, then there is a separating axis and the Triangle and AABB do not overlap.

        // Axis = obb.m_axis[0] x triEdges[0]
        TVector3<Type> axis = TVector3<Type>::Cross(obb.m_axes[0], triEdges[0]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_axes[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_axes[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_axes[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[0] x triEdges[1]
        axis = TVector3<Type>::Cross(obb.m_axes[0], triEdges[1]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_axes[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_axes[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_axes[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[0] x triEdges[2]
        axis = TVector3<Type>::Cross(obb.m_axes[0], triEdges[2]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_axes[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_axes[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_axes[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[1] x triEdges[0]
        axis = TVector3<Type>::Cross(obb.m_axes[1], triEdges[0]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_axes[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_axes[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_axes[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[1] x triEdges[1]
        axis = TVector3<Type>::Cross(obb.m_axes[1], triEdges[1]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_axes[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_axes[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_axes[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[1] x triEdges[2]
        axis = TVector3<Type>::Cross(obb.m_axes[1], triEdges[2]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_axes[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_axes[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_axes[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[2] x triEdges[0]
        axis = TVector3<Type>::Cross(obb.m_axes[2], triEdges[0]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_axes[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_axes[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_axes[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[2] x triEdges[1]
        axis = TVector3<Type>::Cross(obb.m_axes[2], triEdges[1]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_axes[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_axes[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_axes[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[2] x triEdges[2]
        axis = TVector3<Type>::Cross(obb.m_axes[2], triEdges[2]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_axes[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_axes[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_axes[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Test the 3 axes corresponding to the face normals of the box.
        
        axis = obb.m_axes[0];
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0];
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }
        
        axis = obb.m_axes[1];
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[1];
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        axis = obb.m_axes[2];
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[2];
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Test the separating axis corresponding to the triangle face normal.
        TPlane<Type> plane{};
        plane.m_normal = triangle.Normal();
        plane.m_distance = TVector3<Type>::Dot(plane.m_normal, vertices[0]);
        return OBBIntersectsPlane(obb, plane);
    }
}
