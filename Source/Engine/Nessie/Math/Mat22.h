// Mat22.h
#pragma once
#include "Scalar2.h"
#include "MathTypes.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : 2x2 Matrix of floats. 
    //----------------------------------------------------------------------------------------------------
    class Mat22
    {
    public:
        static constexpr size_t N = 2;

        Mat22() = default;
        Mat22(const Mat22& other) = default;
        Mat22& operator=(const Mat22& other) = default;

        /// Conversions Constructors
        NES_INLINE              Mat22(const Vec2 c1, const Vec2 c2);
        NES_INLINE              Mat22(const Vec2 diagonal);
        NES_INLINE              Mat22(const float uniformDiagonal);

        /// Operators
        NES_INLINE Vec2         operator[](const size_t index) const;
        NES_INLINE Vec2&        operator[](const size_t index);
        NES_INLINE bool         operator==(const Mat22& other) const;
        NES_INLINE bool         operator!=(const Mat22& other) const { return !(*this == other); }
        NES_INLINE Mat22        operator*(const Mat22& other) const;
        NES_INLINE Vec2         operator*(const Vec2 other) const;
        NES_INLINE Mat22        operator*(const float scalar) const;
        friend NES_INLINE Mat22 operator*(const float scalar, const Mat22& mat);
        NES_INLINE Mat22&       operator*=(const float scalar);
        NES_INLINE Mat22        operator+(const Mat22& other) const;
        NES_INLINE Mat22&       operator+=(const Mat22& other);
        NES_INLINE Mat22        operator-(const Mat22& other) const;
        NES_INLINE Mat22&       operator-=(const Mat22& other);
        NES_INLINE Mat22        operator-() const;

        NES_INLINE Vec2         GetColumn(const size_t column) const                { return m_columns[column]; }
        NES_INLINE void         SetColumn(const size_t column, const Vec2 value)    { NES_ASSERT(column < N); m_columns[column] = value; }
        NES_INLINE Vec2         GetRow(const size_t row) const;
        NES_INLINE void         SetRow(const size_t row, const Vec2 value);
        NES_INLINE Vec2         GetDiagonal() const;
        NES_INLINE void         SetDiagonal(const Vec2 diagonal);
        NES_INLINE void         SetDiagonal(const float uniformDiagonal);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set all the components of this matrix to zero. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         SetZero();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if all components are equal to zero.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         IsZero() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set this matrix to the Identity matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         SetIdentity();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if this matrix is equal to the identity matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         IsIdentity() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the transpose of this matrix.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat22        Transposed() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the inverse of the 2x2 matrix.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat22        Inversed() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set this matrix to the inverse of the passed in matrix. Returns false if the matrix 'm' is
        ///     non-invertible.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         SetInversed(const Mat22& m);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the determinant of this Matrix.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Determinant() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Identity matrix. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat22 Identity();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Zero Matrix. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat22 Zero();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Matrix filled with NaN's. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat22 NaN();
        

    private:
        Vec2 m_columns[N];
    };
}

#include "Mat22.inl"