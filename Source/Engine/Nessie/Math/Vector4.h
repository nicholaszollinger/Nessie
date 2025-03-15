// Vector4.h
#pragma once
#include "Vector3.h"

namespace nes
{
    template <ScalarType Type>
    struct TVector4
    {
        Type x{};
        Type y{};
        Type z{};
        Type w{};

        constexpr TVector4() = default;
        constexpr TVector4(const Type x, const Type y, const Type z, const Type w) : x(x) , y(y), z(z), w(w) {}
        constexpr TVector4(const ScalarType auto x, const ScalarType auto y, const ScalarType auto z, const ScalarType auto w);
        constexpr TVector4(const TVector3<Type>& vector3, const Type w = 0) : x(vector3.x), y(vector3.y), z(vector3.z), w(w) {}
        explicit constexpr TVector4(const Type uniformValue) : x(uniformValue), y(uniformValue), z(uniformValue), w(uniformValue) {}
        
        TVector4& operator=(const TVector3<Type>& vector3);
        constexpr bool operator==(const TVector4 right) const;
        constexpr bool operator!=(const TVector4 right) const { return !(*this == right); }

        constexpr TVector4 operator-() const;
        constexpr TVector4 operator+(const TVector4 right) const;
        constexpr TVector4 operator-(const TVector4 right) const;
        constexpr TVector4 operator*(const TVector4 right) const;
        constexpr TVector4 operator/(const TVector4 right) const;
        constexpr TVector4 operator*(const ScalarType auto scalar) const;
        constexpr TVector4 operator/(const ScalarType auto scalar) const;

        TVector4& operator-();
        TVector4& operator+=(const TVector4 right);
        TVector4& operator-=(const TVector4 right);
        TVector4& operator*=(const TVector4 right);
        TVector4& operator*=(const ScalarType auto scalar);
        TVector4& operator/=(const ScalarType auto scalar);
        
        constexpr Type& operator[](const size_t index);
        constexpr Type operator[](const size_t index) const;
        
        Type Magnitude() const;
        constexpr Type SquaredMagnitude() const;
        constexpr Type Dot(const TVector4& right) const;
        constexpr TVector4& Normalize();
        constexpr TVector4 Normalized() const;
        template <ScalarType To> TVector4<To> CastTo() const;
        
        [[nodiscard]] std::string ToString() const;

        static constexpr Type Dot(const TVector4& a, const TVector4& b);
        static constexpr TVector4 Lerp(const TVector4 from, const TVector4 to, const float t);
        
        static constexpr TVector4 GetUnitVector()    { return TVector4(static_cast<Type>(1), static_cast<Type>(1), static_cast<Type>(1)); }
        static constexpr TVector4 GetZeroVector()    { return TVector4(static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(0)); }
        static constexpr TVector4 GetUpVector()      { return TVector4(static_cast<Type>(0), static_cast<Type>(1), static_cast<Type>(0)); }
        static constexpr TVector4 GetRightVector()   { return TVector4(static_cast<Type>(1), static_cast<Type>(0), static_cast<Type>(0)); }
        static constexpr TVector4 GetForwardVector() { return TVector4(static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(1)); }
    };

    using Vector4f = TVector4<float>;
    using Vector4d = TVector4<double>;
    using Vector4i = TVector4<int>;
    using Vector4u = TVector4<unsigned int>;
    using Vector4  = TVector4<NES_MATH_DEFAULT_REAL_TYPE>;

    template <ScalarType Type>
    constexpr TVector4<Type>::TVector4(const ScalarType auto x, const ScalarType auto y, const ScalarType auto z,
        const ScalarType auto w)
        : x(x)
        , y(y)
        , z(z)
        , w(w)
    {
        //
    }

    template <ScalarType Type>
    TVector4<Type>& TVector4<Type>::operator=(const TVector3<Type>& vector3)
    {
        x = vector3.x;
        y = vector3.y;
        z = vector3.z;
        w = 1.f; // 1 or zero?
    }

    template <ScalarType Type>
    constexpr bool TVector4<Type>::operator==(const TVector4 right) const
    {
        return x == right.x && y == right.y && z == right.z && w == right.w;
    }

    template <ScalarType Type>
    constexpr TVector4<Type> TVector4<Type>::operator-() const
    {
        return { -x, -y, -z, -w };
    }

    template <ScalarType Type>
    constexpr TVector4<Type> TVector4<Type>::operator+(const TVector4 right) const
    {
        return { x + right.x, y + right.y, z + right.z, w + right.w };
    }

    template <ScalarType Type>
    constexpr TVector4<Type> TVector4<Type>::operator-(const TVector4 right) const
    {
        return { x - right.x, y - right.y, z - right.z, w - right.w };
    }

    template <ScalarType Type>
    constexpr TVector4<Type> TVector4<Type>::operator*(const TVector4 right) const
    {
        return { x * right.x, y * right.y, z * right.z, w * right.w };
    }

    template <ScalarType Type>
    constexpr TVector4<Type> TVector4<Type>::operator/(const TVector4 right) const
    {
        return { x / right.x, y / right.y, z / right.z, w / right.w };
    }

