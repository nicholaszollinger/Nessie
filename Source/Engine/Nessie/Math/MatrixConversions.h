// MatrixTransform.h
#pragma once
#include "Matrix.h"
#include "Quaternion.h"

namespace nes::math
{
    template <FloatingPointType Type>
    TMatrix3x3<Type> ToMat3(const TQuaternion<Type>& q);

    template <FloatingPointType Type>
    TMatrix3x3<Type> ToMat3(const TMatrix4x4<Type>& m);
    
    template <FloatingPointType Type>
    TMatrix4x4<Type> ToMat4(const TQuaternion<Type>& q);
    
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeTranslationMatrix(const TVector3<Type>& translation);

    template <FloatingPointType Type>
    TMatrix3x3<Type> MakeOrientationMatrix(const TQuaternion<Type>& orientation);
    
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeOrientationMatrix(const TQuaternion<Type>& orientation);

    template <FloatingPointType Type>
    TQuaternion<Type> ToQuat(const TMatrix3x3<Type>& matrix);

    template <FloatingPointType Type>
    TQuaternion<Type> ToQuat(const TMatrix4x4<Type>& matrix);

    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeOrientationFromEuler(const TVector3<Type>& eulerAngles);

    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeOrientationFromEuler(const Type pitch, const Type yaw, const Type roll);

    template <FloatingPointType Type>
    TQuaternion<Type> ExtractOrientation(const TMatrix4x4<Type>& matrix);

    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeScaleMatrix(const TVector3<Type>& scale);

    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeScaleMatrix(const Type uniformScale);

    template <FloatingPointType Type>
    void DecomposeMatrix(const TMatrix4x4<Type>& matrix, TVector3<Type>& translation, TQuaternion<Type>& orientation, TVector3<Type>& scale);

    template <FloatingPointType Type>
    TMatrix4x4<Type> ComposeTransformMatrix(const TVector3<Type>& translation, const TQuaternion<Type>& orientation, const TVector3<Type>& scale);
}

