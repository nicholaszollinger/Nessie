#pragma once
// Vector2.h
#include "Generic.h"
#include "MathUtils.h"
#include "Core/Generic/Concepts.h"
#include "Debug/Assert.h"

namespace nes
{
    template<ScalarType Type>
    struct TVector2
    {
        Type x{};
        Type y{};

        constexpr TVector2() = default;
        constexpr TVector2(const Type x, const Type y) : x(x) , y(y) {}
        constexpr TVector2(const ScalarType auto x, const ScalarType auto y) : x(static_cast<Type>(x)), y(static_cast<Type>(y)) {}

        constexpr bool operator==(const TVector2 right) const;
        constexpr bool operator!=(const TVector2 right) const { return !(*this == right); }

        constexpr TVector2 operator-() const;
        constexpr TVector2 operator+(const TVector2 right) const;
        constexpr TVector2 operator-(const TVector2 right) const;
        constexpr TVector2 operator*(const TVector2 right) const;
        constexpr TVector2 operator*(const ScalarType auto scalar) const;
        constexpr TVector2 operator/(const ScalarType auto scalar) const;
        
        constexpr Type& operator[](const size_t index);
        constexpr Type operator[](const size_t index) const;

        TVector2& operator-();
        TVector2& operator+=(const TVector2 right);
        TVector2& operator-=(const TVector2 right);
        TVector2& operator*=(const TVector2 right);
        TVector2& operator*=(const ScalarType auto scalar);
        TVector2& operator/=(const ScalarType auto scalar);

        Type Magnitude() const;
        constexpr Type SquaredMagnitude() const;
        constexpr Type Dot(const TVector2& right) const;
        constexpr TVector2& Normalize();
        constexpr TVector2 Normalized() const;
        TVector2& Rotate(const float angleDegrees);
        TVector2 Rotated(const float angleDegrees) const;
        void SwapAxes();
        float ToAngle() const;
        float ToAngleDegrees() const;
        template <ScalarType To> TVector2<To> CastTo() const;

        [[nodiscard]] std::string ToString() const;

        static TVector2 FromAngle(const float radians);
        static TVector2 FromAngleDegrees(const float degrees);

        static constexpr Type Dot(const TVector2& a, const TVector2& b);
        static constexpr Type Distance(const TVector2& a, const TVector2& b);
        static constexpr Type DistanceSquared(const TVector2& a, const TVector2& b);

        
        static constexpr TVector2 GetUnitVector()   { return TVector2(static_cast<Type>(1), static_cast<Type>(1)); }
        static constexpr TVector2 GetZeroVector()   { return TVector2(static_cast<Type>(0), static_cast<Type>(0)); }
        static constexpr TVector2 GetUpVector()     { return TVector2(static_cast<Type>(0), static_cast<Type>(1)); }
        static constexpr TVector2 GetRightVector()  { return TVector2(static_cast<Type>(1), static_cast<Type>(0)); }
    };

    template <ScalarType VecType>
    TVector2<VecType> operator*(const ScalarType auto scalar, const TVector2<VecType> vec);

    using Vector2f = TVector2<float>;
    using Vector2d = TVector2<double>;
    using Vector2i = TVector2<int>;
    using Vector2u = TVector2<unsigned int>;
    using Vector2 = TVector2<NES_MATH_DEFAULT_REAL_TYPE>;
}

namespace nes
{
    template <ScalarType Type>
    constexpr bool TVector2<Type>::operator==(const TVector2 right) const
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
    constexpr TVector2<Type> TVector2<Type>::operator-() const
    {
        TVector2 result;
        result.x = -x;
        result.y = -y;
        return result;
    }

    template <ScalarType Type>
    constexpr TVector2<Type> TVector2<Type>::operator+(const TVector2 right) const
    {
        TVector2 result = right;
        result.x += x;
        result.y += y;
        return result;
    }

    template <ScalarType Type>
    constexpr TVector2<Type> TVector2<Type>::operator-(const TVector2 right) const
    {
        TVector2 result = *this;
        result.x -= right.x;
        result.y -= right.y;
        return result;
    }

    template <ScalarType Type>
    constexpr TVector2<Type> TVector2<Type>::operator*(const TVector2 right) const
    {
        TVector2 result = right;
        result.x *= x;
        result.y *= y;
        return result;
    }

    template <ScalarType Type>
    constexpr TVector2<Type> TVector2<Type>::operator*(const ScalarType auto scalar) const
    {
        TVector2 result = *this;
        result.x *= static_cast<Type>(scalar);
        result.y *= static_cast<Type>(scalar);
        return result;
    }

    template <ScalarType Type>
    constexpr TVector2<Type> TVector2<Type>::operator/(const ScalarType auto scalar) const
    {
        NES_ASSERT(scalar != static_cast<decltype(scalar)>(0));

        TVector2 result = *this;
        result.x /= static_cast<Type>(scalar);
        result.y /= static_cast<Type>(scalar);
        return result;
    }

