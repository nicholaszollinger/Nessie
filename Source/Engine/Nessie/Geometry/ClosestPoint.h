// ClosestPoint.h
#pragma once
#include "Math/Vec2.h"
#include "Math/Vec3.h"

namespace nes::ClosestPoint
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Compute the barycentric coordinates of the closest point to origin for the infinite line
    ///     defined by (a, b). The closest point can then be computed as (a * outU) + (b * outB).
    ///     Returns false if the points a, b do not form a line (are the same point).
    //----------------------------------------------------------------------------------------------------
    inline bool GetBaryCentricCoordinates(const Vec3 a, const Vec3 b, float& outU, float& outV)
    {
        const Vec3 aToB = b - a;
        float denominator = aToB.LengthSqr();

        if (denominator < nes::math::Squared(FLT_EPSILON))
        {
            // Degenerate line segment, fallback to points
            if (a.LengthSqr() < b.LengthSqr())
            {
                // A is closest.
                outU = 1.f;
                outV = 0.f;
            }
            else
            {
                // B is closest.
                outU = 0.f;
                outV = 1.f;
            }

            return false;
        }
        
        outV = -a.Dot(aToB) / denominator;
        outU = 1.f - outV;

        return true;
    }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Compute the barycentric coordinates of the closest point to the query point for a plane defined by
    ///     (a, b, c). The closest point can then be computed as (a * outU) + (b * outV) + (c * outW).
    ///     Returns false if the points a, b, c do not form a plane (are on the same line or at the same
    ///     point).
    //----------------------------------------------------------------------------------------------------
    inline bool GetBaryCentricCoordinates(const Vec3 a, const Vec3 b, const Vec3 c, const Vec3 queryPoint, float& outU, float& outV, float& outW)
    {
        // First, calculate the three edges
        const Vec3 v0 = b - a;
        const Vec3 v1 = c - a;
        const Vec3 v2 = queryPoint - a;

        // Make sure that the shortest edge is included in the calculation to keep the products
        // (a * b) - (c * d) as small as possible to preserve accuracy.
        const float d00 = v0.LengthSqr();
        const float d11 = v1.LengthSqr();
        const float d22 = v2.LengthSqr();
        if (d00 <= d22)
        {
            // Use v0 and v1 to calculate the barycentric coordinates.
            const float d01 = v0.Dot(v1);

            // Denominator must be positive:
            // |v0|^2 * |v1|^2 - (v0 . v1)^2 = |v0|^2 * |v1|^2 * (1 - cos(angle)^2) >= 0
            float denominator = (d00 * d11) - (d01 * d01);
            if (denominator < 1.0e-12f)
            {
                // Degenerate triangle, return coordinates along the longest edge:
                if (d00 > d11)
                {
                    GetBaryCentricCoordinates(a, b, outU, outV);
                    outW = 0.f;
                }
                else
                {
                    GetBaryCentricCoordinates(a, c, outU, outW);
                    outV = 0.f;
                }

                return false;
            }
            else
            {
                const float a0 = a.Dot(v0);
                const float a1 = a.Dot(v1);
                outV = (d01 * a1 - d11 * a0) / denominator;
                outW = (d01 * a0 - d00 * a1) / denominator;
                outU = 1.0f - outV - outW;
            }
        }
        else
        {
            // Use v1 and v2, to calculate the barycentric coordinates.
            const float d12 = v1.Dot(v2);

            // The denominator must be positive:
            const float denominator = (d11 * d22) - (d12 * d12);
            if (denominator < 1.0e-12f)
            {
                // Degenerate triangle, return coordinates along the longest edge:
                if (d11 > d22)
                {
                    GetBaryCentricCoordinates(a, c, outU, outW);
                    outV = 0.f;
                }
                else
                {
                    GetBaryCentricCoordinates(b, c, outV, outW);
                    outU = 0.f;
                }

                return false;
            }
            else
            {
                const float c1 = c.Dot(v1);
                const float c2 = c.Dot(v2);
                outV = (d22 * c1 - d12 * c2) / denominator;
                outW = (d11 * c2 - d12 * c1) / denominator;
                outU = 1.0f - outV - outW;
            }
        }

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Compute the barycentric coordinates of the closest point to the query point for a plane defined by
    ///     (a, b, c). The closest point can then be computed as (a * outU) + (b * outV) + (c * outW).
    ///     Returns false if the points a, b, c do not form a plane (are on the same line or at the same
    ///     point).
    //----------------------------------------------------------------------------------------------------
    inline bool GetBaryCentricCoordinates(const Vec2 a, const Vec2 b, const Vec2 c, const Vec2 queryPoint, float& outU, float& outV, float& outW)
    {
        return GetBaryCentricCoordinates
        (
            Vec3(a.x, a.y, 0.f),
            Vec3(b.x, b.y, 0.f),
            Vec3(c.x, c.y, 0.f),
            Vec3(queryPoint.x, queryPoint.y, 0.f),
            outU, outV, outW
        );
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Compute the barycentric coordinates of the closest point to the origin for a plane defined by
    ///     (a, b, c). The closest point can then be computed as (a * outU) + (b * outV) + (c * outW).
    ///     Returns false if the points a, b, c do not form a plane (are on the same line or at the same
    ///     point).
    //----------------------------------------------------------------------------------------------------
    inline bool GetBaryCentricCoordinates(const Vec3 a, const Vec3 b, const Vec3 c, float& outU, float& outV, float& outW)
    {
        // Taken from: Real-Time Collision Detection - Christer Ericson (Section: Barycentric Coordinates)
        // With p = 0
        // Adjusted to always include the shortest edge of the triangle in the calculation to improve numerical accuracy

        // First, calculate the three edges
        const Vec3 v0 = b - a;
        const Vec3 v1 = c - a;
        const Vec3 v2 = c - b;

        // Make sure that the shortest edge is included in the calculation to keep the products
        // (a * b) - (c * d) as small as possible to preserve accuracy.
        const float d00 = v0.LengthSqr();
        const float d11 = v1.LengthSqr();
        const float d22 = v2.LengthSqr();
        if (d00 <= d22)
        {
            // Use v0 and v1 to calculate the barycentric coordinates.
            const float d01 = v0.Dot(v1);

            // Denominator must be positive:
            // |v0|^2 * |v1|^2 - (v0 . v1)^2 = |v0|^2 * |v1|^2 * (1 - cos(angle)^2) >= 0
            float denominator = (d00 * d11) - (d01 * d01);
            if (denominator < 1.0e-12f)
            {
                // Degenerate triangle, return coordinates along the longest edge:
                if (d00 > d11)
                {
                    GetBaryCentricCoordinates(a, b, outU, outV);
                    outW = 0.f;
                }
                else
                {
                    GetBaryCentricCoordinates(a, c, outU, outW);
                    outV = 0.f;
                }

                return false;
            }
            else
            {
                const float a0 = a.Dot(v0);
                const float a1 = a.Dot(v1);
                outV = (d01 * a1 - d11 * a0) / denominator;
                outW = (d01 * a0 - d00 * a1) / denominator;
                outU = 1.0f - outV - outW;
            }
        }
        else
        {
            // Use v1 and v2, to calculate the barycentric coordinates.
            const float d12 = v1.Dot(v2);

            // The denominator must be positive:
            const float denominator = (d11 * d22) - (d12 * d12);
            if (denominator < 1.0e-12f)
            {
                // Degenerate triangle, return coordinates along the longest edge:
                if (d11 > d22)
                {
                    GetBaryCentricCoordinates(a, c, outU, outW);
                    outV = 0.f;
                }
                else
                {
                    GetBaryCentricCoordinates(b, c, outV, outW);
                    outU = 0.f;
                }

                return false;
            }
            else
            {
                const float c1 = c.Dot(v1);
                const float c2 = c.Dot(v2);
                outV = (d22 * c1 - d12 * c2) / denominator;
                outW = (d11 * c2 - d12 * c1) / denominator;
                outU = 1.0f - outV - outW;
            }
        }

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the closest point to the origin of line (a, b).
    ///     outSet describes which features are the closest: 1 = a, 2 = b, 3 = line segment ab.
    //----------------------------------------------------------------------------------------------------
    inline Vec3 GetClosestPointOnLine(const Vec3& a, const Vec3& b, uint32_t& outSet)
    {
        float u, v;
        GetBaryCentricCoordinates(a, b, u, v);
        if (v <= 0.f)
        {
            // a is the closest point.
            outSet = 0b0001;
            return a;
        }
        else if (u <= 0.f)
        {
            // b is the closest point.
            outSet = 0b0010;
            return b;
        }
        else
        {
            // Closest point lies on the line (a, b).
            outSet = 0b0011;
            return (u * a) + (v * b);
        }
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the closest point to the origin of triangle (a, b, c).
    ///     'outSet' describes which features are closest: 1 = a, 2 = b, 4 = c, 5 = line segment ac, 7 = triangle interior etc.
    ///     If MustIncludeC is true, the function assumes that C is part of the closest feature (vertex, edge, face)
    ///     and does less work, if the assumption is not true then the closest point to the other features is returned.
    //----------------------------------------------------------------------------------------------------
    template <bool MustIncludeC = false>
    inline Vec3 GetClosestPointOnTriangle(const Vec3& inA, const Vec3& inB, const Vec3& inC, uint32_t& outSet)
    {
        // Taken from: "Real-Time Collision Detection" - Christer Ericson (Section: Closest Point on Triangle to Point)
        // With p = 0

        // The most accurate normal is calculated by using the two shortest edges
        // See: https://box2d.org/posts/2014/01/troublesome-triangle/
        // The difference in normals is most pronounced when one edge is much smaller than the others (in which case the other 2 must have roughly the same length).
        // Therefore, we can suffice by just picking the shortest from 2 edges and use that with the 3rd edge to calculate the normal.
        // We first check which of the edges is shorter, and if bc is shorter than ac then we swap a with c to a is always on the shortest edge
        UVec4Reg swapAC;
        {
            const Vec3 ac = inC - inA;
            const Vec3 bc = inC - inB;
            swapAC = Vec4Reg::Less(bc.DotV4(bc), ac.DotV4(ac));
        }
        Vec3 a = Vec3::Select( inA, inC, swapAC);
        Vec3 c = Vec3::Select( inC, inA, swapAC);

        // Calculate the normal
        Vec3 ab = inB - a;
        Vec3 ac = c - a;
        Vec3 n = ab.Cross(ac);
        float normalLenSqr = n.LengthSqr();

        // Check degenerate:
        if (normalLenSqr < 1.0e-10f) // Square(FLT_EPSILON) was too small and caused numerical problems
        {
            // Degenerate, fallback to vertices and edges

            // Start with vertex C being the closest.
            uint32_t closestSet = 0b0100;
            Vec3 closestPoint = inC;
            float bestDistSqr = closestPoint.LengthSqr();

            // If the closest point must include C then A or B cannot be the closest.
            if constexpr (!MustIncludeC)
            {
                // Try vertex A
                const float aLenSqr = inA.LengthSqr();
                if (aLenSqr < bestDistSqr)
                {
                    closestSet = 0b0001;
                    closestPoint = inA;
                    bestDistSqr = aLenSqr;
                }

                // Try vertex B
                const float bLenSqr = inB.LengthSqr();
                if (bLenSqr < bestDistSqr)
                {
                    closestSet = 0b0010;
                    closestPoint = inB;
                    bestDistSqr = bLenSqr;
                }
            }

            // Edge AC
            const float acLenSqr = ac.LengthSqr();
            if (acLenSqr < math::Squared(FLT_EPSILON))
            {
                float v = math::Clamp(-a.Dot(ac) / acLenSqr, 0.f, 1.f);
                Vec3 q = a + v * ac;
                const float distSqr = q.LengthSqr();
                if (distSqr < bestDistSqr)
                {
                    closestSet = 0b0101;
                    closestPoint = q;
                    bestDistSqr = distSqr;
                }
            }

            // Edge BC
            Vec3 bc = inC - inB;
            const float bcLenSqr = bc.LengthSqr();
            if (bcLenSqr < math::Squared(FLT_EPSILON))
            {
                float v = math::Clamp(-inB.Dot(bc) / bcLenSqr, 0.f, 1.f);
                Vec3 q = inB + v * bc;
                const float distSqr = q.LengthSqr();
                if (distSqr < bestDistSqr)
                {
                    closestSet = 0b0110;
                    closestPoint = q;
                    bestDistSqr = distSqr;
                }
            }

            // If the closest point must include C then AB cannot be closest
            if constexpr (!MustIncludeC)
            {
                // Edge AB
                ab = inB - inA;
                const float abLenSqr = ab.LengthSqr();
                if (abLenSqr < math::Squared(FLT_EPSILON))
                {
                    float v = math::Clamp(-inA.Dot(ab) / abLenSqr, 0.f, 1.f);
                    Vec3 q = inA + v * bc;
                    const float distSqr = q.LengthSqr();
                    if (distSqr < bestDistSqr)
                    {
                        closestSet = 0b0011;
                        closestPoint = q;
                        bestDistSqr = distSqr;
                    }
                }
            }

            outSet = closestSet;
            return Vec3(closestPoint.x, closestPoint.y, closestPoint.z);
        }

        // Check if P in vertex region is outside A
        Vec3 ap = -a;
        const float d1 = ab.Dot(ap);
        const float d2 = ac.Dot(ap);
        if (d1 <= 0.f && d2 <= 0.f)
        {
            outSet = swapAC.GetX() ? 0b0100 : 0b0001;
            return Vec3(a.x, a.y, a.z); // Barycentric coordinates (1, 0, 0).
        }

        // Check if P in vertex region is outside B
        Vec3 bp = -inB;
        const float d3 = ab.Dot(bp);
        const float d4 = ac.Dot(bp);
        if (d3 <= 0.f && d4 <= d3)
        {
            outSet = 0b0010;
            return inB; // Barycentric coordinates (0, 1, 0).
        }

        // Check if P in edge region of AB, if so return projection of P onto AB
        if (d1 * d4 <= d3 * d2 && d1 >= 0.f && d3 <= 0.f)
        {
            float v = d1 / (d1 - d3);
            outSet = swapAC.GetX() ? 0b0110 : 0b0011;
            const auto result = a + v * ab; // barycentric coordinates (1-v,v,0)
            return Vec3(result.x, result.y, result.z); 
        }

        // Check if P in vertex region outside C
        Vec3 cp = -c;
        const float d5 = ab.Dot(cp);
        const float d6 = ac.Dot(cp);
        if (d6 >= 0.f && d5 <= d6)
        {
            outSet = swapAC.GetX() ? 0b0001 : 0b0100;
            return Vec3(c.x, c.y, c.z); // barycentric coordinates (0,0,1)
        }

        // Check if P in edge region of AC, if so return projection of P onto AC
        if (d5 * d2 <= d1 * d6 && d2 >= 0.f && d3 <= 0.f)
        {
            float w = d2 / (d2 - d6);
            outSet = 0b0101;
            auto result = a + w * ac; // barycentric coordinates (0,1-w,w)
            return Vec3(result.x, result.y, result.z);
        }

        // Check if P in edge region of BC, if so return projection of P onto BC
        float d4_d3 = d4 - d3;
        float d5_d6 = d5 - d6;
        if (d3 * d6 <= d5 * d4 && d4_d3 >= 0.f && d5_d6 >= 0.f)
        {
            float w = d4_d3 / (d4_d3 + d5_d6);
            outSet = swapAC.GetX() ? 0b0011 : 0b0110;
            auto result = inB + w * (c - inB); // barycentric coordinates (0,1-w,w)
            return Vec3(result.x, result.y, result.z);
        }

        // P is inside the face region
        // Here we deviate from Christer Ericson's article to improve accuracy.
        // Determine distance between triangle and origin: distance = (centroid - origin) . normal / |normal|
        // Closest point to origin is then: distance . normal / |normal|
        // Note that this way of calculating the closest point is much more accurate than first calculating barycentric coordinates
        // and then calculating the closest point based on those coordinates.
        outSet = 0b0111;
        auto result = n * (a + inB + c).Dot(n) / (3.f * normalLenSqr);
        return Vec3(result.x, result.y, result.z);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the closest point on triangle (a, b, c) to the query point.
    ///     'outSet' describes which features are closest: 1 = a, 2 = b, 4 = c, 5 = line segment ac, 7 = triangle interior etc.
    ///     If MustIncludeC is true, the function assumes that C is part of the closest feature (vertex, edge, face)
    ///     and does less work, if the assumption is not true then the closest point to the other features is returned.
    //----------------------------------------------------------------------------------------------------
    inline Vec3 GetClosestPointOnTriangle(const Vec3& inA, const Vec3& inB, const Vec3& inC, const Vec3& queryPoint)
    {
        // "Real-Time Collision Detection" (142-143).
        // The basic idea is to find the voronoi region (vertex, edge, or face) that the query point
        // is in, and then use the barycentric coordinates to find the position on or in the triangle.
        // - This uses the Langrange Identity to remove the need for the cross-products.
        
        // ABC = m_vertices[0..2]
        // P = queryPoint
        // AB = "A to B"
        // U,V,W = Barycentric coordinates
        const Vec3 ab = inB - inA;
        const Vec3 ac = inC - inA;
        const Vec3 ap = queryPoint - inA;
        
        // Check if the point lies in the vertex region outside vertex A.
        const float d1 = Vec3::Dot(ab, ap);
        const float d2 = Vec3::Dot(ac, ap);
        if (d1 <= 0.f && d2 <= 0.f)
        {
            // Barycentric Coordinates (1, 0, 0)
            return inA;
        }
        
        // Check if the point lies in the vertex region outside vertex B.
        const Vec3 bp = queryPoint - inB;
        const float d3 = Vec3::Dot(ab, bp);
        const float d4 = Vec3::Dot(ac, bp);
        if (d3 >= 0.f && d4 <= d3)
        {
            // Barycentric Coordinates (0, 1, 0)
            return inB;
        }
        
        // Check if the point is in the edge region of AB, if so return the projection
        // of the query point onto the edge AB
        const float vc = d1 * d4 - d3 * d2;
        if (vc <= 0.f && d1 >= 0.f && d3 <= 0.f)
        {
            // Barycentric Coordinates (1 - v, v, 1)
            const float v = d1 / (d1 - d3);
            return inA + v * ab;
        }
        
        // Check if the point lies in the vertex region outside vertex c
        const Vec3 cp = queryPoint - inC;
        const float d5 = Vec3::Dot(ab, cp);
        const float d6 = Vec3::Dot(ac, cp);
        if (d6 >= 0.f && d5 <= d6)
        {
            // Barycentric Coordinates (0, 0, 1)
            return inC;
        }
        
        // Check if the point is in the edge region of AC, if so return the projection
        // of the query point onto the edge AC.
        const float vb = d5 * d2 - d1 * d6;
        if (vb <= 0.f && d2 >= 0.f && d6 <= 0.f)
        {
            // Barycentric Coordinates (1-w, 0, w). 
            const float w = d2 / (d2 - d6);
            return inA + w * ac;
        }
        
        // Check if the point is in the edge region of BC, if so return the projection
        // of the query point onto BC
        const float va = d3 * d6 - d5 * d4;
        if (va <= 0.f && (d4 - d3) >= 0.f && (d5 - d6) >= 0.f)
        {
            // Barycentric Coordinates (0, 1-w, w)
            const float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
            return inB + w * (inC - inB);
        }
        
        // The Query point is inside the Face region. Compute the resulting point by
        // its barycentric coordinates (u, v, w).
        const float denom = 1.f / (va + vb + vc);
        const float v = vb * denom;
        const float w = vc * denom;
        // u * a + v * b + w * c:
        return inA + ab * v + ac * w;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the closest point on triangle (a, b, c) to the query point.
    ///     'outSet' describes which features are closest: 1 = a, 2 = b, 4 = c, 5 = line segment ac, 7 = triangle interior etc.
    ///     If MustIncludeC is true, the function assumes that C is part of the closest feature (vertex, edge, face)
    ///     and does less work, if the assumption is not true then the closest point to the other features is returned.
    //----------------------------------------------------------------------------------------------------
    inline Vec2 GetClosestPointOnTriangle(const Vec2& inA, const Vec2& inB, const Vec2& inC, const Vec2& queryPoint)
    {
        // "Real-Time Collision Detection" (142-143).
        // The basic idea is to find the voronoi region (vertex, edge, or face) that the query point
        // is in, and then use the barycentric coordinates to find the position on or in the triangle.
        // - This uses the Langrange Identity to remove the need for the cross-products.
        
        // ABC = m_vertices[0..2]
        // P = queryPoint
        // AB = "A to B"
        // U,V,W = Barycentric coordinates
        const Vec2 ab = inB - inA;
        const Vec2 ac = inC - inA;
        const Vec2 ap = queryPoint - inA;
        
        // Check if the point lies in the vertex region outside vertex A.
        const float d1 = Vec2::Dot(ab, ap);
        const float d2 = Vec2::Dot(ac, ap);
        if (d1 <= 0.f && d2 <= 0.f)
        {
            // Barycentric Coordinates (1, 0, 0)
            return inA;
        }
        
        // Check if the point lies in the vertex region outside vertex B.
        const Vec2 bp = queryPoint - inB;
        const float d3 = Vec2::Dot(ab, bp);
        const float d4 = Vec2::Dot(ac, bp);
        if (d3 >= 0.f && d4 <= d3)
        {
            // Barycentric Coordinates (0, 1, 0)
            return inB;
        }
        
        // Check if the point is in the edge region of AB, if so return the projection
        // of the query point onto the edge AB
        const float vc = d1 * d4 - d3 * d2;
        if (vc <= 0.f && d1 >= 0.f && d3 <= 0.f)
        {
            // Barycentric Coordinates (1 - v, v, 1)
            const float v = d1 / (d1 - d3);
            return inA + v * ab;
        }
        
        // Check if the point lies in the vertex region outside vertex c
        const Vec2 cp = queryPoint - inC;
        const float d5 = Vec2::Dot(ab, cp);
        const float d6 = Vec2::Dot(ac, cp);
        if (d6 >= 0.f && d5 <= d6)
        {
            // Barycentric Coordinates (0, 0, 1)
            return inC;
        }
        
        // Check if the point is in the edge region of AC, if so return the projection
        // of the query point onto the edge AC.
        const float vb = d5 * d2 - d1 * d6;
        if (vb <= 0.f && d2 >= 0.f && d6 <= 0.f)
        {
            // Barycentric Coordinates (1-w, 0, w). 
            const float w = d2 / (d2 - d6);
            return inA + w * ac;
        }
        
        // Check if the point is in the edge region of BC, if so return the projection
        // of the query point onto BC
        const float va = d3 * d6 - d5 * d4;
        if (va <= 0.f && (d4 - d3) >= 0.f && (d5 - d6) >= 0.f)
        {
            // Barycentric Coordinates (0, 1-w, w)
            const float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
            return inB + w * (inC - inB);
        }
        
        // The Query point is inside the Face region. Compute the resulting point by
        // its barycentric coordinates (u, v, w).
        const float denom = 1.f / (va + vb + vc);
        const float v = vb * denom;
        const float w = vc * denom;
        // u * a + v * b + w * c:
        return inA + ab * v + ac * w;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Returns for each of the planes of the tetrahedron if the origin is inside of it. 
    //----------------------------------------------------------------------------------------------------
    inline UVec4Reg OriginOutsideOfTetrahedronPlanes(const Vec3 inA, const Vec3& inB, const Vec3& inC, const Vec3& inD)
    {
        Vec3 ab = inB - inA;
        Vec3 ac = inC - inA;
        Vec3 ad = inD - inA;
        Vec3 bd = inD - inB;
        Vec3 bc = inC - inB;

        Vec3 abCrossAC = ab.Cross(ac);
        Vec3 acCrossAD = ac.Cross(ad);
        Vec3 adCrossAB = ad.Cross(ab);
        Vec3 bdCrossBC = bd.Cross(bc);

        // For each plane get the side on which the origin is
        float signP0 = inA.Dot(abCrossAC); // ABC
        float signP1 = inA.Dot(acCrossAD); // ACD
        float signP2 = inA.Dot(adCrossAB); // ADB
        float signP3 = inB.Dot(bdCrossBC); // BDC
        Vec4Reg signP(signP0, signP1, signP2, signP3);

        // For each plane get the side that is outside (determined by the 4th point).
        float signD0 = ad.Dot(abCrossAC);   // D
        float signD1 = ab.Dot(acCrossAD);   // B
        float signD2 = ac.Dot(adCrossAB);   // C
        float signD3 = -ab.Dot(bdCrossBC);  // A
        Vec4Reg signD(signD0, signD1, signD2, signD3);

        // The winding of all triangles has been chosen so that signD should have the
        // same sign for all components. If this is not the case the tetrahedron is
        // degenerate, and we return that the origin is in front of all sides.
        const int signBits = signD.GetSignBits();
        switch (signBits)
        {
            case 0:
            {
                // All positive.
                return Vec4Reg::GreaterOrEqual(signP, Vec4Reg::Replicate(-FLT_EPSILON));
            }

            case 0xf:
            {
                // All negative.
                return Vec4Reg::LessOrEqual(signP, Vec4Reg::Replicate(FLT_EPSILON));
            }

            default:
            {
                // Mixed signs, degenerate tetrahedron
                return UVec4Reg::Replicate(0xffffffff);
            }
        }
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the closest point between tetrahedron (inA, inB, inC, inD) to the origin
    ///     outSet specifies which feature was closest, 1 = a, 2 = b, 4 = c, 8 = d. Edges have 2 bits set, triangles 3 and if the point is in the interior 4 bits are set.
    ///	@tparam MustIncludeD : If true, the function assumes that D is part of the closest feature (vertex, edge, face, tetrahedron)
    ///     and does less work, if the assumption is not true then a closest point to the other features is returned.
    //----------------------------------------------------------------------------------------------------
    template <bool MustIncludeD = false>
    inline Vec3 GetClosestPointOnTetrahedron(const Vec3 inA, const Vec3 inB, const Vec3 inC, const Vec3 inD, uint32_t& outSet)
    {
        // Taken from: Real-Time Collision Detection - Christer Ericson (Section: Closest Point on Tetrahedron to Point)
        // With p = 0

        // Start out assuming point inside all half-spaces, so closest to itself
        uint32_t closestSet = 0b1111;
        Vec3 closestPoint = Vec3::Zero();
        float bestDistSqr = FLT_MAX;

        // Determine for each of the faces of the tetrahedron if the origin is in front of the plane
        UVec4Reg originOutOfPlanes = OriginOutsideOfTetrahedronPlanes(inA, inB, inC, inD);
        
        // If point is outside face abc then compute the closest point on ABC
        if (originOutOfPlanes.GetX())
        {
            if constexpr (MustIncludeD)
            {
                // If the closest point must include D then ABC cannot be closest but the closest point
                // cannot be an interior point either so we return A as closest point
                closestSet = 0b0001;
                closestPoint = inA;
            }
            else
            {
                // Test the face normally
                closestPoint = GetClosestPointOnTriangle<false>(inA, inB, inC, closestSet);
            }

            bestDistSqr = closestPoint.LengthSqr();
        }

        // Repeat test for face ACD
        if (originOutOfPlanes.GetY())
        {
            uint32_t set;
            const Vec3 q = GetClosestPointOnTriangle<MustIncludeD>(inA, inC, inD, set);
            const float distSqr = q.LengthSqr();
            if (distSqr < bestDistSqr)
            {
                bestDistSqr = distSqr;
                closestPoint = q;
                closestSet = (set & 0b0001) + ((set & 0b0110) << 1);
            }
        }

        // Repeat test for face ABD
        if (originOutOfPlanes.GetZ())
        {
            // Keep original vertex order, it doesn't matter if the triangle is facing inward or outward
            // and it improves consistency for GJK which will always add a new vertex D and keep the closest
            // feature from the previous iteration in ABC
            uint32_t set;
            const Vec3 q = GetClosestPointOnTriangle<MustIncludeD>(inA, inB, inD, set);
            const float distSqr = q.LengthSqr();
            if (distSqr < bestDistSqr)
            {
                bestDistSqr = distSqr;
                closestPoint = q;
                closestSet = (set & 0b0011) + ((set & 0b0100) << 1);
            }
        }

        // Repeat test for face BDC
        if (originOutOfPlanes.GetW())
        {
            // Keep original vertex order, it doesn't matter if the triangle is facing inward or outward
            // and it improves consistency for GJK which will always add a new vertex D and keep the closest
            // feature from the previous iteration in ABC
            uint32_t set;
            const Vec3 q = GetClosestPointOnTriangle<MustIncludeD>(inB, inC, inD, set);
            const float distSqr = q.LengthSqr();
            if (distSqr < bestDistSqr)
            {
                bestDistSqr = distSqr;
                closestPoint = q;
                closestSet = set << 1;
            }
        }

        outSet = closestSet;
        return closestPoint;
    }
}
