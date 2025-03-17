// Transform2.h
#pragma once
#include "Matrix.h"
#include "Quaternion.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //      [TODO]: More constructors, including a Matrix as a parameter. This will require me to implement
    //              a way to extract Scale and Rotation from a transform matrix.
    //              
    ///		@brief : A 2D Transform holds a position, scale and rotation. It can be converted to a 3x3 matrix.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TTransform2
    {
        TVector2<Type> m_location{};
        TVector2<Type> m_scale = Vector2::GetUnitVector();
        Type m_rotation = {}; // Angle, in Radians.
        
        constexpr TTransform2() = default;
        constexpr TTransform2(const TVector2<Type>& position, const TVector2<Type>& scale, Type rotation);
        
        [[nodiscard]] TMatrix3x3<Type> ToMatrix() const;
    };
    
    using Transform2D = TTransform2<NES_MATH_DEFAULT_REAL_TYPE>;
}

namespace nes
{
    template <FloatingPointType Type>
    constexpr TTransform2<Type>::TTransform2(const TVector2<Type>& position, const TVector2<Type>& scale, Type rotation)
        : m_location(position)
        , m_scale(scale)
        , m_rotation(rotation)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    //      [TODO]: This was written with my basic understanding of how to mathematically combine the matrix
    //              into the Homogenous 3D space for 2D coordinates. There are definitely more optimized ways
    //              to calculate this.
    //
    ///		@brief : Creates the Matrix representation of the Transform. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix3x3<Type> TTransform2<Type>::ToMatrix() const
    {
        const float sin = std::sin(m_rotation);
        const float cos = std::cos(m_rotation);
        
        TMatrix2x2<Type> scaleMatrix{};
        scaleMatrix[0][0] = m_scale.x;
        scaleMatrix[1][1] = m_scale.y;
        
        TMatrix2x2<Type> rotationMatrix{};
        rotationMatrix[0][0] = cos;
        rotationMatrix[1][0] = -sin;
        rotationMatrix[0][1] = sin;
        rotationMatrix[1][1] = cos;
        const TMatrix2x2<Type> scaleRotation = TMatrix2x2<Type>::Concatenate(scaleMatrix, rotationMatrix);

        // Z Axis rotation:
        TMatrix3x3<Type> result{};
        result[0][0] = scaleRotation[0][0];
        result[0][1] = scaleRotation[0][1];
        result[1][0] = scaleRotation[1][0];
        result[1][1] = scaleRotation[1][1];

        // Apply Translation
        result[2][0] = m_location.x;
        result[2][1] = m_location.y;

        return result;
    }
}

