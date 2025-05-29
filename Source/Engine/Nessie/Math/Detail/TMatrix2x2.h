// TMatrix2x2.h
#pragma once
#include "../EAxis.h"
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

        using ColumnType = TVector2<Type>;
        using RowType    = TVector2<Type>;

    private:
        ColumnType m_columns[N]{};

    public:
        /// Default constructor initializes to zero matrix. 
        constexpr TMatrix2x2() = default;
        constexpr TMatrix2x2(const Type diagonalValue);
        constexpr TMatrix2x2(const Type& x0, const Type& y0,
                            const Type& x1, const Type& y1);
        constexpr TMatrix2x2(const ColumnType& c0, const ColumnType& c1);

        constexpr bool operator==(const TMatrix2x2& other) const;
        constexpr bool operator!=(const TMatrix2x2& other) const { return !(*this == other); }
        TMatrix2x2 operator+(const TMatrix2x2& other) const;
        TMatrix2x2 operator-(const TMatrix2x2& other) const;
        TMatrix2x2 operator*(const TMatrix2x2& other) const;
        TMatrix2x2 operator*(const Type scalar) const;
        TMatrix2x2& operator+=(const TMatrix2x2& other);
        TMatrix2x2& operator-=(const TMatrix2x2& other);
        TMatrix2x2& operator*=(const TMatrix2x2& other);
        TMatrix2x2& operator*=(const Type scalar);
        ColumnType& operator[](size_t index);
        const ColumnType& operator[](size_t index) const;

        bool TryInvert();
        bool TryGetInverse(TMatrix2x2& result) const;
        bool IsIdentity() const;
        TMatrix2x2& Transpose();
        TMatrix2x2 Transposed() const;
        constexpr TVector2<Type> GetAxis(const EAxis axis) const;
        constexpr TVector2<Type> GetAxis(const int axis) const;
        constexpr TVector2<Type> GetColumn(const int column) const;
        constexpr TVector2<Type> GetRow(const int row) const;
        float Determinant() const;

        std::string ToString() const;

        TMatrix2x2& Concatenate(const TMatrix2x2& other);
        static TMatrix2x2 Concatenate(const TMatrix2x2& a, const TMatrix2x2& b);
        
        static constexpr TMatrix2x2 Zero() { return TMatrix2x2(static_cast<Type>(0)); }
        static constexpr TMatrix2x2 Identity() { return TMatrix2x2(static_cast<Type>(1)); }
    };

    template <FloatingPointType Type>
    TVector2<Type> operator*(const TMatrix2x2<Type>& matrix, const TVector2<Type>& vector);
    
    template <FloatingPointType Type>
    TVector2<Type> operator*(const TVector2<Type>& vector, const TMatrix2x2<Type>& matrix);
}

namespace nes
{
    template <FloatingPointType Type>
    constexpr TMatrix2x2<Type>::TMatrix2x2(const Type diagonalValue)
    {
        m_columns[0][0] = diagonalValue;
        m_columns[1][1] = diagonalValue;
    }

    template <FloatingPointType Type>
    constexpr TMatrix2x2<Type>::TMatrix2x2(const Type& x0, const Type& y0, const Type& x1, const Type& y1)
        : m_columns
        {
            ColumnType(x0, y0),
            ColumnType(x1, y1)
        }
    {
        //
    }

    template <FloatingPointType Type>
    constexpr TMatrix2x2<Type>::TMatrix2x2(const ColumnType& c0, const ColumnType& c1)
        : m_columns{ c0, c1 }
    {
        //
    }

