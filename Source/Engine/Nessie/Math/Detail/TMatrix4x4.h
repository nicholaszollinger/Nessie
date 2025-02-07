// TMatrix4x4.h
#pragma once
#include "TMatrix3x3.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : 4x4 Matrix. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TMatrix4x4
    {
        // Dimension of the Square Matrix.
        static constexpr size_t N = 4;
        
        Type m[N][N]
        {
            { 1, 0, 0, 0 },
            { 0, 1, 0, 0 },
            { 0, 0, 1, 0 },
            { 0, 0, 0, 1 },
        };

        /// Default constructor initializes to the Identity Matrix. 
        constexpr TMatrix4x4() = default;
        constexpr TMatrix4x4(const Type values[N * N]);

        constexpr bool operator==(const TMatrix4x4& other) const;
        constexpr bool operator!=(const TMatrix4x4& other) const { return !(*this == other); }
        TMatrix4x4 operator+(const TMatrix4x4& other) const;
        TMatrix4x4 operator-(const TMatrix4x4& other) const;
        TMatrix4x4 operator*(const TMatrix4x4& other) const;
        TMatrix4x4 operator*(const float scalar);
        TMatrix4x4& operator+=(const TMatrix4x4& other);
        TMatrix4x4& operator-=(const TMatrix4x4& other);
        TMatrix4x4& operator*=(const TMatrix4x4& other);
        TMatrix4x4& operator*=(const float scalar);

        bool TryInvert();
        bool TryGetInverse(TMatrix4x4& result) const;
        bool IsIdentity() const;
        TMatrix4x4& Transpose();
        TMatrix4x4 Transposed() const;
        float Determinant() const;
        
        std::string ToString() const;
        
        static constexpr TMatrix4x4 Zero();
        static constexpr TMatrix4x4 Identity() { return TMatrix4x4(); }
    };
}

namespace nes
{
    template <FloatingPointType Type>
    constexpr TMatrix4x4<Type>::TMatrix4x4(const Type values[N * N])
    {
        NES_ASSERT(values != nullptr);
        memcpy(&(m[0][0]), values, N * N * sizeof(Type));
    }

    template <FloatingPointType Type>
    constexpr bool TMatrix4x4<Type>::operator==(const TMatrix4x4& other) const
    {
        for (size_t i = 0; i < N; ++i)
        {
            for (size_t j = 0; j < N; ++j)
            {
                if (m[i][j] != other.m[i][j])
                    return false;
            }
        }

        return true;
    }

    template <FloatingPointType Type>
    constexpr TMatrix4x4<Type> TMatrix4x4<Type>::Zero()
    {
        TMatrix4x4 result{};
        memset(&(result.m[0][0]), 0, N * N * sizeof(Type));
        return result;
    }

    template <FloatingPointType Type>
    TMatrix4x4<Type> TMatrix4x4<Type>::operator+(const TMatrix4x4& other) const
    {
        TMatrix4x4 result(*this);
        
        for (size_t i = 0; i < N; ++i)
        {
            for (size_t j = 0; j < N; ++j)
            {
                result.m[i][j] += other.m[i][j];
            }
        }
        
        return result;
    }

    template <FloatingPointType Type>
    TMatrix4x4<Type> TMatrix4x4<Type>::operator-(const TMatrix4x4& other) const
    {
        TMatrix4x4 result(*this);
        
        for (size_t i = 0; i < N; ++i)
        {
            for (size_t j = 0; j < N; ++j)
            {
                result.m[i][j] -= other.m[i][j];
            }
        }
        
        return result;
    }

