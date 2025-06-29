// RayAABox.h
#pragma once
#include "AABox.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper struct that holds the reciprocal of a ray for Ray vs AABox testing.  
    //----------------------------------------------------------------------------------------------------
    struct RayInvDirection
    {
        Vec3        m_invDirection;     /// 1 / ray direction
        UVec4Reg    m_isParallel;       /// For each component if it is parallel to the coordinate axis.
        
        inline RayInvDirection() = default;
        inline explicit RayInvDirection(const Vec3& direction) { Set(direction); }

        inline void Set(const Vec3& direction)
        {
            // if (abs(inDirection) <= Epsilon) the ray is nearly parallel to the slab.
            m_isParallel = Vec3::LessOrEqual(direction.Abs(), Vec3::Replicate(1.0e-20f));

            // Calculate 1 / direction while avoiding divisions by zero.
            const Vec3 result = Vec3::Select(direction, Vec3::One(), m_isParallel);
            m_invDirection = Vec3(result.x, result.y, result.z).Reciprocal();
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Intersect AABB with ray, returns minimal distance along ray or FLT_MAX if no hit.
    /// @note : Can return negative value if the ray starts in the box.
    //----------------------------------------------------------------------------------------------------
    NES_INLINE float RayAABox(const Vec3& origin, const RayInvDirection& invDirection, const Vec3& boundsMin, const Vec3& boundsMax)
    {
        Vec3 fltMin = Vec3::Replicate(-FLT_MAX);
        Vec3 fltMax = Vec3::Replicate(FLT_MAX);

        // Test against all three axes simultaneously
        Vec3 t1 = (boundsMin - origin) * invDirection.m_invDirection;
        Vec3 t2 = (boundsMax - origin) * invDirection.m_invDirection;

        // Compute the max of min(t1, t2) and the min of max(t1, t2) ensuring that
        // we don't use the results from any directions parallel to the slab
        Vec3 tMin = Vec3::Select(Vec3::Min(t1, t2), fltMin, invDirection.m_isParallel);
        Vec3 tMax = Vec3::Select(Vec3::Max(t1, t2), fltMax, invDirection.m_isParallel);

        // tMin.XYZ = maximum(tMin.x, tMin.y, tMin.z)
        tMin = Vec3::Max(tMin, tMin.Swizzle<ESwizzleY, ESwizzleZ, ESwizzleX>());
        tMin = Vec3::Max(tMin, tMin.Swizzle<ESwizzleZ, ESwizzleX, ESwizzleY>());

        // tMax.xyz = minimum(tMax.x, tMax.y, tMax.z);
        tMax = Vec3::Min(tMax, tMax.Swizzle<ESwizzleY, ESwizzleZ, ESwizzleX>());
        tMax = Vec3::Min(tMax, tMax.Swizzle<ESwizzleZ, ESwizzleX, ESwizzleY>());

        // If (tMin > tMax) return FLT_MAX
        UVec4Reg noIntersection = Vec3::Greater(tMin, tMax);

        // If (tMax < 0.f) return FLT_MAX
        noIntersection = UVec4Reg::Or(noIntersection, Vec3::Less(tMax, Vec3::Zero()));

        // If (invDirection.m_isParallel && !(Min <= origin && origin <= Max)) return FLT_MAX; else return tMin
        UVec4Reg noParallelOverlap = UVec4Reg::Or(Vec3::Less(origin, boundsMin), Vec3::Greater(origin, boundsMax));
        noIntersection = UVec4Reg::Or(noIntersection, UVec4Reg::And(invDirection.m_isParallel, noParallelOverlap));
        noIntersection = UVec4Reg::Or(noIntersection, noIntersection.SplatY());
        noIntersection = UVec4Reg::Or(noIntersection, noIntersection.SplatZ());
        return Vec3::Select(tMin, fltMax, noIntersection).x;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Intersect 4 AABBs with a ray. Returns the minimal distance along the ray, or FLT_MAX if no hit. 
    /// @note : Can return negative value if the ray starts in the box.
    //----------------------------------------------------------------------------------------------------    
    NES_INLINE Vec4Reg RayAABox4(const Vec3 origin, const RayInvDirection& invDirection, const Vec4Reg& boundsMinX, const Vec4Reg& boundsMinY, const Vec4Reg& boundsMinZ, const Vec4Reg& boundsMaxX, const Vec4Reg& boundsMaxY, const Vec4Reg& boundsMaxZ)
    {
        // Constants
        const Vec4Reg fltMin = Vec4Reg::Replicate(-FLT_MAX);
        const Vec4Reg fltMax = Vec4Reg::Replicate(FLT_MAX);

        // Origin
        const Vec4Reg originX = origin.SplatX();
        const Vec4Reg originY = origin.SplatY();
        const Vec4Reg originZ = origin.SplatZ();

        // Parallel
        const UVec4Reg parallelX = invDirection.m_isParallel.SplatX();
        const UVec4Reg parallelY = invDirection.m_isParallel.SplatY();
        const UVec4Reg parallelZ = invDirection.m_isParallel.SplatZ();

        // Inverse Direction
        const Vec4Reg invDirX = invDirection.m_invDirection.SplatX();
        const Vec4Reg invDirY = invDirection.m_invDirection.SplatY();
        const Vec4Reg invDirZ = invDirection.m_invDirection.SplatZ();

        // Test against all three axes simultaneously.
        const Vec4Reg t1X = (boundsMinX - originX) * invDirX;
        const Vec4Reg t1Y = (boundsMinY - originY) * invDirY;
        const Vec4Reg t1Z = (boundsMinZ - originZ) * invDirZ;
        const Vec4Reg t2X = (boundsMaxX - originX) * invDirX;
        const Vec4Reg t2Y = (boundsMaxY - originY) * invDirY;
        const Vec4Reg t2Z = (boundsMaxZ - originZ) * invDirZ;

        // Compute the max of min(t1, t2) and the min of max (t1, t2) ensuring we don't
        // use the results from any directions that are parallel to the slab.
        const Vec4Reg tMinX = Vec4Reg::Select(Vec4Reg::Min(t1X, t2X), fltMin, parallelX);
        const Vec4Reg tMinY = Vec4Reg::Select(Vec4Reg::Min(t1Y, t2Y), fltMin, parallelY);
        const Vec4Reg tMinZ = Vec4Reg::Select(Vec4Reg::Min(t1Z, t2Z), fltMin, parallelZ);
        const Vec4Reg tMaxX = Vec4Reg::Select(Vec4Reg::Max(t1X, t2X), fltMax, parallelX);
        const Vec4Reg tMaxY = Vec4Reg::Select(Vec4Reg::Max(t1Y, t2Y), fltMax, parallelY);
        const Vec4Reg tMaxZ = Vec4Reg::Select(Vec4Reg::Max(t1Z, t2Z), fltMax, parallelZ);
        
        // tMin.xyz = max(tMin.x, tMin.y, tMin.z)
        const Vec4Reg tMin = Vec4Reg::Max(Vec4Reg::Max(tMinX, tMinY), tMinZ);

        // tMax.xyz = min(tMax.x, tMax.y, tMax.z)
        const Vec4Reg tMax = Vec4Reg::Min(Vec4Reg::Min(tMaxX, tMaxY), tMaxZ);

        // If (tMin > tMax) return FLT_MAX
        UVec4Reg noIntersection = Vec4Reg::Greater(tMin, tMax);

        // If (tMax < 0.f) return FLT_MAX
        noIntersection = UVec4Reg::Or(noIntersection, Vec4Reg::Less(tMax, Vec4Reg::Zero()));

        // If bounds are invalid, return FLT_MAX
        const UVec4Reg boundsInvalid = UVec4Reg::Or(UVec4Reg::Or(Vec4Reg::Greater(boundsMinX, boundsMaxX), Vec4Reg::Greater(boundsMinY, boundsMaxY)), Vec4Reg::Greater(boundsMinZ, boundsMaxZ));
        noIntersection = UVec4Reg::Or(noIntersection, boundsInvalid);

        // If (invDirection.m_isParallel && !(min <= origin && origin <= max)) return FLT_MAX; else return tMin.
        const UVec4Reg noParallelOverlapX = UVec4Reg::And(parallelX, UVec4Reg::Or(Vec4Reg::Less(originX, boundsMinX), Vec4Reg::Greater(originX, boundsMaxX))); 
        const UVec4Reg noParallelOverlapY = UVec4Reg::And(parallelY, UVec4Reg::Or(Vec4Reg::Less(originY, boundsMinY), Vec4Reg::Greater(originY, boundsMaxY))); 
        const UVec4Reg noParallelOverlapZ = UVec4Reg::And(parallelZ, UVec4Reg::Or(Vec4Reg::Less(originZ, boundsMinZ), Vec4Reg::Greater(originZ, boundsMaxZ)));
        noIntersection = UVec4Reg::Or(noIntersection, UVec4Reg::Or(UVec4Reg::Or(noParallelOverlapX, noParallelOverlapY), noParallelOverlapZ));
        return Vec4Reg::Select(tMin, fltMax, noIntersection);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Intersect AABB with ray, returns minimal and maximal distance along ray or FLT_MAX, -FLT_MAX if no hit
    /// @note : Can return negative value for outMin if the ray starts in the box.
    //----------------------------------------------------------------------------------------------------
    NES_INLINE void RayAABox(const Vec3& origin, const RayInvDirection& invDirection, const Vec3& boundsMin, const Vec3& boundsMax, float& outMin, float& outMax)
    {
        Vec3 fltMin = Vec3::Replicate(-FLT_MAX);
        Vec3 fltMax = Vec3::Replicate(FLT_MAX);
        
        // Test against all three axes simultaneously
        Vec3 t1 = (boundsMin - origin) * invDirection.m_invDirection;
        Vec3 t2 = (boundsMax - origin) * invDirection.m_invDirection;

        // Compute the max of min(t1, t2) and the min of max(t1, t2) ensuring that
        // we don't use the results from any directions parallel to the slab
        Vec3 tMin = Vec3::Select(Vec3::Min(t1, t2), fltMin, invDirection.m_isParallel);
        Vec3 tMax = Vec3::Select(Vec3::Max(t1, t2), fltMax, invDirection.m_isParallel);

        // tMin.XYZ = maximum(tMin.x, tMin.y, tMin.z)
        tMin = Vec3::Max(tMin, tMin.Swizzle<ESwizzleY, ESwizzleZ, ESwizzleX>());
        tMin = Vec3::Max(tMin, tMin.Swizzle<ESwizzleZ, ESwizzleX, ESwizzleY>());

        // tMax.xyz = minimum(tMax.x, tMax.y, tMax.z);
        tMax = Vec3::Min(tMax, tMax.Swizzle<ESwizzleY, ESwizzleZ, ESwizzleX>());
        tMax = Vec3::Min(tMax, tMax.Swizzle<ESwizzleZ, ESwizzleX, ESwizzleY>());

        // If (tMin > tMax) return FLT_MAX
        UVec4Reg noIntersection = Vec3::Greater(tMin, tMax);

        // If (tMax < 0.f) return FLT_MAX
        noIntersection = UVec4Reg::Or(noIntersection, Vec3::Less(tMax, Vec3::Zero()));

        // If (invDirection.m_isParallel && !(Min <= origin && origin <= Max)) return FLT_MAX; else return tMin
        UVec4Reg noParallelOverlap = UVec4Reg::Or(Vec3::Less(origin, boundsMin), Vec3::Greater(origin, boundsMax));
        noIntersection = UVec4Reg::Or(noIntersection, UVec4Reg::And(invDirection.m_isParallel, noParallelOverlap));
        noIntersection = UVec4Reg::Or(noIntersection, noIntersection.SplatY());
        noIntersection = UVec4Reg::Or(noIntersection, noIntersection.SplatZ());
        outMin = Vec3::Select(tMin, fltMax, noIntersection).x;
        outMax = Vec3::Select(tMax, fltMin, noIntersection).x;
    }
}

