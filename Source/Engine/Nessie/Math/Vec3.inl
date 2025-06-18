// Vec3.inl
#pragma once
#include "Math/Generic.h"
#include "Math/Vec2.h"
#include "Math/Vec4.h"
#include "Math/SIMD/Vec4Reg.h"

namespace nes
{
    Vec3::Vec3(const Vec2 vec, float z)
        : x(vec.x)
        , y(vec.y)
        , z(z)
    {
        //
    }

    Vec3::Vec3(const Vec4 vec)
        : x(vec.x)
        , y(vec.y)
        , z(vec.z)
    {
        //
    }

    Vec3::Vec3(const Float3& value)
        : x(value.x)
        , y(value.y)
        , z(value.z)
    {
        //
    }

    Vec3::Vec3(const float uniformValue)
    {
        *this = Replicate(uniformValue);
    }

    Vec3::Vec3(const float x, const float y, const float z)
        : x(x)
        , y(y)
        , z(z)
    {
        //
    }

    bool Vec3::operator==(const Vec3& other) const
    {
        return Equals(*this, other).TestAllXYZTrue();
    }

    Vec3 Vec3::operator*(const Vec3& other) const
    {
        return (Vec4Reg::LoadVec3Unsafe(*this) * Vec4Reg::LoadVec3Unsafe(other)).ToVec3();
    }

    Vec3& Vec3::operator*=(const Vec3& other)
    {
        *this = (Vec4Reg::LoadVec3Unsafe(*this) * Vec4Reg::LoadVec3Unsafe(other)).ToVec3();
        return *this;
    }

    Vec3 Vec3::operator*(const float value) const
    {
        return (Vec4Reg::LoadVec3Unsafe(*this) * Vec4Reg::Replicate(value)).ToVec3();
    }

    Vec3& Vec3::operator*=(const float value)
    {
        *this = (Vec4Reg::LoadVec3Unsafe(*this) * Vec4Reg::Replicate(value)).ToVec3();
        return *this;
    }

    inline Vec3 operator*(const float value, const Vec3& vec)
    {
        return vec * value;
    }

    Vec3 Vec3::operator/(const Vec3& other) const
    {
        return (Vec4Reg::LoadVec3Unsafe(*this) / Vec4Reg::LoadVec3Unsafe(other)).ToVec3(); 
    }

    Vec3& Vec3::operator/=(const Vec3& other)
    {
        *this = (Vec4Reg::LoadVec3Unsafe(*this) / Vec4Reg::LoadVec3Unsafe(other)).ToVec3();
        return *this;
    }

    Vec3 Vec3::operator/(const float value) const
    {
        return (Vec4Reg::LoadVec3Unsafe(*this) / Vec4Reg::Replicate(value)).ToVec3();
    }

    Vec3& Vec3::operator/=(const float value)
    {
        *this = (Vec4Reg::LoadVec3Unsafe(*this) / Vec4Reg::Replicate(value)).ToVec3();
        return *this;
    }

    Vec3 Vec3::operator+(const Vec3& other) const
    {
        return (Vec4Reg::LoadVec3Unsafe(*this) + Vec4Reg::LoadVec3Unsafe(other)).ToVec3();
    }

    Vec3& Vec3::operator+=(const Vec3& other)
    {
        *this = (Vec4Reg::LoadVec3Unsafe(*this) + Vec4Reg::LoadVec3Unsafe(other)).ToVec3();
        return *this;
    }

    Vec3 Vec3::operator-(const Vec3& other) const
    {
        return (Vec4Reg::LoadVec3Unsafe(*this) - Vec4Reg::LoadVec3Unsafe(other)).ToVec3();
    }

    Vec3& Vec3::operator-=(const Vec3& other)
    {
        *this = (Vec4Reg::LoadVec3Unsafe(*this) - Vec4Reg::LoadVec3Unsafe(other)).ToVec3();
        return *this;
    }

    Vec3 Vec3::operator-() const
    {
        return (-Vec4Reg::LoadVec3Unsafe(*this)).ToVec3();
    }

    Vec3 Vec3::Replicate(const float value)
    {
        return Vec4Reg::Replicate(value).ToVec3();
    }

    Vec3 Vec3::LoadFloat3Unsafe(const Float3& value)
    {    
        return Vec4Reg::LoadFloat3Unsafe(value).ToVec3();
    }
    
