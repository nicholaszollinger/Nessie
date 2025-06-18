// Vec4.inl
#pragma once
#include "Math/Vec3.h"
#include "Math/SIMD/UVec4Reg.h"

namespace nes
{
    Vec4::Vec4(const Vec3 vec)
        : x(vec.x)
        , y(vec.y)
        , z(vec.z)
        , w(0.f)
    {
        //
    }

    Vec4::Vec4(const Vec3 vec, const float w)
        : x(vec.x)
        , y(vec.y)
        , z(vec.z)
        , w(w)
    {
        //
    }

    Vec4::Vec4(const float uniformValue)
    {
        *this = Replicate(uniformValue);
    }

    Vec4::Vec4(const float x, const float y, const float z, const float w)
        : x(x)
        , y(y)
        , z(z)
        , w(w)
    {
        //
    }

    Vec4::Vec4(const Float4& value)
        : x(value.x)
        , y(value.y)
        , z(value.z)
        , w(value.w)
    {
        //
    }

    bool Vec4::operator==(const Vec4& other) const
    {
        return Vec4::Equals(*this, other).TestAllTrue();
    }
    
    Vec4 Vec4::operator*(const Vec4& other) const
    {
        return (Vec4Reg::LoadVec4(this) * Vec4Reg::LoadVec4(&other)).ToVec4();
    }

    Vec4& Vec4::operator*=(const Vec4& other)
    {
        (Vec4Reg::LoadVec4(this) * Vec4Reg::LoadVec4(&other)).StoreVec4(this);
        return *this;
    }

    Vec4 Vec4::operator*(const float value) const
    {
        return (Vec4Reg::LoadVec4(this) * Vec4Reg::Replicate(value)).ToVec4();
    }

    Vec4& Vec4::operator*=(const float value)
    {
        (Vec4Reg::LoadVec4(this) * Vec4Reg::Replicate(value)).StoreVec4(this);
        return *this;
    }

    inline Vec4 operator*(const float value, const Vec4& vec)
    {
        return (Vec4Reg::LoadVec4(&vec) * Vec4Reg::Replicate(value)).ToVec4();
    }

    Vec4 Vec4::operator/(const Vec4& other) const
    {
        return (Vec4Reg::LoadVec4(this) / Vec4Reg::LoadVec4(&other)).ToVec4();
    }

    Vec4& Vec4::operator/=(const Vec4& other)
    {
        (Vec4Reg::LoadVec4(this) / Vec4Reg::LoadVec4(&other)).StoreVec4(this);
        return *this;
    }

    Vec4 Vec4::operator/(const float value) const
    {
        return (Vec4Reg::LoadVec4(this) / Vec4Reg::Replicate(value)).ToVec4();
    }

    Vec4& Vec4::operator/=(const float value)
    {
        (Vec4Reg::LoadVec4(this) / Vec4Reg::Replicate(value)).StoreVec4(this);
        return *this;
    }

    Vec4 Vec4::operator+(const Vec4& other) const
    {
        return (Vec4Reg::LoadVec4(this) + Vec4Reg::LoadVec4(&other)).ToVec4();
    }

    Vec4& Vec4::operator+=(const Vec4& other)
    {
        (Vec4Reg::LoadVec4(this) + Vec4Reg::LoadVec4(&other)).StoreVec4(this);
        return *this;
    }

    Vec4 Vec4::operator-(const Vec4& other) const
    {
        return (Vec4Reg::LoadVec4(this) - Vec4Reg::LoadVec4(&other)).ToVec4();
    }

    Vec4& Vec4::operator-=(const Vec4& other)
    {
        (Vec4Reg::LoadVec4(this) - Vec4Reg::LoadVec4(&other)).StoreVec4(this);
        return *this;
    }

    Vec4 Vec4::operator-() const
    {
        return Vec4(-x, -y, -z, -w);
    }

