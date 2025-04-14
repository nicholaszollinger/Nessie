// TMatrix4x4.h
#pragma once
#include "TMatrix3x3.h"
#include "Math/Quaternion.h"
#include "Math/Vector4.h"

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
        
        using ColumnType = TVector4<Type>;
        using RowType    = TVector4<Type>;

    private:
        ColumnType m_columns[N]{};
        
    public:
        /// Default constructor initializes to the Zero Matrix. 
        constexpr TMatrix4x4() = default;
        constexpr TMatrix4x4(const Type diagonalValue);
        constexpr TMatrix4x4(const Type& x0, const Type& y0, const Type& z0, const Type& w0,
                             const Type& x1, const Type& y1, const Type& z1, const Type& w1,
                             const Type& x2, const Type& y2, const Type& z2, const Type& w2,
                             const Type& x3, const Type& y3, const Type& z3, const Type& w3);
        constexpr TMatrix4x4(const ColumnType& c0, const ColumnType& c1, const ColumnType& c2, const ColumnType& c3);
        constexpr TMatrix4x4(const TMatrix3x3<Type>& mat3);

        constexpr bool operator==(const TMatrix4x4& other) const;
        constexpr bool operator!=(const TMatrix4x4& other) const { return !(*this == other); }
        TMatrix4x4 operator+(const TMatrix4x4& other) const;
        TMatrix4x4 operator-(const TMatrix4x4& other) const;
        TMatrix4x4 operator*(const TMatrix4x4& other) const;
        TMatrix4x4 operator*(const Type scalar) const;
        TMatrix4x4& operator+=(const TMatrix4x4& other);
        TMatrix4x4& operator-=(const TMatrix4x4& other);
        TMatrix4x4& operator*=(const TMatrix4x4& other);
        TMatrix4x4& operator*=(const Type scalar);
        
        TVector4<Type>& operator[](const size_t index);
        const TVector4<Type>& operator[](const size_t index) const;

        bool TryInvert();
        bool TryGetInverse(TMatrix4x4& result) const;
        bool IsIdentity() const;
        TMatrix4x4& Transpose();
        TMatrix4x4 Transposed() const;
        float Determinant() const;
        constexpr TVector3<Type> GetAxis(const Axis axis) const;
        constexpr TVector3<Type> GetAxis(const int axis) const;
        constexpr TVector4<Type> GetColumn(const int column) const;
        constexpr TVector4<Type> GetRow(const int row) const;

        void RemoveScaling();
        TVector3<Type> ExtractScaling();
        TVector3<Type> GetScale() const;
        TMatrix4x4 GetWithoutScale() const;

        TVector3<Type> TransformPoint(const TVector3<Type>& point) const;
        TVector3<Type> TransformVector(const TVector3<Type>& vector) const;

        TMatrix4x4& Concatenate(const TMatrix4x4& other);
        static TMatrix4x4 Concatenate(const TMatrix4x4& a, const TMatrix4x4& b);
        
        std::string ToString() const;
        
        static constexpr TMatrix4x4 Zero()      { return {}; }
        static constexpr TMatrix4x4 Identity()  { return TMatrix4x4(static_cast<Type>(1)); }
    };

    template <FloatingPointType Type>
    TVector4<Type> operator*(const TMatrix4x4<Type>& matrix, const TVector4<Type>& vector);
    
    template <FloatingPointType Type>
    TVector4<Type> operator*(const TVector4<Type>& vector, const TMatrix4x4<Type>& matrix);
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a 4x4 matrix from a 3x3 Matrix. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TMatrix4x4<Type>::TMatrix4x4(const TMatrix3x3<Type>& mat3)
        : m_columns
        {
            TVector4<Type>(mat3[0], static_cast<Type>(0)),
            TVector4<Type>(mat3[1], static_cast<Type>(0)),
            TVector4<Type>(mat3[2], static_cast<Type>(0)),
            TVector4<Type>(TVector3<Type>::Zero(), static_cast<Type>(1)),
        }
    {
        //
    }

    template <FloatingPointType Type>
    constexpr TMatrix4x4<Type>::TMatrix4x4(const Type diagonalValue)
    {
        m_columns[0][0] = diagonalValue;
        m_columns[1][1] = diagonalValue;
        m_columns[2][2] = diagonalValue;
        m_columns[3][3] = diagonalValue;
    }

    template <FloatingPointType Type>
    constexpr TMatrix4x4<Type>::TMatrix4x4(const Type& x0, const Type& y0, const Type& z0, const Type& w0,
        const Type& x1, const Type& y1, const Type& z1, const Type& w1, const Type& x2, const Type& y2, const Type& z2,
        const Type& w2, const Type& x3, const Type& y3, const Type& z3, const Type& w3)
        : m_columns
        {
            ColumnType(x0, y0, z0, w0),
            ColumnType(x1, y1, z1, w1),
            ColumnType(x2, y2, z2, w2),
            ColumnType(x3, y3, z3, w3)
        }
    {
        //
    }

    template <FloatingPointType Type>
    constexpr TMatrix4x4<Type>::TMatrix4x4(const TVector4<Type>& c0, const TVector4<Type>& c1, const TVector4<Type>& c2,
        const TVector4<Type>& c3)
        : m_columns{ c0, c1, c2, c3 }
    {
        //
    }

    template <FloatingPointType Type>
    constexpr bool TMatrix4x4<Type>::operator==(const TMatrix4x4& other) const
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (this->m_columns[i] != other[i])
                return false;
        }

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns an axis of this matrix with scaling included. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TMatrix4x4<Type>::GetAxis(const Axis axis) const
    {
        switch (axis)
        {
            case Axis::X: return TVector3<Type>(m_columns[0].x, m_columns[0].y, m_columns[0].z);
            case Axis::Y: return TVector3<Type>(m_columns[1].x, m_columns[1].y, m_columns[1].z);
            case Axis::Z: return TVector3<Type>(m_columns[2].x, m_columns[2].y, m_columns[2].z);
            case Axis::W: return TVector3<Type>(m_columns[3].x, m_columns[3].y, m_columns[3].z);
            
            default:
                NES_ASSERTV(false, "Invalid Axis request!");
                return TVector4<Type>::Zero();
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns an axis of this matrix with scaling included. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TMatrix4x4<Type>::GetAxis(const int axis) const
    {
        NES_ASSERT(axis >= 0 && axis < static_cast<int>(N));
        return TVector3<Type>(m_columns[axis].x, m_columns[axis].y, m_columns[axis].z);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a column of this matrix as a vector.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector4<Type> TMatrix4x4<Type>::GetColumn(const int column) const
    {
        NES_ASSERT(column >= 0 && column < static_cast<int>(N));
        return m_columns[column];
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a row of this matrix as a vector.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector4<Type> TMatrix4x4<Type>::GetRow(const int row) const
    {
        NES_ASSERT(row >= 0 && row < static_cast<int>(N));
        return RowType(
            this->m_columns[0][row]
            , this->m_columns[1][row]
            , this->m_columns[2][row]
            , this->m_columns[3][row]);
    }

    template <FloatingPointType Type>
    TMatrix4x4<Type> TMatrix4x4<Type>::operator+(const TMatrix4x4& other) const
    {
        TMatrix4x4 result(*this);
        result[0] += other[0];
        result[1] += other[1];
        result[2] += other[2];
        result[3] += other[3];
        return result;
    }

    template <FloatingPointType Type>
    TMatrix4x4<Type> TMatrix4x4<Type>::operator-(const TMatrix4x4& other) const
    {
        TMatrix4x4 result(*this);
        result[0] -= other[0];
        result[1] -= other[1];
        result[2] -= other[2];
        result[3] -= other[3];
        return result;
    }

    template <FloatingPointType Type>
    TMatrix4x4<Type> TMatrix4x4<Type>::operator*(const TMatrix4x4& other) const
    {
        const ColumnType a0 = m_columns[0];
        const ColumnType a1 = m_columns[1];
        const ColumnType a2 = m_columns[2];
        const ColumnType a3 = m_columns[3];

        const ColumnType b0 = other[0];
        const ColumnType b1 = other[1];
        const ColumnType b2 = other[2];
        const ColumnType b3 = other[3];
        
        TMatrix4x4 result;
        result[0] = (a0 * b0[0]) + (a1 * b0[1]) + (a2 * b0[2]) + (a3 * b0[3]);
        result[1] = (a0 * b1[0]) + (a1 * b1[1]) + (a2 * b1[2]) + (a3 * b1[3]);
        result[2] = (a0 * b2[0]) + (a1 * b2[1]) + (a2 * b2[2]) + (a3 * b2[3]);
        result[3] = (a0 * b3[0]) + (a1 * b3[1]) + (a2 * b3[2]) + (a3 * b3[3]);
        return result;
    }

    template <FloatingPointType Type>
    TMatrix4x4<Type> TMatrix4x4<Type>::operator*(const Type scalar) const
    {
        TMatrix4x4 result(*this);
        result[0] *= scalar;
        result[1] *= scalar;
        result[2] *= scalar;
        result[3] *= scalar;
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
    TMatrix4x4<Type>& TMatrix4x4<Type>::operator*=(const Type scalar)
    {
        *this = *this + scalar;
        return *this;
    }

    template <FloatingPointType Type>
    TVector4<Type>& TMatrix4x4<Type>::operator[](const size_t index)
    {
        NES_ASSERT(index < N);
        return m_columns[index];
    }

    template <FloatingPointType Type>
    const TVector4<Type>& TMatrix4x4<Type>::operator[](const size_t index) const
    {
        NES_ASSERT(index < N);
        return m_columns[index];
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
        
        float s0 = math::DifferenceOfProducts(m_columns[0][0], m_columns[1][1], m_columns[0][1], m_columns[1][0]);
        float s1 = math::DifferenceOfProducts(m_columns[0][0], m_columns[2][1], m_columns[0][1], m_columns[2][0]);
        float s2 = math::DifferenceOfProducts(m_columns[0][0], m_columns[3][1], m_columns[0][1], m_columns[3][0]);
        
        float s3 = math::DifferenceOfProducts(m_columns[1][0], m_columns[2][1], m_columns[1][1], m_columns[2][0]);
        float s4 = math::DifferenceOfProducts(m_columns[1][0], m_columns[3][1], m_columns[1][1], m_columns[3][0]);
        float s5 = math::DifferenceOfProducts(m_columns[2][0], m_columns[3][1], m_columns[2][1], m_columns[3][0]);
        
        float c0 = math::DifferenceOfProducts(m_columns[0][2], m_columns[1][3], m_columns[0][3], m_columns[1][2]);
        float c1 = math::DifferenceOfProducts(m_columns[0][2], m_columns[2][3], m_columns[0][3], m_columns[2][2]);
        float c2 = math::DifferenceOfProducts(m_columns[0][2], m_columns[3][3], m_columns[0][3], m_columns[3][2]);
        
        float c3 = math::DifferenceOfProducts(m_columns[1][2], m_columns[2][3], m_columns[1][3], m_columns[2][2]);
        float c4 = math::DifferenceOfProducts(m_columns[1][2], m_columns[3][3], m_columns[1][3], m_columns[3][2]);
        float c5 = math::DifferenceOfProducts(m_columns[2][2], m_columns[3][3], m_columns[2][3], m_columns[3][2]);
        
        const float determinant = (s0 * c5) - (s1 * c4) + (s2 * c3) + (s3 * c2) + (s5 * c0) - (s4 * c1);
        if (math::CheckEqualFloats(determinant, 0.f))
            return false;
        
        const TMatrix4x4 copy = *this;
        float inverseDeterminant = 1.f / determinant;
        
        m_columns[0][0] = inverseDeterminant * ((copy[1][1] * c5)  + (copy[3][1] * c3) + (-copy[2][1] * c4));
        m_columns[1][0] = inverseDeterminant * ((-copy[1][0] * c5) + (copy[2][0] * c4) + (-copy[3][0] * c3));
        m_columns[2][0] = inverseDeterminant * ((copy[1][3] * s5)  + (copy[3][3] * s3) + (-copy[2][3] * s4));
        m_columns[3][0] = inverseDeterminant * ((-copy[1][2] * s5) + (copy[2][2] * s4) + (-copy[3][2] * s3));
        
        m_columns[0][1] = inverseDeterminant * ((-copy[0][1] * c5) + (copy[2][1] * c2) + (-copy[3][1] * c1));
        m_columns[1][1] = inverseDeterminant * ((copy[0][0] * c5)  + (copy[3][0] * c1) + (-copy[2][0] * c2));
        m_columns[2][1] = inverseDeterminant * ((-copy[0][3] * s5) + (copy[2][3] * s2) + (-copy[3][3] * s1));
        m_columns[3][1] = inverseDeterminant * ((copy[0][2] * s5)  + (copy[3][2] * s1) + (-copy[2][2] * s2));
        
        m_columns[0][2] = inverseDeterminant * ((copy[0][1] * c4)  + (copy[3][1] * c0) + (-copy[1][1] * c2));
        m_columns[1][2] = inverseDeterminant * ((-copy[0][0] * c4) + (copy[1][0] * c2) + (-copy[3][0] * c0));
        m_columns[2][2] = inverseDeterminant * ((copy[0][3] * s4)  + (copy[3][3] * s0) + (-copy[1][3] * s2));
        m_columns[3][2] = inverseDeterminant * ((-copy[0][2] * s4) + (copy[1][2] * s2) + (-copy[3][2] * s0));
        
        m_columns[0][3] = inverseDeterminant * ((-copy[0][1] * c3) + (copy[1][1] * c1) + (-copy[2][1] * c0));
        m_columns[1][3] = inverseDeterminant * ((copy[0][0] * c3)  + (copy[2][0] * c0) + (-copy[1][0] * c1));
        m_columns[2][3] = inverseDeterminant * ((-copy[0][3] * s3) + (copy[1][3] * s1) + (-copy[2][3] * s0));
        m_columns[3][3] = inverseDeterminant * ((copy[0][2] * s3)  + (copy[2][2] * s0) + (-copy[1][2] * s1));

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
        return *this == Identity();
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
                result[i][j] = this->m_columns[j][i];
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
        const float s0 = math::DifferenceOfProducts(m_columns[0][0], m_columns[1][1], m_columns[0][1], m_columns[1][0]);
        const float s1 = math::DifferenceOfProducts(m_columns[0][0], m_columns[2][1], m_columns[0][1], m_columns[2][0]);
        const float s2 = math::DifferenceOfProducts(m_columns[0][0], m_columns[3][1], m_columns[0][1], m_columns[3][0]);
        
        const float s3 = math::DifferenceOfProducts(m_columns[1][0], m_columns[2][1], m_columns[1][1], m_columns[2][0]);
        const float s4 = math::DifferenceOfProducts(m_columns[1][0], m_columns[3][1], m_columns[1][1], m_columns[3][0]);
        const float s5 = math::DifferenceOfProducts(m_columns[2][0], m_columns[3][1], m_columns[2][1], m_columns[3][0]);
        
        const float c0 = math::DifferenceOfProducts(m_columns[0][2], m_columns[1][3], m_columns[0][3], m_columns[1][2]);
        const float c1 = math::DifferenceOfProducts(m_columns[0][2], m_columns[2][3], m_columns[0][3], m_columns[2][2]);
        const float c2 = math::DifferenceOfProducts(m_columns[0][2], m_columns[3][3], m_columns[0][3], m_columns[3][2]);
        
        const float c3 = math::DifferenceOfProducts(m_columns[1][2], m_columns[2][3], m_columns[1][3], m_columns[2][2]);
        const float c4 = math::DifferenceOfProducts(m_columns[1][2], m_columns[3][3], m_columns[1][3], m_columns[3][2]);
        const float c5 = math::DifferenceOfProducts(m_columns[2][2], m_columns[3][3], m_columns[2][3], m_columns[3][2]);
            
        return (math::DifferenceOfProducts(s0, c5, s1, c4)
            + math::DifferenceOfProducts(s2, c3, -s3, c2)
            + math::DifferenceOfProducts(s5, c0, s4, c1));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Removes scaling from this Matrix. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void TMatrix4x4<Type>::RemoveScaling()
    {
        for (size_t i = 0; i < N - 1; ++i)
        {
            const Type squaredMag = m_columns[i].SquaredMagnitude();
            const Type axisScaleFactor = squaredMag < math::PrecisionDelta<Type>() ? static_cast<Type>(1) : static_cast<Type>(1) / std::sqrt(squaredMag);
            m_columns[i][0] *= axisScaleFactor;
            m_columns[i][1] *= axisScaleFactor;
            m_columns[i][2] *= axisScaleFactor;
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Removes scaling from this Matrix and returns the scale as a Vector3. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TMatrix4x4<Type>::ExtractScaling()
    {
        TVector3<Type> scale;

        for (size_t i = 0; i < N - 1; ++i)
        {
            const Type squaredMag = m_columns[i].SquaredMagnitude();
            const Type axisScale = squaredMag < math::PrecisionDelta<Type>() ? static_cast<Type>(1) : std::sqrt(squaredMag);
            scale[i] = axisScale;

            // Remove scale from the axis:
            const Type invAxisScale = static_cast<Type>(1) / axisScale;
            m_columns[i][0] *= invAxisScale;
            m_columns[i][1] *= invAxisScale;
            m_columns[i][2] *= invAxisScale;
        }
        
        return scale;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Scale vector from this Matrix. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TMatrix4x4<Type>::GetScale() const
    {
        TVector3<Type> scale;

        for (size_t i = 0; i < N - 1; ++i)
        {
            const Type squaredMag = m_columns[i].SquaredMagnitude();
            const Type axisScale = squaredMag < math::PrecisionDelta<Type>() ? static_cast<Type>(1) : std::sqrt(squaredMag);
            scale[i] = axisScale;
        }
        
        return scale;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a copy of this Matrix with the scaling removed. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TMatrix4x4<Type> TMatrix4x4<Type>::GetWithoutScale() const
    {
        TMatrix4x4 result(*this);
        result.RemoveScaling();
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Transform a 3D point by this Matrix. This will include the Translation defined by this
    ///              matrix.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TMatrix4x4<Type>::TransformPoint(const TVector3<Type>& point) const
    {
        const TVector4<Type> transformed = (*this * TVector4<Type>(point, static_cast<Type>(1)));
        return TVector3<Type>(transformed.x, transformed.y, transformed.z);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Transform a 3D vector by this Matrix. This will NOT include the translation defined by
    ///             this matrix.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TMatrix4x4<Type>::TransformVector(const TVector3<Type>& vector) const
    {
        const TVector4<Type> transformed = (*this * TVector4<Type>(vector, static_cast<Type>(0)));
        return TVector3<Type>(transformed.x, transformed.y, transformed.z);
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
                result += std::to_string(this->m_columns[j][i]) + " ";
            }
            
            result += "\n";
        }

        return result;
    }

    template <FloatingPointType Type>
    TVector4<Type> operator*(const TMatrix4x4<Type>& matrix, const TVector4<Type>& vector)
    {
        TVector4<Type> result;
        result[0] = matrix[0][0] * vector[0] + matrix[1][0] * vector[1] + matrix[2][0] * vector[2] + matrix[3][0] * vector[3];
        result[1] = matrix[0][1] * vector[0] + matrix[1][1] * vector[1] + matrix[2][1] * vector[2] + matrix[3][1] * vector[3];
        result[2] = matrix[0][2] * vector[0] + matrix[1][2] * vector[1] + matrix[2][2] * vector[2] + matrix[3][2] * vector[3];
        result[3] = matrix[0][3] * vector[0] + matrix[1][3] * vector[1] + matrix[2][3] * vector[2] + matrix[3][3] * vector[3];
        return result;
    }

    template <FloatingPointType Type>
    TVector4<Type> operator*(const TVector4<Type>& vector, const TMatrix4x4<Type>& matrix)
    {
        return matrix * vector;
    }
}
