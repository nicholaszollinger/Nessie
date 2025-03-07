﻿// TMatrix2x2.h
#pragma once
#include "../Axis.h"
#include "../Vector2.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : 2x2 Matrix. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TMatrix2x2
    {
        // Dimension of the Square Matrix.
        static constexpr size_t N = 2;
        
        Type m[N][N]
        {
            { 1, 0 },
            { 0, 1 }
        };

        /// Default constructor initializes to the Identity Matrix. 
        constexpr TMatrix2x2() = default;
        constexpr TMatrix2x2(const Type values[N * N]);

        constexpr bool operator==(const TMatrix2x2& other) const;
        constexpr bool operator!=(const TMatrix2x2& other) const { return !(*this == other); }
        TMatrix2x2 operator+(const TMatrix2x2& other) const;
        TMatrix2x2 operator-(const TMatrix2x2& other) const;
        TMatrix2x2 operator*(const TMatrix2x2& other) const;
        TMatrix2x2 operator*(const float scalar);
        TMatrix2x2& operator+=(const TMatrix2x2& other);
        TMatrix2x2& operator-=(const TMatrix2x2& other);
        TMatrix2x2& operator*=(const TMatrix2x2& other);
        TMatrix2x2& operator*=(const float scalar);

        bool TryInvert();
        bool TryGetInverse(TMatrix2x2& result) const;
        bool IsIdentity() const;
        TMatrix2x2& Transpose();
        TMatrix2x2 Transposed() const;
        constexpr TVector2<Type> GetAxis(const Axis axis) const;
        constexpr TVector2<Type> GetAxis(const int axis) const;
        constexpr TVector2<Type> GetColumn(const int column) const;
        constexpr TVector2<Type> GetRow(const int row) const;
        float Determinant() const;

        std::string ToString() const;

        TMatrix2x2& Concatenate(const TMatrix2x2& other);
        static TMatrix2x2 Concatenate(const TMatrix2x2& a, const TMatrix2x2& b);
        
        static constexpr TMatrix2x2 Zero();
        static constexpr TMatrix2x2 Identity() { return TMatrix2x2(); }
    };

    template <FloatingPointType Type>
    TVector2<Type> operator*(const TMatrix2x2<Type>& matrix, const TVector2<Type>& vector);
    
    template <FloatingPointType Type>
    TVector2<Type> operator*(const TVector2<Type>& vector, const TMatrix2x2<Type>& matrix);
}

namespace nes
{
    template <FloatingPointType Type>
    constexpr TMatrix2x2<Type>::TMatrix2x2(const Type values[N * N])
    {
        NES_ASSERT(values != nullptr);
        memcpy(&(m[0][0]), values, N * N * sizeof(Type));
    }

    template <FloatingPointType Type>
    constexpr bool TMatrix2x2<Type>::operator==(const TMatrix2x2& other) const
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
    constexpr TMatrix2x2<Type> TMatrix2x2<Type>::Zero()
    {
        TMatrix2x2 result{};
        memset(&(result.m[0][0]), 0, N * N * sizeof(Type));
        return result;
    }

