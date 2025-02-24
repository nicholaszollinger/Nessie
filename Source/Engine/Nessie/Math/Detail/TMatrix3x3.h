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
        using RowType = TVector3<Type>;
        
        Type m[N][N]
        {
            { 1, 0, 0 },
            { 0, 1, 0 },
            { 0, 0, 1 }
        };

        /// Default constructor initializes to the Identity Matrix. 
        constexpr TMatrix3x3() = default;
        constexpr TMatrix3x3(const Type values[N * N]);

        constexpr bool operator==(const TMatrix3x3& other) const;
        constexpr bool operator!=(const TMatrix3x3& other) const { return !(*this == other); }
        TMatrix3x3 operator+(const TMatrix3x3& other) const;
        TMatrix3x3 operator-(const TMatrix3x3& other) const;
        TMatrix3x3 operator*(const TMatrix3x3& other) const;
        TMatrix3x3 operator*(const float scalar);
        TMatrix3x3& operator+=(const TMatrix3x3& other);
        TMatrix3x3& operator-=(const TMatrix3x3& other);
        TMatrix3x3& operator*=(const TMatrix3x3& other);
        TMatrix3x3& operator*=(const float scalar);

        bool TryInvert();
        bool TryGetInverse(TMatrix3x3& result) const;
        bool IsIdentity() const;
        TMatrix3x3& Transpose();
        TMatrix3x3 Transposed() const;
        float Determinant() const;
        constexpr TVector3<Type> GetAxis(const Axis axis) const;
        constexpr TVector3<Type> GetAxis(const int index) const;
        
        TVector2<Type> TransformPoint(const TVector2<Type>& point) const;
        TVector2<Type> TransformVector(const TVector2<Type>& vector) const;
        
        std::string ToString() const;

        TMatrix3x3& Concatenate(const TMatrix3x3& other);
        static TMatrix3x3 Concatenate(const TMatrix3x3& a, const TMatrix3x3& b);
        
        static constexpr TMatrix3x3 Zero();
        static constexpr TMatrix3x3 Identity() { return {}; }
    };

    template <FloatingPointType Type>
    TVector3<Type> operator*(const TMatrix3x3<Type>& matrix, const TVector3<Type>& vector);
    
    template <FloatingPointType Type>
    TVector3<Type> operator*(const TVector3<Type>& vector, const TMatrix3x3<Type>& matrix);
}

namespace nes
{
    template <FloatingPointType Type>
    constexpr TMatrix3x3<Type>::TMatrix3x3(const Type values[N * N])
    {
        NES_ASSERT(values != nullptr);
        memcpy(&(m[0][0]), values, N * N * sizeof(Type));
    }

    template <FloatingPointType Type>
    constexpr bool TMatrix3x3<Type>::operator==(const TMatrix3x3& other) const
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

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Returns a Matrix with all values set to 0. 
    ///		@returns : 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TMatrix3x3<Type> TMatrix3x3<Type>::Zero()
    {
        TMatrix3x3 result{};
        memset(&(result.m[0][0]), 0, N * N * sizeof(Type));
        return result;
    }

