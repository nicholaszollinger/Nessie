// Geometry.h
#pragma once
#include "Math/Math.h"

namespace nes::math
{
    //----------------------------------------------------------------------------------------------------
    ///	@brief : The value returned represents twice the *signed* area of the triangle ABC.
    ///     (positive if counterclockwise; negative if clockwise).
    ///     - When the result is greater than 0, then point C lies to the left of the directed line A->B.
    ///       Equivalently, the triangle ABC is oriented counterclockwise.
    ///     - When the result is less than 0, then point C lies to the right of the directed line A->B
    ///       and the triangle ABC is oriented clockwise.
    ///     - When the result is equal to 0, then the three points are collinear.
    //----------------------------------------------------------------------------------------------------
    NES_INLINE float Orient2D(const Vec2& a, const Vec2& b, const Vec2& c)
    {
        // pg 32 of "Real-Time Collision Detection"
        // This is the same as the 2x2 determinant calculation:
        return (a[0] - c[0]) * (b[1] - c[1]) - (a[1] - c[1]) * (b[0] - c[0]);
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : The value returned corresponds to six times the *signed* volume of the tetrahedron formed
    ///     by the four points.
    ///     - When the result is less than 0, D lies above the supporting plane of triangle ABC, in
    ///       the sense that ABC appears counterclockwise when viewed from D.
    ///     - When the result is greater than 0, D lies below the plane of ABC.
    ///     - When the result is equal to 0, then all points are coplanar.
    //----------------------------------------------------------------------------------------------------
    NES_INLINE float Orient3D(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d)
    {
        // pg 33 of "Real-Time Collision Detection"
        const Mat44 mat
        (
            Vec4Reg(a.x - d.x, a.y - d.y, a.z - d.z, 0.f),
            Vec4Reg(b.x - d.x, b.y - d.y, b.z - d.z, 0.f),
            Vec4Reg(c.x - d.x, c.y - d.y, c.z - d.z, 0.f),
            Vec4Reg(0.f, 0.f, 0.f, 1.f)
        );
        return mat.Determinant3x3();
    }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Determines if the point D is on, in, or outside the circle that goes through the triangle
    ///     ABC.
    ///     - If the result is <0, then point D is inside the Circle.
    ///     - If the result is ==0, then all points are lie on the bounds of the circle.
    ///     - If the result is >0, then point D is outside the Circle.
    //----------------------------------------------------------------------------------------------------
    NES_INLINE float InCircle2D(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d)
    {
        // pg 34 of "Real-Time Collision Detection"
        const auto aDiff = a - d;
        const auto bDiff = b - d;
        const auto cDiff = c - d;
        
        const Mat44 mat
        (
                Vec4Reg(aDiff.x, aDiff.y, aDiff.LengthSqr(), 0.f),
                Vec4Reg(bDiff.x, bDiff.y, bDiff.LengthSqr(), 0.f),
                Vec4Reg(cDiff.x, cDiff.y, cDiff.LengthSqr(), 0.f),
                Vec4Reg(0.f, 0.f, 0.f, 1.f)
        );
        const float determinant = mat.Determinant3x3();

        // If the determinant is 0, then the four points are co-circular.
        if (math::CheckEqualFloats<float>(determinant, 0.f))
        {
            return 0.f;
        }

        const float isCounterClockwise = Orient2D(a, b, c);
        return determinant * isCounterClockwise;
    }

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Returns whether the 3 points a, b, and c are all collinear. 
    //----------------------------------------------------------------------------------------------------
    NES_INLINE bool PointsAreCollinear(const Vec2& a, const Vec2& b, const Vec2& c)
    {
        return math::CheckEqualFloats(Orient2D(a, b, c), 0.f);
    }

    // //----------------------------------------------------------------------------------------------------
    // ///	@brief : Returns whether the 3 points a, b, and c are all coplanar. 
    // //----------------------------------------------------------------------------------------------------
    // NES_INLINE bool PointsAreCoplanar(const Vec3& a, const Vec3& b, const Vec3& c)
    // {
    //     return math::CheckEqualFloats(Orient3D(a, b, c), 0.f);
    // }
}
