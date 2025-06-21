// Mat22.inl
#pragma once
#include "Vec2.h"

namespace nes
{
    inline Mat22::Mat22(const Vec2 c1, const Vec2 c2)
        : m_columns { c1, c2 }
    {
        //
    }

    inline Mat22::Mat22(const Vec2 diagonal)
    {
        m_columns[0] = Vec2(diagonal.x, 0.f);
        m_columns[1] = Vec2(0.f, diagonal.y);
    }

    inline Mat22::Mat22(const float uniformDiagonal)
    {
        m_columns[0] = Vec2(uniformDiagonal, 0.f);
        m_columns[1] = Vec2(0.f,  uniformDiagonal);
    }

    inline Vec2 Mat22::operator[](const size_t index) const
    {
        NES_ASSERT(index < N);
        return m_columns[index];        
    }

    inline Vec2& Mat22::operator[](const size_t index)
    {
        NES_ASSERT(index < N);
        return m_columns[index];
    }

    inline bool Mat22::operator==(const Mat22& other) const
    {
        for (size_t i = 0; i < N; i++)
        {
            if (m_columns[i] != other.m_columns[i])
                return false;
        }
        
        return true;
    }

    inline Mat22 Mat22::operator*(const Mat22& other) const
    {
        Mat22 result;

        const Vec2 a0 = m_columns[0];
        const Vec2 a1 = m_columns[1];
        
        const Vec2 b0 = other.m_columns[0];
        const Vec2 b1 = other.m_columns[1];

        result[0] = (a0 * b0[0]) + (a1 * b0[1]); 
        result[1] = (a0 * b1[0]) + (a1 * b1[1]); 

        return result;
    }

    inline Vec2 Mat22::operator*(const Vec2 other) const
    {
        Vec2 result;
        result[0] = m_columns[0][0] * other[0] + m_columns[1][0] * other[1];
        result[1] = m_columns[0][1] * other[0] + m_columns[1][1] * other[1];
        return result;
    }

    inline Mat22 Mat22::operator*(const float scalar) const
    {
        return Mat22
        (
            m_columns[0] * scalar,
            m_columns[1] * scalar
        );
    }

    inline Mat22 operator*(const float scalar, const Mat22& mat)
    {
        return mat * scalar;
    }

    inline Mat22& Mat22::operator*=(const float scalar)
    {
        m_columns[0] *= scalar;
        m_columns[1] *= scalar;
        return *this;
    }

    inline Mat22 Mat22::operator+(const Mat22& other) const
    {
        return Mat22
        (
            m_columns[0] + other.m_columns[0],
            m_columns[1] + other.m_columns[1]
        );
    }

    inline Mat22& Mat22::operator+=(const Mat22& other)
    {
        m_columns[0] += other.m_columns[0];
        m_columns[1] += other.m_columns[1];
        return *this;
    }

    inline Mat22 Mat22::operator-(const Mat22& other) const
    {
        return Mat22
        (
            m_columns[0] - other.m_columns[0],
            m_columns[1] - other.m_columns[1]
        );
    }

    inline Mat22& Mat22::operator-=(const Mat22& other)
    {
        m_columns[0] -= other.m_columns[0];
        m_columns[1] -= other.m_columns[1];
        return *this;
    }

    inline Mat22 Mat22::operator-() const
    {
        return Mat22
        (
            -m_columns[0],
            -m_columns[1]
        );
    }

    inline Vec2 Mat22::GetRow(const size_t row) const
    {
        NES_ASSERT(row < N);
        return Vec2(m_columns[0][row], m_columns[1][row]);
    }

    inline void Mat22::SetRow(const size_t row, const Vec2 value)
    {
        NES_ASSERT(row < N);
        m_columns[0][row] = value.x;
        m_columns[1][row] = value.y;
    }

    inline Vec2 Mat22::GetDiagonal() const
    {
        return Vec2(m_columns[0][0], m_columns[1][1]);
    }

    inline void Mat22::SetDiagonal(const Vec2 diagonal)
    {
        m_columns[0][0] = diagonal.x;
        m_columns[1][1] = diagonal.y;
    }

    inline void Mat22::SetDiagonal(const float uniformDiagonal)
    {
        m_columns[0][0] = uniformDiagonal;
        m_columns[1][1] = uniformDiagonal;
    }

    inline void Mat22::SetZero()
    {
        m_columns[0] = Vec2(0.f, 0.f);
        m_columns[1] = Vec2(0.f, 0.f);
    }

    inline bool Mat22::IsZero() const
    {
        for (size_t i = 0; i < N; i++)
        {
            if (!m_columns[i].IsNearZero(0.f))
                return false;
        }

        return true;
    }

    inline void Mat22::SetIdentity()
    {
        m_columns[0] = Vec2(1.f, 0.f);
        m_columns[1] = Vec2(0.f, 1.f);
    }

    inline bool Mat22::IsIdentity() const
    {
        return *this == Mat22::Identity();
    }

    inline Mat22 Mat22::Transposed() const
    {
        return Mat22
        (
            Vec2(m_columns[0][0], m_columns[1][0]),
            Vec2(m_columns[0][1], m_columns[1][1])
        );
    }

    inline Mat22 Mat22::Inversed() const
    {
        const float determinant = Determinant();
        NES_ASSERT(determinant != 0.f);

        return Mat22
        (
            Vec2(m_columns[1][1], -m_columns[0][1]) / determinant,
            Vec2(-m_columns[1][0] , m_columns[0][0]) / determinant
        );
    }

    inline bool Mat22::SetInversed(const Mat22& m)
    {
        const float determinant = m.Determinant();
        if (determinant == 0.f)
            return false;

        m_columns[0] = Vec2(m.m_columns[1][1], -m.m_columns[0][1]) / determinant;
        m_columns[1] = Vec2(-m.m_columns[1][0] , m.m_columns[0][0]) / determinant;

        return true;
    }

    inline float Mat22::Determinant() const
    {
        return m_columns[0][0] * m_columns[1][1] - m_columns[1][0] * m_columns[0][1];   
    }

    inline Mat22 Mat22::Identity()
    {
        return Mat22
        (
            Vec2(1.f, 0.f),
            Vec2(0.f, 1.f)
        );
    }

    inline Mat22 Mat22::Zero()
    {
        return Mat22
        (
            Vec2::Zero(),
            Vec2::Zero()
        );
    }

    inline Mat22 Mat22::NaN()
    {
        return Mat22
        (
            Vec2::NaN(),
            Vec2::NaN()
        );
    }
}