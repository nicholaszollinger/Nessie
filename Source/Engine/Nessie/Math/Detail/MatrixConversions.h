// MatrixTransform.h
#pragma once
#include "TMatrix2x2.h"
#include "TMatrix3x3.h"
#include "TMatrix4x4.h"
#include "Math/Quaternion.h"
#include "Math/Rotation.h"

// namespace nes::math
// {
//     template <FloatingPointType Type>
//     TMatrix3x3<Type> ToMat3(const TQuaternion<Type>& q);
//
//     template <FloatingPointType Type>
//     TMatrix3x3<Type> ToMat3(const TMatrix4x4<Type>& m);
//     
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> ToMat4(const TQuaternion<Type>& q);
//
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> ToMat4(const Rotation<Type>& r);
//
//     template <FloatingPointType Type>
//     TQuaternion<Type> ToQuat(const TMatrix3x3<Type>& matrix);
//
//     template <FloatingPointType Type>
//     TQuaternion<Type> ToQuat(const TMatrix4x4<Type>& matrix);
//
//     template <FloatingPointType Type>
//     Rotation<Type> ToRotation(const TMatrix3x3<Type>& matrix);
//
//     template <FloatingPointType Type>
//     Rotation<Type> ToRotation(const TMatrix4x4<Type>& matrix);
//
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> MakeRotationFromEuler(const TVector3<Type>& eulerAngles);
//
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> MakeRotationFromEuler(const Type pitch, const Type yaw, const Type roll);
//
//     template <FloatingPointType Type>
//     TQuaternion<Type> ExtractRotationQuat(const TMatrix4x4<Type>& matrix);
//
//     template <FloatingPointType Type>
//     Rotation<Type> ExtractRotation(const TMatrix4x4<Type>& matrix);
//
//     template <FloatingPointType Type>
//     TMatrix3x3<Type> ExtractMatrixRotation3x3(const TMatrix4x4<Type>& matrix);
//
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> ExtractMatrixRotation4x4(const TMatrix4x4<Type>& matrix);
// }
//
// namespace nes::math
// {
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Convert a Quaternion to a 3x3 Matrix. 
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     TMatrix3x3<Type> ToMat3(const TQuaternion<Type>& q)
//     {
//         // pg 284 of "3D Math Primer for Graphics and Game Development".
//         // Note that my Matrices are column major, so the representation is
//         // the transpose.
//         const Type xx = q.x * q.x;
//         const Type yy = q.y * q.y;
//         const Type zz = q.z * q.z;
//         const Type xy = q.x * q.y;
//         const Type xz = q.x * q.z;
//         const Type yz = q.y * q.z;
//         const Type wx = q.w * q.x;
//         const Type wy = q.w * q.y;
//         const Type wz = q.w * q.z;
//         
//         TMatrix3x3<Type> result;
//         result[0][0] = static_cast<Type>(1) - static_cast<Type>(2) * (yy + zz);
//         result[0][1] = static_cast<Type>(2) * (xy + wz);
//         result[0][2] = static_cast<Type>(2) * (xz - wy);
//
//         result[1][0] = static_cast<Type>(2) * (xy - wz);
//         result[1][1] = static_cast<Type>(1) - static_cast<Type>(2) * (xx + zz);
//         result[1][2] = static_cast<Type>(2) * (yz + wx);
//
//         result[2][0] = static_cast<Type>(2) * (xz + wy);
//         result[2][1] = static_cast<Type>(2) * (yz - wx);
//         result[2][2] = static_cast<Type>(1) - static_cast<Type>(2) * (xx + yy);
//         
//         return result;
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Converts a 4x4 matrix to a 3x3 Matrix. 
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     TMatrix3x3<Type> ToMat3(const TMatrix4x4<Type>& m)
//     {
//         TMatrix3x3<Type> result;
//         result[0][0] = m[0][0];
//         result[0][1] = m[0][1];
//         result[0][2] = m[0][2];
//         
//         result[1][0] = m[1][0];
//         result[1][1] = m[1][1];
//         result[1][2] = m[1][2];
//
//         result[2][0] = m[2][0];
//         result[2][1] = m[2][1];
//         result[2][2] = m[2][2];
//         return result;
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Convert a Quaternion to a 4x4 Matrix.
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> ToMat4(const TQuaternion<Type>& q)
//     {
//         return TMatrix4x4<Type>(ToMat3<Type>(q));
//     }
//
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> ToMat4(const Rotation<Type>& r)
//     {
//         return MakeRotationFromEuler(r.m_pitch * DegreesToRadians(), r.m_yaw * DegreesToRadians(), r.m_roll * DegreesToRadians());
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Converts the 3x3 Matrix to a Quaternion. Note: this will not remove any scaling present
//     ///             in the matrix!
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     TQuaternion<Type> ToQuat(const TMatrix3x3<Type>& matrix)
//     {
//         // pg 286 of "Math Primer for Graphics and Game Development".
//         const Type fourXSquaredMinus1 = matrix[0][0] - matrix[1][1] - matrix[2][2];
//         const Type fourYSquaredMinus1 = matrix[1][1] - matrix[0][0] - matrix[2][2];
//         const Type fourZSquaredMinus1 = matrix[2][2] - matrix[0][0] - matrix[1][1];
//         const Type fourWSquaredMinus1 = matrix[0][0] + matrix[1][1] + matrix[2][2];
//
//         // Determine which of w, x, y, or z has the largest absolute value.
//         int biggestIndex = 0;
//         Type fourBiggerSquaredMinus1 = fourWSquaredMinus1;
//         if (fourXSquaredMinus1 > fourBiggerSquaredMinus1)
//         {
//             fourBiggerSquaredMinus1 = fourXSquaredMinus1;
//             biggestIndex = 1;
//         }
//
//         if (fourYSquaredMinus1 > fourBiggerSquaredMinus1)
//         {
//             fourBiggerSquaredMinus1 = fourYSquaredMinus1;
//             biggestIndex = 2;
//         }
//
//         if (fourZSquaredMinus1 > fourBiggerSquaredMinus1)
//         {
//             fourBiggerSquaredMinus1 = fourZSquaredMinus1;
//             biggestIndex = 2;
//         }
//
//         const Type biggestValue = std::sqrt(fourBiggerSquaredMinus1 + static_cast<Type>(1)) * static_cast<Type>(0.5);
//         const Type mult = static_cast<Type>(0.25) / biggestValue;
//
//         switch (biggestIndex)
//         {
//             case 0: // W
//                 return TQuaternion<Type>(
//                     biggestValue,
//                     (matrix[1][2] - matrix[2][1]) * mult,
//                     (matrix[2][0] - matrix[0][2]) * mult,
//                     (matrix[0][1] - matrix[1][0]) * mult);
//
//             case 1: // X
//                 return TQuaternion<Type>(
//                     (matrix[1][2] - matrix[2][1]) * mult,
//                     biggestValue,
//                     (matrix[0][1] + matrix[1][0]) * mult,
//                     (matrix[2][0] + matrix[0][2]) * mult);
//
//             case 2: // Y
//                 return TQuaternion<Type>(
//                     (matrix[2][0] - matrix[0][2]) * mult,
//                     (matrix[0][1] + matrix[1][0]) * mult,
//                     biggestValue,
//                     (matrix[1][2] + matrix[2][1]) * mult);
//
//             case 3: // Z
//                 return TQuaternion<Type>(
//                     (matrix[0][1] - matrix[1][0]) * mult,
//                     (matrix[2][0] + matrix[0][2]) * mult,
//                     (matrix[1][2] + matrix[2][1]) * mult,
//                     biggestValue);
//
//             default:
//                 NES_ASSERT(false);
//                 return TQuaternion<Type>(1, 0, 0, 0);
//         }
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Converts the orientation defined by the Matrix as a Quaternion.
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     TQuaternion<Type> ToQuat(const TMatrix4x4<Type>& matrix)
//     {
//         return ToQuat(ToMat3<Type>(matrix));
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Converts the rotation defined by the Matrix as a Rotation object.
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     Rotation<Type> ToRotation(const TMatrix3x3<Type>& matrix)
//     {
//         const Type tanPitch = std::atan2(matrix[2][1], matrix[2][2]);
//         const Type cosYaw = std::sqrt((matrix[0][0] * matrix[0][0]) + (matrix[1][0] * matrix[1][0]));
//         const Type tanYaw = std::atan2(-matrix[2][0], cosYaw);
//         const Type sinPitch = std::sin(tanPitch);
//         const Type cosPitch = std::cos(tanPitch);
//         const Type tanRoll = std::atan2(sinPitch * matrix[0][2] - cosPitch * matrix[0][1], cosPitch * matrix[1][1] - sinPitch * matrix[1][2]);
//         
//         return TRotation<Type>(-tanPitch * RadiansToDegrees(), -tanYaw * RadiansToDegrees(), -tanRoll * RadiansToDegrees());
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Converts the rotation defined by the Matrix as a Rotation object.
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     Rotation<Type> ToRotation(const TMatrix4x4<Type>& matrix)
//     {
//         return ToRotation(TMat3<Type>(matrix));
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Make an Orientation Matrix from a set of Euler Angles. The Angles are expected to
//     ///             be in radians, and in the form: x=pitch, y=yaw, z=roll.
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> MakeRotationFromEuler(const TVector3<Type>& eulerAngles)
//     {
//         const Type cosPitch = std::cos(eulerAngles.x);
//         const Type sinPitch = std::sin(eulerAngles.x);
//         const Type cosYaw = std::cos(eulerAngles.y);
//         const Type sinYaw = std::sin(eulerAngles.y);
//         const Type cosRoll = std::cos(eulerAngles.z);
//         const Type sinRoll = std::sin(eulerAngles.z);
//
//         TMatrix4x4<Type> result = TMatrix4x4<Type>::Identity();
//         result[0][0] = (cosYaw * cosRoll) + (sinYaw * sinPitch * sinRoll);
//         result[0][1] = (sinRoll * cosPitch);
//         result[0][2] = -(sinYaw * cosRoll) + (cosYaw * sinPitch * sinRoll);
//
//         result[1][0] = -(cosYaw * sinRoll) + (sinYaw * sinPitch * cosRoll);
//         result[1][1] = (cosRoll * cosPitch);
//         result[1][2] = (sinRoll * sinYaw) + (cosYaw * sinPitch * cosRoll);
//
//         result[2][0] = (sinYaw * cosPitch);
//         result[2][1] = -sinPitch;
//         result[2][2] = (cosYaw * cosPitch);
//         
//         return result;
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Make an Orientation Matrix from a set of Euler Angles. The Angles are expected to
//     ///             be in radians.
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> MakeRotationFromEuler(const Type pitch, const Type yaw, const Type roll)
//     {
//         return MakeRotationFromEuler(TVector3<Type>(pitch, yaw, roll));
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Returns the unscaled orientation of the matrix as a Quaternion.
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     TQuaternion<Type> ExtractRotationQuat(const TMatrix4x4<Type>& matrix)
//     {
//         TMatrix4x4<Type> copy(matrix);
//         copy.RemoveScaling();
//         return ToQuat(ToMat3<Type>(copy));
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Returns the unscaled rotation of the matrix as a Rotation object. 
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     Rotation<Type> ExtractRotation(const TMatrix4x4<Type>& matrix)
//     {
//         TMatrix4x4<Type> copy(matrix);
//         copy.RemoveScaling();
//         return ToRotation(ToMat3<Type>(copy));
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Returns the unscaled orientation of the matrix as a 3x3 Matrix. 
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     TMatrix3x3<Type> ExtractMatrixRotation3x3(const TMatrix4x4<Type>& matrix)
//     {
//         TMatrix4x4<Type> copy(matrix);
//         copy.RemoveScaling();
//         return ToMat3(copy);
//     }
//
//     //----------------------------------------------------------------------------------------------------
//     ///		@brief : Returns the unscaled orientation of the matrix as a 4x4 matrix. 
//     //----------------------------------------------------------------------------------------------------
//     template <FloatingPointType Type>
//     TMatrix4x4<Type> ExtractMatrixRotation4x4(const TMatrix4x4<Type>& matrix)
//     {
//         TMatrix4x4<Type> copy(matrix);
//         copy.RemoveScaling();
//         // Zero out translation:
//         copy[3] = TVector4<Type>(static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(1));
//         // Set the bottom row to zero.
//         copy[0][3] = static_cast<Type>(0);
//         copy[1][3] = static_cast<Type>(0);
//         copy[2][3] = static_cast<Type>(0);
//         return copy;
//     }
// }
