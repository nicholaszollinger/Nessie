// Mat33.inl
#pragma once
#include "Vec2.h"
#include "Vec3.h"

namespace nes
{
    Mat33::Mat33(const Vec3 c1, const Vec3 c2, const Vec3 c3)
        : m_columns { c1, c2, c3 }
    {
        //
    }

    Mat33::Mat33(const Vec3 c1, const Vec3 c2, const Vec2 c3)
        : m_columns { c1, c2, Vec3(c3.x, c3.y, 1.f)}
    {
        //
    }

    Mat33::Mat33(const Vec2 diagonal)
    {
        m_columns[0] = Vec3(diagonal.x, 0.f, 0.f);
        m_columns[1] = Vec3(0.f, diagonal.y, 0.f);
        m_columns[2] = Vec3(0.f, 0.f, 1.f);
    }

    Mat33::Mat33(const float uniformDiagonal)
    {
        m_columns[0] = Vec3(uniformDiagonal, 0.f, 0.f);
        m_columns[1] = Vec3(0.f, uniformDiagonal, 0.f);
        m_columns[2] = Vec3(0.f, 0.f, 1.f);
    }

    Vec3 Mat33::operator[](const size_t index) const
    {
        NES_ASSERT(index < N);
        return m_columns[index];
    }

    Vec3& Mat33::operator[](const size_t index)
    {
        NES_ASSERT(index < N);
        return m_columns[index];
    }

    bool Mat33::operator==(const Mat33& other) const
    {
        return UVec4Reg::And(
            UVec4Reg::And(Vec3::Equals(m_columns[0], other.m_columns[0]), Vec3::Equals(m_columns[1], other.m_columns[1])),
            Vec3::Equals(m_columns[2], other.m_columns[2]))
            .TestAllTrue();
    }

    Mat33 Mat33::operator*(const Mat33& other) const
    {
        Mat33 result;
#if defined (NES_USE_SSE)
        // Load the Columns into registers.
        const Vec4Reg c0 = Vec4Reg(m_columns[0]);
        const Vec4Reg c1 = Vec4Reg(m_columns[1]);
        const Vec4Reg c2 = Vec4Reg(m_columns[2]);
        
        for (size_t i = 0; i < N; ++i)
        {
            const __m128 c = Vec4Reg(other.m_columns[i]).m_value;
            __m128 t = _mm_mul_ps(c0.m_value, _mm_shuffle_ps(c, c, _MM_SHUFFLE(0, 0, 0, 0)));
            t = _mm_add_ps(t, _mm_mul_ps(c1.m_value, _mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1))));
            t = _mm_add_ps(t, _mm_mul_ps(c2.m_value, _mm_shuffle_ps(c, c, _MM_SHUFFLE(2, 2, 2, 2))));
            result.m_columns[i] = Vec4Reg(t).ToVec3();
        }
#else
        for (size_t i = 0; i < N; ++i)
        {
            result.m_columns[i] = m_columns[0] * other.m_columns[i].x + m_columns[1] * other.m_columns[i].y + m_columns[2] * other.m_columns[i].z;
        }