    template <FloatingPointType Type>
    TMatrix4x4<Type> TMatrix4x4<Type>::operator*(const TMatrix4x4& other) const
    {
        TMatrix4x4 result(*this);
        
        // 1st Row * 1-4 Columns
        result.m[0][0] = (m[0][0] * other.m[0][0]) + (m[0][1] * other.m[1][0]) + (m[0][2] * other.m[2][0]) + (m[0][3] * other.m[3][0]);
        result.m[0][1] = (m[0][0] * other.m[0][1]) + (m[0][1] * other.m[1][1]) + (m[0][2] * other.m[2][1]) + (m[0][3] * other.m[3][1]);
        result.m[0][2] = (m[0][0] * other.m[0][2]) + (m[0][1] * other.m[1][2]) + (m[0][2] * other.m[2][2]) + (m[0][3] * other.m[3][2]);
        result.m[0][3] = (m[0][0] * other.m[0][3]) + (m[0][1] * other.m[1][3]) + (m[0][2] * other.m[2][3]) + (m[0][3] * other.m[3][3]);

        // 2nd Row * 1-4 Columns
        result.m[1][0] = (m[1][0] * other.m[0][0]) + (m[1][1] * other.m[1][0]) + (m[1][2] * other.m[2][0]) + (m[1][3] * other.m[3][0]);
        result.m[1][1] = (m[1][0] * other.m[0][1]) + (m[1][1] * other.m[1][1]) + (m[1][2] * other.m[2][1]) + (m[1][3] * other.m[3][1]);
        result.m[1][2] = (m[1][0] * other.m[0][2]) + (m[1][1] * other.m[1][2]) + (m[1][2] * other.m[2][2]) + (m[1][3] * other.m[3][2]);
        result.m[1][3] = (m[1][0] * other.m[0][3]) + (m[1][1] * other.m[1][3]) + (m[1][2] * other.m[2][3]) + (m[1][3] * other.m[3][3]);

        // 3rd Row * 1-4 Columns
        result.m[2][0] = (m[2][0] * other.m[0][0]) + (m[2][1] * other.m[1][0]) + (m[2][2] * other.m[2][0]) + (m[2][3] * other.m[3][0]);
        result.m[2][1] = (m[2][0] * other.m[0][1]) + (m[2][1] * other.m[1][1]) + (m[2][2] * other.m[2][1]) + (m[2][3] * other.m[3][1]);
        result.m[2][2] = (m[2][0] * other.m[0][2]) + (m[2][1] * other.m[1][2]) + (m[2][2] * other.m[2][2]) + (m[2][3] * other.m[3][2]);
        result.m[2][3] = (m[2][0] * other.m[0][3]) + (m[2][1] * other.m[1][3]) + (m[2][2] * other.m[2][3]) + (m[2][3] * other.m[3][3]);

        // 4th Row * 1-4 Columns
        result.m[3][0] = (m[2][0] * other.m[0][0]) + (m[2][1] * other.m[1][0]) + (m[2][2] * other.m[2][0]) + (m[3][3] * other.m[3][0]);
        result.m[3][1] = (m[2][0] * other.m[0][1]) + (m[2][1] * other.m[1][1]) + (m[2][2] * other.m[2][1]) + (m[3][3] * other.m[3][1]);
        result.m[3][2] = (m[2][0] * other.m[0][2]) + (m[2][1] * other.m[1][2]) + (m[2][2] * other.m[2][2]) + (m[3][3] * other.m[3][2]);
        result.m[3][3] = (m[2][0] * other.m[0][3]) + (m[2][1] * other.m[1][3]) + (m[2][2] * other.m[2][3]) + (m[3][3] * other.m[3][3]);
        
        return result;
    }

    template <FloatingPointType Type>
    TMatrix4x4<Type> TMatrix4x4<Type>::operator*(const float scalar)
    {
        TMatrix4x4 result(*this);
        
        for (size_t i = 0; i < N; ++i)
        {
            for (size_t j = 0; j < N; ++j)
            {
                result.m[i][j] *= scalar;
            }
        }
        
        return result;
    }

    template <FloatingPointType Type>
    TMatrix4x4<Type>& TMatrix4x4<Type>::operator+=(const TMatrix4x4& other)
    {
        *this = *this + other;
        return *this;
    }

    template <FloatingPointType Type>
    TMatrix4x4<Type>& TMatrix4x4<Type>::operator-=(const TMatrix4x4& other)
    {
        *this = *this - other;
        return *this;
    }