namespace nes::math
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Convert a Quaternion to a 3x3 Matrix. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix3x3<Type> ToMat3(const TQuaternion<Type>& q)
    {
        // pg 284 of "3D Math Primer for Graphics and Game Development".
        // Note that my Matrices are column major, so the representation is
        // the transpose.
        const Type xx = q.x * q.x;
        const Type yy = q.y * q.y;
        const Type zz = q.z * q.z;
        const Type xy = q.x * q.y;
        const Type xz = q.x * q.z;
        const Type yz = q.y * q.z;
        const Type wx = q.w * q.x;
        const Type wy = q.w * q.y;
        const Type wz = q.w * q.z;
        
        TMatrix3x3<Type> result;
        result.m[0][0] = static_cast<Type>(1) - static_cast<Type>(2) * (yy + zz);
        result.m[1][0] = static_cast<Type>(2) * (xy + wz);
        result.m[2][0] = static_cast<Type>(2) * (xz - wy);

        result.m[0][1] = static_cast<Type>(2) * (xy - wz);
        result.m[1][1] = static_cast<Type>(1) - static_cast<Type>(2) * (xx + zz);
        result.m[2][1] = static_cast<Type>(2) * (xz + wy);

        result.m[0][2] = static_cast<Type>(2) * (xz + wy);
        result.m[1][2] = static_cast<Type>(2) * (yz - wx);
        result.m[2][2] = static_cast<Type>(1) - static_cast<Type>(2) * (xx + yy);
        
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Converts a 4x4 matrix to a 3x3 Matrix. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix3x3<Type> ToMat3(const TMatrix4x4<Type>& m)
    {
        TMatrix3x3<Type> result;
        result.m[0][0] = m.m[0][0];
        result.m[1][0] = m.m[1][0];
        result.m[2][0] = m.m[2][0];
        
        result.m[0][1] = m.m[0][1];
        result.m[1][1] = m.m[1][1];
        result.m[2][1] = m.m[2][1];

        result.m[0][2] = m.m[0][2];
        result.m[1][2] = m.m[1][2];
        result.m[2][2] = m.m[2][2];
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Convert a Quaternion to a 4x4 Matrix.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> ToMat4(const TQuaternion<Type>& q)
    {
        return TMatrix4x4<Type>(ToMat3<Type>(q));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a Translation Matrix from a 3D Translation. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeTranslationMatrix(const TVector3<Type>& translation)
    {
        TMatrix4x4<Type> result = TMatrix4x4<Type>::Identity();
        result.m[0][3] = translation.x;
        result.m[1][3] = translation.y;
        result.m[2][3] = translation.z;
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create an Orientation Matrix from a quaternion.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix3x3<Type> MakeOrientationMatrix(const TQuaternion<Type>& orientation)
    {
        return ToMat3<Type>(orientation);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create an Orientation Matrix from a Quaternion Orientation. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeOrientationMatrix(const TQuaternion<Type>& orientation)
    {
        return ToMat4<Type>(orientation);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Converts the 3x3 Matrix to a Quaternion. Note: this will not remove any scaling present
    ///             in the matrix!
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TQuaternion<Type> ToQuat(const TMatrix3x3<Type>& matrix)
    {
        // pg 286 of "Math Primer for Graphics and Game Development".
        const Type fourXSquaredMinus1 = matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2];
        const Type fourYSquaredMinus1 = matrix.m[1][1] - matrix.m[0][0] - matrix.m[2][2];
        const Type fourZSquaredMinus1 = matrix.m[2][2] - matrix.m[0][0] - matrix.m[1][1];
        const Type fourWSquaredMinus1 = matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2];

        // Determine which of w, x, y, or z has the largest absolute value.
        int biggestIndex = 0;
        Type fourBiggerSquaredMinus1 = fourWSquaredMinus1;
        if (fourXSquaredMinus1 > fourBiggerSquaredMinus1)
        {
            fourBiggerSquaredMinus1 = fourXSquaredMinus1;
            biggestIndex = 1;
        }

        if (fourYSquaredMinus1 > fourBiggerSquaredMinus1)
        {
            fourBiggerSquaredMinus1 = fourYSquaredMinus1;
            biggestIndex = 2;
        }

        if (fourZSquaredMinus1 > fourBiggerSquaredMinus1)
        {
            fourBiggerSquaredMinus1 = fourZSquaredMinus1;
            biggestIndex = 2;
        }

        const Type biggestValue = std::sqrt(fourBiggerSquaredMinus1 + static_cast<Type>(1)) * static_cast<Type>(0.5);
        const Type mult = static_cast<Type>(0.25) / biggestValue;

        switch (biggestIndex)
        {
            case 0: // W
                return TQuaternion<Type>(
                    biggestValue,
                    (matrix.m[2][1] - matrix.m[1][2]) * mult,
                    (matrix.m[0][2] - matrix.m[2][0]) * mult,
                    (matrix.m[1][0] - matrix.m[0][1]) * mult);

            case 1: // X
                return TQuaternion<Type>(
                    (matrix.m[2][1] - matrix.m[1][2]) * mult,
                    biggestValue,
                    (matrix.m[1][0] + matrix.m[0][1]) * mult,
                    (matrix.m[0][2] + matrix.m[2][0]) * mult);

            case 2: // Y
                return TQuaternion<Type>(
                    (matrix.m[0][2] - matrix.m[2][0]) * mult,
                    (matrix.m[1][0] + matrix.m[0][1]) * mult,
                    biggestValue,
                    (matrix.m[2][1] + matrix.m[1][2]) * mult);

            case 3: // Z
                return TQuaternion<Type>(
                    (matrix.m[1][0] - matrix.m[0][1]) * mult,
                    (matrix.m[0][2] + matrix.m[2][0]) * mult,
                    (matrix.m[2][1] + matrix.m[1][2]) * mult,
                    biggestValue);

            default:
                NES_ASSERT(false);
                return TQuaternion<Type>(1, 0, 0, 0);
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Converts the orientation defined by the Matrix as a Quaternion.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TQuaternion<Type> ToQuat(const TMatrix4x4<Type>& matrix)
    {
        return ToQuat(ToMat3<Type>(matrix));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Make an Orientation Matrix from a set of Euler Angles. The Angles are expected to
    ///             be in radians, and in the form: x=pitch, y=yaw, z=roll.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeOrientationFromEuler(const TVector3<Type>& eulerAngles)
    {
        const Type cosPitch = std::cos(eulerAngles.x);
        const Type sinPitch = std::sin(eulerAngles.x);
        const Type cosYaw = std::cos(eulerAngles.y);
        const Type sinYaw = std::sin(eulerAngles.y);
        const Type cosRoll = std::cos(eulerAngles.z);
        const Type sinRoll = std::sin(eulerAngles.z);

        TMatrix4x4<Type> result = TMatrix4x4<Type>::Identity();
        result.m[0][0] = (cosYaw * cosRoll) + (sinYaw * sinPitch * sinRoll);
        result.m[1][0] = (sinRoll * cosPitch);
        result.m[2][0] = -(sinYaw * cosRoll) + (cosYaw * sinPitch * sinRoll);

        result.m[0][1] = -(cosYaw * sinRoll) + (sinYaw * sinPitch * cosRoll);
        result.m[1][1] = (cosRoll * cosPitch);
        result.m[2][1] = (sinRoll * sinYaw) + (cosYaw * sinPitch * cosRoll);

        result.m[0][2] = (sinYaw * cosPitch);
        result.m[1][2] = -sinPitch;
        result.m[2][2] = (cosYaw * cosPitch);
        
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Make an Orientation Matrix from a set of Euler Angles. The Angles are expected to
    ///             be in radians.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeOrientationFromEuler(const Type pitch, const Type yaw, const Type roll)
    {
        return MakeOrientationFromEuler(TVector3<Type>(pitch, yaw, roll));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the unscaled orientation defined by the matrix.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TQuaternion<Type> ExtractOrientation(const TMatrix4x4<Type>& matrix)
    {
        TMatrix4x4<Type> copy(matrix);
        copy.RemoveScaling();
        return ToQuat(ToMat3<Type>(copy));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Make a Scale Matrix from a 3D Scale factor.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeScaleMatrix(const TVector3<Type>& scale)
    {
        TMatrix4x4<Type> matrix = TMatrix4x4<Type>::Identity();
        matrix.m[0][0] = scale.x;
        matrix.m[1][1] = scale.y;
        matrix.m[2][2] = scale.z;
        return matrix;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Make a Scale Matrix from a uniform scale value.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeScaleMatrix(const Type uniformScale)
    {
        TMatrix4x4<Type> matrix = TMatrix4x4<Type>::Identity();
        matrix.m[0][0] = uniformScale;
        matrix.m[1][1] = uniformScale;
        matrix.m[2][2] = uniformScale;
        return matrix;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      [Consider] glm's implementation accounts for skew and perspective. 
    //		
    ///		@brief : Decompose the Matrix into its discrete translation, orientation, scale, skew, and perspective
    ///             values.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void DecomposeMatrix(const TMatrix4x4<Type>& matrix, TVector3<Type>& translation, TQuaternion<Type>& orientation,
        TVector3<Type>& scale)
    {
        TMatrix4x4<Type> copy(matrix);
        scale = copy.ExtractScaling();
        orientation = ToQuat(ToMat3<Type>(copy));
        translation = copy.GetAxis(3);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Creates a 4x4 matrix containing the translation, orientation and scale values. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> ComposeTransformMatrix(const TVector3<Type>& translation, const TQuaternion<Type>& orientation,
        const TVector3<Type>& scale)
    {
        return MakeTranslationMatrix(translation) * ToMat4(orientation) * MakeScaleMatrix(scale);
    }
}