    Vec3 Vec3::Min(const Vec3& left, const Vec3& right)
    {
        return Vec4Reg::Min(Vec4Reg::LoadVec3Unsafe(left), Vec4Reg::LoadVec3Unsafe(right)).ToVec3(); 
    }

    Vec3 Vec3::Max(const Vec3& left, const Vec3& right)
    {
        return Vec4Reg::Max(Vec4Reg::LoadVec3Unsafe(left), Vec4Reg::LoadVec3Unsafe(right)).ToVec3(); 
    }

    Vec3 Vec3::Clamp(const Vec3& value, const Vec3& min, const Vec3& max)
    {
        return Max(Min(value, max), min);
    }
    
    UVec4Reg Vec3::Equals(const Vec3& left, const Vec3& right)
    {
        return Vec4Reg::Equals(Vec4Reg::LoadVec3Unsafe(left), Vec4Reg::LoadVec3Unsafe(right)); 
    }

    UVec4Reg Vec3::Less(const Vec3& left, const Vec3& right)
    {
        return Vec4Reg::Less(Vec4Reg::LoadVec3Unsafe(left), Vec4Reg::LoadVec3Unsafe(right)); 
    }

    UVec4Reg Vec3::LessOrEqual(const Vec3& left, const Vec3& right)
    {
        return Vec4Reg::LessOrEqual(Vec4Reg::LoadVec3Unsafe(left), Vec4Reg::LoadVec3Unsafe(right)); 
    }

    UVec4Reg Vec3::Greater(const Vec3& left, const Vec3& right)
    {
        return Vec4Reg::Greater(Vec4Reg::LoadVec3Unsafe(left), Vec4Reg::LoadVec3Unsafe(right));   
    }

    UVec4Reg Vec3::GreaterOrEqual(const Vec3& left, const Vec3& right)
    {
        return Vec4Reg::GreaterOrEqual(Vec4Reg::LoadVec3Unsafe(left), Vec4Reg::LoadVec3Unsafe(right));   
    }

    Vec3 Vec3::FusedMultiplyAdd(const Vec3& mul1, const Vec3& mul2, const Vec3& add)
    {
        return Vec4Reg::FusedMultiplyAdd(Vec4Reg::LoadVec3Unsafe(mul1), Vec4Reg::LoadVec3Unsafe(mul2), Vec4Reg::LoadVec3Unsafe(add)).ToVec3();   
    }

    Vec3 Vec3::Select(const Vec3& notSet, const Vec3& set, const UVec4Reg& mask)
    {
        return Vec4Reg::Select(Vec4Reg::LoadVec3Unsafe(notSet), Vec4Reg::LoadVec3Unsafe(set), mask).ToVec3();
    }

    Vec3 Vec3::Or(const Vec3& left, const Vec3& right)
    {
        return Vec4Reg::Or(Vec4Reg::LoadVec3Unsafe(left), Vec4Reg::LoadVec3Unsafe(right)).ToVec3();  
    }

    Vec3 Vec3::Xor(const Vec3& left, const Vec3& right)
    {
        return Vec4Reg::Xor(Vec4Reg::LoadVec3Unsafe(left), Vec4Reg::LoadVec3Unsafe(right)).ToVec3();  
    }

    Vec3 Vec3::And(const Vec3& left, const Vec3& right)
    {
        return Vec4Reg::And(Vec4Reg::LoadVec3Unsafe(left), Vec4Reg::LoadVec3Unsafe(right)).ToVec3();  
    }

    Vec3 Vec3::Lerp(const Vec3& from, const Vec3& to, const float t)
    {
        const Vec4Reg fromR = Vec4Reg::LoadVec3Unsafe(from);
        const Vec4Reg toR = Vec4Reg::LoadVec3Unsafe(to);
        return (fromR + ((toR - fromR) * t)).ToVec3();
    }

    Vec3 Vec3::UnitSpherical(const float theta, const float phi)
    {
        Vec4Reg sin, cos;
        Vec4Reg(theta, phi, 0.f, 0.f).SinCos(sin, cos);
        return Vec3(sin.m_f32[0] * cos.m_f32[1], sin.m_f32[0] * sin.m_f32[1], cos.m_f32[0]);
    }

    float Vec3::AngleBetween(const Vec3& a, const Vec3& b)
    {
        return math::ACos(a.Normalized().Dot(b.Normalized()));
    }

    float Vec3::AngleBetweenDegrees(const Vec3& a, const Vec3& b)
    {
        const float angleRadians = AngleBetween(a, b);
        return math::ToDegrees(angleRadians);
    }