    template <FloatingPointType Type>
    constexpr bool TMatrix2x2<Type>::operator==(const TMatrix2x2& other) const
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (this->m_columns[i] != other[i])
                return false;
        }

        return true;
    }

    template <FloatingPointType Type>
    TMatrix2x2<Type> TMatrix2x2<Type>::operator+(const TMatrix2x2& other) const
    {
        TMatrix2x2 result(*this);
        result[0] += other[0];
        result[1] += other[1];
        return result;
    }

    template <FloatingPointType Type>
    TMatrix2x2<Type> TMatrix2x2<Type>::operator-(const TMatrix2x2& other) const
    {
        TMatrix2x2 result(*this);
        
        for (size_t i = 0; i < N; ++i)
        {
            result.m_columns[i] -= other[i];
        }
        
        return result;
    }

    template <FloatingPointType Type>
    TMatrix2x2<Type> TMatrix2x2<Type>::operator*(const TMatrix2x2& other) const
    {
        const ColumnType a0 = m_columns[0];
        const ColumnType a1 = m_columns[1];

        const ColumnType b0 = other[0];
        const ColumnType b1 = other[1];
        
        TMatrix2x2 result;
        result[0] = (a0 * b0[0]) + (a1 * b0[1]);
        result[1] = (a0 * b1[0]) + (a1 * b1[1]);
        return result;
    }

    template <FloatingPointType Type>
    TMatrix2x2<Type> TMatrix2x2<Type>::operator*(const Type scalar) const
    {
        TMatrix2x2 result(*this);
        result[0] *= scalar;
        result[1] *= scalar;
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
    TMatrix2x2<Type>& TMatrix2x2<Type>::operator*=(const Type scalar)
    {
        *this = *this + scalar;
        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a column of the matrix. The index must less than 2. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    typename TMatrix2x2<Type>::ColumnType& TMatrix2x2<Type>::operator[](const size_t index)
    {
        NES_ASSERT(index < N);
        return m_columns[index];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a column of the matrix. The index must less than 2. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    const typename TMatrix2x2<Type>::ColumnType& TMatrix2x2<Type>::operator[](const size_t index) const
    {
        NES_ASSERT(index < N);
        return m_columns[index];
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
        this->m_columns[0][0] = invDeterminant * copy[1][1];
        this->m_columns[1][1] = invDeterminant * copy[0][0];

        this->m_columns[1][0] = invDeterminant * -copy[1][0];
        this->m_columns[0][1] = invDeterminant * -copy[0][1];

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
        return *this == TMatrix2x2<Type>::Identity();
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
                result[i][j] = this->m_columns[j][i];
            }
        }
        
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns an axis of this matrix with scaling included. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TMatrix2x2<Type>::GetAxis(const EAxis axis) const
    {
        switch (axis)
        {
            case EAxis::X: return m_columns[0];
            case EAxis::Y: return m_columns[1];
            
            default:
                NES_ASSERT(false, "Invalid Axis request!");
                return TVector2<Type>::Zero();
        }
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns an axis of this matrix with scaling included. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TMatrix2x2<Type>::GetAxis(const int axis) const
    {
        NES_ASSERT(axis >= 0 && axis < static_cast<int>(N));
        return m_columns[axis];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a column of this matrix as a vector.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TMatrix2x2<Type>::GetColumn(const int column) const
    {
        NES_ASSERT(column >= 0 && column < static_cast<int>(N));
        return m_columns[column];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a row of this matrix as a vector.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector2<Type> TMatrix2x2<Type>::GetRow(const int row) const
    {
        NES_ASSERT(row >= 0 && row < static_cast<int>(N));
        return TVector2<Type>(this->m_columns[0][row], this->m_columns[1][row]);
    }

    template <FloatingPointType Type>
    float TMatrix2x2<Type>::Determinant() const
    {
        // Page 162 of "3D Math Primer for Graphics and Game Development".
        // Page 27 of "Real-Time Collision Detection".
        // Difference of the products of the two diagonals
        return (this->m_columns[0][0] * this->m_columns[1][1]) - (this->m_columns[1][0] * this->m_columns[0][1]);
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
                result += std::to_string(this->m_columns[j][i]) + " ";
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
        result[0] = matrix[0][0] * vector[0] + matrix[1][0] * vector[1];
        result[1] = matrix[0][1] * vector[0] + matrix[1][1] * vector[1];
        return result;
    }

    template <FloatingPointType Type>
    TVector2<Type> operator*(const TVector2<Type>& vector, const TMatrix2x2<Type>& matrix)
    {
        return matrix * vector;
    }
}
