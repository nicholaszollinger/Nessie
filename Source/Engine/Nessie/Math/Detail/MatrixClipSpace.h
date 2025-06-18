// MatrixClipSpace.h
#pragma once
#include "MatrixConversions.h"

#define NES_MATH_CLIP_ZERO_TO_ONE
//
// namespace nes::math
// {
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> PerspectiveFOV(const Type fovRadians, const Type width, const Type height, const Type nearPlane, const Type farPlane);
//
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> PerspectiveFOV(const Type fovRadians, const Type aspectRatio, const Type nearPlane, const Type farPlane);
//
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> Orthographic(const Type left, const Type right, const Type bottom, const Type top, const Type nearPlane, const Type farPlane);
// }
//
// namespace nes::math
// {
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Create a left-handed, perspective projection matrix based on a field of view.
//     ///             The near and farPlane clip planes are normalized to [0, 1] if NES_MATH_CLIP_ZERO_TO_ONE is equal to 1,
//     ///             otherwise, it will be normalized to [-1, 1].
//     ///		@param fovRadians : Field of View, expressed in radians.
//     ///		@param width : Width of the viewport.
//     ///		@param height : Height of the viewport.
//     ///		@param nearPlane : Distance of the viewer to the near clip plane (must always be positive).
//     ///		@param farPlane : Distance of the viewer to the farPlane clip plane (must always be positive).
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> PerspectiveFOV(const Type fovRadians, const Type width, const Type height, const Type nearPlane,
//         const Type farPlane)
//     {
//         NES_ASSERT(width > static_cast<Type>(0));
//         NES_ASSERT(height > static_cast<Type>(0));
//         NES_ASSERT(fovRadians > static_cast<Type>(0));
//
//         const Type zoomY = std::cos(static_cast<Type>(0.5) * fovRadians) / std::sin(static_cast<Type>(0.5) * fovRadians);
//         const Type zoomX = zoomY * height / width; // Max(width, height) / Min(width/height);
//
//         TMatrix4x4<Type> result = TMatrix4x4<Type>::Zero();
//         result[0][0] = zoomX;
//         result[1][1] = zoomY;
//         result[2][3] = static_cast<Type>(1);
//
// #ifdef NES_MATH_CLIP_ZERO_TO_ONE
//         result[2][2] = farPlane / (farPlane - nearPlane);
//         result[3][2] = -(farPlane * nearPlane) / (farPlane - nearPlane);
// #else
//         result.m[2][2] = (farPlane + nearPlane) / (farPlane - nearPlane);
//         result.m[3][2] = -(static_cast<Type>(2) * farPlane * nearPlane) / (farPlane - nearPlane);
// #endif
//         return result;
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Create a left-handed, perspective projection matrix based on a field of view.
//     ///             The nearPlane and farPlane clip planes are normalized to [0, 1] if NES_MATH_CLIP_ZERO_TO_ONE is equal to 1,
//     ///             otherwise, it will be normalized to [-1, 1].
//     ///		@param fovRadians : Vertical Field of View, expressed in radians. 
//     ///		@param aspectRatio : Aspect Ratio of the Viewport. Equal to width / height.
//     ///		@param nearPlane : Distance of the viewer to the nearPlane clip plane (must always be positive).
//     ///		@param farPlane : Distance of the viewer to the farPlane clip plane (must always be positive).
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> PerspectiveFOV(const Type fovRadians, const Type aspectRatio, const Type nearPlane, const Type farPlane)
//     {
//         // Aspect ratio cannot be zero.
//         NES_ASSERT(!math::CheckEqualFloats(aspectRatio, static_cast<Type>(0)));
//
//         const Type tangentHalfFOVy = std::tan(fovRadians / static_cast<Type>(2));
//
//         TMatrix4x4<Type> result = TMatrix4x4<Type>::Zero();
//         result[0][0] = static_cast<Type>(1) / (aspectRatio * tangentHalfFOVy);
//         result[1][1] = static_cast<Type>(1) / (tangentHalfFOVy);
//         result[2][3] = static_cast<Type>(1);
//
// #ifdef NES_MATH_CLIP_ZERO_TO_ONE
//         result[2][2] = farPlane / (farPlane - nearPlane);
//         result[3][2] = -(farPlane * nearPlane) / (farPlane - nearPlane);
// #else
//         result[2][2] = (farPlane + nearPlane) / (farPlane - nearPlane);
//         result[3][2] = -(static_cast<Type>(2) * farPlane * nearPlane) / (farPlane - nearPlane);
// #endif
//         return result;
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Create a left-handed orthographic project matrix.
//     ///             The nearPlane and farPlane clip planes are normalized to [0, 1] if NES_MATH_CLIP_ZERO_TO_ONE is equal to 1,
//     ///             otherwise, it will be normalized to [-1, 1].
//     ///		@param left : Left side of the projection.
//     ///		@param right : Right side of the projection.
//     ///		@param bottom : Bottom of the projection.
//     ///		@param top : Top of the projection.
//     ///		@param nearPlane : Distance of the viewer to the nearPlane clip plane (must always be positive).
//     ///		@param farPlane : Distance of the viewer to the farPlane clip plane (must always be positive).
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> Orthographic(const Type left, const Type right, const Type bottom, const Type top, const Type nearPlane,
//         const Type farPlane)
//     {
//         TMatrix4x4<Type> result = TMatrix4x4<Type>::Identity();
//         result[0][0] = static_cast<Type>(2) / (right - left);
//         result[1][1] = static_cast<Type>(2) / (top - bottom);
//         result[3][0] = -(right + left) / (right - left);
//         result[3][1] = -(top + bottom) / (top - bottom);
//         
// #ifdef NES_MATH_CLIP_ZERO_TO_ONE
//         result[2][2] = (static_cast<Type>(1) / (farPlane - nearPlane));
//         result[3][2] = (-nearPlane / (farPlane - nearPlane));
// #else
//         result[2][2] = static_cast<Type>(2) / (farPlane - nearPlane);
//         result[3][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
// #endif
//         
//         return result;
//     }
// }