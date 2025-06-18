// Mat4.h
#pragma once
#include "Scalar4.h"
#include "Math/MathTypes.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : 4x4 Matrix of floats. Supports operations on the upper 3x3 part of the matrix. 
    //----------------------------------------------------------------------------------------------------
    class alignas(NES_VECTOR_ALIGNMENT) Mat44
    {
    public:
        using Type = Vec4Reg::Type;     /// Underlying Column Type. 
        static constexpr size_t N = 4;  /// Number of dimensions, both columns and rows.

        Mat44() = default;
        Mat44(const Mat44& other) = default;
        Mat44& operator=(const Mat44& other) = default;

        // Conversion Constructors
        explicit NES_INLINE     Mat44(const Vec4& c1, const Vec4& c2, const Vec4& c3, const Vec4& c4);
        NES_INLINE              Mat44(const Vec4Reg& c1, const Vec4Reg& c2, const Vec4Reg& c3, const Vec4Reg& c4);
        NES_INLINE              Mat44(const Vec4& c1, const Vec4& c2, const Vec4& c3, const Vec3& c4);
        NES_INLINE              Mat44(Type c1, Type c2, Type c3, Type c4);
        explicit NES_INLINE     Mat44(const Vec3& diagonal);
        explicit NES_INLINE     Mat44(const float uniformDiagonal);

        // Operators
        NES_INLINE Vec4Reg      operator[](uint index) const;
        NES_INLINE Vec4Reg&     operator[](uint index);
        NES_INLINE bool         operator==(const Mat44& other) const;
        NES_INLINE bool         operator!=(const Mat44& other) const        { return !(*this == other); }
        NES_INLINE Mat44        operator*(const Mat44& other) const;
        NES_INLINE Vec3         operator*(const Vec3& vec) const;
        NES_INLINE Vec4         operator*(const Vec4& vec) const;
        NES_INLINE Vec4Reg      operator*(const Vec4Reg& vec) const;
        NES_INLINE Mat44        operator*(const float scalar) const;
        friend NES_INLINE Mat44 operator*(const float scalar, const Mat44& mat);
        NES_INLINE Mat44&       operator*=(const float scalar);
        NES_INLINE Mat44        operator+(const Mat44& other) const;
        NES_INLINE Mat44&       operator+=(const Mat44& other);
        NES_INLINE Mat44        operator-(const Mat44& other) const;
        NES_INLINE Mat44&       operator-=(const Mat44& other);
        NES_INLINE Mat44        operator-() const;
        
        NES_INLINE Vec3         GetAxisX() const                                        { return m_columns[0].ToVec3(); }
        NES_INLINE Vec3         GetAxisY() const                                        { return m_columns[1].ToVec3(); }
        NES_INLINE Vec3         GetAxisZ() const                                        { return m_columns[2].ToVec3(); }
        NES_INLINE void         SetAxisX(const Vec3& axis)                              { m_columns[0] = Vec4Reg(axis, 0.f); }
        NES_INLINE void         SetAxisY(const Vec3& axis)                              { m_columns[1] = Vec4Reg(axis, 0.f); }
        NES_INLINE void         SetAxisZ(const Vec3& axis)                              { m_columns[2] = Vec4Reg(axis, 0.f); }
        NES_INLINE Vec3         GetColumn3(const uint column) const                     { NES_ASSERT(column < 4); return m_columns[column].ToVec3(); }
        NES_INLINE void         SetColumn3(const uint column, const Vec3& val);
        NES_INLINE Vec4         GetColumn4(const uint column) const                     { NES_ASSERT(column < 4); return m_columns[column].ToVec4(); }
        NES_INLINE void         SetColumn4(const uint column, const Vec4& val);
        NES_INLINE void         SetColumn4(const uint column, const Vec4Reg& val);
        NES_INLINE Vec3         GetRow3(const uint row) const;                        
        NES_INLINE void         SetRow3(const uint column, const Vec3& val);                        
        NES_INLINE Vec4         GetRow4(const uint row) const;                        
        NES_INLINE void         SetRow4(const uint column, const Vec4& val);                        
        NES_INLINE Vec3         GetDiagonal3() const                                    { return Vec3(m_columns[0][0], m_columns[1][1], m_columns[2][2]); }
        NES_INLINE void         SetDiagonal3(const Vec3& diagonal);
        NES_INLINE Vec4         GetDiagonal4() const                                    { return Vec4(m_columns[0][0], m_columns[1][1], m_columns[2][2], m_columns[3][3]); }  
        NES_INLINE void         SetDiagonal4(const Vec4& diagonal);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the translation described by this matrix (XYZ components of the 4th column).
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         GetTranslation() const                                  { return m_columns[3].ToVec3(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the translation component of this matrix (XYZ components of the 4th column). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         SetTranslation(const Vec3& translation)                 { m_columns[3] = Vec4(translation, 1.f); }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if two matrices are close to one another, testing each column.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         IsClose(const Mat44& other, const float maxSqrDist = 1.0e-12f) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Multiply a vector only by the 3x3 part of the matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         Multiply3x3(const Vec3& vec) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Multiply a vector only by the 3x3 part of the transpose of the matrix.
        ///     Result = this^T * vec.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         Multiply3x3Transposed(const Vec3& vec) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Multiply 3x3 matrix by 3x3 matrix.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        Multiply3x3(const Mat44& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Multiply transpose of this 3x3 matrix by another 3x3 matrix. Result = this^T * other.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        Multiply3x3LeftTransposed(const Mat44& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Multiply this 3x3 matrix by the transpose of the other 3x3 matrix. Result = this * other^T.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        Multiply3x3RightTransposed(const Mat44& other) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Transform a 3D point by this Matrix. This will include the translation defined by this matrix.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         TransformPoint(const Vec3& point) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Transform a 2D point by this Matrix. This will include the translation defined by this matrix.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec2         TransformPoint(const Vec2 point) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Transform a 3D point by this Matrix. This will NOT include the translation defined by
        ///     this matrix.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         TransformVector(const Vec3& vector) const { return Multiply3x3(vector); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Store a matrix to memory. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         StoreFloat4x4(Float4* pOutFloats) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the transpose of this matrix.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        Transposed() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the transpose of the 3x3 part of the matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        Transposed3x3() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the inverse of the 4x4 matrix.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        Inversed() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Inverse 4x4 matrix when it only contains the rotation and translation. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        InversedRotationTranslation() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the determinant of the 3x3 part of the matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Determinant3x3() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the determinant of the 4x4 matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float        Determinant() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the adjoint of the 3x3 matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        Adjoint3x3() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the inverse of the 3x3 matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        Inversed3x3() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set this equal to the inverse of the 3x3 matrix (*this = matrix.Inverse3x3()).
        ///     This returns false if the matrix is singular, in which case *this is unchanged.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE bool         SetInversed3x3(const Mat44& matrix);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the rotation part only.
        /// @note : The result will retain the first 3 values from the bottom row.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        GetRotation() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the rotation part only.
        /// @note : Unlike GetRotation(), this will clear the bottom row as well.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        GetRotationSafe() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Updates the rotation part of the matrix (first 3 columns). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         SetRotation(const Mat44& rotation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Convert to a quaternion. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Quat         ToQuaternion() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the scale from this matrix. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Vec3         GetScale() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a matrix that transforms a direction with the same transform as this matrix (length is
        ///     not preserved).
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        GetDirectionPreservingMatrix() const                { return GetRotation().Inversed3x3().Transposed3x3(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Pre-multiply by translation matrix: result = this * Mat44::Translation(translation).
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        PreTranslated(const Vec3& translation) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Post-multiply by translation matrix: result = Mat44::Translation(translation) * this,
        ///     effectively adds the translation to the 4th column.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        PostTranslated(const Vec3& translation) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Scale a matrix: result = this * Mat44::Scale(scale). 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        PreScaled(const Vec3& scale) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Scale a matrix: result = Mat44::Scale(scale) * this.. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        PostScaled(const Vec3& scale) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Decompose this matrix into a rotation-translation part and a scale part so that
        ///     this = returnValue * Mat44::Scale(outScale).
        ///     This equation only holds when the matrix is orthogonal, if it is not, the returned matrix
        ///     will be made orthogonal using the modified Gram-Schmidt algorithm (see: https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process)
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Mat44        Decompose(Vec3& outScale) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Decompose this matrix into the individual translation, rotation and scale elements.
        ///     This equation only holds when the matrix is orthogonal, if it is not, the returned matrix
        ///     will be made orthogonal using the modified Gram-Schmidt algorithm (see: https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process)
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         Decompose(Vec3& outTranslation, Quat& outRotation, Vec3& outScale) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Decompose this matrix into the individual translation, rotation and scale elements.
        ///     This equation only holds when the matrix is orthogonal, if it is not, the returned matrix
        ///     will be made orthogonal using the modified Gram-Schmidt algorithm (see: https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process)
        //----------------------------------------------------------------------------------------------------
        NES_INLINE void         Decompose(Vec3& outTranslation, Rotation& outRotation, Vec3& outScale) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Identity Matrix - represents no rotation.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 Identity();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Zero Matrix. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 Zero();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Matrix filled with NaN's. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 NaN();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load 16 floats from memory.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 LoadFloat4x4(const Float4* pFloats);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Load 16 floats from memory, 16 bytes aligned. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 LoadFloat4x4Aligned(const Float4* pFloats);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a rotation matrix around the X-axis by the angle (in radians).
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 MakeRotationX(const float angle);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a rotation matrix around the Y-axis by the angle (in radians).
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 MakeRotationY(const float angle);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a rotation matrix around the Z-axis by the angle (in radians).
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 MakeRotationZ(const float angle);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a rotation matrix around an arbitrary axis by the angle (in radians).
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 MakeRotation(const Vec3& axis, const float angle);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a rotation matrix from a Quaternion.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 MakeRotation(const Quat& quat);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a translation matrix from the given translation.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 MakeTranslation(const Vec3& translation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a matrix that rotates and translates by 'rotation' and 'translation', respectively.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 MakeRotationTranslation(const Quat& rotation, const Vec3& translation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a matrix that performs the inverse rotation and translation. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 MakeInverseRotationTranslation(const Quat& rotation, const Vec3& translation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a matrix that scales uniformly. 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 MakeScale(float scale);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a matrix that scales by the given scale (its diagonal is set to (scale, 1)). 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 MakeScale(const Vec3& scale);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Compose a transformation matrix.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 ComposeTransform(const Vec3& translation, const Quat& rotation, const Vec3& scale);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Compose a transformation matrix.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 ComposeTransform(const Vec3& translation, const Rotation& rotation, const Vec3& scale);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the outer product of a and b (equivalent to a times b).
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 OuterProduct(const Vec3& a, const Vec3& b);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a matrix that represents a cross-product A x B = CrossProduct(A) * B.  
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 CrossProduct(const Vec3& vec);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a matrix ML so that ML * p = q * p (where p and q are quaternions).  
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 QuatLeftMultiply(const Quat& quat);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a matrix MR so that MR * p = p * q (where p and q are quaternions).  
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 QuatRightMultiply(const Quat& quat);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a left-handed View Matrix that transforms from world space to view space.
        ///	@param eyePosition : Position of the Camera in World space.era.
        ///	@param target : Target position that the Camera is looking at.
        ///	@param upVector : Normalized up vector that determines the Camera orientation.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 LookAt(const Vec3& eyePosition, const Vec3& target, const Vec3& upVector);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a left-handed, perspective projection matrix based on a field of view.
        ///     The nearPlane and farPlane clip planes are normalized to [0, 1] if NES_CLIP_VIEW_ZERO_TO_ONE is equal to 1,
        ///     otherwise, it will be normalized to [-1, 1].
        /// @param fovRadians : Vertical Field of View, expressed in radians. 
        /// @param aspectRatio : Aspect Ratio of the Viewport. Equal to width / height.
        /// @param nearPlane : Distance of the viewer to the nearPlane clip plane (must always be positive).
        /// @param farPlane : Distance of the viewer to the farPlane clip plane (must always be positive). 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 Perspective(const float fovRadians, const float aspectRatio, const float nearPlane, const float farPlane);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a left-handed, perspective projection matrix based on a field of view.
        ///     The near and farPlane clip planes are normalized to [0, 1] if NES_CLIP_VIEW_ZERO_TO_ONE is equal to 1,
        ///     otherwise, it will be normalized to [-1, 1].
        ///	@param fovRadians : Vertical Field of View, expressed in radians. 
        ///	@param width : Width of the viewport. 
        ///	@param height : Height of the viewport.
        ///	@param nearPlane : Distance of the viewer to the near clip plane (must always be positive).
        ///	@param farPlane : Distance of the viewer to the farPlane clip plane (must always be positive). 
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 Perspective(const float fovRadians, const float width, const float height, const float nearPlane, const float farPlane);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a left-handed orthographic project matrix.
        ///    The nearPlane and farPlane clip planes are normalized to [0, 1] if NES_CLIP_VIEW_ZERO_TO_ONE is equal to 1,
        ///    otherwise, it will be normalized to [-1, 1].
        /// @param left : Left side of the projection.
        /// @param right : Right side of the projection.
        /// @param bottom : Bottom of the projection.
        /// @param top : Top of the projection.
        /// @param nearPlane : Distance of the viewer to the nearPlane clip plane (must always be positive).
        /// @param farPlane : Distance of the viewer to the farPlane clip plane (must always be positive).
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE Mat44 Orthographic(const float left, const float right, const float bottom, const float top, const float nearPlane, const float farPlane);
        
    private:
        Vec4Reg m_columns[N];
    };
}

#include "Mat4.inl"
