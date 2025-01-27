#pragma once
// Vector2.h
#include "Generic.h"
#include "Debug/Assert.h"
#include "Core/Generic/Concepts.h"

namespace nes
{
    template<ScalarType Type>
    struct Vector2
    {
        Type x{};
        Type y{};

        constexpr Vector2() = default;
        constexpr Vector2(const Type x, const Type y) : x(x) , y(y) {}
        constexpr Vector2(const ScalarType auto x, const ScalarType auto y) : x(static_cast<Type>(x)), y(static_cast<Type>(y)) {}

        constexpr bool operator==(const Vector2 right) const;
        constexpr bool operator!=(const Vector2 right) const { return !(*this == right); }

        constexpr Vector2 operator-() const;
        constexpr Vector2 operator+(const Vector2 right) const;
        constexpr Vector2 operator-(const Vector2 right) const;
        constexpr Vector2 operator*(const Vector2 right) const;
        constexpr Vector2 operator*(const ScalarType auto scalar) const;
        constexpr Vector2 operator/(const ScalarType auto scalar) const;
        
        constexpr Type& operator[](const size_t index);
        constexpr Type operator[](const size_t index) const;

        Vector2& operator-();
        Vector2& operator+=(const Vector2 right);
        Vector2& operator-=(const Vector2 right);
        Vector2& operator*=(const Vector2 right);
        Vector2& operator*=(const ScalarType auto scalar);
        Vector2& operator/=(const ScalarType auto scalar);

        Type Magnitude() const;
        constexpr Type SquaredMagnitude() const;
        constexpr Type Dot(const Vector2& right) const;
        constexpr Vector2& Normalize();
        constexpr Vector2 GetNormalized() const;
        void SwapAxes();
        float ToAngle() const;
        float ToAngleDegrees() const;
        template <ScalarType To> Vector2<To> CastTo() const;

        [[nodiscard]] std::string ToString() const;

        static Vector2 FromAngle(const float radians);
        static Vector2 FromAngleDegrees(const float degrees);
        
        static constexpr Type Dot(const Vector2& a, const Vector2& b);
        static constexpr Type Distance(const Vector2& a, const Vector2& b);
        static constexpr Type DistanceSquared(const Vector2& a, const Vector2& b);

        static constexpr Vector2 GetUnitVector()   { return Vector2(static_cast<Type>(1), static_cast<Type>(1)); }
        static constexpr Vector2 GetZeroVector()   { return Vector2(static_cast<Type>(0), static_cast<Type>(0)); }
        static constexpr Vector2 GetUpVector()     { return Vector2(static_cast<Type>(0), static_cast<Type>(1)); }
        static constexpr Vector2 GetRightVector()  { return Vector2(static_cast<Type>(1), static_cast<Type>(0)); }
    };

    using Vec2 = Vector2<float>;
    using Vec2d = Vector2<double>;
    using Vec2i = Vector2<int>;
    using Vec2u = Vector2<unsigned int>;

    template <ScalarType VecType>
    Vector2<VecType> operator*(const ScalarType auto scalar, const Vector2<VecType> vec);
}

namespace nes
{
    template <ScalarType Type>
    constexpr bool Vector2<Type>::operator==(const Vector2 right) const
    {
        if constexpr (std::is_floating_point_v<Type>)
        {
            return math::CheckEqualFloats(x, right.x)
                && math::CheckEqualFloats(y, right.y);
        }

        else
        {
            return x == right.x && y == right.y;
        }
    }

    template <ScalarType Type>
    constexpr Vector2<Type> Vector2<Type>::operator-() const
    {
        Vector2 result;
        result.x = -x;
        result.y = -y;
        return result;
    }

    template <ScalarType Type>
    constexpr Vector2<Type> Vector2<Type>::operator+(const Vector2 right) const
    {
        Vector2 result = right;
        result.x += x;
        result.y += y;
        return result;
    }

    template <ScalarType Type>
    constexpr Vector2<Type> Vector2<Type>::operator-(const Vector2 right) const
    {
        Vector2 result = *this;
        result.x -= right.x;
        result.y -= right.y;
        return result;
    }

    template <ScalarType Type>
    constexpr Vector2<Type> Vector2<Type>::operator*(const Vector2 right) const
    {
        Vector2 result = right;
        result.x *= x;
        result.y *= y;
        return result;
    }

    template <ScalarType Type>
    constexpr Vector2<Type> Vector2<Type>::operator*(const ScalarType auto scalar) const
    {
        Vector2 result = *this;
        result.x *= static_cast<Type>(scalar);
        result.y *= static_cast<Type>(scalar);
        return result;
    }

    template <ScalarType Type>
    constexpr Vector2<Type> Vector2<Type>::operator/(const ScalarType auto scalar) const
    {
        NES_ASSERT(scalar != static_cast<decltype(scalar)>(0));

        Vector2 result = *this;
        result.x /= static_cast<Type>(scalar);
        result.y /= static_cast<Type>(scalar);
        return result;
    }