    template <ScalarType Type>
    constexpr Type& TVector2<Type>::operator[](const size_t index)
    {
        NES_ASSERT(index < 2);
        return *(&x + index);
    }

    template <ScalarType Type>
    constexpr Type TVector2<Type>::operator[](const size_t index) const
    {
        NES_ASSERT(index < 2);
        return *(&x + index);
    }

    template <ScalarType Type>
    TVector2<Type>& TVector2<Type>::operator-()
    {
        x = -x;
        y = -y;
        return *this;
    }

    template <ScalarType Type>
    TVector2<Type>& TVector2<Type>::operator+=(const TVector2 right)
    {
        *this = *this + right;
        return *this;
    }

    template <ScalarType Type>
    TVector2<Type>& TVector2<Type>::operator-=(const TVector2 right)
    {
        *this = *this - right;
        return *this;
    }

    template <ScalarType Type>
    TVector2<Type>& TVector2<Type>::operator*=(const TVector2 right)
    {
        *this = *this * right;
        return *this;
    }

    template <ScalarType Type>
    TVector2<Type>& TVector2<Type>::operator*=(const ScalarType auto scalar)
    {
        *this = *this * scalar;
        return *this;
    }

    template <ScalarType Type>
    TVector2<Type>& TVector2<Type>::operator/=(const ScalarType auto scalar)
    {
        *this = *this / scalar;
        return *this;
    }

    template <ScalarType Type>
    Type TVector2<Type>::Magnitude() const
    {
        return math::SafeSqrt(SquaredMagnitude());
    }

    template <ScalarType Type>
    constexpr Type TVector2<Type>::SquaredMagnitude() const
    {
        return (x * x) + (y * y);
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Dot Product between this and another vector.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type TVector2<Type>::Dot(const TVector2& right) const
    {
        return static_cast<float>((x * right.x) + (y * right.y));
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Normalizes this Vector. If you want to preserve this Vector, you can use GetNormalized().
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr TVector2<Type>& TVector2<Type>::Normalize()
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
    constexpr TVector2<Type> TVector2<Type>::Normalized() const
    {
        static_assert(std::is_floating_point_v<Type>, "Vector2 must have floating point components to be Normalized!");

        TVector2 output = *this;
        output.Normalize();
        return output;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Rotate this vector by an angle in degrees. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    TVector2<Type>& TVector2<Type>::Rotate(const float angleDegrees)
    {
        *this = Rotated(angleDegrees);
        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Return the result of rotating this vector by an angle in degrees. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    TVector2<Type> TVector2<Type>::Rotated(const float angleDegrees) const
    {
        const float radians = math::ToRadians(angleDegrees);
        const float cos = std::cos(radians); 
        const float sin = std::sin(radians);

        return TVector2(cos * x - sin * y, sin * x + cos * y);
    }

    template <ScalarType Type>
    void TVector2<Type>::SwapAxes()
    {
        const auto temp = x;
        x = y;
        y = temp;
    }

    template <ScalarType Type>
    float TVector2<Type>::ToAngle() const
    {
        return std::atan2(y, x);
    }

    template <ScalarType Type>
    float TVector2<Type>::ToAngleDegrees() const
    {
        return math::ToDegrees(ToAngle());
    }

    template <ScalarType Type>
    template <ScalarType To>
    TVector2<To> TVector2<Type>::CastTo() const
    {
        static_assert(!std::same_as<Type, To>, "Attempting to cast to the same type!");
        
        return TVector2<To>(static_cast<To>(x), static_cast<To>(y));
    }

    template <ScalarType Type>
    std::string TVector2<Type>::ToString() const
    {
        return CombineIntoString("(x=", x, ", y=", y, ")");
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Dot Product between two vectors.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type TVector2<Type>::Dot(const TVector2& a, const TVector2& b)
    {
        return a.Dot(b);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the distance between two Vectors. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type TVector2<Type>::Distance(const TVector2& a, const TVector2& b)
    {
        const TVector2 diff = b - a;
        return diff.Magnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the distance squared between two vectors. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type TVector2<Type>::DistanceSquared(const TVector2& a, const TVector2& b)
    {
        const TVector2 diff = b - a;
        return diff.SquaredMagnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a 2D Vector from an angle in radians. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    TVector2<Type> TVector2<Type>::FromAngle(const float radians)
    {
        return TVector2(std::cos(radians), std::sin(radians));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create a 2D Vector from an angle in degrees. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    TVector2<Type> TVector2<Type>::FromAngleDegrees(const float degrees)
    {
        return FromAngle(math::ToRadians(degrees));
    }

    template <ScalarType VecType>
    TVector2<VecType> operator*(const ScalarType auto scalar, const TVector2<VecType> vec)
    {
        return vec * scalar;
    }
}
