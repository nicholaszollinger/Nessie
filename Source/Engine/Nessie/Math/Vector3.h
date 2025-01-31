// Vector3.h
#pragma once
#include "Generic.h"
#include "Vector2.h"
#include "Core/String/FormatString.h"

namespace nes
{
    template <ScalarType Type>
    struct TVector3
    {
        Type x{};
        Type y{};
        Type z{};

        constexpr TVector3() = default;
        constexpr TVector3(const Type x, const Type y, const Type z) : x(x) , y(y), z(z) {}
        constexpr TVector3(const ScalarType auto x, const ScalarType auto y, const ScalarType auto z) : x(static_cast<Type>(x)), y(static_cast<Type>(y)), z(static_cast<Type>(z)) {}
        constexpr TVector3(const TVector2<Type>& vector2, const Type z = 0) : x(vector2.x), y(vector2.y), z(z) {}
        
        TVector3& operator=(const TVector2<Type>& vector2);

        constexpr bool operator==(const TVector3 right) const;
        constexpr bool operator!=(const TVector3 right) const { return !(*this == right); }

        constexpr TVector3 operator-() const;
        constexpr TVector3 operator+(const TVector3 right) const;
        constexpr TVector3 operator-(const TVector3 right) const;
        constexpr TVector3 operator*(const TVector3 right) const;
        constexpr TVector3 operator/(const TVector3 right) const;
        constexpr TVector3 operator*(const ScalarType auto scalar) const;
        constexpr TVector3 operator/(const ScalarType auto scalar) const;

        TVector3& operator-();
        TVector3& operator+=(const TVector3 right);
        TVector3& operator-=(const TVector3 right);
        TVector3& operator*=(const TVector3 right);
        TVector3& operator*=(const ScalarType auto scalar);
        TVector3& operator/=(const ScalarType auto scalar);
        
        constexpr Type& operator[](const size_t index);
        constexpr Type operator[](const size_t index) const;
        
        Type Magnitude() const;
        constexpr Type SquaredMagnitude() const;
        constexpr Type Dot(const TVector3& right) const;
        constexpr TVector3 Cross(const TVector3& right) const;
        constexpr TVector3& Normalize();
        constexpr TVector3 Normalized() const;
        constexpr TVector2<Type> GetXY() const { return TVector2<Type>(this->x, this->y); }
        template <ScalarType To> TVector3<To> CastTo() const;
        
        [[nodiscard]] std::string ToString() const;

        static constexpr Type Dot(const TVector3& a, const TVector3& b);
        static constexpr Type Distance(const TVector3& a, const TVector3& b);
        static constexpr Type DistanceSquared(const TVector3& a, const TVector3& b);
        static constexpr TVector3 Cross(const TVector3& a, const TVector3& b);
        static constexpr TVector3 Lerp(const TVector3 from, const TVector3 to, const float t);
        
        static float AngleBetweenVectors(const TVector3& a, const TVector3& b);
        static float AngleBetweenVectorsDegrees(const TVector3& a, const TVector3& b);

        static constexpr TVector3 GetUnitVector()    { return TVector3(static_cast<Type>(1), static_cast<Type>(1)), static_cast<Type>(1); }
        static constexpr TVector3 GetZeroVector()    { return TVector3(static_cast<Type>(0), static_cast<Type>(0)), static_cast<Type>(0); }
        static constexpr TVector3 GetUpVector()      { return TVector3(static_cast<Type>(0), static_cast<Type>(1), static_cast<Type>(0)); }
        static constexpr TVector3 GetRightVector()   { return TVector3(static_cast<Type>(1), static_cast<Type>(0), static_cast<Type>(0)); }
        static constexpr TVector3 GetForwardVector() { return TVector3(static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(1)); }
        static constexpr TVector3 GetYawAxis()       { return GetUpVector(); }
        static constexpr TVector3 GetPitchAxis()     { return GetRightVector(); }
        static constexpr TVector3 GetRollAxis()      { return GetForwardVector(); }
    };