    template <FloatingPointType Type>
    TMatrix4x4<Type>& TMatrix4x4<Type>::operator*=(const TMatrix4x4& other)
    {
        *this = *this * other;
        return *this;
    }

    template <FloatingPointType Type>
    TMatrix4x4<Type>& TMatrix4x4<Type>::operator*=(const float scalar)
    {
        *this = *this + scalar;
        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Attempt to Invert this Matrix. If it is non-invertible, then this will return false and
    ///             the Matrix will remain unchanged.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool TMatrix4x4<Type>::TryInvert()
    {
        static_assert(false, "Invert for 4x4 not working yet...");
        
        const TMatrix4x4 copy = *this;

        // [TODO]: THIS IS BROKEN.
        // Calculate the Determinant of the 4 2x2 matrices.
        // I am looking at Unreal's implementation, because in my research, it seems that a lot of implementations try to avoid
        // calculating part of the determinant more than once. Honestly, it is a bit above my head, but I need to get things in
        // to move on for now.
        // File: UnrealMath.cpp, Line: 834
        // float determinants[4]{};
        // float tmp[4][4];
        //
        // tmp[0][0] = copy.m[2][2] * copy.m[3][3] - copy.m[3][2] * copy.m[2][3];
        // tmp[0][1] = copy.m[2][1] * copy.m[3][3] - copy.m[3][1] * copy.m[2][3];
        // tmp[0][2] = copy.m[2][1] * copy.m[3][2] - copy.m[3][1] * copy.m[2][2];
        //
        // tmp[1][0] = copy.m[2][2] * copy.m[3][3] - copy.m[3][2] * copy.m[2][3];
        // tmp[1][1] = copy.m[2][0] * copy.m[3][3] - copy.m[3][0] * copy.m[2][3];
        // tmp[1][2] = copy.m[2][0] * copy.m[3][2] - copy.m[3][0] * copy.m[2][2];
        //
        // tmp[2][0] = copy.m[2][1] * copy.m[3][3] - copy.m[3][1] * copy.m[2][3];
        // tmp[2][1] = copy.m[2][0] * copy.m[3][3] - copy.m[3][0] * copy.m[2][3];
        // tmp[2][2] = copy.m[2][0] * copy.m[3][1] - copy.m[3][0] * copy.m[2][1];
        //
        // tmp[3][0] = copy.m[2][1] * copy.m[3][2] - copy.m[3][1] * copy.m[2][2];
        // tmp[3][1] = copy.m[2][0] * copy.m[3][2] - copy.m[3][0] * copy.m[2][2];
        // tmp[3][2] = copy.m[2][0] * copy.m[3][1] - copy.m[3][0] * copy.m[2][1];
        //
        // determinants[0] = copy.m[1][1] * tmp[0][0] - copy.m[1][2] * tmp[1][0] + copy.m[1][3] * tmp[2][0];
        // determinants[1] = copy.m[1][0] * tmp[0][1] - copy.m[1][2] * tmp[1][1] + copy.m[1][3] * tmp[2][1];
        // determinants[2] = copy.m[1][0] * tmp[0][2] - copy.m[1][1] * tmp[1][2] + copy.m[1][3] * tmp[2][2];
        // determinants[3] = copy.m[1][0] * tmp[0][3] - copy.m[1][1] * tmp[1][3] + copy.m[1][2] * tmp[2][3];
        //
        // const float determinant = copy.m[0][0] * determinants[0] - copy.m[0][1] * determinants[1] + copy.m[0][2] * determinants[2] + copy.m[0][3] * determinants[3];
        // if (determinant == 0.f)
        // {
        //     return false;
        // }
        //
        // const float invDeterminant = 1.f / determinant;
        // m[0][0] =  invDeterminant * determinants[0];
        // m[0][1] = -invDeterminant * determinants[1];
        // m[0][2] =  invDeterminant * determinants[2];
        // m[0][3] = -invDeterminant * determinants[3];
        //
        // //
        // m[1][0] = -invDeterminant * (copy.m[0][1] * tmp[0][0] - copy.m[0][2] * tmp[1][0] + copy.m[0][3] * tmp[2][0]);
        // m[1][1] =  invDeterminant * (copy.m[0][0] * tmp[0][1] - copy.m[0][2] * tmp[1][1] + copy.m[0][3] * tmp[2][1]);
        // m[1][2] = -invDeterminant * (copy.m[0][0] * tmp[0][2] - copy.m[0][2] * tmp[1][2] + copy.m[0][3] * tmp[2][2]);
        // m[1][3] =  invDeterminant * (copy.m[0][0] * tmp[0][3] - copy.m[0][2] * tmp[1][3] + copy.m[0][3] * tmp[2][3]);
        //
        // m[2][0] =  invDeterminant * (
        //     copy.m[0][1] * (copy.m[1][2] * copy.m[3][3] - copy.m[3][2] * copy.m[1][3]) -
        //     copy.m[0][2] * (copy.m[1][1] * copy.m[3][3] - copy.m[3][1] * copy.m[1][3]) +
        //     copy.m[0][3] * (copy.m[1][1] * copy.m[3][2] - copy.m[3][1] * copy.m[1][2])
        //     );
        // m[2][1] = -invDeterminant * (
        //     copy.m[0][0] * (copy.m[1][2] * copy.m[3][3] - copy.m[3][1] * copy.m[1][3]) -
        //     copy.m[0][2] * (copy.m[1][0] * copy.m[3][3] - copy.m[3][0] * copy.m[1][3]) +
        //     copy.m[0][3] * (copy.m[1][0] * copy.m[3][2] - copy.m[3][0] * copy.m[1][2])
        //     );
        // m[2][2] =  invDeterminant * (
        //     copy.m[0][0] * (copy.m[1][1] * copy.m[3][3] - copy.m[3][1] * copy.m[1][3]) -
        //     copy.m[0][1] * (copy.m[1][0] * copy.m[3][3] - copy.m[3][0] * copy.m[1][3]) +
        //     copy.m[0][3] * (copy.m[1][0] * copy.m[3][1] - copy.m[3][0] * copy.m[1][1])
        //     );
        // m[2][3] = -invDeterminant * (
        //     copy.m[0][0] * (copy.m[1][1] * copy.m[3][2] - copy.m[3][1] * copy.m[1][2]) -
        //     copy.m[0][1] * (copy.m[1][0] * copy.m[3][2] - copy.m[3][0] * copy.m[1][2]) +
        //     copy.m[0][2] * (copy.m[1][0] * copy.m[3][1] - copy.m[3][0] * copy.m[1][1])
        //     );
        //
        // m[3][0] = -invDeterminant * (
        //     copy.m[0][0] * (copy.m[1][2] * copy.m[2][3] - copy.m[2][2] * copy.m[1][3]) -
        //     copy.m[0][1] * (copy.m[1][1] * copy.m[2][3] - copy.m[2][1] * copy.m[1][3]) +
        //     copy.m[0][3] * (copy.m[1][1] * copy.m[2][2] - copy.m[2][1] * copy.m[1][2])
        //     );
        // m[3][1] =  invDeterminant * (
        //     copy.m[0][0] * (copy.m[1][2] * copy.m[2][3] - copy.m[2][2] * copy.m[1][3]) -
        //     copy.m[0][2] * (copy.m[1][0] * copy.m[2][3] - copy.m[2][0] * copy.m[1][3]) +
        //     copy.m[0][3] * (copy.m[1][0] * copy.m[2][2] - copy.m[2][0] * copy.m[1][2])
        //     );
        // m[3][2] = -invDeterminant * (
        //     copy.m[0][0] * (copy.m[1][1] * copy.m[2][3] - copy.m[2][1] * copy.m[1][3]) -
        //     copy.m[0][1] * (copy.m[1][0] * copy.m[2][3] - copy.m[2][0] * copy.m[1][3]) +
        //     copy.m[0][3] * (copy.m[1][0] * copy.m[2][1] - copy.m[2][0] * copy.m[1][1])
        //     );
        // m[3][3] =  invDeterminant * (
        //     copy.m[0][0] * (copy.m[1][1] * copy.m[2][2] - copy.m[2][1] * copy.m[1][2]) -
        //     copy.m[0][1] * (copy.m[1][0] * copy.m[2][2] - copy.m[2][0] * copy.m[1][2]) +
        //     copy.m[0][2] * (copy.m[1][0] * copy.m[2][1] - copy.m[2][0] * copy.m[1][1])
        //     );
        //
        // return true;

        return false;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Attempt to get the inverse of this Matrix. If no inverse is possible, this will return
    ///              false and "result" will be equal to this Matrix.
    ///		@param result : The resulting inverse of the Matrix, or equal to the Matrix if no inverse is possible.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool TMatrix4x4<Type>::TryGetInverse(TMatrix4x4& result) const
    {
        result = *this;
        return result.TryInvert();
    }

    template <FloatingPointType Type>
    bool TMatrix4x4<Type>::IsIdentity() const
    {
        return m[0][0] == 1.f && m[0][1] == 0.f && m[0][2] == 0.f && m[0][3] == 0.f
            && m[1][0] == 0.f && m[1][1] == 1.f && m[1][2] == 0.f && m[1][3] == 0.f
            && m[2][0] == 0.f && m[2][1] == 0.f && m[2][2] == 1.f && m[2][3] == 0.f
            && m[3][0] == 0.f && m[3][1] == 0.f && m[3][2] == 0.f && m[3][3] == 1.f;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Transpose this Matrix. If you want to preserve this Matrix, used Transposed(); 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type>& TMatrix4x4<Type>::Transpose()
    {
        *this = Transposed();
        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return the transposed Matrix. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> TMatrix4x4<Type>::Transposed() const
    {
        TMatrix4x4 result;

        for (size_t i = 0; i < N; ++i)
        {
            for (size_t j = 0; j < N; ++j)
            {
                result.m[i][j] = m[j][i];
            }
        }
        
        return result;
    }

    template <FloatingPointType Type>
    float TMatrix4x4<Type>::Determinant() const
    {
        // Page 162 of "3D Math Primer for Graphics and Game Development".
        // Page 27 of "Real-Time Collision Detection".
        const float s0 = math::DifferenceOfProducts(m[0][0], m[1][1], m[1][0], m[0][1]);
        const float s1 = math::DifferenceOfProducts(m[0][0], m[1][2], m[1][0], m[0][2]);
        const float s2 = math::DifferenceOfProducts(m[0][0], m[1][3], m[1][0], m[0][3]);

        const float s3 = math::DifferenceOfProducts(m[0][1], m[1][2], m[1][1], m[0][2]);
        const float s4 = math::DifferenceOfProducts(m[0][1], m[1][3], m[1][1], m[0][3]);
        const float s5 = math::DifferenceOfProducts(m[0][2], m[1][3], m[1][2], m[0][3]);

        const float c0 = math::DifferenceOfProducts(m[2][0], m[3][1], m[3][0], m[2][1]);
        const float c1 = math::DifferenceOfProducts(m[2][0], m[3][2], m[3][0], m[2][2]);
        const float c2 = math::DifferenceOfProducts(m[2][0], m[3][3], m[3][0], m[2][3]);

        const float c3 = math::DifferenceOfProducts(m[2][1], m[3][2], m[3][1], m[2][2]);
        const float c4 = math::DifferenceOfProducts(m[2][1], m[3][3], m[3][1], m[2][3]);
        const float c5 = math::DifferenceOfProducts(m[2][2], m[3][3], m[3][2], m[2][3]);
            
        return (math::DifferenceOfProducts(s0, c5, s1, c4)
            + math::DifferenceOfProducts(s2, c3, -s3, c2)
            + math::DifferenceOfProducts(s5, c0, s4, c1));
    }

    template <FloatingPointType Type>
    std::string TMatrix4x4<Type>::ToString() const
    {
        std::string result;
        result.reserve(N * N);

        for (size_t i = 0; i < N; ++i)
        {
            for (size_t j = 0; j < N; ++j)
            {
                result += std::to_string(m[i][j]) + " ";
            }
            
            result += "\n";
        }

        return result;
    }
}
