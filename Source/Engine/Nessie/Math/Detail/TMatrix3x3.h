// TMatrix3x3.h
#pragma once
#include "TMatrix2x2.h"
#include "../Vector3.h"

namespace nes
{
    namespace math
    {
        //----------------------------------------------------------------------------------------------------
        //      [TODO]: I should probably move this to a more generic location... I will probably tackle this
        //      when I have a central location for more performant Vector operations.
        //
        ///		@brief : Returns the difference of (a1 * a2) and (b1 * b2).
        ///		@tparam Type : Scalar Type of the values.
        //----------------------------------------------------------------------------------------------------
        template <ScalarType Type = float>
        constexpr Type DifferenceOfProducts(const Type a1, const Type a2, const Type b1, const Type b2)
        {
            return a1 * a2 - b1 * b2;
        }
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : 3x3 Matrix. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    struct TMatrix3x3
    {
        // Dimension of the Square Matrix.
        static constexpr size_t N = 3;
        using ColumnType = TVector3<Type>;
        using RowType    = TVector3<Type>;

    private:
        ColumnType m_columns[N]{};

    public:
        /// Default constructor initializes to the Zero Matrix. 
        constexpr TMatrix3x3() = default;
        constexpr TMatrix3x3(const Type& diagonalValue);
        constexpr TMatrix3x3(const Type& x0, const Type& y0, const Type& z0,
                            const Type& x1, const Type& y1, const Type& z1,
                            const Type& x2, const Type& y2, const Type& z2);
        constexpr TMatrix3x3(const ColumnType& c0, const ColumnType& c1, const ColumnType& c2);

        constexpr bool operator==(const TMatrix3x3& other) const;
        constexpr bool operator!=(const TMatrix3x3& other) const { return !(*this == other); }
        TMatrix3x3 operator+(const TMatrix3x3& other) const;
        TMatrix3x3 operator-(const TMatrix3x3& other) const;
        TMatrix3x3 operator*(const TMatrix3x3& other) const;
        TMatrix3x3 operator*(const Type scalar) const;
        TMatrix3x3& operator+=(const TMatrix3x3& other);
        TMatrix3x3& operator-=(const TMatrix3x3& other);
        TMatrix3x3& operator*=(const TMatrix3x3& other);
        TMatrix3x3& operator*=(const Type scalar);

        ColumnType& operator[](const size_t index);
        const ColumnType& operator[](const size_t index) const;

        bool TryInvert();
        bool TryGetInverse(TMatrix3x3& result) const;
        bool IsIdentity() const;
        TMatrix3x3& Transpose();
        TMatrix3x3 Transposed() const;
        float Determinant() const;
        constexpr TVector3<Type> GetAxis(const Axis axis) const;
        constexpr TVector3<Type> GetAxis(const int axis) const;
        constexpr TVector3<Type> GetColumn(const int column) const;
        constexpr TVector3<Type> GetRow(const int row) const;
        
        TVector2<Type> TransformPoint(const TVector2<Type>& point) const;
        TVector2<Type> TransformVector(const TVector2<Type>& vector) const;
        
        std::string ToString() const;

        TMatrix3x3& Concatenate(const TMatrix3x3& other);
        static TMatrix3x3 Concatenate(const TMatrix3x3& a, const TMatrix3x3& b);
        
        static constexpr TMatrix3x3 Zero() { return {}; }
        static constexpr TMatrix3x3 Identity() { return TMatrix3x3(static_cast<Type>(1)); }
    };

    template <FloatingPointType Type>
    TVector3<Type> operator*(const TMatrix3x3<Type>& matrix, const TVector3<Type>& vector);
    
    template <FloatingPointType Type>
    TVector3<Type> operator*(const TVector3<Type>& vector, const TMatrix3x3<Type>& matrix);
}

namespace nes
{
    template <FloatingPointType Type>
    constexpr TMatrix3x3<Type>::TMatrix3x3(const Type& diagonalValue)
    {
        m_columns[0][0] = diagonalValue;
        m_columns[1][1] = diagonalValue;
        m_columns[2][2] = diagonalValue;
    }

    template <FloatingPointType Type>
    constexpr TMatrix3x3<Type>::TMatrix3x3(const Type& x0, const Type& y0, const Type& z0, const Type& x1,
        const Type& y1, const Type& z1, const Type& x2, const Type& y2, const Type& z2)
        : m_columns
        {
            ColumnType(x0, y0, z0),
            ColumnType(x1, y1, z1),
            ColumnType(x2, y2, z2)
        }
    {
        //
    }

    template <FloatingPointType Type>
    constexpr TMatrix3x3<Type>::TMatrix3x3(const ColumnType& c0, const ColumnType& c1,
        const ColumnType& c2)
        : m_columns{ c0, c1, c2 }
    {
        //
    }

    template <FloatingPointType Type>
    constexpr bool TMatrix3x3<Type>::operator==(const TMatrix3x3& other) const
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (this->m_columns[i] != other[i])
                return false;
        }