    bool Vec3::IsClose(const Vec3& other, const float maxDistSqr) const
    {
        return (other - *this).LengthSqr() <= maxDistSqr;
    }

    bool Vec3::IsNearZero(const float maxDistSqr) const
    {
        return LengthSqr() <= maxDistSqr;    
    }

    bool Vec3::IsNormalized(const float tolerance) const
    {
        return math::Abs(LengthSqr() - 1.0f) <= tolerance;
    }

    bool Vec3::IsNaN() const
    {
        return Vec4Reg(*this).IsNaN();
    }

    Vec4Reg Vec3::SplatX() const
    {
        return Vec4Reg::Replicate(x);
    }
    
    Vec4Reg Vec3::SplatY() const
    {
        return Vec4Reg::Replicate(y);
    }

    Vec4Reg Vec3::SplatZ() const
    {
        return Vec4Reg::Replicate(z);
    }

    template <uint32 SwizzleX, uint32 SwizzleY, uint32 SwizzleZ>
    Vec3 Vec3::Swizzle() const
    {
        static_assert(SwizzleX <= 3, "SwizzleX out of range!");
        static_assert(SwizzleY <= 3, "SwizzleY out of range!");
        static_assert(SwizzleZ <= 3, "SwizzleZ out of range!");
        
        return Vec4Reg::LoadVec3Unsafe(*this).Swizzle<SwizzleX, SwizzleY, SwizzleZ, SwizzleZ>().ToVec3();
    }

    Vec3 Vec3::Abs() const
    {
        return Vec4Reg::LoadVec3Unsafe(*this).Abs().ToVec3();
    }

    Vec3 Vec3::Reciprocal() const
    {
        return Vec4Reg::LoadVec3Unsafe(*this).Reciprocal().ToVec3();
    }

    Vec3 Vec3::DotV(const Vec3& other) const
    {
        return Vec4Reg(*this, 0.f).DotV(Vec4Reg(other, 0.f)).ToVec3();
    }

    Vec4Reg Vec3::DotV4(const Vec3& other) const
    {
        return Vec4Reg(*this, 0.f).DotV(Vec4Reg(other, 0.f));
    }

    float Vec3::Dot(const Vec3& other) const
    {
        return x * other.x + y * other.y + z * other.z;
        //return Vec4Reg(*this).Dot(Vec4Reg(other));
    }

    Vec3 Vec3::Cross(const Vec3& other) const
    {
        return Vec4Reg::Cross3(*this, other);
    }

    float Vec3::LengthSqr() const
    {
        return Vec4Reg::LengthSqr3(*this);
    }

    float Vec3::Length() const
    {
        return Vec4Reg::Length3(*this);
    }

    Vec3& Vec3::Normalize()
    {
        *this = *this / Length();
        return *this;
    }
    
    Vec3 Vec3::Normalized() const
    {
        return *this / Length();
    }

    Vec3 Vec3::NormalizedOr(const Vec3& zeroValue) const
    {
        return Vec4Reg::NormalizedOr3(*this, zeroValue);
    }

    void Vec3::StoreFloat3(Float3* pOutFloats) const
    {
        pOutFloats->x = x;
        pOutFloats->y = y;
        pOutFloats->z = z;
    }

    float Vec3::MinComponent() const
    {
        const Vec4Reg result(*this);
        return result.MinComponent();
    }

    float Vec3::MaxComponent() const
    {
        const Vec4Reg result(*this);
        return result.MaxComponent();
    }

    int Vec3::MinComponentIndex() const
    {
        return x < y ? (z < x ? 2 : 0) : (z < y ? 2 : 1);
    }

    int Vec3::MaxComponentIndex() const
    {
        return x > y ? (z > x ? 2 : 0) : (z > y ? 2 : 1);
    }

    Vec3 Vec3::Sqrt() const
    {
        return Vec4Reg::LoadVec3Unsafe(*this).Sqrt().ToVec3();
    }

    Vec3 Vec3::GetSign() const
    {
        return Vec4Reg::LoadVec3Unsafe(*this).GetSign().ToVec3();
    }

    Vec3 Vec3::NormalizedPerpendicular() const
    {
        if (math::Abs(x) > math::Abs(y))
        {
            const float length = std::sqrt(x * x + z * z);
            return Vec3(z, 0.f, -x) / length;
        }

        const float length = std::sqrt(y * y + z * z);
        return Vec3(0.f, z, -y) / length;
    }
}