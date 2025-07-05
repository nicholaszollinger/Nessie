// IVec2.h
#pragma once
#include "Nessie/Math/MathTypes.h"
#include "Nessie/Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Templated integral vector class with 2 components (x, y). 
    //----------------------------------------------------------------------------------------------------
    template <IntegralType Type>
    struct TIntVec2
    {
        static constexpr size_t N = 2;
        
        Type x;
        Type y;

        constexpr NES_INLINE                    TIntVec2() = default;
        constexpr NES_INLINE                    TIntVec2(const TIntVec2&) = default;
        constexpr NES_INLINE                    TIntVec2(const IntegralType auto x, const IntegralType auto y) : x(static_cast<Type>(x)), y(static_cast<Type>(y)) {}
        constexpr NES_INLINE                    TIntVec2(const IntegralType auto uniformValue) : x(static_cast<Type>(uniformValue)), y(static_cast<Type>(uniformValue)) {}
        constexpr NES_INLINE TIntVec2&          operator=(const TIntVec2&) = default;

        /// Operators
        constexpr NES_INLINE bool               operator==(const TIntVec2& other) const       { return x == other.x && y == other.y; }
        constexpr NES_INLINE bool               operator!=(const TIntVec2& other) const       { return x != other.x || y != other.y; }
        constexpr NES_INLINE bool               operator<(const TIntVec2& other) const        { return x < other.x && y < other.y; }
        constexpr NES_INLINE bool               operator>(const TIntVec2& other) const        { return x > other.x && y > other.y; }
        constexpr NES_INLINE bool               operator<=(const TIntVec2& other) const       { return x <= other.x && y <= other.y; }
        constexpr NES_INLINE bool               operator>=(const TIntVec2& other) const       { return x >= other.x && y >= other.y; }
        NES_INLINE Type                         operator[](const size_t index) const          { NES_ASSERT(index < N); return *(&x + index); }
        NES_INLINE Type&                        operator[](const size_t index)                { NES_ASSERT(index < N); return *(&x + index); }
        constexpr NES_INLINE TIntVec2           operator-() const                             { return TIntVec2(-x, -y); }
        constexpr NES_INLINE TIntVec2           operator+(const TIntVec2 other) const;
        constexpr NES_INLINE TIntVec2           operator-(const TIntVec2 other) const;
        constexpr NES_INLINE TIntVec2           operator*(const TIntVec2 other) const;
        constexpr NES_INLINE TIntVec2           operator/(const TIntVec2 other) const;
        constexpr friend NES_INLINE TIntVec2    operator*(const TIntVec2 other, const Type scalar);
        constexpr NES_INLINE TIntVec2           operator*(const Type scalar) const;
        constexpr NES_INLINE TIntVec2           operator/(const Type scalar) const;
        constexpr NES_INLINE TIntVec2&          operator+=(const TIntVec2& other);
        constexpr NES_INLINE TIntVec2&          operator-=(const TIntVec2& other);
        constexpr NES_INLINE TIntVec2&          operator*=(const TIntVec2& other);
        constexpr NES_INLINE TIntVec2&          operator/=(const TIntVec2& other);
        constexpr NES_INLINE TIntVec2&          operator*=(const Type scalar);
        constexpr NES_INLINE TIntVec2&          operator/=(const Type scalar);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the absolute value of each component.
        //----------------------------------------------------------------------------------------------------
        NES_INLINE TIntVec2                     Abs() const;

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
        static constexpr NES_INLINE TIntVec2    Zero()                                      { return TIntVec2(0, 0); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Vector with all components set to one.
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE TIntVec2    One()                                       { return TIntVec2(1, 1); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the X Axis vector. 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE TIntVec2    AxisX()                                     { return TIntVec2(1, 0); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Y Axis vector. 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE TIntVec2    AxisY()                                     { return TIntVec2(0, 1); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Right axis vector (equal to X Axis). 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE TIntVec2    Right()                                     { return TIntVec2(1, 0); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the Up axis vector (equal to Y Axis). 
        //----------------------------------------------------------------------------------------------------
        static constexpr NES_INLINE TIntVec2    Up()                                        { return TIntVec2(0, 1); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a vector with the minimum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE TIntVec2              Min(const TIntVec2 a, const TIntVec2 b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a vector with the maximum value of each component.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE TIntVec2              Max(const TIntVec2 a, const TIntVec2 b);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Compute the distance between two points.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float                 Distance(const TIntVec2& a, const TIntVec2& b)      { return (a - b).Length(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Compute the squared distance between two points.
        //----------------------------------------------------------------------------------------------------
        static NES_INLINE float                 DistanceSqr(const TIntVec2& a, const TIntVec2& b)   { return (a - b).LengthSqr(); }
    };

    using IVec2 = TIntVec2<int>;
    using UVec2 = TIntVec2<uint32>;
}

#include "IVec2.inl"