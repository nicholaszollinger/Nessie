// MatrixTransform.h
#pragma once
#include "MatrixConversions.h"

namespace nes::math
{
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeTranslationMatrix4(const TVector3<Type>& translation);

    template <FloatingPointType Type>
    TMatrix3x3<Type> MakeRotationMatrix3(const TQuaternion<Type>& orientation);
    
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeRotationMatrix4(const TQuaternion<Type>& orientation);

    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeRotationTranslationMatrix(const TVector3<Type>& translation, const TQuaternion<Type>& rotation);

    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeInverseRotationTranslationMatrix(const TVector3<Type>& translation, const TQuaternion<Type>& rotation);
    
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeScaleMatrix(const TVector3<Type>& scale);

    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeScaleMatrix(const Type uniformScale);

    template <FloatingPointType Type>
    TMatrix4x4<Type> LookAt(const TVector3<Type>& eyeLocation, const TVector3<Type>& targetLocation, const TVector3<Type>& upVector);
    
    template <FloatingPointType Type>
    void DecomposeMatrix(const TMatrix4x4<Type>& matrix, TVector3<Type>& translation, TQuaternion<Type>& orientation, TVector3<Type>& scale);

    template <FloatingPointType Type>
    void DecomposeMatrix(const TMatrix4x4<Type>& matrix, TVector3<Type>& translation, TRotation<Type>& rotation, TVector3<Type>& scale);

    template <FloatingPointType Type>
    TMatrix4x4<Type> ComposeTransformMatrix(const TVector3<Type>& translation, const TQuaternion<Type>& orientation, const TVector3<Type>& scale);

    template <FloatingPointType Type>
    TMatrix4x4<Type> ComposeTransformMatrix(const TVector3<Type>& translation, const TRotation<Type>& rotation, const TVector3<Type>& scale);
}

namespace nes::math
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a Translation Matrix from a 3D Translation. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeTranslationMatrix4(const TVector3<Type>& translation)
    {
        TMatrix4x4<Type> result = TMatrix4x4<Type>::Identity();
        result[3] = TVector4<Type>(translation, static_cast<Type>(1));
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create an Orientation Matrix from a quaternion.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix3x3<Type> MakeRotationMatrix3(const TQuaternion<Type>& orientation)
    {
        return ToMat3<Type>(orientation);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create an Orientation Matrix from a Quaternion Orientation. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeRotationMatrix4(const TQuaternion<Type>& orientation)
    {
        return ToMat4<Type>(orientation);
    }

    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeRotationTranslationMatrix(const TVector3<Type>& translation, const TQuaternion<Type>& rotation)
    {
        TMatrix4x4<Type> result = MakeRotationMatrix4(rotation);
        result[3] = TVector4<Type>(translation, static_cast<Type>(1));
        return result;
    }

    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeInverseRotationTranslationMatrix(const TVector3<Type>& translation, const TQuaternion<Type>& rotation)
    {
        TMatrix4x4<Type> result = MakeRotationMatrix4(rotation.Conjugate());
        result[3] = (-result.TransformVector(translation)); 
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Make a Scale Matrix from a 3D Scale factor.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeScaleMatrix(const TVector3<Type>& scale)
    {
        TMatrix4x4<Type> matrix = TMatrix4x4<Type>::Identity();
        matrix[0][0] = scale.x;
        matrix[1][1] = scale.y;
        matrix[2][2] = scale.z;
        return matrix;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Make a Scale Matrix from a uniform scale value.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeScaleMatrix(const Type uniformScale)
    {
        return TMatrix4x4<Type>(uniformScale);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Creates a left-handed View Matrix.
    ///		@param eyeLocation : Position of the Camera in World space.
    ///		@param targetLocation : Target position that the Camera is looking at.
    ///		@param upVector : Normalized up vector that determines the Camera orientation.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> LookAt(const TVector3<Type>& eyeLocation, const TVector3<Type>& targetLocation,
        const TVector3<Type>& upVector)
    {
        TMatrix4x4<Type> result = TMatrix4x4<Type>::Identity();
        
        const TVector3<Type> forward = (targetLocation - eyeLocation).Normalized();
        const TVector3<Type> right = (TVector3<Type>::Cross(upVector, forward)).Normalized();
        const TVector3<Type> up = TVector3<Type>::Cross(forward, right);

        result[0][0] = right.x;
        result[1][0] = right.y;
        result[2][0] = right.z;

        result[0][1] = up.x;
        result[1][1] = up.y;
        result[2][1] = up.z;

        result[0][2] = forward.x;
        result[1][2] = forward.y;
        result[2][2] = forward.z;

        result[3][0] = -TVector3<Type>::Dot(right, eyeLocation);
        result[3][1] = -TVector3<Type>::Dot(up, eyeLocation);
        result[3][2] = -TVector3<Type>::Dot(forward, eyeLocation);
        
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      [Consider] glm's implementation accounts for skew and perspective. 
    //		
    ///		@brief : Decompose the Matrix into its discrete translation, orientation, and scale values.
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
    //		NOTES:
    //      [Consider] glm's implementation accounts for skew and perspective. 
    //		
    ///		@brief : Decompose the Matrix into its discrete translation, rotation, and scale values.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void DecomposeMatrix(const TMatrix4x4<Type>& matrix, TVector3<Type>& translation, TRotation<Type>& rotation,
        TVector3<Type>& scale)
    {
        TMatrix4x4<Type> copy(matrix);
        scale = copy.ExtractScaling();
        rotation = ToRotation(ToMat3<Type>(copy));
        translation = copy.GetAxis(3);
    }

    template <FloatingPointType Type>
    TMatrix4x4<Type> ComposeTransformMatrix(const TVector3<Type>& translation, const TRotation<Type>& rotation,
        const TVector3<Type>& scale)
    {
        return MakeTranslationMatrix4(translation) * ToMat4(rotation) * MakeScaleMatrix(scale);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Creates a 4x4 matrix containing the translation, orientation and scale values. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> ComposeTransformMatrix(const TVector3<Type>& translation, const TQuaternion<Type>& orientation,
        const TVector3<Type>& scale)
    {
        return MakeTranslationMatrix4(translation) * ToMat4(orientation) * MakeScaleMatrix(scale);
    }
}