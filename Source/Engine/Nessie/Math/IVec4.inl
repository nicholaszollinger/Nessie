// IVec4.inl
#pragma once
#include "Generic.h"
#include "IVec4.h"

namespace nes
{
    template <IntegralType Type>
    constexpr TIntVec4<Type>::TIntVec4(const TIntVec3<Type> vec, const IntegralType auto w)
        : x(vec.x), y(vec.y), z(vec.z), w(w)
    {
        //
    }

    template <IntegralType Type>
    constexpr TIntVec4<Type> TIntVec4<Type>::operator+(const TIntVec4 other) const
    {
        return TIntVec4<Type>(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    template <IntegralType Type>
    constexpr TIntVec4<Type> TIntVec4<Type>::operator-(const TIntVec4 other) const
    {
        return TIntVec4<Type>(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    template <IntegralType Type>
    constexpr TIntVec4<Type> TIntVec4<Type>::operator*(const TIntVec4 other) const
    {
        return TIntVec4<Type>(x * other.x, y * other.y, z * other.z, w * other.w); 
    }

    template <IntegralType Type>
    constexpr TIntVec4<Type> TIntVec4<Type>::operator/(const TIntVec4 other) const
    {
        return TIntVec4<Type>(x / other.x, y / other.y, z / other.z, w / other.w);
    }

    template <IntegralType Type>
    constexpr TIntVec4<Type> TIntVec4<Type>::operator*(const Type scalar) const
    {
        return TIntVec4<Type>(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    template <IntegralType Type>
    constexpr TIntVec4<Type> TIntVec4<Type>::operator/(const Type scalar) const
    {
        return TIntVec4<Type>(x / scalar, y / scalar, z / scalar, w / scalar);
    }

    template <IntegralType Type>
    constexpr TIntVec4<Type>& TIntVec4<Type>::operator+=(const TIntVec4& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    template <IntegralType Type>
    constexpr TIntVec4<Type>& TIntVec4<Type>::operator-=(const TIntVec4& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    template <IntegralType Type>
    constexpr TIntVec4<Type>& TIntVec4<Type>::operator*=(const TIntVec4& other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
        return *this;
    }

    template <IntegralType Type>
    constexpr TIntVec4<Type>& TIntVec4<Type>::operator/=(const TIntVec4& other)
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        w /= other.w;
        return *this;
    }

    template <IntegralType Type>
    constexpr TIntVec4<Type>& TIntVec4<Type>::operator*=(const Type scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    template <IntegralType Type>
    constexpr TIntVec4<Type>& TIntVec4<Type>::operator/=(const Type scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        w /= scalar;
        return *this;
    }

    template <IntegralType Type>
    TIntVec4<Type> TIntVec4<Type>::Abs() const
    {
        if constexpr (UnsignedIntegralType<Type>)
            return *this;

        return TIntVec4
        (
            x < 0 ? -x : x,
            y < 0 ? -y : y,
            z < 0 ? -z : z,
            w < 0 ? -w : w
        );
    }

    template <IntegralType Type>
    float TIntVec4<Type>::LengthSqr() const
    {
        return x * x + y * y + z * z + w * w;
    }

    template <IntegralType Type>
    float TIntVec4<Type>::Length() const
    {
        return std::sqrt(LengthSqr());
    }

    template <IntegralType Type>
    Type TIntVec4<Type>::MinComponent() const
    {
        Type result = math::Min(x, y);
        result = math::Min(result, z);
        result = math::Min(result, w);
        return result;
    }

    template <IntegralType Type>
    Type TIntVec4<Type>::MaxComponent() const
    {
        Type result = math::Max(x, y);
        result = math::Max(result, z);
        result = math::Max(result, w);
        return result;
    }

    template <IntegralType Type>
    int TIntVec4<Type>::MinComponentIndex() const
    {
        int result = x < y ? 0 : 1;
        result = z < result ? 2 : result;
        result = w < result ? 3 : result;
        return result;
    }

    template <IntegralType Type>
    int TIntVec4<Type>::MaxComponentIndex() const
    {
        int result = x > y ? 0 : 1;
        result = z > result ? 2 : result;
        result = w > result ? 3 : result;
        return result;
    }

    template <IntegralType Type>
    TIntVec4<Type> TIntVec4<Type>::Min(const TIntVec4 a, const TIntVec4 b)
    {
        return TIntVec4<Type>
        (
            math::Min(a.x, b.x),
            math::Min(a.y, b.y),
            math::Min(a.z, b.z),
            math::Min(a.w, b.w)
        );
    }

    template <IntegralType Type>
    TIntVec4<Type> TIntVec4<Type>::Max(const TIntVec4 a, const TIntVec4 b)
    {
        return TIntVec4<Type>
        (
            math::Max(a.x, b.x),
            math::Max(a.y, b.y),
            math::Max(a.z, b.z),
            math::Max(a.w, b.w)
        );
    }
}
