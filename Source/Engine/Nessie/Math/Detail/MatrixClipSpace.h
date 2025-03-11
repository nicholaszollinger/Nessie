// MatrixClipSpace.h
#pragma once
#include "MatrixConversions.h"

#define NES_MATH_CLIP_ZERO_TO_ONE 1

namespace nes::math
{
    template <FloatingPointType Type>
    TMatrix4x4<Type> PerspectiveFOV(const Type fovRadians, const Type width, const Type height, const Type near, const Type far);

    template <FloatingPointType Type>
    TMatrix4x4<Type> PerspectiveFOV(const Type fovRadians, const Type aspectRatio, const Type near, const Type far);

    template <FloatingPointType Type>
    TMatrix4x4<Type> Orthographic(const Type left, const Type right, const Type bottom, const Type top, const Type near, const Type far);
}

namespace nes::math
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a left-handed, perspective projection matrix based on a field of view.
    ///             The near and far clip planes are normalized to [0, 1] if NES_MATH_CLIP_ZERO_TO_ONE is equal to 1,
    ///             otherwise, it will be normalized to [-1, 1].
    ///		@param fovRadians : Field of View, expressed in radians.
    ///		@param width : Width of the viewport.
    ///		@param height : Height of the viewport.
    ///		@param near : Distance of the viewer to the near clip plane (must always be positive).
    ///		@param far : Distance of the viewer to the far clip plane (must always be positive).
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> PerspectiveFOV(const Type fovRadians, const Type width, const Type height, const Type near,
        const Type far)
    {
        NES_ASSERT(width > static_cast<Type>(0));
        NES_ASSERT(height > static_cast<Type>(0));
        NES_ASSERT(fovRadians > static_cast<Type>(0));

        const Type zoomY = std::cos(static_cast<Type>(0.5) * fovRadians / std::sin(static_cast<Type>(0.5) * fovRadians));
        const Type zoomX = zoomY * height / width; // Max(width, height) / Min(width/height);

        TMatrix4x4<Type> result = TMatrix4x4<Type>::Zero();
        result.m[0][0] = zoomX;
        result.m[1][1] = zoomY;
        result.m[3][2] = static_cast<Type>(1);

#if NES_MATH_CLIP_ZERO_TO_ONE
        result.m[2][2] = far / (far - near);
        result.m[2][3] = -(far * near) / (far - near);
#else
        result.m[2][2] = (far + near) / (far - near);
        result.m[2][3] = -(static_cast<Type>(2) * far * near) / (far - near);
#endif
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a left-handed, perspective projection matrix based on a field of view.
    ///             The near and far clip planes are normalized to [0, 1] if NES_MATH_CLIP_ZERO_TO_ONE is equal to 1,
    ///             otherwise, it will be normalized to [-1, 1].
    ///		@param fovRadians : Vertical Field of View, expressed in radians. 
    ///		@param aspectRatio : Aspect Ratio of the Viewport. Equal to width / height.
    ///		@param near : Distance of the viewer to the near clip plane (must always be positive).
    ///		@param far : Distance of the viewer to the far clip plane (must always be positive).
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> PerspectiveFOV(const Type fovRadians, const Type aspectRatio, const Type near, const Type far)
    {
        // Aspect ratio cannot be zero.
        NES_ASSERT(!math::CheckEqualFloats(aspectRatio, static_cast<Type>(0)));

        const Type tangentHalfFOVy = std::tan(fovRadians / static_cast<Type>(2));

        TMatrix4x4<Type> result = TMatrix4x4<Type>::Zero();
        result.m[0][0] = static_cast<Type>(1) / (aspectRatio * tangentHalfFOVy);
        result.m[1][1] = static_cast<Type>(1) / (tangentHalfFOVy);
        result.m[3][2] = static_cast<Type>(1);

#if NES_MATH_CLIP_ZERO_TO_ONE
        result.m[2][2] = far / (far - near);
        result.m[2][3] = -(far * near) / (far - near);
#else
        result.m[2][2] = (far + near) / (far - near);
        result.m[2][3] = -(static_cast<Type>(2) * far * near) / (far - near);
#endif
        
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a left-handed orthographic project matrix.
    ///             The near and far clip planes are normalized to [0, 1] if NES_MATH_CLIP_ZERO_TO_ONE is equal to 1,
    ///             otherwise, it will be normalized to [-1, 1].
    ///		@param left : Left side of the projection.
    ///		@param right : Right side of the projection.
    ///		@param bottom : Bottom of the projection.
    ///		@param top : Top of the projection.
    ///		@param near : Distance of the viewer to the near clip plane (must always be positive).
    ///		@param far : Distance of the viewer to the far clip plane (must always be positive).
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> Orthographic(const Type left, const Type right, const Type bottom, const Type top, const Type near,
        const Type far)
    {
        TMatrix4x4<Type> result = TMatrix4x4<Type>::Identity();
        result.m[0][0] = static_cast<Type>(2) / (right - left);
        result.m[1][1] = static_cast<Type>(2) / (top - bottom);
        result.m[0][3] = -(right + left) / (right - left);
        result.m[1][3] = -(top + bottom) / (top - bottom);
        
#if NES_MATH_CLIP_ZERO_TO_ONE
        result.m[2][2] = static_cast<Type>(1) / (far - near);
        result.m[2][3] = -near / (far - near);
#else
        result.m[2][2] = static_cast<Type>(2) / (far - near);
        result.m[2][3] = -(far + near) / (far - near);
#endif
        
        return result;
    }
}