#endif
        return result;
    }

    Vec2 Mat33::operator*(const Vec2 vec) const
    {
        return Vec2
        (
            m_columns[0].x * vec.x + m_columns[1].x * vec.y,
            m_columns[0].y * vec.x + m_columns[1].y * vec.y
        );
    }

    Vec3 Mat33::operator*(const Vec3 vec) const
    {
#if defined (NES_USE_SSE)
        // Load the columns and vector into registers.
        const Vec4Reg c0 = Vec4Reg(m_columns[0]);
        const Vec4Reg c1 = Vec4Reg(m_columns[1]);
        const Vec4Reg c2 = Vec4Reg(m_columns[2]);
        const Vec4Reg rVec = Vec4Reg(vec);
        
        __m128 t = _mm_mul_ps(c0.m_value, _mm_shuffle_ps(rVec.m_value, rVec.m_value, _MM_SHUFFLE(0, 0, 0, 0)));
        t = _mm_add_ps(t, _mm_mul_ps(c1.m_value, _mm_shuffle_ps(rVec.m_value, rVec.m_value, _MM_SHUFFLE(1, 1, 1, 1))));
        t = _mm_add_ps(t, _mm_mul_ps(c2.m_value, _mm_shuffle_ps(rVec.m_value, rVec.m_value, _MM_SHUFFLE(2, 2, 2, 2))));
        t = Vec4Reg::FixW(t);
        return Vec4Reg(t).ToVec3();
#else
        return Vec3
        (
            m_columns[0].x * vec.x + m_columns[1].x * vec.y + m_columns[2].x * vec.z,
            m_columns[0].y * vec.x + m_columns[1].y * vec.y + m_columns[2].y * vec.z,
            m_columns[0].z * vec.x + m_columns[1].z * vec.y + m_columns[2].z * vec.z
        );
#endif
    }

    Mat33 Mat33::operator*(const float scalar) const
    {
        const Vec3 multiplier = Vec3::Replicate(scalar);

        Mat33 result;
        for (size_t col = 0; col < N; ++col)
        {
            result.m_columns[col] = m_columns[col] * multiplier;
        }
        return result;
    }

    Mat33 operator*(const float scalar, const Mat33& mat)
    {
        return mat * scalar;
    }

    Mat33& Mat33::operator*=(const float scalar)
    {
        for (size_t col = 0; col < N; ++col)
        {
            m_columns[col] *= scalar;
        }
        return *this;
    }

    Mat33 Mat33::operator+(const Mat33& other) const
    {
        Mat33 result;
        for (size_t col = 0; col < N; ++col)
        {
            result.m_columns[col] = m_columns[col] + other.m_columns[col];
        }
        return result;
    }

    Mat33& Mat33::operator+=(const Mat33& other)
    {
        for (size_t col = 0; col < N; ++col)
        {
            m_columns[col] += other.m_columns[col];
        }
        return *this;
    }

    Mat33 Mat33::operator-(const Mat33& other) const
    {
        Mat33 result;
        for (size_t col = 0; col < N; ++col)
        {
            result.m_columns[col] = m_columns[col] - other.m_columns[col];
        }
        return result;
    }

    Mat33& Mat33::operator-=(const Mat33& other)
    {
        for (size_t col = 0; col < N; ++col)
        {
            m_columns[col] -= other.m_columns[col];
        }
        return *this;
    }

    Mat33 Mat33::operator-() const
    {
        Mat33 result;
        for (size_t col = 0; col < N; ++col)
        {
            result.m_columns[col] = -m_columns[col];
        }
        return result;
    }

    void Mat33::SetAxisX(const Vec2 axis)
    {
        m_columns[0].x = axis.x;
        m_columns[0].y = axis.y;
    }

    void Mat33::SetAxisY(const Vec2 axis)
    {
        m_columns[1].x = axis.x;
        m_columns[1].y = axis.y;
    }

    void Mat33::SetColumn2(const size_t column, const Vec2 value)
    {
        NES_ASSERT(column < N);
        m_columns[column].x = value.x;
        m_columns[column].y = value.y;
    }

    void Mat33::SetColumn3(const size_t column, const Vec3 value)
    {
        NES_ASSERT(column < N);
        m_columns[column] = value;
    }

    Vec2 Mat33::GetRow2(const size_t row) const
    {
        NES_ASSERT(row < N);
        return Vec2(m_columns[0][row], m_columns[1][row]);
    }

    Vec3 Mat33::GetRow3(const size_t row) const
    {
        NES_ASSERT(row < N);
        return Vec3(m_columns[0][row], m_columns[1][row], m_columns[2][row]);
    }

    void Mat33::SetRow2(const size_t row, const Vec2 value)
    {
        NES_ASSERT(row < N);
        m_columns[0][row] = value.x;
        m_columns[1][row] = value.y;
    }

    void Mat33::SetRow3(const size_t row, const Vec3 value)
    {
        NES_ASSERT(row < N);
        m_columns[0][row] = value.x;
        m_columns[1][row] = value.y;
        m_columns[2][row] = value.z;
    }

    void Mat33::SetDiagonal2(const Vec2 diagonal)
    {
        m_columns[0][0] = diagonal.x;
        m_columns[1][1] = diagonal.y;
    }

    void Mat33::SetDiagonal3(const Vec3 diagonal)
    {
        m_columns[0][0] = diagonal.x;
        m_columns[1][1] = diagonal.y;
        m_columns[2][2] = diagonal.z;
    }

    bool Mat33::IsClose(const Mat33& other, const float maxSqrDist) const
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (!m_columns[i].IsClose(other.m_columns[i], maxSqrDist))
                return false;
        }

        return true;
    }

    Vec2 Mat33::Multiply2x2(const Vec2 vec) const
    {
        return *this * vec;
    }

    Vec2 Mat33::Multiply2x2Transposed(const Vec2 vec) const
    {
        return Transposed2x2() * vec;
    }

    Mat33 Mat33::Multiply2x2(const Mat33& other) const
    {
        // Check that the bottom row is zeroed out.
        NES_ASSERT(m_columns[0][2] == 0.f);
        NES_ASSERT(m_columns[1][2] == 0.f);

        Mat33 result;
    #if defined (NES_USE_SSE)
        const Vec4Reg c0 = Vec4Reg(m_columns[0]);
        const Vec4Reg c1 = Vec4Reg(m_columns[1]);
        
        for (int i = 0; i < 2; ++i)
        {
            const __m128 c = Vec4Reg(other.m_columns[i]).m_value;
            __m128 t = _mm_mul_ps(c0.m_value, _mm_shuffle_ps(c, c, _MM_SHUFFLE(0, 0, 0, 0)));
            t = _mm_add_ps(t, _mm_mul_ps(c1.m_value, _mm_shuffle_ps(c, c, _MM_SHUFFLE(1, 1, 1, 1))));
            result.m_columns[i] = Vec4Reg(t).ToVec3();
        }
    #else
        for (int i = 0; i < 2; ++i)
        {
            result.m_columns[i] = m_columns[0] * other.m_columns[i].x + m_columns[1] * other.m_columns[i].y;
        }
    #endif

        return result;
    }

    Mat33 Mat33::Multiply2x2LeftTransposed(const Mat33& other) const
    {
        // Transpose the left-hand side.
        const Mat33 transposed = Transposed2x2();

        // Do a 2x2 multiply
        Mat33 result;
        for (int i = 0; i < 2; ++i)
        {
            result.m_columns[i] = transposed.m_columns[0] * other.m_columns[i].x + transposed.m_columns[1] * other.m_columns[i].y;
        }
        result.m_columns[2] = Vec3(0.f, 0.f, 1.f);
        
        return result;
    }

    Mat33 Mat33::Multiply2x2RightTransposed(const Mat33& other) const
    {
        // Check that the bottom row is zeroed out.
        NES_ASSERT(m_columns[0][2] == 0.f);
        NES_ASSERT(m_columns[1][2] == 0.f);

        // Do a transposed 2x2 multiply
        Mat33 result;
        for (int i = 0; i < 2; ++i)
        {
            result.m_columns[i] = m_columns[0] * other.m_columns[0][i] + m_columns[1] * other.m_columns[1][i];
        }

        result.m_columns[2] = Vec3(0.f, 0.f, 1.f);
        return result;
    }

    Vec2 Mat33::TransformPoint(const Vec2 point) const
    {
        const Vec3 result = *this * Vec3(point.x, point.y, 1.f);
        return Vec2(result.x, result.y);
    }

    void Mat33::StoreFloat3x3(Float3* pOutFloats) const
    {
        for (size_t i = 0; i < N; ++i)
        {
            m_columns[i].StoreFloat3(pOutFloats + i);
        }
    }

    Mat33 Mat33::Transposed() const
    {
        Mat33 result;
        for (size_t col = 0; col < N; ++col)
        {
            for (size_t row = 0; row < N; ++row)
            {
                result.m_columns[col][row] = m_columns[row][col];
            }
        }
        return result;
    }

    Mat33 Mat33::Transposed2x2() const
    {
        Mat33 result;
        for (size_t col = 0; col < 2; ++col)
        {
            for (size_t row = 0; row < 2; ++row)
            {
                result.m_columns[col][row] = m_columns[row][col];
            }
            result.m_columns[col].z = 0.f;
        }
        result.m_columns[2] = Vec3(0.f, 0.f, 1.f);
        
        return result;
    }

    Mat33 Mat33::Inversed() const
    {
        // The Inverse of a Matrix is on page 168 of my Math Textbook:
        const float determinant = Determinant();
        NES_ASSERT(determinant != 0.f);
        
        const float invDeterminant = 1.0f / determinant;

        Mat33 result;
        result.m_columns[0][0] = invDeterminant * (m_columns[1][1] * m_columns[2][2] - m_columns[2][1] * m_columns[1][2]);
        result.m_columns[0][1] = invDeterminant * (m_columns[2][1] * m_columns[0][2] - m_columns[0][1] * m_columns[2][2]);
        result.m_columns[0][2] = invDeterminant * (m_columns[0][1] * m_columns[1][2] - m_columns[1][1] * m_columns[0][2]);
        
        result.m_columns[1][0] = invDeterminant * (m_columns[2][0] * m_columns[1][2] - m_columns[1][0] * m_columns[2][2]);
        result.m_columns[1][1] = invDeterminant * (m_columns[0][0] * m_columns[2][2] - m_columns[2][0] * m_columns[0][2]);
        result.m_columns[1][2] = invDeterminant * (m_columns[1][0] * m_columns[0][2] - m_columns[0][0] * m_columns[1][2]);
        
        result.m_columns[2][0] = invDeterminant * (m_columns[1][0] * m_columns[2][1] - m_columns[2][0] * m_columns[1][1]);
        result.m_columns[2][1] = invDeterminant * (m_columns[2][0] * m_columns[0][1] - m_columns[0][0] * m_columns[2][1]);
        result.m_columns[2][2] = invDeterminant * (m_columns[0][0] * m_columns[1][1] - m_columns[1][0] * m_columns[0][1]);
        return result;
    }

    Mat33 Mat33::Inversed2x2() const
    {
        // The Inverse of a Matrix is on page 168 of my Math Textbook:
        const float determinant = Determinant2x2();
        NES_ASSERT(determinant != 0.f);
        
        const float invDeterminant = 1.0f / determinant;

        Mat33 result;
        // The inverse determinant * the adjugate
        // Nice video explaining the process: https://www.youtube.com/watch?v=01c12NaUQDw
        result.m_columns[0][0] = invDeterminant * m_columns[1][1];
        result.m_columns[1][1] = invDeterminant * m_columns[0][0];
        result.m_columns[1][0] = invDeterminant * -m_columns[1][0]; 
        result.m_columns[0][1] = invDeterminant * -m_columns[0][1];

        result.m_columns[0][2] = 0.f;
        result.m_columns[1][2] = 0.f;
        result.m_columns[2] = Vec3(0.f, 0.f, 1.f);
        return result;
    }

    bool Mat33::SetInversed2x2(const Mat33& matrix)
    {
        const float det = matrix.Determinant2x2();

        // If the determinant is zero the matrix is singular and we return false.
        if (det == 0.f)
            return false;

        *this = matrix.Adjoint2x2();
        m_columns[0] /= det;
        m_columns[1] /= det;
        return true;
    }

    Mat33 Mat33::Adjoint2x2() const
    {
        return Mat33
        (
            Vec3(m_columns[1][1], -m_columns[0][1], 0.f),
            Vec3(-m_columns[1][0], m_columns[0][0], 0.f),
            Vec3(0.f, 0.f, 1.f)
        );
    }

    Mat33 Mat33::InversedRotationTranslation() const
    {
        Mat33 result = Transposed2x2();
        result.SetTranslation(-result.Multiply2x2(GetTranslation()));
        return result;
    }

    float Mat33::Determinant2x2() const
    {
        return (m_columns[0][0] * m_columns[1][1]) - (m_columns[1][0] * m_columns[0][1]);
    }

    float Mat33::Determinant() const
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
    
    Mat33 Mat33::GetRotation() const
    {
        // Make sure the bottom row is zeroed out.
        NES_ASSERT(m_columns[0][2] == 0.0f);
        NES_ASSERT(m_columns[1][2] == 0.0f);
        return Mat33(m_columns[0], m_columns[1], Vec3(0.f, 0.f, 1.f));
    }

    Mat33 Mat33::GetRotationSafe() const
    {
        return Mat33
        (
            Vec3(m_columns[0].x, m_columns[0].y, 0.f),
            Vec3(m_columns[1].x, m_columns[1].y, 0.f),
            Vec3(0, 0, 1.f)
        );
    }

    void Mat33::SetRotation(const Mat33& rotation)
    {
        m_columns[0] = rotation.m_columns[0];
        m_columns[1] = rotation.m_columns[1];
    }

    Vec2 Mat33::GetScale() const
    {
        Vec2 result;
        [[maybe_unused]] auto rotationTranslation = Decompose(result);
        return result;
    }

    Mat33 Mat33::PreTranslated(const Vec2 translation) const
    {
        return Mat33(m_columns[0], m_columns[1], Vec3(GetTranslation() + Multiply2x2(translation), 1.f));
    }

    Mat33 Mat33::PostTranslated(const Vec2 translation) const
    {
        return Mat33(m_columns[0], m_columns[1], Vec3(GetTranslation() + translation, 1.f));
    }

    Mat33 Mat33::PreScaled(const Vec2 scale) const
    {
        return Mat33
        (
            m_columns[0] * scale.x,
            m_columns[1] * scale.y,
            m_columns[2]
        );
    }

    Mat33 Mat33::PostScaled(const Vec2 scale) const
    {
        const Vec3 scale3(scale, 1.f);
        return Mat33
        (
            scale3 * m_columns[0],
            scale3 * m_columns[1],
            scale3 * m_columns[2]
        );
    }

    Mat33 Mat33::Decompose(Vec2& outScale) const
    {
        // Start the modified Gram-Schmidt algorithm
        // X axis will just be normalized
        const Vec2 x = GetAxisX();

        // Make Y axis perpendicular to X
        Vec2 y = GetAxisY();
        const float xDotX = x.LengthSqr();
        y -= (x.Dot(y) / xDotX) * x;
        const float yDotY = y.LengthSqr();

        // Determine Scale
        outScale = Vec2(xDotX, yDotY).Sqrt();

        // Determine the rotation and translation
        return Mat33
        (
            Vec3(x / outScale.x, 0.f),
            Vec3(y / outScale.y, 0.f),
            GetColumn3(2)
        );
    }

    void Mat33::Decompose(Vec2& outTranslation, float& outRotation, Vec2& outScale) const
    {
        const Mat33 rotationTranslation = Decompose(outScale);
        outTranslation = rotationTranslation.GetTranslation();
        outRotation = math::ACos(m_columns[0][0]);
    }

    Mat33 Mat33::Identity()
    {
        return Mat33
        (
            Vec3(1.f, 0.f, 0.f),
            Vec3(0.f, 1.f, 0.f),
            Vec3(0.f, 0.f, 1.f)
        );
    }

    Mat33 Mat33::Zero()
    {
        return Mat33(Vec3::Zero(), Vec3::Zero(), Vec3::Zero());
    }

    Mat33 Mat33::NaN()
    {
        return Mat33(Vec3::NaN(), Vec3::NaN(), Vec3::NaN());
    }

    Mat33 Mat33::LoadFloat3x3(const Float3* pFloats)
    {
        Mat33 result;
        for (size_t col = 0; col < N; ++col)
        {
            result.m_columns[col] = Vec3::LoadFloat3Unsafe(*(pFloats + col));
        }
        return result;
    }

    Mat33 Mat33::LoadFloat3x3Aligned(const Float3* pFloats)
    {
        Mat33 result;
        for (size_t col = 0; col < N; ++col)
        {
            result.m_columns[col] = Vec3::LoadFloat3Unsafe(*(pFloats + col));
        }
        return result;
    }

    Mat33 Mat33::MakeRotation(const float angleRadians)
    {
        return Mat33
        (
            Vec3(math::Cos(angleRadians), -math::Sin(angleRadians), 0.f),
            Vec3(math::Sin(angleRadians), math::Cos(angleRadians), 0.f),
            Vec3(0.f, 0.f, 1.f)
        );
    }

    Mat33 Mat33::MakeTranslation(const Vec2 translation)
    {
        return Mat33
        (
            Vec3(1.f, 0.f, 0.f),
            Vec3(0.f, 1.f, 0.f),
            Vec3(translation, 1.f)
        );
    }

    Mat33 Mat33::MakeRotationTranslation(const float rotation, const Vec2 translation)
    {
        Mat33 result = Mat33::MakeRotation(rotation);
        result.SetTranslation(translation);
        return result;
    }

    Mat33 Mat33::MakeInverseRotationTranslation(const float rotation, const Vec2 translation)
    {
        Mat33 result = Mat33::MakeRotation(-rotation);
        result.SetTranslation(-result.Multiply2x2(translation));
        return result;
    }

    Mat33 Mat33::MakeScale(const float scale)
    {
        return Mat33(Vec2(scale, scale));
    }

    Mat33 Mat33::MakeScale(const Vec2 scale)
    {
        return Mat33(Vec2(scale));
    }

    Mat33 Mat33::ComposeTransform(const Vec2 translation, const float rotation, const Vec2 scale)
    {
        const Mat33 scaleM = MakeScale(scale);
        return MakeRotationTranslation(rotation, translation) * scaleM;
    }
}