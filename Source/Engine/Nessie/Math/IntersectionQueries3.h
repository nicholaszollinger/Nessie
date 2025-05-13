// IntersectionQueries3.h
#pragma once
#include "Line.h"
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
    [[nodiscard]] bool AABBIntersectsPlane(const TAABox3<Type>& box, const TPlane<Type>& plane);

    template <FloatingPointType Type>
    [[nodiscard]] bool AABBInsidePlane(const TAABox3<Type>& box, const TPlane<Type>& plane);

    template <FloatingPointType Type>
    [[nodiscard]] bool AABBIntersectsHalfspace(const TAABox3<Type>& box, const TPlane<Type>& plane);

    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsAABB(const TSphere3<Type>& sphere, const TAABox3<Type>& box);
    
    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsAABB(const TSphere3<Type>& sphere, const TAABox3<Type>& box, TVector3<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsOBB(const TSphere3<Type>& sphere, const TOrientedBox3<Type>& obb);
    
    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsOBB(const TSphere3<Type>& sphere, const TOrientedBox3<Type>& obb, TVector3<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsTriangle(const TSphere3<Type>& sphere, const TTriangle3<Type>& triangle);
    
    template <FloatingPointType Type>
    [[nodiscard]] bool SphereIntersectsTriangle(const TSphere3<Type>& sphere, const TTriangle3<Type>& triangle, TVector3<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool AABBIntersectsTriangle(const TAABox3<Type>& box, const TTriangle3<Type>& triangle);

    template <FloatingPointType Type>
    [[nodiscard]] bool OBBIntersectsTriangle(const TOrientedBox3<Type>& obb, const TTriangle3<Type>& triangle);

    template <FloatingPointType Type>
    [[nodiscard]] bool SegmentIntersectsPlane(const TSegment3<Type>& segment, const TPlane<Type>& plane, TVector3<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool SegmentIntersectsSphere(const TSegment3<Type>& segment, const TSphere3<Type>& sphere, TVector3<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool SegmentIntersectsAABB(const TSegment3<Type>& segment, const TAABox3<Type>& box, TVector3<Type>& intersectionPoint);

    //template <FloatingPointType Type>
    //[[nodiscard]] bool SegmentIntersectsOBB(const TSegment3<Type>& segment, const TOrientedBox2<Type>& obb, TVector3<Type>& intersectionPoint);
    
    template <FloatingPointType Type>
    [[nodiscard]] bool LineIntersectsPlane(const TLine3<Type>& line, const TPlane<Type>& plane, TVector3<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool LineIntersectsSphere(const TLine3<Type>& line, const TSphere3<Type>& sphere, TVector3<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool LineIntersectsAABB(const TLine3<Type>& line, const TAABox3<Type>& box, TVector3<Type>& intersectionPoint);

    //template <FloatingPointType Type>
    //[[nodiscard]] bool LineIntersectsOBB(const TLine3<Type>& line, const TOrientedBox3<Type>& obb, TVector3<Type>& intersectionPoint);
    
    template <FloatingPointType Type>
    [[nodiscard]] bool RayIntersectsPlane(const TRay3<Type>& ray, const TPlane<Type>& plane, TVector3<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool RayIntersectsSphere(const TRay3<Type>& ray, const TSphere3<Type>& sphere, TVector3<Type>& intersectionPoint);

    template <FloatingPointType Type>
    [[nodiscard]] bool RayIntersectsSphere(const TRay3<Type>& ray, const TSphere3<Type>& sphere);

    template <FloatingPointType Type>
    [[nodiscard]] bool RayIntersectsAABB(const TRay3<Type>& ray, const TAABox3<Type>& box, TVector3<Type>& intersectionPoint);

    //template <FloatingPointType Type>
    //[[nodiscard]] bool RayIntersectsOBB(const TRay3<Type>& ray, const TOrientedBox3<Type>& obb, TVector3<Type>& intersectionPoint);
    
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
    bool AABBIntersectsPlane(const TAABox3<Type>& box, const TPlane<Type>& plane)
    {
        // Compute the projection interval radius of the AABB onto the Line(t) = box.center + t * plane.normal
        const TVector2<Type> extents = box.GetExtent();
        const Type radius = extents[0] * math::Abs(plane.m_normal[0])
            + extents[1] * math::Abs(plane.m_normal[1])
            + extents[2] * math::Abs(plane.m_normal[2]);

        const Type signedDistance = plane.SignedDistanceToPoint(box.Center());
        
        // Intersection occurs when the signedDistance falls within the [-radius, +radius] interval.
        return math::Abs(signedDistance) <= radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the AABB is fully behind (in the negative halfspace of) the plane.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool AABBInsidePlane(const TAABox3<Type>& box, const TPlane<Type>& plane)
    {
        // Compute the projection interval radius of the AABB onto the Line(t) = box.center + t * plane.normal
        const TVector2<Type> extents = box.GetExtent();
        const Type radius = extents[0] * math::Abs(plane.m_normal[0])
            + extents[1] * math::Abs(plane.m_normal[1])
            + extents[2] * math::Abs(plane.m_normal[2]);

        const Type signedDistance = plane.SignedDistanceToPoint(box.Center());
        return signedDistance <= -radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the AABB intersects the negative halfspace of the Plane. In other words,
    ///             this test treats anything behind the plane as solid; so if the AABB is intersecting or
    ///             fully behind the plane, this will return true.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool AABBIntersectsHalfspace(const TAABox3<Type>& box, const TPlane<Type>& plane)
    {
        // Compute the projection interval radius of the AABB onto the Line(t) = box.center + t * plane.normal
        const TVector2<Type> extents = box.GetExtent();
        const Type radius = extents[0] * math::Abs(plane.m_normal[0])
            + extents[1] * math::Abs(plane.m_normal[1])
            + extents[2] * math::Abs(plane.m_normal[2]);

        const Type signedDistance = plane.SignedDistanceToPoint(box.Center());
        return signedDistance <= radius;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if a Sphere intersects an AABB. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SphereIntersectsAABB(const TSphere3<Type>& sphere, const TAABox3<Type>& box)
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
    bool SphereIntersectsAABB(const TSphere3<Type>& sphere, const TAABox3<Type>& box, TVector3<Type>& intersectionPoint)
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
    bool AABBIntersectsTriangle(const TAABox3<Type>& box, const TTriangle3<Type>& triangle)
    {
        const auto center = box.Center();
        const auto extents = box.GetExtent();
        
        // Translate the triangle's vertices as conceptually moving the AABB to the origin.
        TVector3<Type> vertices[3]
        {
            triangle.m_vertices[0] - center,  
            triangle.m_vertices[1] - center,  
            triangle.m_vertices[2] - center,  
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
        TVector3<Type> axis = TVector3<Type>::Cross(obb.m_orientation[0], triEdges[0]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[0] x triEdges[1]
        axis = TVector3<Type>::Cross(obb.m_orientation[0], triEdges[1]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[0] x triEdges[2]
        axis = TVector3<Type>::Cross(obb.m_orientation[0], triEdges[2]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[1] x triEdges[0]
        axis = TVector3<Type>::Cross(obb.m_orientation[1], triEdges[0]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[1] x triEdges[1]
        axis = TVector3<Type>::Cross(obb.m_orientation[1], triEdges[1]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[1] x triEdges[2]
        axis = TVector3<Type>::Cross(obb.m_orientation[1], triEdges[2]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[2] x triEdges[0]
        axis = TVector3<Type>::Cross(obb.m_orientation[2], triEdges[0]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[2] x triEdges[1]
        axis = TVector3<Type>::Cross(obb.m_orientation[2], triEdges[1]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Axis = obb.m_axis[2] x triEdges[2]
        axis = TVector3<Type>::Cross(obb.m_orientation[2], triEdges[2]);
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[0], axis))
            + obb.m_extents[1] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[1], axis))
            + obb.m_extents[2] * math::Abs(TVector3<Type>::Dot(obb.m_orientation[2], axis));
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        // Test the 3 axes corresponding to the face normals of the box.
        
        axis = obb.m_orientation[0];
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[0];
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }
        
        axis = obb.m_orientation[1];
        p0 = TVector3<Type>::Dot(vertices[0], axis);
        p1 = TVector3<Type>::Dot(vertices[1], axis);
        p2 = TVector3<Type>::Dot(vertices[2], axis);
        radius = obb.m_extents[1];
        if (math::Max(p0, p1, p2) < -radius || math::Min(p0, p1, p2) > radius)
        {
            return false;
        }

        axis = obb.m_orientation[2];
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

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the segment intersects the plane and calculates the point of intersection
    ///             if valid.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SegmentIntersectsPlane(const TSegment3<Type>& segment, const TPlane<Type>& plane,
        TVector3<Type>& intersectionPoint)
    {
        // Compute the t value for the directed line ab intersecting the plane.
        const TVector3<Type> ab = segment.m_end - segment.m_start;
        const Type t = (plane.m_distance - segment.m_start.Dot(plane.m_normal)) / ab.Dot(plane.m_normal);

        // If t is within [0, 1] then there is an intersection:
        if (t >= 0 && t <= 1)
        {
            intersectionPoint = segment.m_start + (t * ab);
            return true;
        }

        return false;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the segment intersects the plane and calculates the point of intersection
    ///             if valid.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SegmentIntersectsSphere(const TSegment3<Type>& segment, const TSphere3<Type>& sphere,
        TVector3<Type>& intersectionPoint)
    {
        TVector3<Type> sphereCenterToStart = segment.m_start - sphere.m_center;
        TVector3<Type> direction = segment.m_end - segment.m_start;
        const Type sqrSegmentLength = direction.SquaredMagnitude();
        direction.Normalize();
        
        const Type projection = TVector3<Type>::Dot(sphereCenterToStart, direction);
        const Type distSqrDif = sphereCenterToStart.SquaredMagnitude() - math::Squared(sphere.m_radius);

        // Exit if the segment's origin is outside the sphere and the segment points away from the sphere.
        if (distSqrDif > static_cast<Type>(0.f) && projection > static_cast<Type>(0.f))
            return false;

        // A negative discriminant means that the segment misses the sphere.
        const Type discriminant = math::Squared(projection) - distSqrDif;
        if (discriminant < static_cast<Type>(0.f))
            return false;
        
        Type t = -projection - std::sqrt(discriminant);

        // If t is past our end point, then return false.
        if (t > std::sqrt(sqrSegmentLength))
            return false;

        // If t is negative, then the segment started outside the sphere, so clamp it to zero.
        if (t <= static_cast<Type>(0.f))
            t = static_cast<Type>(0.f);

        intersectionPoint = segment.m_start + (t * direction);

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the segment intersects the AABB and calculates the point of intersection
    ///             if valid.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool SegmentIntersectsAABB(const TSegment3<Type>& segment, const TAABox3<Type>& box, TVector3<Type>& intersectionPoint)
    {
        Type tMin = static_cast<Type>(0.f);
        Type tMax = segment.SquaredLength();
        const TVector3<Type> direction = segment.Direction();

        // For each slab (pair of 2 planes that make up opposing faces of the box)
        for (int i = 0; i < 3; ++i)
        {
            // Segment is parallel to the slab. There will be no hit if the origin is not
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
                // Compute the intersection t value of segment with
                // the near and far plane of the slab
                const Type ood = static_cast<Type>(1.f) / direction[i];
                Type t1 = (box.m_min[i] - segment.m_start[i]) * ood;
                Type t2 = (box.m_max[i] - segment.m_start[i]) * ood;

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

    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Determines if the segment intersects the OBB and calculates the point of intersection
    // ///             if valid.
    // //----------------------------------------------------------------------------------------------------
    // template <FloatingPointType Type>
    // bool SegmentIntersectsOBB(const TSegment3<Type>& segment, const TOrientedBox2<Type>& obb,
    //     TVector3<Type>& intersectionPoint)
    // {
    //     // [TODO]:
    //     return false;
    // }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the line intersects the plane and calculates the point of intersection
    ///             if valid.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool LineIntersectsPlane(const TLine3<Type>& line, const TPlane<Type>& plane, TVector3<Type>& intersectionPoint)
    {
        // A line intersects a plane if they are not parallel.
        const Type dot = TVector3<Type>::Dot(line.m_direction, plane.m_normal);
        if (math::Abs(dot) <= math::PrecisionDelta())
        {
            return false;
        }

        // Compute the t value along the line that hits the plane.
        const Type t = -(TVector3<Type>::Dot(plane.m_normal, line.m_origin) + plane.m_distance) / dot;
        intersectionPoint = line.m_origin + t * plane.m_normal;
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the line intersects the plane and calculates the point of intersection
    ///             if valid.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool LineIntersectsSphere(const TLine3<Type>& line, const TSphere3<Type>& sphere, TVector3<Type>& intersectionPoint)
    {
        TVector3<Type> sphereCenterToOrigin = line.m_origin - sphere.m_center;
        
        const Type b = TVector3<Type>::Dot(sphereCenterToOrigin, line.m_direction);
        const Type c = sphereCenterToOrigin.SquaredMagnitude() - math::Squared(sphere.m_radius);

        // A negative discriminant means that the line misses the sphere.
        const Type discriminant = math::Squared(b) - c;
        if (discriminant < static_cast<Type>(0.f))
            return false;
        
        Type t = -b - std::sqrt(discriminant);
        intersectionPoint = line.m_origin + (t * line.m_direction);

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    //      NOTES:
    //      If the line is being intersected against a number of boxes, the three divisions involved can be
    //      precomputed (one-over-direction-value, "ood") beforehand and reused for all tests.
    //
    ///		@brief : Determines if the line intersects the AABB and calculates the point of intersection
    ///             if valid.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool LineIntersectsAABB(const TLine3<Type>& line, const TAABox3<Type>& box, TVector3<Type>& intersectionPoint)
    {
        Type tMin = std::numeric_limits<Type>::min();
        Type tMax = std::numeric_limits<Type>::max();

        // For each slab (pair of 2 planes that make up opposing faces of the box)
        for (int i = 0; i < 3; ++i)
        {
            // Line is parallel to the slab. There will be no hit if the origin is not
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
                const Type ood = static_cast<Type>(1.f) / line.m_direction[i];
                Type t1 = (box.m_min[i] - line.m_origin[i]) * ood;
                Type t2 = (box.m_max[i] - line.m_origin[i]) * ood;

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

    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Determines if the line intersects the OBB and calculates the point of intersection
    // ///             if valid.
    // //----------------------------------------------------------------------------------------------------
    // template <FloatingPointType Type>
    // bool LineIntersectsOBB(const TLine3<Type>& line, const TOrientedBox3<Type>& obb, TVector3<Type>& intersectionPoint)
    // {
    //     return false;
    // }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the ray intersects the plane and calculates the point of intersection
    ///             if valid.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool RayIntersectsPlane(const TRay3<Type>& ray, const TPlane<Type>& plane, TVector3<Type>& intersectionPoint)
    {
        const Type denom = plane.m_normal.Dot(ray.m_direction);
        
        // Prevent division by 0 (parallel ray deemed to not intersect).
        // - Should I check for the origin point being on the plane?
        if (math::Abs(denom) <= math::PrecisionDelta())
        {
            return false;
        }
        
        // Compute the t value along the ray to hit the plane.
        const Type t = -(TVector3<Type>::Dot(plane.m_normal, ray.m_origin) + plane.m_distance) / denom;

        // If t is negative, (opposite direction of the ray) then there is no intersection.
        if (t <= static_cast<Type>(0.f))
            return false;

        intersectionPoint = ray.m_origin + t * ray.m_direction;
        return true;
    }


    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the ray intersects the plane and calculates the point of intersection
    ///             if valid.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool RayIntersectsSphere(const TRay3<Type>& ray, const TSphere3<Type>& sphere, TVector3<Type>& intersectionPoint)
    {
        TVector3<Type> sphereCenterToRay = ray.m_origin - sphere.m_center;
        const Type projection = TVector3<Type>::Dot(sphereCenterToRay, ray.m_direction);
        const Type distSqrDif = sphereCenterToRay.SquaredMagnitude() - math::Squared(sphere.m_radius);

        // Exit if the ray's origin is outside the sphere and the ray points away from the sphere
        if (distSqrDif > static_cast<Type>(0.f) && projection > static_cast<Type>(0.f))
        {
            return false;
        }

        // A negative discriminant means that the ray misses the sphere.
        const Type discriminant = math::Squared(projection) - distSqrDif;
        if (discriminant < static_cast<Type>(0.f))
        {
            return false;
        }

        // Ray is now found to intersect the sphere. Compute the *smallest* t value of intersection.
        // - We want the first intersection point along the ray in the case of piercing through.
        Type t = -projection - std::sqrt(discriminant);

        // If t is negative, then the ray started outside the sphere, so clamp it to zero.
        if (t <= static_cast<Type>(0.f))
            t = static_cast<Type>(0.f);

        intersectionPoint = ray.m_origin + (t * ray.m_direction);

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines if the ray intersects the plane.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool RayIntersectsSphere(const TRay3<Type>& ray, const TSphere3<Type>& sphere)
    {
        TVector3<Type> sphereCenterToRay = ray.m_origin - sphere.m_center;
        const Type distSqrDif = sphereCenterToRay.SquaredMagnitude() - math::Squared(sphere.m_radius);

        // If there is definitely at least one real root, then there must be an intersection.
        if (distSqrDif <= static_cast<Type>(0.f))
            return true;

        const Type projection = TVector3<Type>::Dot(sphereCenterToRay, ray.m_direction);
        // Early exit if the ray's origin is outside the sphere and ray is pointing away from the
        // sphere.
        if (projection > static_cast<Type>(0.f))
            return false;

        const Type discriminant = math::Squared(projection) - distSqrDif;
        
        // A negative discriminant means that the ray misses the sphere.
        if (discriminant < static_cast<Type>(0.f))
            return false;

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    //      NOTES:
    //      If the ray is being intersected against a number of boxes, the three divisions involved can be
    //      precomputed (one-over-direction-value, "ood") beforehand and reused for all tests.
    //
    ///		@brief : Determines if the ray intersects the AABB and calculates the point of intersection
    ///             if valid.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool RayIntersectsAABB(const TRay3<Type>& ray, const TAABox3<Type>& box, TVector3<Type>& intersectionPoint)
    {
        Type tMin = static_cast<Type>(0.f);
        Type tMax = std::numeric_limits<Type>::max();

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
                const Type ood = static_cast<Type>(1.f) / ray.m_direction[i];
                Type t1 = (box.m_min[i] - ray.m_origin[i]) * ood;
                Type t2 = (box.m_max[i] - ray.m_origin[i]) * ood;

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

    // //----------------------------------------------------------------------------------------------------
    // ///		@brief : Determines if the ray intersects the OBB and calculates the point of intersection
    // ///             if valid.
    // //----------------------------------------------------------------------------------------------------
    // template <FloatingPointType Type>
    // bool RayIntersectsOBB(const TRay3<Type>& ray, const TOrientedBox3<Type>& obb, TVector3<Type>& intersectionPoint)
    // {
    //     // [TODO]:
    //     // - Convert the Ray to obb coordinates, then both back to the world coordinates.
    //     // - Then pass the transformed ray and the obb as an AABB to the RayIntersectsAABB()
    //
    //     return false;
    // }
}