    template <ScalarType Type>
    constexpr TVector4<Type> TVector4<Type>::operator*(const ScalarType auto scalar) const
    {
        return { x * scalar, y * scalar, z * scalar, w * scalar };
    }

    template <ScalarType Type>
    constexpr TVector4<Type> TVector4<Type>::operator/(const ScalarType auto scalar) const
    {
        NES_ASSERT(scalar != static_cast<decltype(scalar)>(0));
        return { x / scalar, y / scalar, z / scalar, w / scalar };
    }

    template <ScalarType Type>
    TVector4<Type>& TVector4<Type>::operator-()
    {
        x = -x;
        y = -y;
        z = -z;
        w = -w;
        return *this;
    }

    template <ScalarType Type>
    TVector4<Type>& TVector4<Type>::operator+=(const TVector4 right)
    {
        *this = *this + right;
        return *this;
    }

    template <ScalarType Type>
    TVector4<Type>& TVector4<Type>::operator-=(const TVector4 right)
    {
        *this = *this - right;
        return *this;
    }

    template <ScalarType Type>
    TVector4<Type>& TVector4<Type>::operator*=(const TVector4 right)
    {
        *this = *this * right;
        return *this;
    }

    template <ScalarType Type>
    TVector4<Type>& TVector4<Type>::operator*=(const ScalarType auto scalar)
    {
        *this = *this * scalar;
        return *this;
    }

    template <ScalarType Type>
    TVector4<Type>& TVector4<Type>::operator/=(const ScalarType auto scalar)
    {
        *this = *this / scalar;
        return *this;
    }

    template <ScalarType Type>
    constexpr Type& TVector4<Type>::operator[](const size_t index)
    {
        NES_ASSERT(index < 4);
        return *(&this->x + index);
    }

    template <ScalarType Type>
    constexpr Type TVector4<Type>::operator[](const size_t index) const
    {
        NES_ASSERT(index < 4);
        return *(&this->x + index);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the scalar length of this vector. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    Type TVector4<Type>::Magnitude() const
    {
        return std::sqrt((x * x) + (y * y) + (z * z) + (w * w));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the squared scalar length of this vector. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type TVector4<Type>::SquaredMagnitude() const
    {
        return (x * x) + (y * y) + (z * z) + (w * w);
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Dot Product between this and another vector.
    ///            The dot product geometrically represents the cosine of the angle between the two
    ///            Vectors, and can be used to determine if two Vectors are pointing in roughly
    ///            the same direction. For unit vectors, the dot product is equal to 1 if they are
    ///            the same, 0 if they are perpendicular, and -1 if they are opposite.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type TVector4<Type>::Dot(const TVector4& right) const
    {
        return x * right.x + y * right.y + z * right.z + w * right.w;
    }

    //-------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Normalize this Vector (magnitude of 1). If you want to preserve this Vector, you can use GetNormalized(). 
    //-------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr TVector4<Type>& TVector4<Type>::Normalize()
    {
        static_assert(std::floating_point<Type>, "Type must be floating point");

        const auto magnitude = Magnitude();
        if (!math::CheckEqualFloats(magnitude, 0.f, 0.0001f))
        {
            x = x / magnitude;
            y = y / magnitude;
            z = z / magnitude;
            w = w / magnitude;
        }

        return *this;
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Returns a normalized Vector based on this Vector's components.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr TVector4<Type> TVector4<Type>::Normalized() const
    {
        static_assert(std::floating_point<Type>, "Type must be floating point");
        
        TVector3 result = *this;
        result.Normalize();
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a Vector casted to another Scalar Type.
    ///		@tparam To : Type to cast each of this Vector's components to.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    template <ScalarType To>
    TVector4<To> TVector4<Type>::CastTo() const
    {
        static_assert(!std::same_as<Type, To>, "Attempting to cast Vector to the same type!");
        return TVector4<To>(static_cast<To>(x), static_cast<To>(y), static_cast<To>(z), static_cast<To>(w));
    }

    template <ScalarType Type>
    std::string TVector4<Type>::ToString() const
    {
        return CombineIntoString("(x=", x, ", y=", y, ", z=", z, ", w=", w, ")");
    }
    
    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Dot Product between two vectors.
    ///            The dot product geometrically represents the cosine of the angle between the two
    ///            Vectors, and can be used to determine if two Vectors are pointing in roughly
    ///            the same direction. For unit vectors, the dot product is equal to 1 if they are
    ///            the same, 0 if they are perpendicular, and -1 if they are opposite.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type TVector4<Type>::Dot(const TVector4& a, const TVector4& b)
    {
        return a.Dot(b);
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Linearly interpolate between two vectors.
    ///		@param from : The Vector we are starting from.
    ///		@param to : The Vector we are going toward.
    ///		@param t : The percentage the resulting vector will be between the two vectors. Should be a value between [0, 1].
    ///		@returns : Linearly interpolated Vector.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr TVector4<Type> TVector4<Type>::Lerp(const TVector4 from, const TVector4 to, const float t)
    {
        return from + ((to - from) * t);
    }
}
