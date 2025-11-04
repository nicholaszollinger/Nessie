// IVec2.inl
#pragma once

namespace nes
{
    template <IntegralType Type>
    constexpr TIntVec2<Type> TIntVec2<Type>::operator+(const TIntVec2 other) const
    {
        return TIntVec2<Type>(x + other.x, y + other.y);
    }

    template <IntegralType Type>
    constexpr TIntVec2<Type> TIntVec2<Type>::operator-(const TIntVec2 other) const
    {
        return TIntVec2<Type>(x - other.x, y - other.y);
    }

    template <IntegralType Type>
    constexpr TIntVec2<Type> TIntVec2<Type>::operator*(const TIntVec2 other) const
    {
        return TIntVec2<Type>(x * other.x, y * other.y);
    }

    template <IntegralType Type>
    constexpr TIntVec2<Type> TIntVec2<Type>::operator/(const TIntVec2 other) const
    {
        return TIntVec2<Type>(x / other.x, y / other.y);
    }

    template <IntegralType Type>
    constexpr TIntVec2<Type> TIntVec2<Type>::operator*(const Type scalar) const
    {
        return TIntVec2<Type>(x * scalar, y * scalar);
    }

    template <IntegralType Type>
    constexpr TIntVec2<Type> TIntVec2<Type>::operator/(const Type scalar) const
    {
        return TIntVec2<Type>(x / scalar, y / scalar);
    }

    template <IntegralType Type>
    constexpr TIntVec2<Type>& TIntVec2<Type>::operator+=(const TIntVec2& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    template <IntegralType Type>
    constexpr TIntVec2<Type>& TIntVec2<Type>::operator-=(const TIntVec2& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    template <IntegralType Type>
    constexpr TIntVec2<Type>& TIntVec2<Type>::operator*=(const TIntVec2& other)
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    template <IntegralType Type>
    constexpr TIntVec2<Type>& TIntVec2<Type>::operator/=(const TIntVec2& other)
    {
        x /= other.x;
        y /= other.y;
        return *this;
    }

    template <IntegralType Type>
    constexpr TIntVec2<Type>& TIntVec2<Type>::operator*=(const Type scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    template <IntegralType Type>
    constexpr TIntVec2<Type>& TIntVec2<Type>::operator/=(const Type scalar)
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    template <IntegralType Type>
    TIntVec2<Type> TIntVec2<Type>::Abs() const
    {
        if constexpr (UnsignedIntegralType<Type>)
            return *this;
        
        return TIntVec2<Type>(x < 0 ? -x : x, y < 0 ? -y : y);
    }

    template <IntegralType Type>
    float TIntVec2<Type>::LengthSqr() const
    {
        return static_cast<float>(x * x + y * y);
    }

    template <IntegralType Type>
    float TIntVec2<Type>::Length() const
    {
        return std::sqrt(LengthSqr());
    }

    template <IntegralType Type>
    Type TIntVec2<Type>::MinComponent() const
    {
        return math::Min(x, y);
    }

    template <IntegralType Type>
    Type TIntVec2<Type>::MaxComponent() const
    {
        return math::Max(x, y);
    }

    template <IntegralType Type>
    int TIntVec2<Type>::MinComponentIndex() const
    {
        return x < y ? 0 : 1;
    }

    template <IntegralType Type>
    int TIntVec2<Type>::MaxComponentIndex() const
    {
        return x > y ? 0 : 1;
    }

    template <IntegralType Type>
    TIntVec2<Type> TIntVec2<Type>::Min(const TIntVec2 a, const TIntVec2 b)
    {
        return TIntVec2(math::Min(a.x, b.x), math::Min(a.y, b.y));
    }

    template <IntegralType Type>
    TIntVec2<Type> TIntVec2<Type>::Max(const TIntVec2 a, const TIntVec2 b)
    {
        return TIntVec2(math::Max(a.x, b.x), math::Max(a.y, b.y));
    }
}