// Plane.cpp
#include "Plane.h"
#include "Nessie/Math/Mat44.h"

namespace nes
{
    Plane Plane::Offset(const float distance) const
    {
        return Plane(m_normalAndConstant - Vec4(Vec3::Zero(), distance));
    }

    Plane Plane::Transformed(const Mat44& transform) const
    {
        const Vec3 transformedNormal = transform.Multiply3x3(GetNormal());
        return Plane(transformedNormal, GetConstant() - transform.GetTranslation().Dot(transformedNormal));
    }

    Plane Plane::Scaled(const Vec3 scale) const
    {
        const Vec3 scaledNormal = GetNormal() / scale;
        const float scaledNormalLength = scaledNormal.Length();
        return Plane(scaledNormal / scaledNormalLength, GetConstant() / scaledNormalLength);
    }

    bool Plane::IntersectPlanes(const Plane& plane1, const Plane& plane2, const Plane& plane3, Vec3& outPoint)
    {
        // We solve the equation:
        // |ax, ay, az, aw|   | x |   | 0 |
        // |bx, by, bz, bw| * | y | = | 0 |
        // |cx, cy, cz, cw|   | z |   | 0 |
        // | 0,	 0,	 0,	 1|   | 1 |   | 1 |
        // Where normal of plane 1 = (ax, ay, az), plane constant of 1 = aw, normal of plane 2 = (bx, by, bz) etc.
        // This involves inverting the matrix and multiplying it with [0, 0, 0, 1]

        // Fetch the normals and plane constants for the three planes
        const Vec4 a = plane1.m_normalAndConstant;
        const Vec4 b = plane2.m_normalAndConstant;
        const Vec4 c = plane3.m_normalAndConstant;

        // The result is a vector that we have to divide by:
        const float denominator = Vec3(a).Dot(Vec3(b).Cross(Vec3(c)));
        if (denominator == 0.0f)
            return false;

        // The numerator is:
        // [aw*(bz*cy-by*cz)+ay*(bw*cz-bz*cw)+az*(by*cw-bw*cy)]
        // [aw*(bx*cz-bz*cx)+ax*(bz*cw-bw*cz)+az*(bw*cx-bx*cw)]
        // [aw*(by*cx-bx*cy)+ax*(bw*cy-by*cw)+ay*(bx*cw-bw*cx)]
        const Vec4Reg numerator =
            a.SplatW() * (b.Swizzle<ESwizzleZ, ESwizzleX, ESwizzleY, ESwizzleUnused>() * c.Swizzle<ESwizzleY, ESwizzleZ, ESwizzleX, ESwizzleUnused>() - b.Swizzle<ESwizzleY, ESwizzleZ, ESwizzleX, ESwizzleUnused>() * c.Swizzle<ESwizzleZ, ESwizzleX, ESwizzleY, ESwizzleUnused>())
            + a.Swizzle<ESwizzleY, ESwizzleX, ESwizzleX, ESwizzleUnused>() * (b.Swizzle<ESwizzleW, ESwizzleZ, ESwizzleW, ESwizzleUnused>() * c.Swizzle<ESwizzleZ, ESwizzleW, ESwizzleY, ESwizzleUnused>() - b.Swizzle<ESwizzleZ, ESwizzleW, ESwizzleY, ESwizzleUnused>() * c.Swizzle<ESwizzleW, ESwizzleZ, ESwizzleW, ESwizzleUnused>())
            + a.Swizzle<ESwizzleZ, ESwizzleZ, ESwizzleY, ESwizzleUnused>() * (b.Swizzle<ESwizzleY, ESwizzleW, ESwizzleX, ESwizzleUnused>() * c.Swizzle<ESwizzleW, ESwizzleX, ESwizzleW, ESwizzleUnused>() - b.Swizzle<ESwizzleW, ESwizzleX, ESwizzleW, ESwizzleUnused>() * c.Swizzle<ESwizzleY, ESwizzleW, ESwizzleX, ESwizzleUnused>());

        outPoint = (numerator / denominator).ToVec3();
        return true;
    }
}
