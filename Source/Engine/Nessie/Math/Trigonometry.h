// Trigonometry.h
#pragma once

namespace nes::math
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Sine of the angle (in radians). 
    //----------------------------------------------------------------------------------------------------
    NES_INLINE float Sin(const float angle)
    {
        Vec4Reg sin, cosine;
        Vec4Reg::Replicate(angle).SinCos(sin, cosine);
        return sin.GetX();
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Cosine of the angle (in radians). 
    //----------------------------------------------------------------------------------------------------
    NES_INLINE float Cos(const float angle)
    {
        Vec4Reg sin, cos;
        Vec4Reg::Replicate(angle).SinCos(sin, cos);
        return cos.GetX();
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Tangent of the angle (in radians). 
    //----------------------------------------------------------------------------------------------------
    NES_INLINE float Tan(const float angle)
    {
        return Vec4Reg::Replicate(angle).Tan().GetX();
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Arc sine of the angle (returns a value in the range [-PI / 2, PI / 2]). 
    ///	@note : All input values will be clamped to the range [-1, 1] and this function will not return NaNs
    ///     like std::asin.
    /// @note : Angle should be in radians.
    //----------------------------------------------------------------------------------------------------
    NES_INLINE float ASin(const float angle)
    {
        return Vec4Reg::Replicate(angle).ASin().GetX();
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Arc cosine of the angle (returns a value in the range [0, PI]). 
    ///	@note : All input values will be clamped to the range [-1, 1] and this function will not return NaNs
    ///     like std::acos.
    /// @note : Angle should be in radians.
    //----------------------------------------------------------------------------------------------------
    NES_INLINE float ACos(const float angle)
    {
        return Vec4Reg::Replicate(angle).ACos().GetX();
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : An approximation of ACos, max error is 4.2e-3 over the entire range [-1, 1].
    ///     This is approximately 2.5x faster than ACos.
    /// @note : Angle should be in radians.
    //----------------------------------------------------------------------------------------------------
    NES_INLINE float ACosApproximate(const float angle)
    {
        // See: https://www.johndcook.com/blog/2022/09/06/inverse-cosine-near-1/
        // See also: https://seblagarde.wordpress.com/2014/12/01/inverse-trigonometric-functions-gpu-optimization-for-amd-gcn-architecture/
        // Taylor of cos(x) = 1 - x^2 / 2 + ...
        // Substitute x = sqrt(2 y) we get: cos(sqrt(2 y)) = 1 - y
        // Substitute z = 1 - y we get: cos(sqrt(2 (1 - z))) = z <=> acos(z) = sqrt(2 (1 - z))
        // To avoid the discontinuity at 1, instead of using the Taylor expansion of acos(x) we use acos(x) / sqrt(2 (1 - x)) = 1 + (1 - x) / 12 + ...
        // Since the approximation was made at 1, it has quite a large error at 0 meaning that if we want to extend to the
        // range [-1, 1] by mirroring the range [0, 1], the value at 0+ is not the same as 0-.
        // So we observe that the form of the Taylor expansion is f(x) = sqrt(1 - x) * (a + b x) and we fit the function so that f(0) = pi / 2
        // this gives us a = pi / 2. f(1) = 0 regardless of b. We search for a constant b that minimizes the error in the range [0, 1].
        const float absAngle = std::min(Abs(angle), 1.0f); // Ensure that we don't get a value larger than 1
        const float val = std::sqrt(1.f - absAngle) * (Pi<float>() / 2.f - 0.175394f * absAngle);

        // Our approximation is valid in the range [0, 1], extend it to the range [-1, 1]
        return angle < 0.f? (Pi<float>() - val) : val;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Arc tangent of the angle (returns value in the range [-PI / 2, PI / 2]).
    /// @note : Angle should be in radians.
    //----------------------------------------------------------------------------------------------------
    NES_INLINE float ATan(const float angle)
    {
        return Vec4Reg::Replicate(angle).ATan().GetX();
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Arc tangent of y / x using the sings of the arguments to determine the correct quadrant
    ///     (returns value in the range [-PI, PI]).
    //----------------------------------------------------------------------------------------------------
    NES_INLINE float ATan2(const float y, const float x)
    {
        return Vec4Reg::ATan2(Vec4Reg::Replicate(y), Vec4Reg::Replicate(x)).GetX();
    }
}
