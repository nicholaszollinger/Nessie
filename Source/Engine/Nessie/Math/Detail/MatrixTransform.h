// MatrixTransform.h
#pragma once
#include "MatrixConversions.h"

namespace nes::math
{
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeTranslationMatrix(const TVector3<Type>& translation);

    template <FloatingPointType Type>
    TMatrix3x3<Type> MakeOrientationMatrix(const TQuaternion<Type>& orientation);
    
    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeOrientationMatrix(const TQuaternion<Type>& orientation);

    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeScaleMatrix(const TVector3<Type>& scale);

    template <FloatingPointType Type>
    TMatrix4x4<Type> MakeScaleMatrix(const Type uniformScale);

    template <FloatingPointType Type>
    TMatrix4x4<Type> LookAt(const TVector3<Type>& eyeLocation, const TVector3<Type>& targetLocation, const TVector3<Type>& upVector);

}

namespace nes::math
{
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


        result.m[0][0] = right.x;
        result.m[1][0] = right.y;
        result.m[2][0] = right.z;
        result.m[0][1] = up.x;
        result.m[1][1] = up.y;
        result.m[2][1] = up.z;
        result.m[2][0] = forward.x;
        result.m[2][1] = forward.y;
        result.m[2][2] = forward.z;
        result.m[0][3] = -TVector3<Type>::Dot(right, eyeLocation);
        result.m[1][3] = -TVector3<Type>::Dot(up, eyeLocation);
        result.m[2][3] = -TVector3<Type>::Dot(forward, eyeLocation);
        
        return result;
    }
}