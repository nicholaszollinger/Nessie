// TMatrix4x4.h
#pragma once
#include "TMatrix3x3.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //      NOTES:
    //      Add TransformPoint(), TransformVector(), GetAxis(), GetRow/Column()
    //      when you make the Vector4 class.
    //
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

        TMatrix4x4& Concatenate(const TMatrix4x4& other);
        static TMatrix4x4 Concatenate(const TMatrix4x4& a, const TMatrix4x4& b);
        
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
        // This was written by following along from pbrt, then following along from the pdf linked below.
        
        // Via: https://github.com/google/ion/blob/master/ion/math/matrixutils.cc,
        // (c) Google, Apache license.

        // For 4x4 do not compute the adjugate as the transpose of the cofactor
        // matrix, because this results in extra work. Several calculations can be
        // shared across the sub-determinants.
        //
        // *This approach is explained in David Eberly's Geometric Tools book,
        // excerpted here:
        //   http://www.geometrictools.com/Documentation/LaplaceExpansionTheorem.pdf
        
        float s0 = math::DifferenceOfProducts(m[0][0], m[1][1], m[1][0], m[0][1]);
        float s1 = math::DifferenceOfProducts(m[0][0], m[1][2], m[1][0], m[0][2]);
        float s2 = math::DifferenceOfProducts(m[0][0], m[1][3], m[1][0], m[0][3]);

        float s3 = math::DifferenceOfProducts(m[0][1], m[1][2], m[1][1], m[0][2]);
        float s4 = math::DifferenceOfProducts(m[0][1], m[1][3], m[1][1], m[0][3]);
        float s5 = math::DifferenceOfProducts(m[0][2], m[1][3], m[1][2], m[0][3]);

        float c0 = math::DifferenceOfProducts(m[2][0], m[3][1], m[3][0], m[2][1]);
        float c1 = math::DifferenceOfProducts(m[2][0], m[3][2], m[3][0], m[2][2]);
        float c2 = math::DifferenceOfProducts(m[2][0], m[3][3], m[3][0], m[2][3]);
        
        float c3 = math::DifferenceOfProducts(m[2][1], m[3][2], m[3][1], m[2][2]);
        float c4 = math::DifferenceOfProducts(m[2][1], m[3][3], m[3][1], m[2][3]);
        float c5 = math::DifferenceOfProducts(m[2][2], m[3][3], m[3][2], m[2][3]);

        const float determinant = (s0 * c5) - (s1 * c4) + (s2 * c3) + (s3 * c2) + (s5 * c0) - (s4 * c1);
        if (math::CheckEqualFloats(determinant, 0.f))
            return false;
        
        const TMatrix4x4 copy = *this;
        float inverseDeterminant = 1.f / determinant;

        m[0][0] = inverseDeterminant * ((copy.m[1][1] * c5)  + (copy.m[1][3] * c3) + (-copy.m[1][2] * c4));
        m[0][1] = inverseDeterminant * ((-copy.m[0][1] * c5) + (copy.m[0][2] * c4) + (-copy.m[0][3] * c3));
        m[0][2] = inverseDeterminant * ((copy.m[3][1] * s5)  + (copy.m[3][3] * s3) + (-copy.m[3][2] * s4));
        m[0][3] = inverseDeterminant * ((-copy.m[2][1] * s5) + (copy.m[2][2] * s4) + (-copy.m[2][3] * s3));

        m[1][0] = inverseDeterminant * ((-copy.m[1][0] * c5) + (copy.m[1][2] * c2) + (-copy.m[1][3] * c1));
        m[1][1] = inverseDeterminant * ((copy.m[0][0] * c5)  + (copy.m[0][3] * c1) + (-copy.m[0][2] * c2));
        m[1][2] = inverseDeterminant * ((-copy.m[3][0] * s5) + (copy.m[3][2] * s2) + (-copy.m[3][3] * s1));
        m[1][3] = inverseDeterminant * ((copy.m[2][0] * s5)  + (copy.m[2][3] * s1) + (-copy.m[2][2] * s2));

        m[2][0] = inverseDeterminant * ((copy.m[1][0] * c4)  + (copy.m[1][3] * c0) + (-copy.m[1][1] * c2));
        m[2][1] = inverseDeterminant * ((-copy.m[0][0] * c4) + (copy.m[0][1] * c2) + (-copy.m[0][3] * c0));
        m[2][2] = inverseDeterminant * ((copy.m[3][0] * s4)  + (copy.m[3][3] * s0) + (-copy.m[3][1] * s2));
        m[2][3] = inverseDeterminant * ((-copy.m[2][0] * s4) + (copy.m[2][1] * s2) + (-copy.m[2][3] * s0));

        m[3][0] = inverseDeterminant * ((-copy.m[1][0] * c3) + (copy.m[1][1] * c1) + (-copy.m[1][2] * c0));
        m[3][1] = inverseDeterminant * ((copy.m[0][0] * c3)  + (copy.m[0][2] * c0) + (-copy.m[0][1] * c1));
        m[3][2] = inverseDeterminant * ((-copy.m[3][0] * s3) + (copy.m[3][1] * s1) + (-copy.m[3][2] * s0));
        m[3][3] = inverseDeterminant * ((copy.m[2][0] * s3)  + (copy.m[2][2] * s0) + (-copy.m[2][1] * s1));

        return true;
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
        // This implementation is from pbrt.
        
        // Resources:
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

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Sets this matrix to the result of applying this matrix, and then the "other". This
    ///         returns a reference to the combined matrix, so they can be stringed together.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type>& TMatrix4x4<Type>::Concatenate(const TMatrix4x4& other)
    {
        *this = TMatrix4x4::Concatenate(*this, other);
        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Apply the matrix "a", then the matrix "b".
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> TMatrix4x4<Type>::Concatenate(const TMatrix4x4& a, const TMatrix4x4& b)
    {
        return b * a;
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
