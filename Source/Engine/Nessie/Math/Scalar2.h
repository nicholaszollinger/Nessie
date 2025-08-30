// Scalar2.h
#pragma once
#include "Nessie/Core/Concepts.h"
#include "Nessie/Math/Generic.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Generic storage class for two scalar values. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    struct TScalar2
    {
        Type                x;
        Type                y;

        /// Constructors
        constexpr           TScalar2() = default; // Uninitialized intentionally for performance reasons.
        constexpr           TScalar2(const Type x, const Type y) : x(x), y(y) {}
        explicit constexpr  TScalar2(const Type uniformValue) : x(uniformValue), y(uniformValue) {} 
        constexpr           TScalar2(const TScalar2& other) = default;
        TScalar2&           operator=(const TScalar2& other) = default;

        /// Index operators.
        Type                operator[] (const size_t index) const   { NES_ASSERT(index < 2); return *(&x + index); }
        Type&               operator[] (const size_t index)         { NES_ASSERT(index < 2); return *(&x + index); }

        /// Equality operators.
        constexpr bool      operator==(const TScalar2& other) const { return x == other.x && y == other.y; }
        constexpr bool      operator!=(const TScalar2& other) const { return !(*this == other); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Cast each element to another type.
        //----------------------------------------------------------------------------------------------------
        template <ScalarType Type2>
        TScalar2<Type2>     CastTo() const { return TScalar2<Type2>(static_cast<Type2>(x), static_cast<Type2>(y)); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a Scalar2 with both values set to zero.
        //----------------------------------------------------------------------------------------------------
        static constexpr TScalar2 Zero() { return TScalar2(0, 0); }
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Storage class for two floats. Convert to Vec2 to perform calculations. 
    //----------------------------------------------------------------------------------------------------
    using Float2 = TScalar2<float>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Storage class for two doubles. Convert to DVec2 to perform calculations. 
    //----------------------------------------------------------------------------------------------------
    using Double2 = TScalar2<double>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Storage class for two ints. Convert to IVec2 to perform calculations. 
    //----------------------------------------------------------------------------------------------------
    using Int2 = TScalar2<int>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Storage class for two 32-bit unsigned integers. Convert to UVec2 to perform calculations. 
    //----------------------------------------------------------------------------------------------------
    using UInt2 = TScalar2<uint32>;
}