    template <ScalarType Type>
    constexpr Type& Vector2<Type>::operator[](const size_t index)
    {
        NES_ASSERT(index < 2);
        return *(&x + index);
    }

    template <ScalarType Type>
    constexpr Type Vector2<Type>::operator[](const size_t index) const
    {
        NES_ASSERT(index < 2);
        return *(&x + index);
    }

    template <ScalarType Type>
    Vector2<Type>& Vector2<Type>::operator-()
    {
        x = -x;
        y = -y;
        return *this;
    }

    template <ScalarType Type>
    Vector2<Type>& Vector2<Type>::operator+=(const Vector2 right)
    {
        *this = *this + right;
        return *this;
    }

    template <ScalarType Type>
    Vector2<Type>& Vector2<Type>::operator-=(const Vector2 right)
    {
        *this = *this - right;
        return *this;
    }

    template <ScalarType Type>
    Vector2<Type>& Vector2<Type>::operator*=(const Vector2 right)
    {
        *this = *this * right;
        return *this;
    }

    template <ScalarType Type>
    Vector2<Type>& Vector2<Type>::operator*=(const ScalarType auto scalar)
    {
        *this = *this * scalar;
        return *this;
    }

    template <ScalarType Type>
    Vector2<Type>& Vector2<Type>::operator/=(const ScalarType auto scalar)
    {
        *this = *this / scalar;
        return *this;
    }

    template <ScalarType Type>
    Type Vector2<Type>::Magnitude() const
    {
        return math::SafeSqrt(SquaredMagnitude());
    }

    template <ScalarType Type>
    constexpr Type Vector2<Type>::SquaredMagnitude() const
    {
        return (x * x) + (y * y);
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Dot Product between this and another vector.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type Vector2<Type>::Dot(const Vector2& right) const
    {
        return static_cast<float>((x * right.x) + (y * right.y));
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Normalizes this Vector. If you want to preserve this Vector, you can use GetNormalized().
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Vector2<Type>& Vector2<Type>::Normalize()
    {
        static_assert(std::is_floating_point_v<Type>, "Vector2 must have floating point components to be Normalized!");

        const auto magnitude = Magnitude();

        // If our magnitude is zero, the return immediately.
        if (math::CheckEqualFloats(magnitude, 0.0f, 0.0001f))
            return *this;

        x = x / magnitude;
        y = y / magnitude;
        return *this;
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Returns a normalized Vector based on this vector's components, without changing this Vector.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Vector2<Type> Vector2<Type>::GetNormalized() const
    {
        static_assert(std::is_floating_point_v<Type>, "Vector2 must have floating point components to be Normalized!");

        Vector2 output = *this;
        output.Normalize();
        return output;
    }

    template <ScalarType Type>
    void Vector2<Type>::SwapAxes()
    {
        const auto temp = x;
        x = y;
        y = temp;
    }

    template <ScalarType Type>
    float Vector2<Type>::ToAngle() const
    {
        return std::atan2(y, x);
    }

    template <ScalarType Type>
    float Vector2<Type>::ToAngleDegrees() const
    {
        return math::ToDegrees(ToAngle());
    }

    template <ScalarType Type>
    template <ScalarType To>
    Vector2<To> Vector2<Type>::CastTo() const
    {
        static_assert(!std::same_as<Type, To>, "Attempting to cast to the same type!");
        
        return Vector2<To>(static_cast<To>(x), static_cast<To>(y));
    }

    template <ScalarType Type>
    std::string Vector2<Type>::ToString() const
    {
        return CombineIntoString("(x=", x, ", y=", y, ")");
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Dot Product between two vectors.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type Vector2<Type>::Dot(const Vector2& a, const Vector2& b)
    {
        return a.Dot(b);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the distance between two Vectors. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type Vector2<Type>::Distance(const Vector2& a, const Vector2& b)
    {
        const Vector2 diff = b - a;
        return diff.Magnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the distance squared between two vectors. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type Vector2<Type>::DistanceSquared(const Vector2& a, const Vector2& b)
    {
        const Vector2 diff = b - a;
        return diff.SquaredMagnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a 2D Vector from an angle in radians. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    Vector2<Type> Vector2<Type>::FromAngle(const float radians)
    {
        return Vector2(std::cos(radians), std::sin(radians));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a 2D Vector from an angle in degrees. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    Vector2<Type> Vector2<Type>::FromAngleDegrees(const float degrees)
    {
        return FromAngle(math::ToRadians(degrees));
    }

    template <ScalarType VecType>
    Vector2<VecType> operator*(const ScalarType auto scalar, const Vector2<VecType> vec)
    {
        return vec * scalar;
    }
}
