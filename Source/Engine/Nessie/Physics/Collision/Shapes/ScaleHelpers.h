// ScaleHelpers.h
#pragma once
#include "Math/Vec3.h"
#include "Physics/PhysicsSettings.h"

namespace nes::ScaleHelpers
{
    /// Minimum valid scale value. This is used to prevent division by zero when scaling a shape with a zero scale.
    static constexpr float kMinScale = 1.0e-6f;
    
    /// The tolerance used to check if components of the scale vector are the same.
    static constexpr float kScaleToleranceSqr = 1.0e-8f;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Test if scale is identity.
    //----------------------------------------------------------------------------------------------------
    inline bool     IsNotScaled(const Vec3& scale)       { return scale.IsClose(Vec3::One(), kScaleToleranceSqr); }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Test if scale is uniform.
    //----------------------------------------------------------------------------------------------------
    inline bool     IsUniformScale(const Vec3& scale)    { return scale.Swizzle<ESwizzleY, ESwizzleZ, ESwizzleX>().IsClose(scale, kScaleToleranceSqr); }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Test if any of the components of the scale have a value below kMinScale
    //----------------------------------------------------------------------------------------------------
    inline bool     IsZeroScale(const Vec3& scale)
    {
        return Vec3::Less(scale.Abs(), Vec3::Replicate(kMinScale)).TestAnyXYZTrue();
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Test if a scale flips an object inside out (which requires flipping all normals and polygon windings.
    //----------------------------------------------------------------------------------------------------
    inline bool     IsInsideOut(const Vec3& scale)
    {
        return (math::CountBits(Vec3::Less(scale, Vec3::Zero()).GetTrues() & 0x7) & 1) != 0;
    }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Ensure that the scale for each component is at least kMinScale
    //----------------------------------------------------------------------------------------------------
    inline Vec3     MakeNonZeroScale(const Vec3& scale) { return scale.GetSign() * Vec3::Max(scale.Abs(), Vec3::Replicate(kMinScale)); }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the scaled convex radius of an object. 
    //----------------------------------------------------------------------------------------------------
    inline float    ScaleConvexRadius(const float convexRadius, const Vec3& scale)
    {
        return math::Min(convexRadius * scale.Abs().MinComponent(), physics::kDefaultConvexRadius);
    }
    
}
