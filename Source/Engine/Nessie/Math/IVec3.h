// IVec3.h
#pragma once
#include "Nessie/Math/MathTypes.h"
#include "Nessie/Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Templated integral vector class with 3 components (x, y, z). 
    //----------------------------------------------------------------------------------------------------
    template <IntegralType Type>
    struct TIntVec3
    {
        static constexpr size_t N = 3;
        
        Type x;
        Type y;
        Type z;

        constexpr NES_INLINE                    TIntVec3() = default;
        constexpr NES_INLINE                    TIntVec3(const TIntVec3&) = default;
        constexpr NES_INLINE                    TIntVec3(const IntegralType auto x, const IntegralType auto y, const IntegralType auto z) : x(static_cast<Type>(x)), y(static_cast<Type>(y)), z(static_cast<Type>(z)) {}
        constexpr NES_INLINE                    TIntVec3(const TIntVec2<Type> vec, const IntegralType auto z);
        constexpr NES_INLINE                    TIntVec3(const IntegralType auto uniformValue) : x(static_cast<Type>(uniformValue)), y(static_cast<Type>(uniformValue)), z(static_cast<Type>(uniformValue)) {}
        constexpr NES_INLINE TIntVec3&          operator=(const TIntVec3&) = default;

        /// Operators
        constexpr NES_INLINE bool               operator==(const TIntVec3& other) const       { return x == other.x && y == other.y && z == other.z; }
        constexpr NES_INLINE bool               operator!=(const TIntVec3& other) const       { return !(*this == other); }
        constexpr NES_INLINE bool               operator<(const TIntVec3& other) const        { return x < other.x && y < other.y && z < other.z; }
        constexpr NES_INLINE bool               operator>(const TIntVec3& other) const        { return x > other.x && y > other.y && z > other.z; }
        constexpr NES_INLINE bool               operator<=(const TIntVec3& other) const       { return x <= other.x && y <= other.y && z <= other.z; }
        constexpr NES_INLINE bool               operator>=(const TIntVec3& other) const       { return x >= other.x && y >= other.y && z >= other.z; }
        NES_INLINE Type                         operator[](const size_t index) const          { NES_ASSERT(index < N); return *(&x + index); }
        NES_INLINE Type&                        operator[](const size_t index)                { NES_ASSERT(index < N); return *(&x + index); }
        constexpr NES_INLINE TIntVec3           operator-() const                             { return TIntVec3(-x, -y, -z); }
        constexpr NES_INLINE TIntVec3           operator+(const TIntVec3 other) const;
        constexpr NES_INLINE TIntVec3           operator-(const TIntVec3 other) const;
        constexpr NES_INLINE TIntVec3           operator*(const TIntVec3 other) const;
        constexpr NES_INLINE TIntVec3           operator/(const TIntVec3 other) const;
        constexpr friend NES_INLINE TIntVec3    operator*(const TIntVec3 other, const Type scalar);
        constexpr NES_INLINE TIntVec3           operator*(const Type scalar) const;
        constexpr NES_INLINE TIntVec3           operator/(const Type scalar) const;
        constexpr NES_INLINE TIntVec3&          operator+=(const TIntVec3& other);
        constexpr NES_INLINE TIntVec3&          operator-=(const TIntVec3& other);
        constexpr NES_INLINE TIntVec3&          operator*=(const TIntVec3& other);
        constexpr NES_INLINE TIntVec3&          operator/=(const TIntVec3& other);
        constexpr NES_INLINE TIntVec3&          operator*=(const Type scalar);
        constexpr NES_INLINE TIntVec3&          operator/=(const Type scalar);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the absolute value of each component.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE TIntVec3                     Abs() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the squared length (magnitude) of the vector. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float                        LengthSqr() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the length (magnitude) of the vector. 
        //----------------------------------------------------------------------------------------------------
        NES_INLINE float                        Length() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the minimum value of X, Y.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Type                         MinComponent() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the maximum value of X, Y.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE Type                         MaxComponent() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the index of the minimum value between X, Y.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE int                          MinComponentIndex() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the index of the maximum value between X, Y.  
        //----------------------------------------------------------------------------------------------------
        NES_INLINE int                          MaxComponentIndex() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to zero. 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE TIntVec3    Zero()                                      { return TIntVec3(0, 0, 0); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to one.
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE TIntVec3    One()                                       { return TIntVec3(1, 1, 1); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the X Axis vector. 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE TIntVec3    AxisX()                                     { return TIntVec3(1, 0, 0); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Y Axis vector. 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE TIntVec3    AxisY()                                     { return TIntVec3(0, 1, 0); }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Y Axis vector. 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE TIntVec3    AxisZ()                                     { return TIntVec3(0, 0, 1); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Right axis vector (equal to X Axis). 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE TIntVec3    Right()                                     { return TIntVec3(1, 0, 0); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Up axis vector (equal to Y Axis). 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE TIntVec3    Up()                                        { return TIntVec3(0, 1, 0); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Forward axis vector (equal to Z Axis). 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE TIntVec3    Forward()                                   { return TIntVec3(0, 0, 1); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a vector with the minimum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE TIntVec3              Min(const TIntVec3 a, const TIntVec3 b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a vector with the maximum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE TIntVec3              Max(const TIntVec3 a, const TIntVec3 b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Compute the distance between two points.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float                 Distance(const TIntVec3& a, const TIntVec3& b)      { return (a - b).Length(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Compute the squared distance between two points.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float                 DistanceSqr(const TIntVec3& a, const TIntVec3& b)   { return (a - b).LengthSqr(); }
    };

    using IVec3 = TIntVec3<int>;
    using UVec3 = TIntVec3<uint32>;
}

#include "IVec3.inl"