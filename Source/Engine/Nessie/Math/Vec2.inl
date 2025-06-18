// Vec2.inl
#pragma once
#include "Math/SIMD/Vec4Reg.h"

namespace nes
{
    Vec2 Vec2::operator+(const Vec2 other) const
    {
        return { x + other.x, y + other.y };
    }

    Vec2& Vec2::operator+=(const Vec2 other)
    {
        return *this = *this + other;
    }

    Vec2 Vec2::operator-(const Vec2 other) const
    {
        return { x - other.x, y - other.y };
    }

    Vec2& Vec2::operator-=(const Vec2 other)
    {
        return *this = *this - other;   
    }

    Vec2 Vec2::operator*(const Vec2 other) const
    {
        return { x * other.x, y * other.y };
    }

    inline Vec2 operator*(const float scalar, const Vec2 vec)
    {
        return Vec2( vec.x * scalar, vec.y * scalar);
    }

    Vec2& Vec2::operator*=(const Vec2 other)
    {
        return *this = *this * other;  
    }

    Vec2 Vec2::operator/(const Vec2 other) const
    {
        return { x / other.x, y / other.y };
    }

    Vec2& Vec2::operator/=(const Vec2 other)
    {
        return *this = *this / other;
    }

    Vec2 Vec2::operator*(const float scalar) const
    {
        return { x * scalar, y * scalar };   
    }

    Vec2& Vec2::operator*=(const float scalar)
    {
        return *this = *this * scalar; 
    }

    Vec2 Vec2::operator/(const float scalar) const
    {
        return { x / scalar, y / scalar };   
    }

    Vec2& Vec2::operator/=(const float scalar)
    {
        return *this = *this / scalar; 
    }

    bool Vec2::IsClose(const Vec2& other, const float maxDistSqr) const
    {
        return (other - *this).LengthSqr() <= maxDistSqr;
    }
    
    bool Vec2::IsNearZero(const float maxDistSqr) const
    {
        return LengthSqr() <= maxDistSqr;
    }

    bool Vec2::IsNormalized(const float tolerance) const
    {
        return math::Abs(LengthSqr() - 1.0f) <= tolerance;
    }

    bool Vec2::IsNaN() const
    {
        return math::IsNan(x) || math::IsNan(y);
    }

    Vec4Reg Vec2::SplatX() const
    {
        return Vec4Reg::Replicate(x);
    }

    Vec4Reg Vec2::SplatY() const
    {
        return Vec4Reg::Replicate(y);
    }

    template <uint32 SwizzleX, uint32 SwizzleY>
    Vec2 Vec2::Swizzle() const
    {
        return Vec2((*this)[SwizzleX], (*this)[SwizzleY]);
    }

    Vec2 Vec2::Abs() const
    {
        return { math::Abs(x), math::Abs(y) };
    }

    Vec2 Vec2::Reciprocal() const
    {
        return { 1.0f / x, 1.0f / y };
    }

    Vec4Reg Vec2::DotV4(const Vec2& other) const
    {
        const float dot = Dot(other);
        return Vec4Reg::Replicate(dot);
    }

    float Vec2::Dot(const Vec2& other) const
    {
        return x * other.x + y * other.y;
    }

    float Vec2::LengthSqr() const
    {
        return x * x + y * y;
    }

    float Vec2::Length() const
    {
        return std::sqrt(LengthSqr());
    }

    Vec2& Vec2::Normalize()
    {
        return *this = *this / Length();
    }

    Vec2 Vec2::Normalized() const
    {
        return *this / Length();
    }

    Vec2 Vec2::NormalizedOr(const Vec2& zeroValue) const
    {
        return IsNearZero() ? zeroValue : *this / Length();
    }

    Vec2 Vec2::Perpendicular() const
    {
        return Vec2(-y, x);
    }

    Vec2 Vec2::NormalizedPerpendicular() const
    {
        return Perpendicular().Normalized();
    }

    Vec2& Vec2::Rotate(const float angle)
    {
        const float cos = math::Cos(angle);
        const float sin = math::Sin(angle);
        return *this = Vec2(cos * x - sin * y, sin * x + cos * y);
    }

    Vec2 Vec2::Rotated(const float angle) const
    {
        const float cos = math::Cos(angle);
        const float sin = math::Sin(angle);
        return Vec2(cos * x - sin * y, sin * x + cos * y);
    }

    void Vec2::StoreFloat2(Float2* pOutFloats) const
    {
        pOutFloats->x = x;
        pOutFloats->y = y;
    }

    float Vec2::MinComponent() const
    {
        return math::Min(x, y);
    }

    float Vec2::MaxComponent() const
    {
        return math::Max(x, y);
    }

    int Vec2::MinComponentIndex() const
    {
        return x < y ? 0 : 1;
    }

    int Vec2::MaxComponentIndex() const
    {
        return x > y ? 0 : 1;
    }

    Vec2 Vec2::Sqrt() const
    {
        return { std::sqrt(x), std::sqrt(y) };
    }

    Vec2 Vec2::GetSign() const
    {
        return Vec2
        (
            std::signbit(x) ? -1.0f : 1.0f,
            std::signbit(y) ? -1.0f : 1.0f
        );
    }

    Vec2 Vec2::Min(const Vec2 a, const Vec2 b)
    {
        return Vec2(math::Min(a.x, b.x), math::Min(a.y, b.y));
    }

    Vec2 Vec2::Max(const Vec2 a, const Vec2 b)
    {
        return Vec2(math::Max(a.x, b.x), math::Max(a.y, b.y));
    }

    Vec2 Vec2::FromAngle(const float radians)
    {
        return Vec2(math::Cos(radians), math::Sin(radians));
    }

    Vec2 Vec2::Lerp(const Vec2 from, const Vec2 to, const float t)
    {
        return from + (to - from) * t;
    }
}