    template <FloatingPointType Type>
    TMatrix2x2<Type> TMatrix2x2<Type>::operator+(const TMatrix2x2& other) const
    {
        TMatrix2x2 result(*this);
        
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
    TMatrix2x2<Type> TMatrix2x2<Type>::operator-(const TMatrix2x2& other) const
    {
        TMatrix2x2 result(*this);
        
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
    TMatrix2x2<Type> TMatrix2x2<Type>::operator*(const TMatrix2x2& other) const
    {
        TMatrix2x2 result;
        
        // 1st Row * 1-2 Columns
        result.m[0][0] = (m[0][0] * other.m[0][0]) + (m[0][1] * other.m[1][0]);
        result.m[0][1] = (m[0][0] * other.m[0][1]) + (m[0][1] * other.m[1][1]);

        // 2nd Row * 1-2 Columns
        result.m[1][0] = (m[1][0] * other.m[0][0] + (m[1][1] * other.m[1][0]));
        result.m[1][1] = (m[1][0] * other.m[0][1] + (m[1][1] * other.m[1][1]));
        
        return result;
    }

    template <FloatingPointType Type>
    TMatrix2x2<Type> TMatrix2x2<Type>::operator*(const float scalar)
    {
        TMatrix2x2 result(*this);
        
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
    TMatrix2x2<Type>& TMatrix2x2<Type>::operator+=(const TMatrix2x2& other)
    {
        *this = *this + other;
        return *this;
    }

    template <FloatingPointType Type>
    TMatrix2x2<Type>& TMatrix2x2<Type>::operator-=(const TMatrix2x2& other)
    {
        *this = *this - other;
        return *this;
    }

    template <FloatingPointType Type>
    TMatrix2x2<Type>& TMatrix2x2<Type>::operator*=(const TMatrix2x2& other)
    {
        *this = *this * other;
        return *this;
    }

    template <FloatingPointType Type>
    TMatrix2x2<Type>& TMatrix2x2<Type>::operator*=(const float scalar)
    {
        *this = *this + scalar;
        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Attempt to Invert this Matrix. If it is non-invertible, then this will return false and
    ///             the Matrix will remain unchanged.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool TMatrix2x2<Type>::TryInvert()
    {
        const TMatrix2x2 copy = *this;
        
        // The Inverse of a Matrix is on page 168 of my Math Textbook:
        const float determinant = Determinant();
        if (determinant == 0.f)
        {
            return false;
        }

        const float invDeterminant = 1.0f / determinant;
            
        // The inverse determinant * the adjugate
        // Nice video explaining the process: https://www.youtube.com/watch?v=01c12NaUQDw
        m[0][0] = invDeterminant * copy.m[1][1];
        m[1][1] = invDeterminant * copy.m[0][0];

        m[0][1] = invDeterminant * -copy.m[0][1];
        m[1][0] = invDeterminant * -copy.m[1][0];

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Attempt to get the inverse of this Matrix. If no inverse is possible, this will return
    ///              false and "result" will be equal to this Matrix.
    ///		@param result : The resulting inverse of the Matrix, or equal to the Matrix if no inverse is possible.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool TMatrix2x2<Type>::TryGetInverse(TMatrix2x2& result) const
    {
        result = *this;
        return result.TryInvert();
    }

    template <FloatingPointType Type>
    bool TMatrix2x2<Type>::IsIdentity() const
    {
        return m[0][0] == 1.0f && m[0][1] == 0.0f
            && m[1][0] == 0.0f && m[1][1] == 1.0f;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Transpose this Matrix. If you want to preserve this Matrix, used Transposed(); 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix2x2<Type>& TMatrix2x2<Type>::Transpose()
    {
        *this = Transposed();
        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return the transposed Matrix. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix2x2<Type> TMatrix2x2<Type>::Transposed() const
    {
        TMatrix2x2 result;

        for (size_t i = 0; i < N; ++i)
        {
            for (size_t j = 0; j < N; ++j)
            {
                result.m[i][j] = m[j][i];
            }
        }
        
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns an axis of this matrix with scaling included. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TMatrix2x2<Type>::GetAxis(const Axis axis) const
    {
        switch (axis)
        {
            case Axis::X: return TVector2<Type>(m[0][0], m[1][0]);
            case Axis::Y: return TVector2<Type>(m[0][1], m[1][1]);
            
            default:
                NES_ASSERTV(false, "Invalid Axis request!");
                return TVector3<Type>::GetZeroVector();
        }
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns an axis of this matrix with scaling included. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TMatrix2x2<Type>::GetAxis(const int axis) const
    {
        NES_ASSERT(axis >= 0 && axis < N);
        return TVector2<Type>(m[0][axis], m[1][axis]);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a column of this matrix as a vector.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TMatrix2x2<Type>::GetColumn(const int column) const
    {
        NES_ASSERT(column >= 0 && column < N);
        return TVector2<Type>(m[0][column], m[1][column]);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a row of this matrix as a vector.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TMatrix2x2<Type>::GetRow(const int row) const
    {
        NES_ASSERT(row >= 0 && row < N);
        return TVector2<Type>(m[row][0], m[row][1]);
    }

    template <FloatingPointType Type>
    float TMatrix2x2<Type>::Determinant() const
    {
        // Page 162 of "3D Math Primer for Graphics and Game Development".
        // Page 27 of "Real-Time Collision Detection".
        // Difference of the products of the two diagonals
        return (m[0][0] * m[1][1]) - (m[0][1] * m[1][0]);
    }

    template <FloatingPointType Type>
    std::string TMatrix2x2<Type>::ToString() const
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

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Sets this matrix to the result of applying this matrix, and then the "other". This
    ///         returns a reference to the combined matrix, so they can be stringed together.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix2x2<Type>& TMatrix2x2<Type>::Concatenate(const TMatrix2x2& other)
    {
        *this = TMatrix2x2::Concatenate(*this, other);
        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Apply the matrix "a", then the matrix "b".
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix2x2<Type> TMatrix2x2<Type>::Concatenate(const TMatrix2x2& a, const TMatrix2x2& b)
    {
        return b * a;
    }

    template <FloatingPointType Type>
    TVector2<Type> operator*(const TMatrix2x2<Type>& matrix, const TVector2<Type>& vector)
    {
        TVector2<Type> result;
        // Column Orientation
        result.x = matrix.m[0][0] * vector.x + matrix.m[0][1] * vector.y;
        result.y = matrix.m[1][0] * vector.x + matrix.m[1][1] * vector.y;

        // Row Orientation
        //result.x = matrix.m[0][0] * vector.x + matrix.m[1][0] * vector.y;
        //result.y = matrix.m[0][1] * vector.x + matrix.m[1][1] * vector.y;
        return result;
    }

    template <FloatingPointType Type>
    TVector2<Type> operator*(const TVector2<Type>& vector, const TMatrix2x2<Type>& matrix)
    {
        return matrix * vector;
    }
}
