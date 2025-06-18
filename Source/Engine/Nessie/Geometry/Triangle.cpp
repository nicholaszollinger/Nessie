// Triangle.cpp
#include "Triangle.h"
#include "ClosestPoint.h"
#include "Math/Mat4.h"

namespace nes
{
    namespace geo
    {
        float CalculateSignedAreaOfTriangle(const Vec3 a, const Vec3 b, const Vec3 c)
        {
            return 0.5f * Vec3::Cross(b - a, c - a).Length();
        }
        
        float CalculateSignedAreaOfTriangle2D(const Vec2 a, const Vec2 b, const Vec2 c)
        {
            return 0.5f * ((a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x));
        }
        
        bool TriangleContainsPoint(const Vec3 a, const Vec3 b, const Vec3 c, const Vec3 p)
        {
            // This uses Barycentric Coordinates to determine if the point is contained by the triangle. This is true
            //  if the u, v, w coordinates are all within the range [0, 1].
            float u, v, w;
            ClosestPoint::GetBaryCentricCoordinates(a, b, c, p, u, v, w);
            return v >= 0.f && w >= 0.f && v + w <= 1.f;
        }
        
        bool TriangleContainsPoint2D(const Vec2 a, const Vec2 b, const Vec2 c, const Vec2 p)
        {
            float u, v, w;
            ClosestPoint::GetBaryCentricCoordinates(a, b, c, p, u, v, w);
            return v >= 0.f && w >= 0.f && v + w <= 1.f;
        }
    }
    
    Triangle2::Triangle2(const Vec2 v0, const Vec2 v1, const Vec2 v2)
    {
        v0.StoreFloat2(&m_vertices[0]);
        v1.StoreFloat2(&m_vertices[1]);
        v2.StoreFloat2(&m_vertices[2]);
    }
    
    Triangle2::Triangle2(const Vec2 vertices[3])
    {
        vertices[0].StoreFloat2(&m_vertices[0]);
        vertices[1].StoreFloat2(&m_vertices[1]);
        vertices[2].StoreFloat2(&m_vertices[2]);
    }

    Vec2 Triangle2::Centroid() const
    {
        return (Vec2(m_vertices[0]) + Vec2(m_vertices[1]) + Vec2(m_vertices[2])) * (1.f / 3.f);
    }
    
    float Triangle2::SignedArea() const
    {
        return geo::CalculateSignedAreaOfTriangle2D(Vec2(m_vertices[0]), Vec2(m_vertices[1]), Vec2(m_vertices[2]));
    }

    bool Triangle2::Contains(const Vec2 point) const
    {
        return geo::TriangleContainsPoint2D(Vec2(m_vertices[0]), Vec2(m_vertices[1]), Vec2(m_vertices[2]), point);
    }

