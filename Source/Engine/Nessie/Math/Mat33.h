// Mat33.h
#pragma once
#include "Scalar3.h"
#include "MathTypes.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : 3x3 Matrix of floats. Supports operations on the upper 2x2 part of the matrix. 
    //----------------------------------------------------------------------------------------------------
    class Mat33
    {
    public:
        static constexpr size_t N = 3;
        
        Mat33() = default;
        Mat33(const Mat33& other) = default;
        Mat33& operator=(const Mat33& other) = default;

        /// Conversions Constructors
        NES_INLINE              Mat33(const Vec3 c1, const Vec3 c2, const Vec3 c3);
        NES_INLINE              Mat33(const Vec3 c1, const Vec3 c2, const Vec2 c3);
        NES_INLINE              Mat33(const Vec2 diagonal);
        NES_INLINE              Mat33(const float uniformDiagonal);

        /// Operators
        NES_INLINE Vec3         operator[](const size_t index) const;
        NES_INLINE Vec3&        operator[](const size_t index);
        NES_INLINE bool         operator==(const Mat33& other) const;
        NES_INLINE bool         operator!=(const Mat33& other) const { return !(*this == other); }
        NES_INLINE Mat33        operator*(const Mat33& other) const;
        NES_INLINE Vec2         operator*(const Vec2 other) const;
        NES_INLINE Vec3         operator*(const Vec3 other) const;
        NES_INLINE Mat33        operator*(const float scalar) const;
        friend NES_INLINE Mat33 operator*(const float scalar, const Mat33& mat);
        NES_INLINE Mat33&       operator*=(const float scalar);
        NES_INLINE Mat33        operator+(const Mat33& other) const;
        NES_INLINE Mat33&       operator+=(const Mat33& other);
        NES_INLINE Mat33        operator-(const Mat33& other) const;
        NES_INLINE Mat33&       operator-=(const Mat33& other);
        NES_INLINE Mat33        operator-() const;

        NES_INLINE Vec2         GetAxisX() const { return Vec2(m_columns[0].x, m_columns[0].y); }
        NES_INLINE Vec2         GetAxisY() const { return Vec2(m_columns[1].x, m_columns[1].y); }
        NES_INLINE void         SetAxisX(const Vec2 axis);
        NES_INLINE void         SetAxisY(const Vec2 axis);
        NES_INLINE Vec2         GetColumn2(const size_t column) const { NES_ASSERT(column < N); return Vec2(m_columns[column].x, m_columns[column].y); }
        NES_INLINE Vec3         GetColumn3(const size_t column) const { NES_ASSERT(column < N); return m_columns[column]; }
        NES_INLINE void         SetColumn2(const size_t column, const Vec2 value);
        NES_INLINE void         SetColumn3(const size_t column, const Vec3 value);
        NES_INLINE Vec2         GetRow2(const size_t row) const;
        NES_INLINE Vec3         GetRow3(const size_t row) const;
        NES_INLINE void         SetRow2(const size_t row, const Vec2 value);
        NES_INLINE void         SetRow3(const size_t row, const Vec3 value);
        NES_INLINE Vec2         GetDiagonal2() const                        { return Vec2(m_columns[0][0], m_columns[1][1]); }
        NES_INLINE Vec3         GetDiagonal3() const                        { return Vec3(m_columns[0][0], m_columns[1][1], m_columns[2][2]); }
        NES_INLINE void         SetDiagonal2(const Vec2 diagonal);
        NES_INLINE void         SetDiagonal3(const Vec3 diagonal);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the 2D translation described by this matrix (XY components of the 3rd column).
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         GetTranslation() const { return Vec2(m_columns[2].x, m_columns[2].y); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the 2D translation component of this matrix (XY components of the 3rd column).
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         SetTranslation(const Vec2 translation) { m_columns[2] = Vec3(translation.x, translation.y, 1.f); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if two matrices are close to one another, testing each column.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         IsClose(const Mat33& other, const float maxSqrDist = 1.0e-12f) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Multiply a vector only by the 2x2 part of the matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         Multiply2x2(const Vec2 vec) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Multiply a vector only by the 2x2 part of the transpose of the matrix.
        ///     Result = this^T * vec.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         Multiply2x2Transposed(const Vec2 vec) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Multiply 2x2 matrix by 2x2 matrix.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        Multiply2x2(const Mat33& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Multiply transpose of this 2x2 matrix by another 2x2 matrix. Result = this^T * other.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        Multiply2x2LeftTransposed(const Mat33& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Multiply this 2x2 matrix by the transpose of the other 2x2 matrix. Result = this * other^T.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        Multiply2x2RightTransposed(const Mat33& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Transform a 2D point by this Matrix. This will include the translation defined by this matrix.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         TransformPoint(const Vec2 point) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Transform a 2D point by this Matrix. This will NOT include the translation defined by
        ///     this matrix.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         TransformVector(const Vec2 vector) const { return Multiply2x2(vector); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Store a matrix to memory. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         StoreFloat3x3(Float3* pOutFloats) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the transpose of this matrix.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        Transposed() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the transpose of the 2x2 part of the matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        Transposed2x2() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the determinant of the 2x2 part of the matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Determinant2x2() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the determinant of the 3x3 matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Determinant() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the inverse of the 3x3 matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        Inversed() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the inverse of the 2x2 matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        Inversed2x2() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set this equal to the inverse of the 2x2 matrix (*this = matrix.Inverse2x2()).
        ///     This returns false if the matrix is singular, in which case *this is unchanged.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         SetInversed2x2(const Mat33& matrix);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the adjoint of the 2x2 matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        Adjoint2x2() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Inverse 3x3 matrix when it only contains the rotation and translation. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        InversedRotationTranslation() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the rotation part only.
        /// @note : The result will retain the first 2 values from the bottom row.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        GetRotation() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the rotation part only.
        /// @note : Unlike GetRotation(), this will clear the bottom row as well.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        GetRotationSafe() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Updates the rotation part of the matrix (first 2 columns). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         SetRotation(const Mat33& rotation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the scale from this matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         GetScale() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Pre-multiply by translation matrix: result = this * Mat33::Translation(translation).
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        PreTranslated(const Vec2 translation) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Post-multiply by translation matrix: result = Mat33::Translation(translation) * this,
        ///     effectively adds the translation to the 4th column.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        PostTranslated(const Vec2 translation) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Scale a matrix: result = this * Mat33::Scale(scale). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        PreScaled(const Vec2 scale) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Scale a matrix: result = Mat33::Scale(scale) * this.. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        PostScaled(const Vec2 scale) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Decompose this matrix into a rotation-translation part and a scale part so that
        ///     this = returnValue * Mat33::Scale(outScale).
        ///     This equation only holds when the matrix is orthogonal, if it is not, the returned matrix
        ///     will be made orthogonal using the modified Gram-Schmidt algorithm (see: https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process)
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat33        Decompose(Vec2& outScale) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Decompose this matrix into the individual translation, rotation and scale elements.
        ///     This equation only holds when the matrix is orthogonal, if it is not, the returned matrix
        ///     will be made orthogonal using the modified Gram-Schmidt algorithm (see: https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process)
        ///     - The rotation angle will be in radians. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         Decompose(Vec2& outTranslation, float& outRotation, Vec2& outScale) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Identity Matrix - represents no rotation.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat33 Identity();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Zero Matrix. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat33 Zero();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Matrix filled with NaN's. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat33 NaN();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load 9 floats from memory.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat33 LoadFloat3x3(const Float3* pFloats);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load 9 floats from memory, 16 bytes aligned. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat33 LoadFloat3x3Aligned(const Float3* pFloats);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Make a rotation matrix with the given angle.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat33 MakeRotation(const float angleRadians);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a translation matrix from the given translation.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat33 MakeTranslation(const Vec2 translation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a matrix that rotates and translates by 'rotation' and 'translation', respectively.
        ///     Rotation angle is expected to be in radians. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat33 MakeRotationTranslation(const float rotation, const Vec2 translation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a matrix that performs the inverse rotation and translation.
        ///     Rotation angle is expected to be in radians. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat33 MakeInverseRotationTranslation(const float rotation, const Vec2 translation);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a matrix that scales uniformly. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat33 MakeScale(float scale);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a matrix that scales by the given scale (its diagonal is set to (scale, 1)). 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat33 MakeScale(const Vec2 scale);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Compose a transformation matrix.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat33 ComposeTransform(const Vec2 translation, const float rotation, const Vec2 scale);

    private:
        Vec3 m_columns[N];
    };
}

#include "Mat33.inl"