    template <FloatingPointType Type>
    TMatrix3x3<Type> TMatrix3x3<Type>::operator+(const TMatrix3x3& other) const
    {
        TMatrix3x3 result(*this);
        
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
    TMatrix3x3<Type> TMatrix3x3<Type>::operator-(const TMatrix3x3& other) const
    {
        TMatrix3x3 result(*this);
        
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
    TMatrix3x3<Type> TMatrix3x3<Type>::operator*(const TMatrix3x3& other) const
    {
        TMatrix3x3 result;
        
        // 1st Row * 1-3 Columns
        result.m[0][0] = (m[0][0] * other.m[0][0]) + (m[0][1] * other.m[1][0]) + (m[0][2] * other.m[2][0]);
        result.m[0][1] = (m[0][0] * other.m[0][1]) + (m[0][1] * other.m[1][1]) + (m[0][2] * other.m[2][1]);
        result.m[0][2] = (m[0][0] * other.m[0][2]) + (m[0][1] * other.m[1][2]) + (m[0][2] * other.m[2][2]);
        
        // 2nd Row * 1-3 Columns
        result.m[1][0] = (m[1][0] * other.m[0][0]) + (m[1][1] * other.m[1][0]) + (m[1][2] * other.m[2][0]);
        result.m[1][1] = (m[1][0] * other.m[0][1]) + (m[1][1] * other.m[1][1]) + (m[1][2] * other.m[2][1]);
        result.m[1][2] = (m[1][0] * other.m[0][2]) + (m[1][1] * other.m[1][2]) + (m[1][2] * other.m[2][2]);

        // 3rd Row * 1-3 Columns
        result.m[2][0] = (m[2][0] * other.m[0][0]) + (m[2][1] * other.m[1][0]) + (m[2][2] * other.m[2][0]);
        result.m[2][1] = (m[2][0] * other.m[0][1]) + (m[2][1] * other.m[1][1]) + (m[2][2] * other.m[2][1]);
        result.m[2][2] = (m[2][0] * other.m[0][2]) + (m[2][1] * other.m[1][2]) + (m[2][2] * other.m[2][2]);
        
        return result;
    }

    template <FloatingPointType Type>
    TMatrix3x3<Type> TMatrix3x3<Type>::operator*(const float scalar)
    {
        TMatrix3x3 result(*this);
        
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
    TMatrix3x3<Type>& TMatrix3x3<Type>::operator*=(const float scalar)
    {
        *this = *this + scalar;
        return *this;
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
            
        m[0][0] = invDeterminant * math::DifferenceOfProducts(copy.m[1][1], copy.m[2][2], copy.m[1][2], copy.m[2][1]);
        m[1][0] = invDeterminant * math::DifferenceOfProducts(copy.m[1][2], copy.m[2][0], copy.m[1][0], copy.m[2][2]);
        m[2][0] = invDeterminant * math::DifferenceOfProducts(copy.m[1][0], copy.m[2][1], copy.m[1][1], copy.m[2][0]);
            
        m[0][1] = invDeterminant * math::DifferenceOfProducts(copy.m[0][2], copy.m[2][1], copy.m[0][1], copy.m[2][2]);
        m[1][1] = invDeterminant * math::DifferenceOfProducts(copy.m[0][0], copy.m[2][2], copy.m[0][2], copy.m[2][0]);
        m[2][1] = invDeterminant * math::DifferenceOfProducts(copy.m[0][1], copy.m[2][0], copy.m[0][0], copy.m[2][1]);
            
        m[0][2] = invDeterminant * math::DifferenceOfProducts(copy.m[0][1], copy.m[1][2], copy.m[0][2], copy.m[1][1]);
        m[1][2] = invDeterminant * math::DifferenceOfProducts(copy.m[0][2], copy.m[1][0], copy.m[0][0], copy.m[1][2]);
        m[2][2] = invDeterminant * math::DifferenceOfProducts(copy.m[0][0], copy.m[1][1], copy.m[0][1], copy.m[1][0]);

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
        return m[0][0] == 1.f && m[0][1] == 0.f && m[0][2] == 0.f
            && m[1][0] == 0.f && m[1][1] == 1.f && m[1][2] == 0.f
            && m[2][0] == 0.f && m[2][1] == 0.f && m[2][2] == 1.f;
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
                result.m[i][j] = m[j][i];
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
        determinant += (m[0][0] * m[1][1] * m[2][2]);
        determinant += (m[0][1] * m[1][2] * m[2][0]);
        determinant += (m[0][2] * m[1][0] * m[2][1]);

        determinant -= (m[0][0] * m[1][2] * m[2][1]);
        determinant -= (m[0][1] * m[1][0] * m[2][2]);
        determinant -= (m[0][2] * m[1][1] * m[2][0]);

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
            case Axis::X: return TVector3<Type>(m[0][0], m[0][1], m[0][2]);
            case Axis::Y: return TVector3<Type>(m[1][0], m[1][1], m[1][2]);
            case Axis::Z: return TVector3<Type>(m[2][0], m[2][1], m[2][2]);
            
            default:
                NES_ASSERTV(false, "Invalid Axis request!");
                return TVector3<Type>::GetZeroVector();
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns an axis of this matrix with scaling included. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TMatrix3x3<Type>::GetAxis(const int index) const
    {
        NES_ASSERT(index >= 0 && index < 3);
        return TVector3<Type>(m[index][0], m[index][1], m[index][2]);
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
        // Column Orientation:
        result[0] = (matrix.m[0][0] * vector[0]) + (matrix.m[0][1] * vector[1]) + (matrix.m[0][2] * vector[2]);
        result[1] = (matrix.m[1][0] * vector[0]) + (matrix.m[1][1] * vector[1]) + (matrix.m[1][2] * vector[2]);
        result[2] = (matrix.m[2][0] * vector[0]) + (matrix.m[2][1] * vector[1]) + (matrix.m[2][2] * vector[2]);

        // Row Orientation:
        //result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0];
        //result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1];
        //result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2];
        return result;
    }

    template <FloatingPointType Type>
    TVector3<Type> operator*(const TVector3<Type>& vector, const TMatrix3x3<Type>& matrix)
    {
        return matrix * vector;
    }
}