        return true;
    }
    
    template <FloatingPointType Type>
    TMatrix3x3<Type> TMatrix3x3<Type>::operator+(const TMatrix3x3& other) const
    {
        TMatrix3x3 result(*this);
        result[0] += other[0];
        result[1] += other[1];
        result[2] += other[2];
        return result;
    }

    template <FloatingPointType Type>
    TMatrix3x3<Type> TMatrix3x3<Type>::operator-(const TMatrix3x3& other) const
    {
        TMatrix3x3 result(*this);
        result[0] -= other[0];
        result[1] -= other[1];
        result[2] -= other[2];
        return result;
    }

    template <FloatingPointType Type>
    TMatrix3x3<Type> TMatrix3x3<Type>::operator*(const TMatrix3x3& other) const
    {
        const ColumnType a0 = m_columns[0];
        const ColumnType a1 = m_columns[1];
        const ColumnType a2 = m_columns[2];

        const ColumnType b0 = other[0];
        const ColumnType b1 = other[1];
        const ColumnType b2 = other[2];
        
        TMatrix3x3 result;
        result[0] = (a0 * b0[0]) + (a1 * b0[1]) + (a2 * b0[2]);
        result[1] = (a0 * b1[0]) + (a1 * b1[1]) + (a2 * b1[2]);
        result[2] = (a0 * b2[0]) + (a1 * b2[1]) + (a2 * b2[2]);
        return result;
    }

    template <FloatingPointType Type>
    TMatrix3x3<Type> TMatrix3x3<Type>::operator*(const Type scalar) const
    {
        TMatrix3x3 result(*this);
        result[0] *= scalar;
        result[1] *= scalar;
        result[2] *= scalar;
        return result;
    }

    template <FloatingPointType Type>
    TMatrix3x3<Type>& TMatrix3x3<Type>::operator+=(const TMatrix3x3& other)
    {
        *this = *this + other;
        return *this;
    }

    template <FloatingPointType Type>
    TMatrix3x3<Type>& TMatrix3x3<Type>::operator-=(const TMatrix3x3& other)
    {
        *this = *this - other;
        return *this;
    }

    template <FloatingPointType Type>
    TMatrix3x3<Type>& TMatrix3x3<Type>::operator*=(const TMatrix3x3& other)
    {
        *this = *this * other;
        return *this;
    }

    template <FloatingPointType Type>
    TMatrix3x3<Type>& TMatrix3x3<Type>::operator*=(const Type scalar)
    {
        *this = *this + scalar;
        return *this;
    }

    template <FloatingPointType Type>
    typename TMatrix3x3<Type>::ColumnType& TMatrix3x3<Type>::operator[](const size_t index)
    {
        NES_ASSERT(index < N);
        return m_columns[index];
    }

    template <FloatingPointType Type>
    const typename TMatrix3x3<Type>::ColumnType& TMatrix3x3<Type>::operator[](const size_t index) const
    {
        NES_ASSERT(index < N);
        return m_columns[index];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Attempt to Invert this Matrix. If it is non-invertible, then this will return false and
    ///             the Matrix will remain unchanged.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool TMatrix3x3<Type>::TryInvert()
    {
        const TMatrix3x3 copy = *this;
        
        // The Inverse of a Matrix is on page 168 of my Math Textbook:
        const float determinant = Determinant();
        if (determinant == 0.f)
        {
            return false;
        }

        const float invDeterminant = 1.0f / determinant;
            
        this->m_columns[0][0] = invDeterminant * math::DifferenceOfProducts(copy[1][1], copy[2][2], copy[2][1], copy[1][2]);
        this->m_columns[0][1] = invDeterminant * math::DifferenceOfProducts(copy[2][1], copy[0][2], copy[0][1], copy[2][2]);
        this->m_columns[0][2] = invDeterminant * math::DifferenceOfProducts(copy[0][1], copy[1][2], copy[1][1], copy[0][2]);
            
        this->m_columns[1][0] = invDeterminant * math::DifferenceOfProducts(copy[2][0], copy[1][2], copy[1][0], copy[2][2]);
        this->m_columns[1][1] = invDeterminant * math::DifferenceOfProducts(copy[0][0], copy[2][2], copy[2][0], copy[0][2]);
        this->m_columns[1][2] = invDeterminant * math::DifferenceOfProducts(copy[1][0], copy[0][2], copy[0][0], copy[1][2]);
            
        this->m_columns[2][0] = invDeterminant * math::DifferenceOfProducts(copy[1][0], copy[2][1], copy[2][0], copy[1][1]);
        this->m_columns[2][1] = invDeterminant * math::DifferenceOfProducts(copy[2][0], copy[0][1], copy[0][0], copy[2][1]);
        this->m_columns[2][2] = invDeterminant * math::DifferenceOfProducts(copy[0][0], copy[1][1], copy[1][0], copy[0][1]);

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Attempt to get the inverse of this Matrix. If no inverse is possible, this will return
    ///              false and "result" will be equal to this Matrix.
    ///		@param result : The resulting inverse of the Matrix, or equal to the Matrix if no inverse is possible.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool TMatrix3x3<Type>::TryGetInverse(TMatrix3x3& result) const
    {
        result = *this;
        return result.TryInvert();
    }

    template <FloatingPointType Type>
    bool TMatrix3x3<Type>::IsIdentity() const
    {
        return *this == Identity(); 
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Transpose this Matrix. If you want to preserve this Matrix, used Transposed(); 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix3x3<Type>& TMatrix3x3<Type>::Transpose()
    {
        *this = Transposed();
        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return the transposed Matrix. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix3x3<Type> TMatrix3x3<Type>::Transposed() const
    {
        TMatrix3x3 result;

        for (size_t i = 0; i < N; ++i)
        {
            for (size_t j = 0; j < N; ++j)
            {
                result[i][j] = this->m_columns[j][i];
            }
        }
        
        return result;
    }

    template <FloatingPointType Type>
    float TMatrix3x3<Type>::Determinant() const
    {
        // Page 162 of "3D Math Primer for Graphics and Game Development".
        // Page 27 of "Real-Time Collision Detection".
        float determinant = 0.0f;

        // Difference of the products of the 3 forward and 3 backward diagonals
        // An image can be found in the book that shows it clearly.
        determinant += (this->m_columns[0][0] * this->m_columns[1][1] * this->m_columns[2][2]);
        determinant += (this->m_columns[1][0] * this->m_columns[2][1] * this->m_columns[0][2]);
        determinant += (this->m_columns[2][0] * this->m_columns[0][1] * this->m_columns[1][2]);

        determinant -= (this->m_columns[0][0] * this->m_columns[2][1] * this->m_columns[1][2]);
        determinant -= (this->m_columns[1][0] * this->m_columns[0][1] * this->m_columns[2][2]);
        determinant -= (this->m_columns[2][0] * this->m_columns[1][1] * this->m_columns[0][2]);

        return determinant;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns an axis of this matrix with scaling included. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TMatrix3x3<Type>::GetAxis(const Axis axis) const
    {
        switch (axis)
        {
            case Axis::X: return m_columns[0];
            case Axis::Y: return m_columns[1];
            case Axis::Z: return m_columns[2];
            
            default:
                NES_ASSERTV(false, "Invalid Axis request!");
                return TVector3<Type>::GetZeroVector();
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns an axis of this matrix with scaling included. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TMatrix3x3<Type>::GetAxis(const int axis) const
    {
        NES_ASSERT(axis >= 0 && axis < static_cast<int>(N));
        return m_columns[axis];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a column of this matrix as a vector.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TMatrix3x3<Type>::GetColumn(const int column) const
    {
        NES_ASSERT(column >= 0 && column < static_cast<int>(N));
        return m_columns[column];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a row of this matrix as a vector.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TMatrix3x3<Type>::GetRow(const int row) const
    {
        NES_ASSERT(row >= 0 && row < static_cast<int>(N));
        return RowType(
            this->m_columns[0][row]
            , this->m_columns[1][row]
            , this->m_columns[2][row]);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Transform a 2D point by this Matrix. This will include the Translation defined by this
    ///              matrix.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector2<Type> TMatrix3x3<Type>::TransformPoint(const TVector2<Type>& point) const
    {
        return (*this * TVector3<Type>(point.x, point.y, 1.f)).GetXY();
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Transform a 2D vector by this Matrix. This will NOT include the translation defined by
    ///             this matrix.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector2<Type> TMatrix3x3<Type>::TransformVector(const TVector2<Type>& vector) const
    {
        return (*this * TVector3<Type>(vector.x, vector.y, 0.f)).GetXY();
    }

    template <FloatingPointType Type>
    std::string TMatrix3x3<Type>::ToString() const
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
    TMatrix3x3<Type>& TMatrix3x3<Type>::Concatenate(const TMatrix3x3& other)
    {
        *this = TMatrix3x3::Concatenate(*this, other);
        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Apply the matrix "a", then the matrix "b".
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix3x3<Type> TMatrix3x3<Type>::Concatenate(const TMatrix3x3& a, const TMatrix3x3& b)
    {
        return b * a;
    }

    template <FloatingPointType Type>
    TVector3<Type> operator*(const TMatrix3x3<Type>& matrix, const TVector3<Type>& vector)
    {
        TVector3<Type> result;
        result[0] = matrix[0][0] * vector[0] + matrix[1][0] * vector[1] + matrix[2][0] * vector[2];
        result[1] = matrix[0][1] * vector[0] + matrix[1][1] * vector[1] + matrix[2][1] * vector[2];
        result[2] = matrix[0][2] * vector[0] + matrix[1][2] * vector[1] + matrix[2][2] * vector[2];
        return result;
    }

    template <FloatingPointType Type>
    TVector3<Type> operator*(const TVector3<Type>& vector, const TMatrix3x3<Type>& matrix)
    {
        return matrix * vector;
    }
}