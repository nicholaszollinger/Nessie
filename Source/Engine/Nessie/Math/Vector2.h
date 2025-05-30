#pragma once
// Vector2.h
#include "Generic.h"
#include "MathConfig.h"
#include "Core/Generic/Concepts.h"
#include "Debug/Assert.h"

namespace nes
{
    template<ScalarType Type>
    struct TVector2
    {
        static constexpr size_t N = 2;
        
        Type x{};
        Type y{};

        constexpr TVector2() = default;
        constexpr TVector2(const Type x, const Type y) : x(x) , y(y) {}
        constexpr TVector2(const ScalarType auto x, const ScalarType auto y) : x(static_cast<Type>(x)), y(static_cast<Type>(y)) {}
        explicit constexpr TVector2(const Type uniformValue) : x(uniformValue), y(uniformValue) {}

        constexpr bool operator==(const TVector2& right) const;
        constexpr bool operator!=(const TVector2& right) const { return !(*this == right); }
        constexpr bool operator<(const TVector2& right) const;
        constexpr bool operator>(const TVector2& right) const;
        constexpr bool operator<=(const TVector2& right) const;
        constexpr bool operator>=(const TVector2& right) const;

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
        TVector2 Perpendicular() const;
        TVector2& Rotate(const float angleDegrees);
        TVector2 Rotated(const float angleDegrees) const;
        void SwapAxes();
        float ToAngle() const;
        float ToAngleDegrees() const;
        template <ScalarType To> TVector2<To> CastTo() const;

        [[nodiscard]] std::string ToString() const;

        static TVector2 FromAngle(const float radians);
        static TVector2 FromAngleDegrees(const float degrees);
        static TVector2 PerpendicularTo(const TVector2 vec);

        static constexpr Type Dot(const TVector2& a, const TVector2& b);
        static constexpr Type Distance(const TVector2& a, const TVector2& b);
        static constexpr Type DistanceSquared(const TVector2& a, const TVector2& b);
        static constexpr TVector2 Min(const TVector2& a, const TVector2& b);
        static constexpr TVector2 Max(const TVector2& a, const TVector2& b);
        
        static constexpr TVector2 Unit()   { return TVector2(static_cast<Type>(1), static_cast<Type>(1)); }
        static constexpr TVector2 Zero()   { return TVector2(static_cast<Type>(0), static_cast<Type>(0)); }
        static constexpr TVector2 Up()     { return TVector2(static_cast<Type>(0), static_cast<Type>(1)); }
        static constexpr TVector2 Right()  { return TVector2(static_cast<Type>(1), static_cast<Type>(0)); }
    };

    template <ScalarType VecType>
    TVector2<VecType> operator*(const ScalarType auto scalar, const TVector2<VecType> vec);

    using Vector2f = TVector2<float>;
    using Vector2d = TVector2<double>;
    using Vector2i = TVector2<int>;
    using Vector2u = TVector2<unsigned int>;
    using Vector2 = TVector2<NES_PRECISION_TYPE>;
}

namespace nes
{
    template <ScalarType Type>
    constexpr bool TVector2<Type>::operator==(const TVector2& right) const
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

    //----------------------------------------------------------------------------------------------------
    /// @brief : True if all components are less than the right's.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr bool TVector2<Type>::operator<(const TVector2& right) const
    {
        return x < right.x && y < right.y;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : True if all components are greater than the right's.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr bool TVector2<Type>::operator>(const TVector2& right) const
    {
        return x > right.x && y > right.y;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : True if all components are less than or equal to the right's.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr bool TVector2<Type>::operator<=(const TVector2& right) const
    {
        return x <= right.x && y <= right.y;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : True if all components are greater than or equal to the right's.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr bool TVector2<Type>::operator>=(const TVector2& right) const
    {
        return x >= right.x && y >= right.y;
    }

    template <ScalarType Type>
    constexpr TVector2<Type> TVector2<Type>::operator-() const
    {
        return { -x, -y };
    }

    template <ScalarType Type>
    constexpr TVector2<Type> TVector2<Type>::operator+(const TVector2 right) const
    {
        return { x + right.x, y + right.y };
    }

    template <ScalarType Type>
    constexpr TVector2<Type> TVector2<Type>::operator-(const TVector2 right) const
    {
        return { x - right.x, y - right.y };
    }

    template <ScalarType Type>
    constexpr TVector2<Type> TVector2<Type>::operator*(const TVector2 right) const
    {
        return { x * right.x, y * right.y };
    }

    template <ScalarType Type>
    constexpr TVector2<Type> TVector2<Type>::operator*(const ScalarType auto scalar) const
    {
        return { x * static_cast<Type>(scalar), y * static_cast<Type>(scalar) };
    }

    template <ScalarType Type>
    constexpr TVector2<Type> TVector2<Type>::operator/(const ScalarType auto scalar) const
    {
        NES_ASSERT(scalar != static_cast<decltype(scalar)>(0));

        return { x / static_cast<Type>(scalar), y / static_cast<Type>(scalar) };
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
    ///		@brief : Returns a perpendicular vector to this. (Rotating by 90 degrees counterclockwise) 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    TVector2<Type> TVector2<Type>::Perpendicular() const
    {
        return TVector2(-y, x);
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
    /// @brief : Returns the minimum values of each of the components.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr TVector2<Type> TVector2<Type>::Min(const TVector2& a, const TVector2& b)
    {
        TVector2 result;
        result.x = math::Min(a.x, b.x);
        result.y = math::Min(a.y, b.y);
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Returns the maximum values of each of the components.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr TVector2<Type> TVector2<Type>::Max(const TVector2& a, const TVector2& b)
    {
        TVector2 result;
        result.x = math::Max(a.x, b.x);
        result.y = math::Max(a.y, b.y);
        return result;
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

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a perpendicular vector to the passed in vector (same as rotating the
    ///             vector by 90 degrees counterclockwise).
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    TVector2<Type> TVector2<Type>::PerpendicularTo(const TVector2 vec)
    {
        return TVector2(-vec.y, vec.x);
    }

    template <ScalarType VecType>
    TVector2<VecType> operator*(const ScalarType auto scalar, const TVector2<VecType> vec)
    {
        return vec * scalar;
    }
}