    Vec4 Vec4::Zero()
    {
        return Vec4Reg::Zero().ToVec4(); 
    }

    Vec4 Vec4::One()
    {
        return Replicate(1.f);
    }

    Vec4 Vec4::NaN()
    {
        return Replicate(std::numeric_limits<float>::quiet_NaN());
    }

    Vec4 Vec4::Replicate(const float value)
    {
        return Vec4Reg::Replicate(value).ToVec4();
    }

    Vec4 Vec4::LoadFloat4(const Float4* pFloats)
    {
        return Vec4Reg::LoadFloat4(pFloats).ToVec4();
    }

    Vec4 Vec4::LoadFloat4Aligned(const Float4* pFloats)
    {
        return Vec4Reg::LoadFloat4Aligned(pFloats).ToVec4();
    }

    template <const int Scale>
    Vec4 Vec4::GatherFloat4(const float* pBase, const UVec4Reg& offsets)
    {
        return Vec4Reg::GatherFloat4<Scale>(pBase, offsets).ToVec4();
    }
    
    Vec4 Vec4::Min(const Vec4& left, const Vec4& right)
    {
        return Vec4Reg::Min(Vec4Reg::LoadVec4(&left), Vec4Reg::LoadVec4(&right)).ToVec4();
    }

    Vec4 Vec4::Max(const Vec4& left, const Vec4& right)
    {
        return Vec4Reg::Max(Vec4Reg::LoadVec4(&left), Vec4Reg::LoadVec4(&right)).ToVec4();
    }

    UVec4Reg Vec4::Equals(const Vec4& left, const Vec4& right)
    {
        return Vec4Reg::Equals(Vec4Reg::LoadVec4(&left), Vec4Reg::LoadVec4(&right));  
    }

    UVec4Reg Vec4::Less(const Vec4& left, const Vec4& right)
    {
        return Vec4Reg::Less(Vec4Reg::LoadVec4(&left), Vec4Reg::LoadVec4(&right));
    }

    UVec4Reg Vec4::LessOrEqual(const Vec4& left, const Vec4& right)
    {
        return Vec4Reg::LessOrEqual(Vec4Reg::LoadVec4(&left), Vec4Reg::LoadVec4(&right));  
    }

    UVec4Reg Vec4::Greater(const Vec4& left, const Vec4& right)
    {
        return Vec4Reg::Greater(Vec4Reg::LoadVec4(&left), Vec4Reg::LoadVec4(&right));  
    }
    
    UVec4Reg Vec4::GreaterOrEqual(const Vec4& left, const Vec4& right)
    {
        return Vec4Reg::GreaterOrEqual(Vec4Reg::LoadVec4(&left), Vec4Reg::LoadVec4(&right));  
    }

    Vec4 Vec4::FusedMultiplyAdd(const Vec4& mul1, const Vec4& mul2, const Vec4& add)
    {
        return Vec4Reg::FusedMultiplyAdd(Vec4Reg::LoadVec4(&mul1), Vec4Reg::LoadVec4(&mul2), Vec4Reg::LoadVec4(&add)).ToVec4();
    }

    Vec4 Vec4::Select(const Vec4 notSet, const Vec4 set, const UVec4Reg mask)
    {
        return Vec4Reg::Select(Vec4Reg::LoadVec4(&notSet), Vec4Reg::LoadVec4(&set), mask).ToVec4();
    }

    Vec4 Vec4::Or(const Vec4 left, const Vec4 right)
    {
        return Vec4Reg::Or(Vec4Reg::LoadVec4(&left), Vec4Reg::LoadVec4(&right)).ToVec4();  
    }

    Vec4 Vec4::Xor(const Vec4 left, const Vec4 right)
    {
        return Vec4Reg::Xor(Vec4Reg::LoadVec4(&left), Vec4Reg::LoadVec4(&right)).ToVec4();
    }

