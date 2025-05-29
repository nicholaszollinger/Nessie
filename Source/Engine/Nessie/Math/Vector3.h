// Vector3.h
#pragma once
#include "Generic.h"
#include "Vector2.h"
#include "Core/String/FormatString.h"
#include "Detail/ESwizzle.h"
#include "Math/Float3.h"

// [TODO]: Combine with VectorRegisterF.

namespace nes
{
    
    template <ScalarType Type>
    struct alignas (NES_VECTOR_ALIGNMENT) TVector3
    {
        static constexpr size_t N = 3;
        
        Type x{};
        Type y{};
        Type z{};

        constexpr TVector3() = default;
        constexpr TVector3(const Type x, const Type y, const Type z) : x(x) , y(y), z(z) {}
        constexpr TVector3(const ScalarType auto x, const ScalarType auto y, const ScalarType auto z) : x(static_cast<Type>(x)), y(static_cast<Type>(y)), z(static_cast<Type>(z)) {}
        constexpr TVector3(const TVector2<Type>& vector2, const Type z = 0) : x(vector2.x), y(vector2.y), z(z) {}
        explicit constexpr TVector3(const Type uniformValue) : x(uniformValue), y(uniformValue), z(uniformValue) {}
        
        TVector3&                   operator=(const TVector2<Type>& vector2);

        constexpr bool              operator==(const TVector3 right) const;
        constexpr bool              operator!=(const TVector3 right) const { return !(*this == right); }
        constexpr bool              operator<(const TVector3 right) const;
        constexpr bool              operator>(const TVector3 right) const;
        constexpr bool              operator<=(const TVector3 right) const;
        constexpr bool              operator>=(const TVector3 right) const;

        constexpr TVector3          operator-() const;
        constexpr TVector3          operator+(const TVector3 right) const;
        constexpr TVector3          operator-(const TVector3 right) const;
        constexpr TVector3          operator*(const TVector3 right) const;
        constexpr TVector3          operator/(const TVector3 right) const;
        constexpr TVector3          operator*(const ScalarType auto scalar) const;
        constexpr TVector3          operator/(const ScalarType auto scalar) const;

        TVector3&                   operator-();
        TVector3&                   operator+=(const TVector3 right);
        TVector3&                   operator-=(const TVector3 right);
        TVector3&                   operator*=(const TVector3 right);
        TVector3&                   operator*=(const ScalarType auto scalar);
        TVector3&                   operator/=(const ScalarType auto scalar);
        
        constexpr Type&             operator[](const size_t index);
        constexpr Type              operator[](const size_t index) const;
        
        Type                        Magnitude() const;
        constexpr Type              SquaredMagnitude() const;
        constexpr Type              Dot(const TVector3& right) const;
        constexpr TVector3          Cross(const TVector3& right) const;
        constexpr TVector3&         Normalize();
        constexpr TVector3          Normalized() const;
        constexpr TVector3          GetReciprocal() const                   { return Unit() / *this;}
        constexpr TVector2<Type>    GetXY() const                           { return TVector2<Type>(this->x, this->y); }
        constexpr int               GetHighestComponentIndex() const;
        constexpr int               GetLowestComponentIndex() const;
        TVector3                    Sqrt() const;
        TVector3                    Abs() const;
        TVector3                    GetSign() const;
        TVector3                    NormalizedOr(const TVector3& inZeroValue) const;
        TVector3                    GetNormalizedPerpendicular() const;
        bool                        IsNaN() const;
        bool                        IsClose(const TVector3& other, const float maxDistSqr = 1.0e-12f) const;
        bool                        IsNearZero(float maxDistSqr = 1.0e-12f) const;
        void                        StoreFloat3(Float3* pValue) const        { pValue->x = x; pValue->y = y; pValue->z = z; }
        float                       ReduceMin() const;
        

        template <ESwizzle X, ESwizzle Y, ESwizzle Z>
        TVector3                    Swizzle() const;
        
        template <ScalarType To>
        TVector3<To>                CastTo() const;
        
        [[nodiscard]] std::string   ToString() const;
        
        static constexpr Type       Dot(const TVector3& a, const TVector3& b);
        static constexpr Type       Distance(const TVector3& a, const TVector3& b);
        static constexpr Type       DistanceSquared(const TVector3& a, const TVector3& b);
        static constexpr TVector3   Min(const TVector3& a, const TVector3& b);
        static constexpr TVector3   Max(const TVector3& a, const TVector3& b);
        //static VectorRegisterUint   Less(const TVector3& a, const TVector3& b);
        static constexpr TVector3   Cross(const TVector3& a, const TVector3& b);
        static constexpr TVector3   Lerp(const TVector3 from, const TVector3 to, const float t);
        static constexpr TVector3   Replicate(const Type value);
        
