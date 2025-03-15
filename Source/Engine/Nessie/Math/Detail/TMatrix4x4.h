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
        constexpr TMatrix4x4(const TMatrix3x3<Type>& matrix);

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
        
        TVector4<Type> operator[](const size_t index) const;

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
        
        static constexpr TMatrix4x4 Zero();
        static constexpr TMatrix4x4 Identity() { return TMatrix4x4(); }
    };

    template <FloatingPointType Type>
    TVector4<Type> operator*(const TMatrix4x4<Type>& matrix, const TVector4<Type>& vector);
    
    template <FloatingPointType Type>
    TVector4<Type> operator*(const TVector4<Type>& vector, const TMatrix4x4<Type>& matrix);
}

namespace nes
{
    template <FloatingPointType Type>
    constexpr TMatrix4x4<Type>::TMatrix4x4(const Type values[N * N])
    {
        NES_ASSERT(values != nullptr);
        memcpy(&(m[0][0]), values, N * N * sizeof(Type));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Constructs a 4x4 matrix from a 3x3 Matrix. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TMatrix4x4<Type>::TMatrix4x4(const TMatrix3x3<Type>& matrix)
        : TMatrix4x4() // Default construct to the identity matrix
    {
        m[0][0] = matrix.m[0][0];
        m[0][1] = matrix.m[0][1];
        m[0][2] = matrix.m[0][2];

        m[1][0] = matrix.m[1][0];
        m[1][1] = matrix.m[1][1];
        m[1][2] = matrix.m[1][2];
        
        m[2][0] = matrix.m[2][0];
        m[2][1] = matrix.m[2][1];
        m[2][2] = matrix.m[2][2];
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

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns an axis of this matrix with scaling included. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TMatrix4x4<Type>::GetAxis(const Axis axis) const
    {
        switch (axis)
        {
            case Axis::X: return TVector3<Type>(m[0][0], m[1][0], m[2][0]);
            case Axis::Y: return TVector3<Type>(m[0][1], m[1][1], m[2][1]);
            case Axis::Z: return TVector3<Type>(m[0][2], m[1][2], m[2][2]);
            case Axis::W: return TVector3<Type>(m[0][3], m[1][3], m[2][3]);
            
            default:
                NES_ASSERTV(false, "Invalid Axis request!");
                return TVector4<Type>::GetZeroVector();
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns an axis of this matrix with scaling included. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector3<Type> TMatrix4x4<Type>::GetAxis(const int axis) const
    {
        NES_ASSERT(axis >= 0 && axis < static_cast<int>(N));
        return TVector3<Type>(m[0][axis], m[1][axis], m[2][axis]);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a column of this matrix as a vector.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector4<Type> TMatrix4x4<Type>::GetColumn(const int column) const
    {
        NES_ASSERT(column >= 0 && column < static_cast<int>(N));
        return TVector4<Type>(m[0][column], m[1][column], m[2][column], m[3][column]);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a row of this matrix as a vector.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr TVector4<Type> TMatrix4x4<Type>::GetRow(const int row) const
    {
        NES_ASSERT(row >= 0 && row < static_cast<int>(N));
        return TVector4<Type>(m[row][0], m[row][1], m[row][2], m[row][3]);
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

    template <FloatingPointType Type>
    TVector4<Type> TMatrix4x4<Type>::operator[](const size_t index) const
    {
        NES_ASSERT(index < N);
        return GetColumn(static_cast<int>(index));
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
    ///		@brief : Removes scaling from this Matrix. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void TMatrix4x4<Type>::RemoveScaling()
    {
        for (size_t i = 0; i < N - 1; ++i)
        {
            const Type squaredMag = GetAxis(static_cast<int>(i)).SquaredMagnitude();
            const Type axisScaleFactor = squaredMag < math::PrecisionDelta<Type>() ? static_cast<Type>(1) : static_cast<Type>(1) / std::sqrt(squaredMag);
            m[0][i] *= axisScaleFactor;
            m[1][i] *= axisScaleFactor;
            m[2][i] *= axisScaleFactor;
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
            const Type squaredMag = GetAxis(static_cast<int>(i)).SquaredMagnitude();
            const Type axisScale = squaredMag < math::PrecisionDelta<Type>() ? static_cast<Type>(1) : std::sqrt(squaredMag);
            scale[i] = axisScale;

            // Remove scale from the axis:
            const Type invAxisScale = static_cast<Type>(1) / axisScale;
            m[0][i] *= invAxisScale;
            m[1][i] *= invAxisScale;
            m[2][i] *= invAxisScale;
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
            const Type squaredMag = GetAxis(static_cast<int>(i)).SquaredMagnitude();
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
        const Vector4 transformed = (*this * Vector4(point, static_cast<Type>(1)));
        return Vector3(transformed.x, transformed.y, transformed.z);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Transform a 3D vector by this Matrix. This will NOT include the translation defined by
    ///             this matrix.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    TVector3<Type> TMatrix4x4<Type>::TransformVector(const TVector3<Type>& vector) const
    {
        const Vector4 transformed = (*this * Vector4(vector, static_cast<Type>(0)));
        return Vector3(transformed.x, transformed.y, transformed.z);
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

    template <FloatingPointType Type>
    TVector4<Type> operator*(const TMatrix4x4<Type>& matrix, const TVector4<Type>& vector)
    {
        TVector4<Type> result;
        
        // Column Orientation:
        result[0] = (matrix.m[0][0] * vector[0]) + (matrix.m[0][1] * vector[1]) + (matrix.m[0][2] * vector[2]) + (matrix.m[0][3] * vector[3]);
        result[1] = (matrix.m[1][0] * vector[0]) + (matrix.m[1][1] * vector[1]) + (matrix.m[1][2] * vector[2]) + (matrix.m[1][3] * vector[3]);
        result[2] = (matrix.m[2][0] * vector[0]) + (matrix.m[2][1] * vector[1]) + (matrix.m[2][2] * vector[2]) + (matrix.m[2][3] * vector[3]);
        result[3] = (matrix.m[3][0] * vector[0]) + (matrix.m[3][1] * vector[1]) + (matrix.m[3][2] * vector[2]) + (matrix.m[3][3] * vector[3]);
        
        return result;
    }

    template <FloatingPointType Type>
    TVector4<Type> operator*(const TVector4<Type>& vector, const TMatrix4x4<Type>& matrix)
    {
        return matrix * vector;
    }
}