    Vec2 Triangle2::PointFromBaryCoordinates(const float bary0, const float bary1, const float bary2) const
    {
        return bary0 * Vec2(m_vertices[0]) + bary1 * Vec2(m_vertices[1]) + bary2 * Vec2(m_vertices[2]);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Barycentric coordinate for the point p.
    //----------------------------------------------------------------------------------------------------
    void Triangle2::CalculateBarycentricCoordinate(const Vec2 p, float& bary0, float& bary1,
        float& bary2) const
    {
        ClosestPoint::GetBaryCentricCoordinates(Vec2(m_vertices[0]), Vec2(m_vertices[1]), Vec2(m_vertices[2]), p, bary0, bary1, bary2);
    }
    
    Vec2 Triangle2::ClosestPointTo(const Vec2 queryPoint) const
    {
        return ClosestPoint::GetClosestPointOnTriangle(
            Vec2(m_vertices[0])
            , Vec2(m_vertices[1])
            , Vec2(m_vertices[2])
            , queryPoint);
    }

    float Triangle2::DistanceSqrTo(const Vec2 queryPoint) const
    {
        const Vec2 closestPoint = ClosestPointTo(queryPoint);
        return (queryPoint - closestPoint).LengthSqr();
    }
    
    Triangle2 Triangle2::Transformed(const Mat44& m) const
    {
        const Vec2 v0 = m.TransformPoint(Vec2(m_vertices[0]));
        const Vec2 v1 = m.TransformPoint(Vec2(m_vertices[1]));
        const Vec2 v2 = m.TransformPoint(Vec2(m_vertices[2]));
        
        Triangle2 result;
        v0.StoreFloat2(&result.m_vertices[0]);
        v1.StoreFloat2(&result.m_vertices[1]);
        v2.StoreFloat2(&result.m_vertices[2]);
        return result;
    }

    Triangle::Triangle(const Vec3& v0, const Vec3& v1, const Vec3& v2)
    {
        v0.StoreFloat3(&m_vertices[0]);
        v1.StoreFloat3(&m_vertices[1]);
        v2.StoreFloat3(&m_vertices[2]);
    }

    Triangle::Triangle(const Vec3 vertices[3])
    {
        vertices[0].StoreFloat3(&m_vertices[0]);
        vertices[1].StoreFloat3(&m_vertices[1]);
        vertices[2].StoreFloat3(&m_vertices[2]);
    }

    Vec3 Triangle::Centroid() const
    {
        return (Vec3::LoadFloat3Unsafe(m_vertices[0])
            + Vec3::LoadFloat3Unsafe(m_vertices[1])
            + Vec3::LoadFloat3Unsafe(m_vertices[2]))
            * (1.f / 3.f);
    }
    
    float Triangle::SignedArea() const
    {
        return geo::CalculateSignedAreaOfTriangle(m_vertices[0], m_vertices[1], m_vertices[2]);
    }

    bool Triangle::Contains(const Vec3& point) const
    {
        return geo::TriangleContainsPoint(m_vertices[0], m_vertices[1], m_vertices[2], point);
    }
    
    Vec3 Triangle::PointFromBaryCoordinates(float bary0, float bary1, float bary2) const
    {
        return bary0 * m_vertices[0] + bary1 * m_vertices[1] + bary2 * m_vertices[2];
    }

    void Triangle::CalculateBarycentricCoordinate(const Vec3& p, float& bary0, float& bary1, float& bary2) const
    {
        ClosestPoint::GetBaryCentricCoordinates(
            Vec3::LoadFloat3Unsafe(m_vertices[0]),
            Vec3::LoadFloat3Unsafe(m_vertices[1]),
            Vec3::LoadFloat3Unsafe(m_vertices[2]),
            p,
            bary0, bary1, bary2);
    }
    
    Vec3 Triangle::ClosestPointTo(const Vec3& queryPoint) const
    {
        return ClosestPoint::GetClosestPointOnTriangle(
            Vec3::LoadFloat3Unsafe(m_vertices[0]),
            Vec3::LoadFloat3Unsafe(m_vertices[1]),
            Vec3::LoadFloat3Unsafe(m_vertices[2]),
            queryPoint);
    }
    
    float Triangle::DistanceSqr(const Vec3& queryPoint) const
    {
        const Vec3 closestPoint = ClosestPointTo(queryPoint);
        return (queryPoint - closestPoint).LengthSqr();
    }
    
    Vec3 Triangle::Normal() const
    {
        const Vec3 v0 = Vec3::LoadFloat3Unsafe(m_vertices[0]);
        const Vec3 edge0 = Vec3::LoadFloat3Unsafe(m_vertices[1]) - v0;
        const Vec3 edge1 = Vec3::LoadFloat3Unsafe(m_vertices[2]) - v0;
        return Vec3::Cross(edge0, edge1).Normalized();
    }

    Triangle Triangle::Transformed(const Mat44& m) const
    {
        const Vec3 v0 = m.TransformPoint(Vec3::LoadFloat3Unsafe(m_vertices[0]));
        const Vec3 v1 = m.TransformPoint(Vec3::LoadFloat3Unsafe(m_vertices[1]));
        const Vec3 v2 = m.TransformPoint(Vec3::LoadFloat3Unsafe(m_vertices[2]));
        
        Triangle result;
        v0.StoreFloat3(&result.m_vertices[0]);
        v1.StoreFloat3(&result.m_vertices[1]);
        v2.StoreFloat3(&result.m_vertices[2]);
        return result;
    }
}