        static float                AngleBetweenVectors(const TVector3& a, const TVector3& b);
        static float                AngleBetweenVectorsDegrees(const TVector3& a, const TVector3& b);

        
        static constexpr TVector3   AxisX()       { return TVector3(static_cast<Type>(1), static_cast<Type>(0), static_cast<Type>(0)); }
        static constexpr TVector3   AxisY()       { return TVector3(static_cast<Type>(0), static_cast<Type>(1), static_cast<Type>(0)); }
        static constexpr TVector3   AxisZ()       { return TVector3(static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(1)); }
        static constexpr TVector3   Unit()        { return TVector3(static_cast<Type>(1), static_cast<Type>(1), static_cast<Type>(1)); }
        static constexpr TVector3   Zero()        { return TVector3(static_cast<Type>(0), static_cast<Type>(0), static_cast<Type>(0)); }
        static constexpr TVector3   Up()          { return AxisY(); }
        static constexpr TVector3   Right()       { return AxisX(); }
        static constexpr TVector3   Forward()     { return AxisZ(); }
        static constexpr TVector3   YawAxis()     { return AxisY(); }
        static constexpr TVector3   PitchAxis()   { return AxisX(); }
        static constexpr TVector3   RollAxis()    { return AxisZ(); }
    };

    template <ScalarType VecType>
    TVector3<VecType> operator*(const ScalarType auto scalar, const TVector3<VecType> vec);

    template <FloatingPointType Type>
    Type ScalarTripleProduct(const TVector3<Type>& u, const TVector3<Type>& v, const TVector3<Type>& w);

    template <FloatingPointType Type>
    bool IsLeftHanded(const TVector3<Type>& x, const TVector3<Type>& y, const TVector3<Type>& z)
    {
        return ScalarTripleProduct(x, y, z) < 0.f;
    }

    using Vector3f = TVector3<float>;
    using Vector3d = TVector3<double>;
    using Vector3i = TVector3<int>;
    using Vector3u = TVector3<uint32_t>;
    using Vector3 = TVector3<NES_PRECISION_TYPE>;
}
                                                     