    template <FloatingPointType Type>
    Type ScalarTripleProduct(const TVector3<Type>& u, const TVector3<Type>& v, const TVector3<Type>& w);

    using Vector3f = TVector3<float>;
    using Vector3d = TVector3<double>;
    using Vector3i = TVector3<int>;
    using Vector3u = TVector3<unsigned int>;
    using Vector3 = TVector3<NES_MATH_DEFAULT_REAL_TYPE>;
}
                                                     
namespace nes
{
    template <ScalarType Type>
    constexpr bool TVector3<Type>::operator==(const TVector3 right) const
    {
        return x == right.x && y == right.y && z == right.z;
    }

    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::operator-() const
    {
        TVector3 result;
        result.x = -x;
        result.y = -y;
        result.z = -z;
        return result;
    }

    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::operator+(const TVector3 right) const
    {
        TVector3 result = right;
        result.x += x;
        result.y += y;
        result.z += z;
        return result;
    }

    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::operator-(const TVector3 right) const
    {
        TVector3 result = right;
        result.x -= x;
        result.y -= y;
        result.z -= z;
        return result;
    }

    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::operator*(const TVector3 right) const
    {
        TVector3 result = right;
        result.x *= x;
        result.y *= y;
        result.z *= z;
        return result;
    }

    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::operator/(const TVector3 right) const
    {
        TVector3 result = right;
        result.x /= x;
        result.y /= y;
        result.z /= z;
        return result;
    }

    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::operator*(const ScalarType auto scalar) const
    {
        TVector3 result = *this;
        result.x *= scalar;
        result.y *= scalar;
        result.z *= scalar;
        return result;
    }

    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::operator/(const ScalarType auto scalar) const
    {
        NES_ASSERT(scalar != static_cast<decltype(scalar)>(0));
        
        TVector3 result = *this;
        result.x /= scalar;
        result.y /= scalar;
        result.z /= scalar;
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Sets the xy component values to match the 2D vector. This Z component is set to 0!
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    TVector3<Type>& TVector3<Type>::operator=(const TVector2<Type>& vector2)
    {
        x = vector2.x;
        y = vector2.y;
        z = 0.f;
        return *this;
    }

    template <ScalarType Type>
    TVector3<Type>& TVector3<Type>::operator-()
    {
        x = -x;
        y = -y;
        z = -z;
        return *this;
    }

    template <ScalarType Type>
    TVector3<Type>& TVector3<Type>::operator+=(const TVector3 right)
    {
        *this = *this + right;
        return *this;
    }

    template <ScalarType Type>
    TVector3<Type>& TVector3<Type>::operator-=(const TVector3 right)
    {
        *this = *this - right;
        return *this;
    }

    template <ScalarType Type>
    TVector3<Type>& TVector3<Type>::operator*=(const TVector3 right)
    {
        *this = *this * right;
        return *this;
    }

    template <ScalarType Type>
    TVector3<Type>& TVector3<Type>::operator*=(const ScalarType auto scalar)
    {
        *this = *this * scalar;
        return *this;
    }

    template <ScalarType Type>
    TVector3<Type>& TVector3<Type>::operator/=(const ScalarType auto scalar)
    {
        *this = *this / scalar;
        return *this;
    }
    
    template <ScalarType Type>
    constexpr Type& TVector3<Type>::operator[](const size_t index)
    {
        NES_ASSERT(index < 3);
        return *(&this->x + index);
    }

    template <ScalarType Type>
    constexpr Type TVector3<Type>::operator[](const size_t index) const
    {
        NES_ASSERT(index < 3);
        return *(&this->x + index);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the scalar length of this vector. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    Type TVector3<Type>::Magnitude() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the squared scalar length of this vector. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type TVector3<Type>::SquaredMagnitude() const
    {
        return (x * x) + (y * y) + (z * z);
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Dot Product between this and another vector.
    ///            The dot product geometrically represents the cosine of the angle between the two
    ///            Vectors, and can be used to determine if two Vectors are pointing in roughly
    ///            the same direction. For unit vectors, the dot product is equal to 1 if they are
    ///            the same, 0 if they are perpendicular, and -1 if they are opposite.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type TVector3<Type>::Dot(const TVector3& right) const
    {
        return x * right.x + y * right.y + z * right.z;
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Cross Product between this and another vector. The Cross product yields a vector that is
    ///             perpendicular to both vectors.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::Cross(const TVector3& right) const
    {
        TVector3 result;
        result.x = (y * right.z) - (right.y * z);
        result.y = (z * right.x) - (right.z * x);
        result.z = (x * right.y) - (right.x * y);
        return result;
    }

    //-------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Normalize this Vector (magnitude of 1). If you want to preserve this Vector, you can use GetNormalized(). 
    //-------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr TVector3<Type>& TVector3<Type>::Normalize()
    {
        static_assert(std::floating_point<Type>, "Type must be floating point");

        const auto magnitude = Magnitude();
        if (!math::CheckEqualFloats(magnitude, 0.f, 0.0001f))
        {
            x = x / magnitude;
            x = y / magnitude;
            x = z / magnitude;
        }

        return *this;
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Returns a normalized Vector based on this Vector's components.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::Normalized() const
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
    TVector3<To> TVector3<Type>::CastTo() const
    {
        static_assert(!std::same_as<Type, To>, "Attempting to cast Vector to the same type!");

        return TVector3<To>(static_cast<To>(x), static_cast<To>(y), static_cast<To>(z));
    }

    template <ScalarType Type>
    std::string TVector3<Type>::ToString() const
    {
        return CombineIntoString("(x=", x, ", y=", y, ", z=", z, ")");
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Dot Product between two vectors.
    ///            The dot product geometrically represents the cosine of the angle between the two
    ///            Vectors, and can be used to determine if two Vectors are pointing in roughly
    ///            the same direction. For unit vectors, the dot product is equal to 1 if they are
    ///            the same, 0 if they are perpendicular, and -1 if they are opposite.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type TVector3<Type>::Dot(const TVector3& a, const TVector3& b)
    {
        return a.Dot(b);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the distance between the two Vectors. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type TVector3<Type>::Distance(const TVector3& a, const TVector3& b)
    {
        TVector3 diff = b - a;
        return diff.Magnitude();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the squared distance between the two Vectors. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type TVector3<Type>::DistanceSquared(const TVector3& a, const TVector3& b)
    {
        TVector3 diff = b - a;
        return diff.SquaredMagnitude();
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Calculate the Cross Product between two vectors. The Cross product yields a vector that is
    ///             perpendicular to both vectors.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::Cross(const TVector3& a, const TVector3& b)
    {
        return a.Cross(b);
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
    constexpr TVector3<Type> TVector3<Type>::Lerp(const TVector3 from, const TVector3 to, const float t)
    {
        return from + ((to - from) * t);
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Returns the angle (in radians) between two vectors. This assumes both vectors' origins are equal.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    float TVector3<Type>::AngleBetweenVectors(const TVector3& a, const TVector3& b)
    {
        const auto dot = a.Normalized().Dot(b.Normalized());
        return std::acos(dot);
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Returns the angle (in degrees) between two vectors. This assumes both vectors' origins are equal.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    float TVector3<Type>::AngleBetweenVectorsDegrees(const TVector3& a, const TVector3& b)
    {
        const float angleRadians = AngleBetweenVectors(a, b);
        return math::ToDegrees(angleRadians);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : The Scalar Triple Product is found by taking the cross product between u and v, and then
    ///              using that result and getting the dot product with w: (u x v) * w. The resulting value represents
    ///             the *signed* volume of the parallelepiped formed by the three vectors.
    ///             - If the result is 0, then the three vectors are all coplanar.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type ScalarTripleProduct(const TVector3<Type>& u, const TVector3<Type>& v, const TVector3<Type>& w)
    {
        return TVector3<Type>::Cross(u, v).Dot(w);
    }
}