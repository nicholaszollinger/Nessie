// IntersectionQueries3.h
#pragma once
#include "Line.h"
#include "OrientedBox.h"
#include "Plane.h"
#include "Ray.h"
#include "Segment.h"
#include "Sphere.h"
#include "Triangle.h"

namespace nes::geo
{
    //----------------------------------------------------------------------------------------------------
    ///	@brief : Test whether a Segment intersects a Plane. If this returns false, the point of intersection
    ///     will be invalid.
    //----------------------------------------------------------------------------------------------------
    inline bool SegmentIntersectsPlane(const Segment& segment, const Plane& plane, Vec3& intersectionPoint)
    {
        const float projStart = Vec3::Dot(plane.GetNormal(), segment.m_start);
        const float projEnd = Vec3::Dot(plane.GetNormal(), segment.m_end);

        // If the segment end points are on the same side of the plane, then there
        // is no intersection:
        if (math::SameSign(projStart, projEnd))
        {
            return false;
        }

        intersectionPoint = segment.m_start + ((segment.m_end - segment.m_start) * projStart / segment.Length());
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Test whether a line segment intersects a triangle. If this returns false, then the
    ///     intersection point is invalid.
    //----------------------------------------------------------------------------------------------------
    inline bool SegmentIntersectsTriangle(const Segment& segment, const Triangle& triangle, Vec3& intersectionPoint)
    {
        Vec3 ab = triangle.GetVertex(1) - triangle.GetVertex(0);
        ab.Normalize();
        Vec3 ac = triangle.GetVertex(2) - triangle.GetVertex(0);
        ac.Normalize();

        // Construct a plane from the triangle normal:
        const Vec3 triangleNormal = ab.Cross(ac);
        const Plane plane(triangleNormal, triangle.GetVertex(0).Length());

        // If the segment does not intersect the place of the triangle, then no intersection occurs.
        if (!SegmentIntersectsPlane(segment, plane, intersectionPoint))
        {
            return false;
        }

        float bary0;
        float bary1;
        float bary2;
        triangle.CalculateBarycentricCoordinate(intersectionPoint, bary0, bary1, bary2);

        // If the barycentric coordinates are within the triangle (all greater than zero), then
        // the segment intersects the plane within the triangle's dimensions, and thus intersects. 
        if (bary0 >= 0.f && bary1 >= 0.f && bary2 >= 0.f)
        {
            return true;
        }

        return false;
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the Sphere intersects the plane.
    //----------------------------------------------------------------------------------------------------
    inline bool SphereIntersectsPlane(const Sphere& sphere, const Plane& plane)
    {
        const float signedDistance = plane.SignedDistanceTo(sphere.GetCenter());
        
        // If the total distance is less than the radius, then the Sphere intersects.
        return math::Abs(signedDistance) <= sphere.GetRadius();
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the Sphere is fully behind (in the negative halfspace of) the plane. 
    //----------------------------------------------------------------------------------------------------
    inline bool SphereInsidePlane(const Sphere& sphere, const Plane& plane)
    {
        const float signedDistance = plane.SignedDistanceTo(sphere.GetCenter());
        return signedDistance <= -sphere.GetRadius();
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the Sphere intersects the negative halfspace of the Plane. In other words,
    ///     this test treats anything behind the plane as solid; so if the Sphere is intersecting or
    ///     fully behind the plane, this will return true.
    //----------------------------------------------------------------------------------------------------
    inline bool SphereIntersectsHalfspace(const Sphere& sphere, const Plane& plane)
    {
        const float signedDistance = plane.SignedDistanceTo(sphere.GetCenter());
        return signedDistance <= sphere.GetRadius();
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the OBB intersects the plane. 
    //----------------------------------------------------------------------------------------------------
    inline bool OBBIntersectsPlane(const OBB& obb, const Plane& plane)
    { 
        // Compute the projection interval radius of the OBB onto the Line(t) = obb.center + t * plane.normal
        const float radius = obb.m_halfExtents[0] * math::Abs(Vec3::Dot(plane.GetNormal(), obb.m_orientation.GetColumn3(0)))
                            + obb.m_halfExtents[1] * math::Abs(Vec3::Dot(plane.GetNormal(), obb.m_orientation.GetColumn3(1)))
                            + obb.m_halfExtents[2] * math::Abs(Vec3::Dot(plane.GetNormal(), obb.m_orientation.GetColumn3(2)));

        const float signedDistance = plane.SignedDistanceTo(obb.Center());
        
        // Intersection occurs when the signedDistance falls within the [-radius, +radius] interval.
        return math::Abs(signedDistance) <= radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the OBB is fully behind (in the negative halfspace of) the plane.
    //----------------------------------------------------------------------------------------------------
    inline bool OBBInsidePlane(const OBB& obb, const Plane& plane)
    {
        // Compute the projection interval radius of the OBB onto the Line(t) = obb.center + t * plane.normal
        const float radius = obb.m_halfExtents[0] * math::Abs(Vec3::Dot(plane.GetNormal(), obb.m_orientation.GetColumn3(0)))
                            + obb.m_halfExtents[1] * math::Abs(Vec3::Dot(plane.GetNormal(), obb.m_orientation.GetColumn3(1)))
                            + obb.m_halfExtents[2] * math::Abs(Vec3::Dot(plane.GetNormal(), obb.m_orientation.GetColumn3(2)));

        const float signedDistance = plane.SignedDistanceTo(obb.Center());
        return signedDistance <= -radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the OBB intersects the negative halfspace of the Plane. In other words,
    ///     this test treats anything behind the plane as solid; so if the OBB is intersecting or
    ///     fully behind the plane, this will return true.
    //----------------------------------------------------------------------------------------------------
    inline bool OBBIntersectsHalfspace(const OBB& obb, const Plane& plane)
    {
        // Compute the projection interval radius of the OBB onto the Line(t) = obb.center + t * plane.normal
        const float radius = obb.m_halfExtents[0] * math::Abs(Vec3::Dot(plane.GetNormal(), obb.m_orientation.GetColumn3(0)))
                            + obb.m_halfExtents[1] * math::Abs(Vec3::Dot(plane.GetNormal(), obb.m_orientation.GetColumn3(1)))
                            + obb.m_halfExtents[2] * math::Abs(Vec3::Dot(plane.GetNormal(), obb.m_orientation.GetColumn3(2)));

        const float signedDistance = plane.SignedDistanceTo(obb.Center());
        return signedDistance <= radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the AABB intersects the plane. 
    //----------------------------------------------------------------------------------------------------
    inline bool AABBIntersectsPlane(const AABox& box, const Plane& plane)
    {
        // Compute the projection interval radius of the AABB onto the Line(t) = box.center + t * plane.normal
        const Vec3 extents = box.Extent();
        const Vec3 planeNormal = plane.GetNormal();
        const float radius = extents[0] * math::Abs(planeNormal[0])
            + extents[1] * math::Abs(planeNormal[1])
            + extents[2] * math::Abs(planeNormal[2]);

        const float signedDistance = plane.SignedDistanceTo(box.Center());
        
        // Intersection occurs when the signedDistance falls within the [-radius, +radius] interval.
        return math::Abs(signedDistance) <= radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the AABB is fully behind (in the negative halfspace of) the plane.
    //----------------------------------------------------------------------------------------------------
    inline bool AABBInsidePlane(const AABox& box, const Plane& plane)
    {
        // Compute the projection interval radius of the AABB onto the Line(t) = box.center + t * plane.normal
        const Vec3 extents = box.Extent();
        const Vec3 planeNormal = plane.GetNormal();
        const float radius = extents[0] * math::Abs(planeNormal[0])
            + extents[1] * math::Abs(planeNormal[1])
            + extents[2] * math::Abs(planeNormal[2]);

        const float signedDistance = plane.SignedDistanceTo(box.Center());
        return signedDistance <= -radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the AABB intersects the negative halfspace of the Plane. In other words,
    ///     this test treats anything behind the plane as solid; so if the AABB is intersecting or
    ///     fully behind the plane, this will return true.
    //----------------------------------------------------------------------------------------------------
    inline bool AABBIntersectsHalfspace(const AABox& box, const Plane& plane)
    {
        // Compute the projection interval radius of the AABB onto the Line(t) = box.center + t * plane.normal
        const Vec3 extents = box.Extent();
        const Vec3 planeNormal = plane.GetNormal();
        const float radius = extents[0] * math::Abs(planeNormal[0])
            + extents[1] * math::Abs(planeNormal[1])
            + extents[2] * math::Abs(planeNormal[2]);

        const float signedDistance = plane.SignedDistanceTo(box.Center());
        return signedDistance <= radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if a Sphere intersects an AABB. 
    //----------------------------------------------------------------------------------------------------
    inline bool SphereIntersectsAABB(const Sphere& sphere, const AABox& box)
    {
        const float sqrDist = box.GetSqrDistanceTo(sphere.GetCenter());
        return sqrDist <= math::Squared(sphere.GetRadius());
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Determines if a Sphere intersects an AABB, and returns the point of intersection. If this
    ///     function returns false, they do not intersect, and the point will still be the closest point on the AABB
    ///     to the sphere.
    //----------------------------------------------------------------------------------------------------
    inline bool SphereIntersectsAABB(const Sphere& sphere, const AABox& box, Vec3& intersectionPoint)
    {
        intersectionPoint = box.ClosestPointTo(sphere.GetCenter());
        const float sqrDist = (intersectionPoint - sphere.GetCenter()).LengthSqr();
        return sqrDist <= math::Squared(sphere.GetRadius());
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if a Sphere intersects an OBB. 
    //----------------------------------------------------------------------------------------------------
    inline bool SphereIntersectsOBB(const Sphere& sphere, const OBB& obb)
    {
        const float sqrDist = obb.DistanceSqrToPoint(sphere.GetCenter());
        return sqrDist <= math::Squared(sphere.GetRadius());
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if a Sphere intersects an OBB, and returns the point of intersection. If this
    ///     function returns false, they do not intersect, and the point will still be the closest point on the OBB
    ///     to the sphere.
    //----------------------------------------------------------------------------------------------------
    inline bool SphereIntersectsOBB(const Sphere& sphere, const OBB& obb, Vec3& intersectionPoint)
    {
        intersectionPoint = obb.ClosestPointTo(sphere.GetCenter());
        const float sqrDist = (intersectionPoint - sphere.GetCenter()).LengthSqr();
        return sqrDist <= math::Squared(sphere.GetRadius());
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if a Sphere intersects a Triangle. 
    //----------------------------------------------------------------------------------------------------
    inline bool SphereIntersectsTriangle(const Sphere& sphere, const Triangle& triangle)
    {
        const float sqrDist = triangle.DistanceSqr(sphere.GetCenter());
        return sqrDist <= math::Squared(sphere.GetRadius());
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if a Sphere intersects a triangle, and returns the point of intersection. If this
    ///     function returns false, they do not intersect, and the point will still be the closest point on the
    ///     triangle to the sphere.
    //----------------------------------------------------------------------------------------------------
    inline bool SphereIntersectsTriangle(const Sphere& sphere, const Triangle& triangle, Vec3& intersectionPoint)
    {
        intersectionPoint = triangle.ClosestPointTo(sphere.GetCenter());
        const float sqrDist = (intersectionPoint - sphere.GetCenter()).LengthSqr();
        return sqrDist <= math::Squared(sphere.GetRadius());
    }

    //----------------------------------------------------------------------------------------------------
    // NOTES:
    // pg 170-172 of "Real-Time Collision Detection".
    // - This uses the separating axis theorem (SAT) to test the 13 axes required to check for intersection.
    //     1. Three face normals of the AABB
    //     2. One face normal of the Triangle
    //     3. Nine axes given by the cross-products of the combination of edges from both.
    // Since the box axes are the basis vectors, much of the implementation can be simplified. The OBB
    // Triangle intersection method is the same, but without the shortcuts.
    //
    // [TODO]: To make this more robust, I need to check for a degenerate or oversized triangle
    //         and check for parallel edges with the 9 test axes. The solution to this is on
    //         pg 159
    //
    /// @brief : Determine if an AABB intersects with a Triangle.
    //----------------------------------------------------------------------------------------------------
    inline bool AABBIntersectsTriangle(const AABox& box, const Triangle& triangle)
    {
        const auto center = box.Center();
        const auto extents = box.Extent();
        
        // Translate the triangle's vertices as conceptually moving the AABB to the origin.
        Vec3 vertices[3]
        {
            triangle.GetVertex(0) - center,  
            triangle.GetVertex(1) - center,  
            triangle.GetVertex(2) - center,  
        };

        // Compute the Edge Vectors for the Triangle:
        Vec3 triEdges[3]
        {
            vertices[1] - vertices[0],  
            vertices[2] - vertices[1],  
            vertices[0] - vertices[2],  
        };

        // p0, p1, p2 represent the distances from the origin to the projections of the triangle
        // vertices onto the axis we are testing.
        float p0;
        float p1;
        float p2;
        float radius;
        
        // Test the 9 axes pertaining to the cross product of the box axes and the triangle edges.
        // If the projection intervals [-radius, radius] and [Min(p0, p1, p2), Max(p0, p1, p2)] are
        // disjoint, then there is a separating axis and the Triangle and AABB do not overlap.
        
        // axis = (1, 0, 0) x TriEdge[0] = (0, -TriEdge[0].z, TriEdge[0].y)
        p0 = vertices[0].z * vertices[1].y - vertices[0].y * vertices[1].z;
        // p1 == p0
        p2 = vertices[2].y * -triEdges[0].z + vertices[2].z * triEdges[0].y;
        radius = extents[1] * math::Abs(triEdges[0].z) + extents[2] * math::Abs(triEdges[0].y);
        if (math::Max(-math::Max(p0, p2), math::Min(p0, p2)) > radius)
        {
            return false;
        }

        // axis = (1, 0, 0) x TriEdge[1] = (0, -TriEdge[1].z, TriEdge[1].y)
        p0 = vertices[0].y * -triEdges[1].z + vertices[0].z * triEdges[1].y;
        p1 = vertices[1].y * -triEdges[1].z + vertices[1].z * triEdges[1].y;
        // p2 == p1
        radius = extents[1] * math::Abs(triEdges[1].z) + extents[2] * math::Abs(triEdges[1].y);
        if (math::Max(-math::Max(p0, p1), math::Min(p0, p1)) > radius)
        {
            return false;
        }
        
        // axis = (1, 0, 0) x TriEdge[2] = (0, -TriEdge[2].z, TriEdge[2].y)
        p0 = vertices[0].y * vertices[2].z - vertices[0].z * vertices[2].y;
        p1 = vertices[1].y * -triEdges[2].z + vertices[1].z * triEdges[2].y;
        // p2 == p0
        radius = extents[1] * math::Abs(triEdges[2].z) + extents[2] * math::Abs(triEdges[2].y);
        if (math::Max(-math::Max(p0, p1), math::Min(p0, p1)) > radius)
        {
            return false;
        }

        // axis = (0, 1, 0) x TriEdge[0] = (TriEdge[0].z, 0, -TriEdge[0].x)
        p0 = vertices[0].x * vertices[1].z - vertices[0].z * vertices[1].x;
        // p1 == p0
        p2 = vertices[2].x * triEdges[0].z - vertices[2].z * triEdges[0].x;
        radius = extents[0] * math::Abs(triEdges[0].z) + extents[2] * math::Abs(triEdges[0].x);
        if (math::Max(-math::Max(p0, p2), math::Min(p0, p2)) > radius)
        {
            return false;
        }
        
        // axis = (0, 1, 0) x TriEdge[1] = (TriEdge[1].z, 0, -TriEdge[1].x)
        p0 = vertices[0].x * triEdges[1].z - vertices[0].z * triEdges[1].x;
        p1 = vertices[1].x * vertices[2].z - vertices[1].z * vertices[2].x;
        // p2 == p1
        radius = extents[0] * math::Abs(triEdges[1].z) + extents[2] * math::Abs(triEdges[1].x);
        if (math::Max(-math::Max(p0, p1), math::Min(p0, p1)) > radius)
        {
            return false;
        }
        
        // axis = (0, 1, 0) x TriEdge[2] = (TriEdge[2].z, 0, -TriEdge[2].x)
        p0 = vertices[0].z * vertices[2].x - vertices[0].x * vertices[2].z;
        p1 = vertices[1].x * triEdges[2].z - vertices[1].z * triEdges[2].x;
        // p2 == p0
        radius = extents[0] * math::Abs(triEdges[2].z) + extents[2] * math::Abs(triEdges[2].x);
        if (math::Max(-math::Max(p0, p1), math::Min(p0, p1)) > radius)
        {
            return false;
        }

        // axis = (0, 0, 1) x TriEdge[0] = (-TriEdge[0].y, TriEdge[0].x, 0)
        p0 = vertices[0].y * vertices[1].x - vertices[0].x * vertices[1].y;
        // p1 == p0
        p2 = vertices[2].y * triEdges[0].x - vertices[2].x * triEdges[0].y;
        radius = extents[0] * math::Abs(triEdges[0].y) + extents[1] * math::Abs(triEdges[0].x);
        if (math::Max(-math::Max(p0, p2), math::Min(p0, p2)) > radius)
        {
            return false;
        }
        
        // axis = (0, 0, 1) x TriEdge[1] = (-TriEdge[1].y, TriEdge[1].x, 0)
        p0 = vertices[0].y * triEdges[1].x - vertices[0].x * triEdges[1].y;
        p1 = vertices[1].y * vertices[2].x - vertices[1].x * vertices[2].y;
        // p2 == p1
        radius = extents[0] * math::Abs(triEdges[1].y) + extents[1] * math::Abs(triEdges[1].x);
        if (math::Max(-math::Max(p0, p1), math::Min(p0, p1)) > radius)
        {
            return false;
        }
        
        // axis = (0, 0, 1) x TriEdge[2] = (-TriEdge[2].y, TriEdge[2].x, 0)
        p0 = vertices[0].x * vertices[2].y - vertices[0].y * vertices[2].x;
        p1 = vertices[1].y * triEdges[2].x - vertices[1].x * triEdges[2].y;
        // p2 == p0
        radius = extents[0] * math::Abs(triEdges[2].y) + extents[1] * math::Abs(triEdges[2].x);
        if (math::Max(-math::Max(p0, p1), math::Min(p0, p1)) > radius)
        {
            return false;
        }
        
        // Test the 3 axes corresponding to the face normals of the box.

        // X
        if (math::Max(vertices[0].x, vertices[1].x, vertices[2].x) < -extents.x
            || math::Min(vertices[0].x, vertices[1].x, vertices[2].x) > extents.x)
        {
            return false;
        }

        // Y
        if (math::Max(vertices[0].y, vertices[1].y, vertices[2].y) < -extents.y
            || math::Min(vertices[0].y, vertices[1].y, vertices[2].y) > extents.y)
        {
            return false;
        }

        // Z
        if (math::Max(vertices[0].z, vertices[1].z, vertices[2].z) < -extents.z
            || math::Min(vertices[0].z, vertices[1].z, vertices[2].z) > extents.z)
        {
            return false;
        }

        // Test the separating axis corresponding to the triangle face normal.
        Plane plane{};
        plane.GetNormal() = triangle.Normal();
        plane.SetConstant(Vec3::Dot(plane.GetNormal(), vertices[0]));
        return AABBIntersectsPlane(box, plane);
    }

    //----------------------------------------------------------------------------------------------------
    // pg 170-172 of "Real-Time Collision Detection".
    // - This uses the separating axis theorem (SAT) to test the 13 axes required to check for intersection.
    //      1. Three face normals of the OBB
    //      2. One face normal of the Triangle
    //      3. Nine axes given by the cross-products of the combination of edges from both.
    //
    // [TODO]: To make this more robust, I need to check for a degenerate or oversized triangle
    //      and check for parallel edges with the 9 test axes. The solution to this is on
    //      pg 159
    //
    ///	@brief : Determine if an OBB intersects with a Triangle.
    //----------------------------------------------------------------------------------------------------
    inline bool OBBIntersectsTriangle(const OBB& obb, const Triangle& triangle)
    {
        // Translate the triangle's vertices as conceptually moving the OBB to the origin.
        Vec3 vertices[3]
        {
            triangle.GetVertex(0) - obb.Center(),  
            triangle.GetVertex(1) - obb.Center(),  
            triangle.GetVertex(2) - obb.Center(),  
        };

        // Compute the Edge Vectors for the Triangle:
        Vec3 triEdges[3]
        {
            vertices[1] - vertices[0],  
            vertices[2] - vertices[1],  
            vertices[0] - vertices[2],  
        };

        // p0, p1, p2 represent the distances from the origin to the projections of the triangle
        // vertices onto the axis we are testing.
        float p0;
        float p1;
        float p2;
        float radius;

        // Test the 9 axes pertaining to the cross product of the box axes and the triangle edges.
        // If the projection intervals [-radius, radius] and [Min(p0, p1, p2), Max(p0, p1, p2)] are
        // disjoint, then there is a separating axis and the Triangle and AABB do not overlap.

        // Axis = obb.m_axis[0] x triEdges[0]
        Vec3 axis = Vec3::Cross(obb.m_orientation.GetColumn3(0), triEdges[0]);
        p0 = Vec3::Dot(vertices[0], axis);
        p1 = Vec3::Dot(vertices[1], axis);
        p2 = Vec3::Dot(vertices[2], axis);
        radius = obb.m_halfExtents[0] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(0), axis))
            + obb.m_halfExtents[1] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(1), axis))
            + obb.m_halfExtents[2] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(2), axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[0] x triEdges[1]
        axis = Vec3::Cross(obb.m_orientation.GetColumn3(0), triEdges[1]);
        p0 = Vec3::Dot(vertices[0], axis);
        p1 = Vec3::Dot(vertices[1], axis);
        p2 = Vec3::Dot(vertices[2], axis);
        radius = obb.m_halfExtents[0] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(0), axis))
            + obb.m_halfExtents[1] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(1), axis))
            + obb.m_halfExtents[2] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(2), axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[0] x triEdges[2]
        axis = Vec3::Cross(obb.m_orientation.GetColumn3(0), triEdges[2]);
        p0 = Vec3::Dot(vertices[0], axis);
        p1 = Vec3::Dot(vertices[1], axis);
        p2 = Vec3::Dot(vertices[2], axis);
        radius = obb.m_halfExtents[0] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(0), axis))
            + obb.m_halfExtents[1] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(1), axis))
            + obb.m_halfExtents[2] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(2), axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[1] x triEdges[0]
        axis = Vec3::Cross(obb.m_orientation.GetColumn3(1), triEdges[0]);
        p0 = Vec3::Dot(vertices[0], axis);
        p1 = Vec3::Dot(vertices[1], axis);
        p2 = Vec3::Dot(vertices[2], axis);
        radius = obb.m_halfExtents[0] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(0), axis))
            + obb.m_halfExtents[1] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(1), axis))
            + obb.m_halfExtents[2] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(2), axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[1] x triEdges[1]
        axis = Vec3::Cross(obb.m_orientation.GetColumn3(1), triEdges[1]);
        p0 = Vec3::Dot(vertices[0], axis);
        p1 = Vec3::Dot(vertices[1], axis);
        p2 = Vec3::Dot(vertices[2], axis);
        radius = obb.m_halfExtents[0] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(0), axis))
            + obb.m_halfExtents[1] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(1), axis))
            + obb.m_halfExtents[2] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(2), axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[1] x triEdges[2]
        axis = Vec3::Cross(obb.m_orientation.GetColumn3(1), triEdges[2]);
        p0 = Vec3::Dot(vertices[0], axis);
        p1 = Vec3::Dot(vertices[1], axis);
        p2 = Vec3::Dot(vertices[2], axis);
        radius = obb.m_halfExtents[0] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(0), axis))
            + obb.m_halfExtents[1] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(1), axis))
            + obb.m_halfExtents[2] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(2), axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[2] x triEdges[0]
        axis = Vec3::Cross(obb.m_orientation.GetColumn3(2), triEdges[0]);
        p0 = Vec3::Dot(vertices[0], axis);
        p1 = Vec3::Dot(vertices[1], axis);
        p2 = Vec3::Dot(vertices[2], axis);
        radius = obb.m_halfExtents[0] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(0), axis))
            + obb.m_halfExtents[1] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(1), axis))
            + obb.m_halfExtents[2] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(2), axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[2] x triEdges[1]
        axis = Vec3::Cross(obb.m_orientation.GetColumn3(2), triEdges[1]);
        p0 = Vec3::Dot(vertices[0], axis);
        p1 = Vec3::Dot(vertices[1], axis);
        p2 = Vec3::Dot(vertices[2], axis);
        radius = obb.m_halfExtents[0] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(0), axis))
            + obb.m_halfExtents[1] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(1), axis))
            + obb.m_halfExtents[2] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(2), axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[2] x triEdges[2]
        axis = Vec3::Cross(obb.m_orientation.GetColumn3(2), triEdges[2]);
        p0 = Vec3::Dot(vertices[0], axis);
        p1 = Vec3::Dot(vertices[1], axis);
        p2 = Vec3::Dot(vertices[2], axis);
        radius = obb.m_halfExtents[0] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(0), axis))
            + obb.m_halfExtents[1] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(1), axis))
            + obb.m_halfExtents[2] * math::Abs(Vec3::Dot(obb.m_orientation.GetColumn3(2), axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Test the 3 axes corresponding to the face normals of the box.
        
        axis = obb.m_orientation.GetColumn3(0);
        p0 = Vec3::Dot(vertices[0], axis);
        p1 = Vec3::Dot(vertices[1], axis);
        p2 = Vec3::Dot(vertices[2], axis);
        radius = obb.m_halfExtents[0];
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }
        
        axis = obb.m_orientation.GetColumn3(1);
        p0 = Vec3::Dot(vertices[0], axis);
        p1 = Vec3::Dot(vertices[1], axis);
        p2 = Vec3::Dot(vertices[2], axis);
        radius = obb.m_halfExtents[1];
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        axis = obb.m_orientation.GetColumn3(2);
        p0 = Vec3::Dot(vertices[0], axis);
        p1 = Vec3::Dot(vertices[1], axis);
        p2 = Vec3::Dot(vertices[2], axis);
        radius = obb.m_halfExtents[2];
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Test the separating axis corresponding to the triangle face normal.
        Plane plane{};
        plane.SetNormal(triangle.Normal());
        plane.SetConstant(Vec3::Dot(plane.GetNormal(), vertices[0]));
        return OBBIntersectsPlane(obb, plane);
    }
    
    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the segment intersects the plane and calculates the point of intersection
    ///     if valid.
    //----------------------------------------------------------------------------------------------------
    inline bool SegmentIntersectsSphere(const Segment& segment, const Sphere& sphere, Vec3& intersectionPoint)
    {
        const Vec3 sphereCenterToStart = segment.m_start - sphere.GetCenter();
        Vec3 direction = segment.m_end - segment.m_start;
        const float sqrSegmentLength = direction.LengthSqr();
        direction.Normalize();
        
        const float projection = Vec3::Dot(sphereCenterToStart, direction);
        const float distSqrDif = sphereCenterToStart.LengthSqr() - math::Squared(sphere.GetRadius());

        // Exit if the segment's origin is outside the sphere and the segment points away from the sphere.
        if (distSqrDif > 0.f && projection > 0.f)
            return false;

        // A negative discriminant means that the segment misses the sphere.
        const float discriminant = math::Squared(projection) - distSqrDif;
        if (discriminant < 0.f)
            return false;
        
        float t = -projection - std::sqrt(discriminant);

        // If t is past our end point, then return false.
        if (t > std::sqrt(sqrSegmentLength))
            return false;

        // If t is negative, then the segment started outside the sphere, so clamp it to zero.
        if (t <= 0.f)
            t = 0.f;

        intersectionPoint = segment.m_start + (t * direction);

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the segment intersects the AABB and calculates the point of intersection
    ///     if valid.
    //----------------------------------------------------------------------------------------------------
    inline bool SegmentIntersectsAABB(const Segment& segment, const AABox& box, Vec3& intersectionPoint)
    {
        float tMin = 0.f;
        float tMax = segment.LengthSqr();
        const Vec3 direction = segment.Direction();

        // For each slab (pair of 2 planes that make up opposing faces of the box)
        for (int i = 0; i < 3; ++i)
        {
            // Then segment is parallel to the slab. There will be no hit if the origin is not
            // within the slab:
            if (math::Abs(direction[i]) < math::PrecisionDelta())
            {
                if (segment.m_start[i] < box.m_min[i] || segment.m_start[i] > box.m_max[i])
                {
                    return false;
                }
            }

            else
            {
                // Compute the intersection t value of the segment with
                // the near and far plane of the slab
                const float ood = 1.f / direction[i];
                float t1 = (box.m_min[i] - segment.m_start[i]) * ood;
                float t2 = (box.m_max[i] - segment.m_start[i]) * ood;

                // Make t1 be the intersection with the near plane, t2 the far plane.
                if (t1 > t2)
                    std::swap(t1, t2);

                // Compute the intersection of slab intersection intervals
                tMin = math::Max(tMin, t1);
                tMax = math::Min(tMax, t2);

                // Exit with no collision as soon as slab intersection becomes empty:
                if (tMin > tMax)
                    return false;
            }
        }

        intersectionPoint = segment.m_start + (direction * tMin);
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the line intersects the plane and calculates the point of intersection
    ///     if valid.
    //----------------------------------------------------------------------------------------------------
    inline bool LineIntersectsPlane(const Line3& line, const Plane& plane, Vec3& intersectionPoint)
    {
        // A line intersects a plane if they are not parallel.
        const float dot = Vec3::Dot(line.m_direction, plane.GetNormal());
        if (math::Abs(dot) <= math::PrecisionDelta())
        {
            return false;
        }

        // Compute the t value along the line that hits the plane.
        const float t = -(Vec3::Dot(plane.GetNormal(), line.m_origin) + plane.GetConstant()) / dot;
        intersectionPoint = line.m_origin + t * plane.GetNormal();
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the line intersects the plane and calculates the point of intersection
    ///     if valid.
    //----------------------------------------------------------------------------------------------------
    inline bool LineIntersectsSphere(const Line3& line, const Sphere& sphere, Vec3& intersectionPoint)
    {
        Vec3 sphereCenterToOrigin = line.m_origin - sphere.GetCenter();
        
        const float b = Vec3::Dot(sphereCenterToOrigin, line.m_direction);
        const float c = sphereCenterToOrigin.LengthSqr() - math::Squared(sphere.GetRadius());

        // A negative discriminant means that the line misses the sphere.
        const float discriminant = math::Squared(b) - c;
        if (discriminant < 0.f)
            return false;
        
        const float t = -b - std::sqrt(discriminant);
        intersectionPoint = line.m_origin + (t * line.m_direction);

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    // NOTES:
    // If the line is being intersected against a number of boxes, the three divisions involved can be
    // precomputed (one-over-direction-value, "ood") beforehand and reused for all tests.
    //
    ///	@brief : Determines if the line intersects the AABB and calculates the point of intersection
    ///     if valid.
    //----------------------------------------------------------------------------------------------------
    inline bool LineIntersectsAABB(const Line3& line, const AABox& box, Vec3& intersectionPoint)
    {
        float tMin = std::numeric_limits<float>::min();
        float tMax = std::numeric_limits<float>::max();

        // For each slab (pair of 2 planes that make up opposing faces of the box)
        for (int i = 0; i < 3; ++i)
        {
            // The line is parallel to the slab. There will be no hit if the origin is not
            // within the slab:
            if (math::Abs(line.m_direction[i]) < math::PrecisionDelta())
            {
                if (line.m_origin[i] < box.m_min[i] || line.m_origin[i] > box.m_max[i])
                {
                    return false;
                }
            }

            else
            {
                // Compute the intersection t value of line with
                // the near and far plane of the slab
                const float ood = 0.f / line.m_direction[i];
                float t1 = (box.m_min[i] - line.m_origin[i]) * ood;
                float t2 = (box.m_max[i] - line.m_origin[i]) * ood;

                // Make t1 be the intersection with the near plane, t2 the far plane.
                if (t1 > t2)
                    std::swap(t1, t2);

                // Compute the intersection of slab intersection intervals
                tMin = math::Min(tMin, t1);
                tMax = math::Min(tMax, t2);

                // Exit with no collision as soon as slab intersection becomes empty:
                if (tMin > tMax)
                    return false;
            }
        }

        intersectionPoint = line.m_origin + (line.m_direction * tMin);
        return true;
    }
    
    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the ray intersects the plane and calculates the point of intersection
    ///     if valid.
    //----------------------------------------------------------------------------------------------------
    inline bool RayIntersectsPlane(const Ray& ray, const Plane& plane, Vec3& intersectionPoint)
    {
        const Vec3 planeNormal = plane.GetNormal();
        const float denom = planeNormal.Dot(ray.m_direction);
        
        // Prevent division by 0 (parallel ray deemed to not intersect).
        // - Should I check for the origin point being on the plane?
        if (math::Abs(denom) <= math::PrecisionDelta())
        {
            return false;
        }
        
        // Compute the t value along the ray to hit the plane.
        const float t = -(Vec3::Dot(planeNormal, ray.m_origin) + plane.GetConstant()) / denom;

        // If t is negative, (opposite direction of the ray) then there is no intersection.
        if (t <= 0.f)
            return false;

        intersectionPoint = ray.m_origin + t * ray.m_direction;
        return true;
    }


    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the ray intersects the plane and calculates the point of intersection
    ///     if valid.
    //----------------------------------------------------------------------------------------------------
    inline bool RayIntersectsSphere(const Ray& ray, const Sphere& sphere, Vec3& intersectionPoint)
    {
        Vec3 sphereCenterToRay = ray.m_origin - sphere.GetCenter();
        const float projection = Vec3::Dot(sphereCenterToRay, ray.m_direction);
        const float distSqrDif = sphereCenterToRay.LengthSqr() - math::Squared(sphere.GetRadius());

        // Exit if the ray's origin is outside the sphere and the ray points away from the sphere
        if (distSqrDif > 0.f && projection > 0.f)
        {
            return false;
        }

        // A negative discriminant means that the ray misses the sphere.
        const float discriminant = math::Squared(projection) - distSqrDif;
        if (discriminant < 0.f)
        {
            return false;
        }

        // Ray is now found to intersect the sphere. Compute the *smallest* t value of intersection.
        // - We want the first intersection point along the ray in the case of piercing through.
        float t = -projection - std::sqrt(discriminant);

        // If t is negative, then the ray started outside the sphere, so clamp it to zero.
        if (t <= 0.f)
            t = 0.f;

        intersectionPoint = ray.m_origin + (t * ray.m_direction);

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Determines if the ray intersects the plane.
    //----------------------------------------------------------------------------------------------------
    inline bool RayIntersectsSphere(const Ray& ray, const Sphere& sphere)
    {
        Vec3 sphereCenterToRay = ray.m_origin - sphere.GetCenter();
        const float distSqrDif = sphereCenterToRay.LengthSqr() - math::Squared(sphere.GetRadius());

        // If there is definitely at least one real root, then there must be an intersection.
        if (distSqrDif <= 0.f)
            return true;

        const float projection = Vec3::Dot(sphereCenterToRay, ray.m_direction);
        // Early exit if the ray's origin is outside the sphere and ray is pointing away from the
        // sphere.
        if (projection > 0.f)
            return false;

        const float discriminant = math::Squared(projection) - distSqrDif;
        
        // A negative discriminant means that the ray misses the sphere.
        if (discriminant < 0.f)
            return false;

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    //  NOTES:
    //  If the ray is being intersected against a number of boxes, the three divisions involved can be
    //  precomputed (one-over-direction-value, "ood") beforehand and reused for all tests.
    //
    ///	@brief : Determines if the ray intersects the AABB and calculates the point of intersection
    ///     if valid.
    //----------------------------------------------------------------------------------------------------
    inline bool RayIntersectsAABB(const Ray& ray, const AABox& box, Vec3& intersectionPoint)
    {
        float tMin = 0.f;
        float tMax = std::numeric_limits<float>::max();

        // For each slab (pair of 2 planes that make up opposing faces of the box)
        for (int i = 0; i < 3; ++i)
        {
            // Ray is parallel to the slab. There will be no hit if the origin is not
            // within the slab:
            if (math::Abs(ray.m_direction[i]) < math::PrecisionDelta())
            {
                if (ray.m_origin[i] < box.m_min[i] || ray.m_origin[i] > box.m_max[i])
                {
                    return false;
                }
            }

            else
            {
                // Compute the intersection t value of ray with
                // the near and far plane of the slab
                const float ood = 1.f / ray.m_direction[i];
                float t1 = (box.m_min[i] - ray.m_origin[i]) * ood;
                float t2 = (box.m_max[i] - ray.m_origin[i]) * ood;

                // Make t1 be the intersection with the near plane, t2 the far plane.
                if (t1 > t2)
                    std::swap(t1, t2);

                // Compute the intersection of slab intersection intervals
                tMin = math::Min(tMin, t1);
                tMax = math::Min(tMax, t2);

                // Exit with no collision as soon as slab intersection becomes empty:
                if (tMin > tMax)
                    return false;
            }
        }

        intersectionPoint = ray.m_origin + (ray.m_direction * tMin);
        return true;
    }
}
