// ScaleHelpers.h
#pragma once
#include "Math/Vector3.h"
#include "Math/SIMD/VectorRegisterUint.h"

namespace nes::ScaleHelpers
{
    /// Minimum valid scale value. This is used to prevent division by zero when scaling a shape with a zero scale.
    static constexpr float kMinScale = 1.0e-6f;
    
    /// The tolerance used to check if components of the scale vector are the same.
    static constexpr float kScaleToleranceSqr = 1.0e-8f;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Test if scale is identity.
    //----------------------------------------------------------------------------------------------------
    inline bool IsNotScaled(const Vector3& scale)       { return scale.IsClose(Vector3::Unit(), kScaleToleranceSqr); }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Test if scale is uniform.
    //----------------------------------------------------------------------------------------------------
    inline bool IsUniformScale(const Vector3& scale)    { return scale.Swizzle<Swizzle::Y, Swizzle::Z, Swizzle::X>().IsClose(scale, kScaleToleranceSqr); }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Test if any of the components of the scale have a value below kMinScale
    //----------------------------------------------------------------------------------------------------
    inline bool IsZeroScale(const Vector3& scale)
    {
        const VectorRegisterF absReg(std::abs(scale.x), std::abs(scale.y), std::abs(scale.z), std::abs(scale.z));
        return VectorRegisterF::Less(absReg, VectorRegisterF::Replicate(kMinScale)).TestAnyXYZTrue();
    }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Ensure that the scale for each component is at least kMinScale
    //----------------------------------------------------------------------------------------------------
    inline Vector3 MakeNonZeroScale(const Vector3& scale) { return scale.GetSign() * Vector3::Max(scale.Abs(), Vector3::Replicate(kMinScale)); }
    
}
