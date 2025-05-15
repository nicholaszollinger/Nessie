// RayAABox.h
#pragma once
#include "Math/AABox.h"
#include "Math/Vector3.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper struct that holds the reciprocal of a ray for Ray vs AABox testing.  
    //----------------------------------------------------------------------------------------------------
    struct RayInvDirection
    {
        inline RayInvDirection() = default;
        inline explicit RayInvDirection(const Vector3& direction) { Set(direction); }

        inline void Set(const Vector3& direction)
        {
            // if (abs(inDirection) <= Epsilon) the ray is nearly parallel to the slab.
            m_isParallel = VectorRegisterF::LesserOrEqual(direction.Abs(), VectorRegisterF::Replicate(1.0e-20f));

            // Calculate 1 / direction while avoiding divisions by zero.
            const VectorRegisterF result = VectorRegisterF::Select(direction, VectorRegisterF::Unit(), m_isParallel);
            m_invDirection = Vector3(result.GetX(), result.GetY(), result.GetZ()).GetReciprocal();
        }

        VectorRegisterF     m_invDirection;     /// 1 / ray direction
        VectorRegisterUint  m_isParallel;       /// For each component if it is parallel to the coordinate axis.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Intersect AABB with ray, returns minimal distance along ray or FLT_MAX if no hit.
    /// @note : Can return negative value if the ray starts in the box.
    //----------------------------------------------------------------------------------------------------
    NES_INLINE float RayAABox(const Vector3& origin, const RayInvDirection& invDirection, const Vector3& boundsMin, const Vector3& boundsMax)
    {
        VectorRegisterF fltMin = VectorRegisterF::Replicate(-FLT_MAX);
        VectorRegisterF fltMax = VectorRegisterF::Replicate(FLT_MAX);

        VectorRegisterF rOrigin = origin;
        VectorRegisterF rBoundsMin = boundsMin;
        VectorRegisterF rBoundsMax = boundsMax;

        // Test against all three axes simultaneously
        VectorRegisterF t1 = (rBoundsMin - rOrigin) * invDirection.m_invDirection;
        VectorRegisterF t2 = (rBoundsMax - rOrigin) * invDirection.m_invDirection;

        // Compute the max of min(t1, t2) and the min of max(t1, t2) ensuring that
        // we don't use the results from any directions parallel to the slab
        VectorRegisterF tMin = VectorRegisterF::Select(VectorRegisterF::Min(t1, t2), fltMin, invDirection.m_isParallel);
        VectorRegisterF tMax = VectorRegisterF::Select(VectorRegisterF::Max(t1, t2), fltMax, invDirection.m_isParallel);

        // tMin.XYZ = maximum(tMin.x, tMin.y, tMin.z)
        tMin = VectorRegisterF::Max(tMin, tMin.Swizzle<Swizzle::Y, Swizzle::Z, Swizzle::X, Swizzle::X>());
        tMin = VectorRegisterF::Max(tMin, tMin.Swizzle<Swizzle::Z, Swizzle::X, Swizzle::Y, Swizzle::Y>());

        // tMax.xyz = minimum(tMax.x, tMax.y, tMax.z);
        tMax = VectorRegisterF::Min(tMax, tMax.Swizzle<Swizzle::Y, Swizzle::Z, Swizzle::X, Swizzle::X>());
        tMax = VectorRegisterF::Min(tMax, tMax.Swizzle<Swizzle::Z, Swizzle::X, Swizzle::Y, Swizzle::Y>());

        // If (tMin > tMax) return FLT_MAX
        VectorRegisterUint noIntersection = VectorRegisterF::Greater(tMin, tMax);

        // If (tMax < 0.f) return FLT_MAX
        noIntersection = VectorRegisterUint::Or(noIntersection, VectorRegisterF::Less(tMax, VectorRegisterF::Zero()));

        // If (invDirection.m_isParallel && !(Min <= origin && origin <= Max)) return FLT_MAX; else return tMin
        VectorRegisterUint noParallelOverlap = VectorRegisterUint::Or(VectorRegisterF::Less(rOrigin, rBoundsMin), VectorRegisterF::Greater(rOrigin, rBoundsMax));
        noIntersection = VectorRegisterUint::Or(noIntersection, VectorRegisterUint::And(invDirection.m_isParallel, noParallelOverlap));
        noIntersection = VectorRegisterUint::Or(noIntersection, noIntersection.SplatY());
        noIntersection = VectorRegisterUint::Or(noIntersection, noIntersection.SplatZ());
        return VectorRegisterF::Select(tMin, fltMax, noIntersection).GetX();
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Intersect AABB with ray, returns minimal and maximal distance along ray or FLT_MAX, -FLT_MAX if no hit
    /// @note : Can return negative value for outMin if the ray starts in the box.
    //----------------------------------------------------------------------------------------------------
    NES_INLINE void RayAABox(const Vector3& origin, const RayInvDirection& invDirection, const Vector3& boundsMin, const Vector3& boundsMax, float& outMin, float& outMax)
    {
        VectorRegisterF fltMin = VectorRegisterF::Replicate(-FLT_MAX);
        VectorRegisterF fltMax = VectorRegisterF::Replicate(FLT_MAX);

        VectorRegisterF rOrigin = origin;
        VectorRegisterF rBoundsMin = boundsMin;
        VectorRegisterF rBoundsMax = boundsMax;

        // Test against all three axes simultaneously
        VectorRegisterF t1 = (rBoundsMin - rOrigin) * invDirection.m_invDirection;
        VectorRegisterF t2 = (rBoundsMax - rOrigin) * invDirection.m_invDirection;

        // Compute the max of min(t1, t2) and the min of max(t1, t2) ensuring that
        // we don't use the results from any directions parallel to the slab
        VectorRegisterF tMin = VectorRegisterF::Select(VectorRegisterF::Min(t1, t2), fltMin, invDirection.m_isParallel);
        VectorRegisterF tMax = VectorRegisterF::Select(VectorRegisterF::Max(t1, t2), fltMax, invDirection.m_isParallel);

        // tMin.XYZ = maximum(tMin.x, tMin.y, tMin.z)
        tMin = VectorRegisterF::Max(tMin, tMin.Swizzle<Swizzle::Y, Swizzle::Z, Swizzle::X, Swizzle::X>());
        tMin = VectorRegisterF::Max(tMin, tMin.Swizzle<Swizzle::Z, Swizzle::X, Swizzle::Y, Swizzle::Y>());

        // tMax.xyz = minimum(tMax.x, tMax.y, tMax.z);
        tMax = VectorRegisterF::Min(tMax, tMax.Swizzle<Swizzle::Y, Swizzle::Z, Swizzle::X, Swizzle::X>());
        tMax = VectorRegisterF::Min(tMax, tMax.Swizzle<Swizzle::Z, Swizzle::X, Swizzle::Y, Swizzle::Y>());

        // If (tMin > tMax) return FLT_MAX
        VectorRegisterUint noIntersection = VectorRegisterF::Greater(tMin, tMax);

        // If (tMax < 0.f) return FLT_MAX
        noIntersection = VectorRegisterUint::Or(noIntersection, VectorRegisterF::Less(tMax, VectorRegisterF::Zero()));

        // If (invDirection.m_isParallel && !(Min <= origin && origin <= Max)) return FLT_MAX; else return tMin
        VectorRegisterUint noParallelOverlap = VectorRegisterUint::Or(VectorRegisterF::Less(rOrigin, rBoundsMin), VectorRegisterF::Greater(rOrigin, rBoundsMax));
        noIntersection = VectorRegisterUint::Or(noIntersection, VectorRegisterUint::And(invDirection.m_isParallel, noParallelOverlap));
        noIntersection = VectorRegisterUint::Or(noIntersection, noIntersection.SplatY());
        noIntersection = VectorRegisterUint::Or(noIntersection, noIntersection.SplatZ());
        outMin = VectorRegisterF::Select(tMin, fltMax, noIntersection).GetX();
        outMax = VectorRegisterF::Select(tMax, fltMin, noIntersection).GetX();
    }
}