namespace nes
{
    template <ScalarType Type>
    constexpr bool TVector3<Type>::operator==(const TVector3 right) const
    {
        return x == right.x && y == right.y && z == right.z;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : True if all components are less than the other vector's.  
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr bool TVector3<Type>::operator<(const TVector3 right) const
    {
        return x < right.x && y < right.y && z < right.z;
    }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : True if all components are greater than the other vector's.  
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr bool TVector3<Type>::operator>(const TVector3 right) const
    {
        return x > right.x && y > right.y && z > right.z;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : True if all components are less than or equal to the right.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr bool TVector3<Type>::operator<=(const TVector3 right) const
    {
        return x <= right.x && y <= right.y && z <= right.z;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : True if all components are greater than or equal to the right.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr bool TVector3<Type>::operator>=(const TVector3 right) const
    {
        return x >= right.x && y >= right.y && z >= right.z;
    }

    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::operator-() const
    {
        return { -x, -y, -z };
    }

    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::operator+(const TVector3 right) const
    {
        return { x + right.x, y + right.y, z + right.z };
    }

    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::operator-(const TVector3 right) const
    {
        return { x - right.x, y - right.y, z - right.z };
    }

    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::operator*(const TVector3 right) const
    {
        return { x * right.x, y * right.y, z * right.z };
    }

    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::operator/(const TVector3 right) const
    {
        return { x / right.x, y / right.y, z / right.z };
    }

    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::operator*(const ScalarType auto scalar) const
    {
        return { x * static_cast<Type>(scalar), y * static_cast<Type>(scalar), z * static_cast<Type>(scalar) };
    }

    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::operator/(const ScalarType auto scalar) const
    {
        NES_ASSERT(scalar != static_cast<decltype(scalar)>(0));
        
        return { x / static_cast<Type>(scalar), y / static_cast<Type>(scalar), z / static_cast<Type>(scalar) };
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
            y = y / magnitude;
            z = z / magnitude;
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
    /// @brief : Returns the index of the component with the highest value. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr int TVector3<Type>::GetHighestComponentIndex() const
    {
        return x > y ? (z > x ? 2 : 0) : (z > y ? 2 : 1);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Returns the index of the component with the lowest value. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr int TVector3<Type>::GetLowestComponentIndex() const
    {
        return x < y ? (z < x ? 2 : 0) : (z < y ? 2 : 1);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Component-wise Square root. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    TVector3<Type> TVector3<Type>::Sqrt() const
    {
        return TVector3<Type>(std::sqrt(x), std::sqrt(y), std::sqrt(z));
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Component-wise Abs(). 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    TVector3<Type> TVector3<Type>::Abs() const
    {
        return TVector3<Type>(math::Abs(x), math::Abs(y), math::Abs(z));
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Returns the sign (-1.0, or 1.0) of each component in a vector.  
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    TVector3<Type> TVector3<Type>::GetSign() const
    {
        return Vector3(std::signbit(x)? -1.0f : 1.0f,
                std::signbit(y)? -1.0f : 1.0f,
                std::signbit(z)? -1.0f : 1.0f);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Normalize vector or return inZeroValue if the length of the vector is zero
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    TVector3<Type> TVector3<Type>::NormalizedOr(const TVector3& inZeroValue) const
    {
        float lengthSqr = SquaredMagnitude();
        if (lengthSqr < FLT_MAX)
            return inZeroValue;

        return *this / std::sqrt(lengthSqr);
    }

    template <ScalarType Type>
    TVector3<Type> TVector3<Type>::GetNormalizedPerpendicular() const
    {
        if (math::Abs(x) > math::Abs(y))
        {
            const float length = std::sqrt(x * x + z * z);
            return Vector3(z, 0.f, -x) / length;
        }
        else
        {
            const float length = std::sqrt(y * y + z * z);
            return Vector3(0.f, z, -y) / length;
        }
    }

    template <ScalarType Type>
    bool TVector3<Type>::IsClose(const TVector3& other, const float maxDistSqr) const
    {
        return (other - *this).SquaredMagnitude() < maxDistSqr;
    }

    template <ScalarType Type>
    bool TVector3<Type>::IsNearZero(float maxDistSqr) const
    {
        return SquaredMagnitude() <= maxDistSqr;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Tests if the vector contains any NaN components. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    bool TVector3<Type>::IsNaN() const
    {
        return math::IsNan(x) || math::IsNan(y) || math::IsNan(z);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Get the lowest value between the X, Y and Z components. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    float TVector3<Type>::ReduceMin() const
    {
        TVector3 vec = Min(*this, Swizzle<ESwizzle::Y, ESwizzle::Unused, ESwizzle::Z>());
        vec = Min(vec, Swizzle<ESwizzle::Z, ESwizzle::Unused, ESwizzle::Unused>());
        return vec.x;
    }

    template <ScalarType Type>
    template <ESwizzle X, ESwizzle Y, ESwizzle Z>
    TVector3<Type> TVector3<Type>::Swizzle() const
    {
        static_assert(static_cast<size_t>(X) < 3, "Swizzle X must be less than 3");
        static_assert(static_cast<size_t>(Y) < 3, "Swizzle X must be less than 3");
        static_assert(static_cast<size_t>(Z) < 3, "Swizzle X must be less than 3");

        // [TODO]: SIMD version
        auto& vec = *this;
        return TVector3(vec[static_cast<size_t>(X)], vec[static_cast<size_t>(Y)], vec[static_cast<size_t>(Z)]);
    }

    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::Replicate(const Type value)
    {
        // [TODO]: SIMD version.
        return TVector3<Type>(value, value, value);
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

    //----------------------------------------------------------------------------------------------------
    /// @brief : Returns the minimum values of each of the components.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::Min(const TVector3& a, const TVector3& b)
    {
        TVector3 result;
        result.x = math::Min(a.x, b.x);
        result.y = math::Min(a.y, b.y);
        result.z = math::Min(a.z, b.z);
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Returns the maximum values of each of the components.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr TVector3<Type> TVector3<Type>::Max(const TVector3& a, const TVector3& b)
    {
        TVector3 result;
        result.x = math::Max(a.x, b.x);
        result.y = math::Max(a.y, b.y);
        result.z = math::Max(a.z, b.z);
        return result;
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

    template <ScalarType VecType>
    TVector3<VecType> operator*(const ScalarType auto scalar, const TVector3<VecType> vec)
    {
        return vec * scalar;
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