    Vec4 Vec4::And(const Vec4 left, const Vec4 right)
    {
        return Vec4Reg::And(Vec4Reg::LoadVec4(&left), Vec4Reg::LoadVec4(&right)).ToVec4();  
    }

    void Vec4::Sort4(Vec4& value, UVec4Reg& index)
    {
        Vec4Reg reg = Vec4Reg::LoadVec4(&value);
        Vec4Reg::Sort4(reg, index);
        value = reg.ToVec4();
    }

    void Vec4::Sort4Reverse(Vec4& value, UVec4Reg& index)
    {
        Vec4Reg reg = Vec4Reg::LoadVec4(&value);
        Vec4Reg::Sort4Reverse(reg, index);
        value = reg.ToVec4();
    }

    bool Vec4::IsClose(const Vec4& other, const float maxDistSqr) const
    {
        return (other - *this).LengthSqr() <= maxDistSqr;
    }

    bool Vec4::IsNormalized(const float tolerance) const
    {
        return math::Abs(LengthSqr() - 1.0f) <= tolerance;
    }

    bool Vec4::IsNaN() const
    {
        return Vec4Reg::LoadVec4(this).IsNaN();
    }

    Vec4Reg Vec4::SplatX() const
    {
        return Vec4Reg::LoadVec4(this).SplatX();
    }

    Vec4Reg Vec4::SplatY() const
    {
        return Vec4Reg::LoadVec4(this).SplatY();
    }

    Vec4Reg Vec4::SplatZ() const
    {
        return Vec4Reg::LoadVec4(this).SplatZ();
    }

    Vec4Reg Vec4::SplatW() const
    {
        return Vec4Reg::LoadVec4(this).SplatW();
    }

    template <uint32 SwizzleX, uint32 SwizzleY, uint32 SwizzleZ, uint32 SwizzleW>
    Vec4 Vec4::Swizzle() const
    {
        static_assert(SwizzleX <= 3, "SwizzleX out of range!");
        static_assert(SwizzleY <= 3, "SwizzleY out of range!");
        static_assert(SwizzleZ <= 3, "SwizzleZ out of range!");
        static_assert(SwizzleW <= 3, "SwizzleW out of range!");

        return Vec4Reg::LoadVec4(this).Swizzle<SwizzleX, SwizzleY, SwizzleZ, SwizzleW>().ToVec4();
    }

    Vec4 Vec4::Abs() const
    {
        return Vec4Reg::LoadVec4(this).Abs().ToVec4();
    }

    Vec4 Vec4::Reciprocal() const
    {
        return One() / *this;
    }

    Vec4Reg Vec4::DotV(const Vec4& other) const
    {
        return Vec4Reg::LoadVec4(this).DotV(Vec4Reg::LoadVec4(&other));
    }

    float Vec4::Dot(const Vec4& other) const
    {
        return Vec4Reg::LoadVec4(this).Dot(Vec4Reg::LoadVec4(&other));
    }

    float Vec4::LengthSqr() const
    {
        return Vec4Reg::LoadVec4(this).LengthSqr();
    }

    float Vec4::Length() const
    {
        return Vec4Reg::LoadVec4(this).Length(); 
    }

    Vec4& Vec4::Normalize()
    {
        *this = *this / Length();
        return *this;
    }

    Vec4 Vec4::Normalized() const
    {
        return *this / Length();
    }

    int Vec4::GetSignBits() const
    {
        return Vec4Reg::LoadVec4(this).GetSignBits();
    }

    float Vec4::MinComponent() const
    {
        return Vec4Reg::LoadVec4(this).MinComponent();
    }

    float Vec4::MaxComponent() const
    {
        return Vec4Reg::LoadVec4(this).MaxComponent();
    }

    Vec4 Vec4::Sqrt() const
    {
        return Vec4Reg::LoadVec4(this).Sqrt().ToVec4();
    }

    Vec4 Vec4::GetSign() const
    {
        return Vec4Reg::LoadVec4(this).GetSign().ToVec4();
    }
}