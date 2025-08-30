// IVec4.h
#pragma once
#include "Nessie/Math/MathTypes.h"
#include "Nessie/Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Templated integral vector class with 4 components (x, y, z, w). 
    //----------------------------------------------------------------------------------------------------
    template <IntegralType Type>
    struct TIntVec4
    {
        static constexpr size_t N = 4;
        
        Type x;
        Type y;
        Type z;
        Type w;

        constexpr NES_INLINE                    TIntVec4() = default;
        constexpr NES_INLINE                    TIntVec4(const TIntVec4&) = default;
        constexpr NES_INLINE                    TIntVec4(const IntegralType auto x, const IntegralType auto y, const IntegralType auto z, const IntegralType auto w) : x(static_cast<Type>(x)), y(static_cast<Type>(y)), z(static_cast<Type>(z)), w(static_cast<Type>(w)) {}
        constexpr NES_INLINE                    TIntVec4(const TIntVec3<Type> vec, const IntegralType auto w);
        constexpr NES_INLINE                    TIntVec4(const IntegralType auto uniformValue) : x(static_cast<Type>(uniformValue)), y(static_cast<Type>(uniformValue)), z(static_cast<Type>(uniformValue)), w(static_cast<Type>(uniformValue)) {}
        constexpr NES_INLINE TIntVec4&          operator=(const TIntVec4&) = default;

        /// Operators
        constexpr NES_INLINE bool               operator==(const TIntVec4& other) const       { return x == other.x && y == other.y && z == other.z && w == other.w; }
        constexpr NES_INLINE bool               operator!=(const TIntVec4& other) const       { return !(*this == other); }
        constexpr NES_INLINE bool               operator<(const TIntVec4& other) const        { return x < other.x && y < other.y && z < other.z && w < other.w; }
        constexpr NES_INLINE bool               operator>(const TIntVec4& other) const        { return x > other.x && y > other.y && z > other.z && w > other.w; }
        constexpr NES_INLINE bool               operator<=(const TIntVec4& other) const       { return x <= other.x && y <= other.y && z <= other.z && w <= other.w; }
        constexpr NES_INLINE bool               operator>=(const TIntVec4& other) const       { return x >= other.x && y >= other.y && z >= other.z && w >= other.w; }
        NES_INLINE Type                         operator[](const size_t index) const          { NES_ASSERT(index < N); return *(&x + index); }
        NES_INLINE Type&                        operator[](const size_t index)                { NES_ASSERT(index < N); return *(&x + index); }
        constexpr NES_INLINE TIntVec4           operator-() const                             { return TIntVec4(-x, -y, -z, -w); }
        constexpr NES_INLINE TIntVec4           operator+(const TIntVec4 other) const;
        constexpr NES_INLINE TIntVec4           operator-(const TIntVec4 other) const;
        constexpr NES_INLINE TIntVec4           operator*(const TIntVec4 other) const;
        constexpr NES_INLINE TIntVec4           operator/(const TIntVec4 other) const;
        constexpr friend NES_INLINE TIntVec4    operator*(const TIntVec4 other, const Type scalar);
        constexpr NES_INLINE TIntVec4           operator*(const Type scalar) const;
        constexpr NES_INLINE TIntVec4           operator/(const Type scalar) const;
        constexpr NES_INLINE TIntVec4&          operator+=(const TIntVec4& other);
        constexpr NES_INLINE TIntVec4&          operator-=(const TIntVec4& other);
        constexpr NES_INLINE TIntVec4&          operator*=(const TIntVec4& other);
        constexpr NES_INLINE TIntVec4&          operator/=(const TIntVec4& other);
        constexpr NES_INLINE TIntVec4&          operator*=(const Type scalar);
        constexpr NES_INLINE TIntVec4&          operator/=(const Type scalar);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the absolute value of each component.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE TIntVec4                     Abs() const;

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
        static constexpr NES_INLINE TIntVec4    Zero()                                      { return TIntVec4(0, 0, 0); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to one.
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE TIntVec4    One()                                       { return TIntVec4(1, 1, 1); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a vector with the minimum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE TIntVec4              Min(const TIntVec4 a, const TIntVec4 b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a vector with the maximum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE TIntVec4              Max(const TIntVec4 a, const TIntVec4 b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Compute the distance between two points.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float                 Distance(const TIntVec4& a, const TIntVec4& b)      { return (a - b).Length(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Compute the squared distance between two points.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float                 DistanceSqr(const TIntVec4& a, const TIntVec4& b)   { return (a - b).LengthSqr(); }
    };

    using IVec4 = TIntVec4<int>;
    using UVec4 = TIntVec4<uint32>;
}

#include "IVec4.inl"
