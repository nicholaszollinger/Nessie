// IVec3.inl
#pragma once
#include "Generic.h"
#include "IVec2.h"

namespace nes
{
    template <IntegralType Type>
    constexpr TIntVec3<Type>::TIntVec3(const TIntVec2<Type> vec, const IntegralType auto z)
        : x(vec.x), y(vec.y), z(z)
    {
        //
    }

    template <IntegralType Type>
    constexpr TIntVec3<Type> TIntVec3<Type>::operator+(const TIntVec3 other) const
    {
        return TIntVec3<Type>(x + other.x, y + other.y, z + other.z);
    }

    template <IntegralType Type>
    constexpr TIntVec3<Type> TIntVec3<Type>::operator-(const TIntVec3 other) const
    {
        return TIntVec3<Type>(x - other.x, y - other.y, z - other.z);
    }

    template <IntegralType Type>
    constexpr TIntVec3<Type> TIntVec3<Type>::operator*(const TIntVec3 other) const
    {
        return TIntVec3<Type>(x * other.x, y * other.y, z * other.z);
    }

    template <IntegralType Type>
    constexpr TIntVec3<Type> TIntVec3<Type>::operator/(const TIntVec3 other) const
    {
        return TIntVec3<Type>(x / other.x, y / other.y, z / other.z);
    }

    template <IntegralType Type>
    constexpr TIntVec3<Type> TIntVec3<Type>::operator*(const Type scalar) const
    {
        return TIntVec3<Type>(x * scalar, y * scalar, z * scalar);
    }

    template <IntegralType Type>
    constexpr TIntVec3<Type> TIntVec3<Type>::operator/(const Type scalar) const
    {
        return TIntVec3<Type>(x / scalar, y / scalar, z / scalar);
    }

    template <IntegralType Type>
    constexpr TIntVec3<Type>& TIntVec3<Type>::operator+=(const TIntVec3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    template <IntegralType Type>
    constexpr TIntVec3<Type>& TIntVec3<Type>::operator-=(const TIntVec3& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    template <IntegralType Type>
    constexpr TIntVec3<Type>& TIntVec3<Type>::operator*=(const TIntVec3& other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    template <IntegralType Type>
    constexpr TIntVec3<Type>& TIntVec3<Type>::operator/=(const TIntVec3& other)
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }

    template <IntegralType Type>
    constexpr TIntVec3<Type>& TIntVec3<Type>::operator*=(const Type scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    template <IntegralType Type>
    constexpr TIntVec3<Type>& TIntVec3<Type>::operator/=(const Type scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    template <IntegralType Type>
    TIntVec3<Type> TIntVec3<Type>::Abs() const
    {
        if constexpr (UnsignedIntegralType<Type>)
            return *this;
        
        return TIntVec3<Type>
        (
            x < 0 ? -x : x,
            y < 0 ? -y : y,
            z < 0 ? -z : z
        );
    }

    template <IntegralType Type>
    float TIntVec3<Type>::LengthSqr() const
    {
        return x * x + y * y + z * z;
    }

    template <IntegralType Type>
    float TIntVec3<Type>::Length() const
    {
        return std::sqrt(LengthSqr());
    }

    template <IntegralType Type>
    Type TIntVec3<Type>::MinComponent() const
    {
        return x < y ? math::Min(x, z) : math::Min(y, z); 
    }

    template <IntegralType Type>
    Type TIntVec3<Type>::MaxComponent() const
    {
        return x > y ? math::Max(x, z) : math::Max(y, z);
    }

    template <IntegralType Type>
    int TIntVec3<Type>::MinComponentIndex() const
    {
        return x < y ? y < z? 1 : 2 : x < z? 0 : 2;
    }

    template <IntegralType Type>
    int TIntVec3<Type>::MaxComponentIndex() const
    {
        return x > y ? y > z? 1 : 2 : x > z? 0 : 2;
    }

    template <IntegralType Type>
    TIntVec3<Type> TIntVec3<Type>::Min(const TIntVec3 a, const TIntVec3 b)
    {
        return TIntVec3<Type>(math::Min(a.x, b.x), math::Min(a.y, b.y), math::Min(a.z, b.z));
    }

    template <IntegralType Type>
    TIntVec3<Type> TIntVec3<Type>::Max(const TIntVec3 a, const TIntVec3 b)
    {
        return TIntVec3<Type>(math::Max(a.x, b.x), math::Max(a.y, b.y), math::Max(a.z, b.z));
    }
}