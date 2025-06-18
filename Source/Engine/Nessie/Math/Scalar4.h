// Scalar4.h
#pragma once
#include "Core/Generic/Concepts.h"
#include "Math/Generic.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Generic storage class for 4 scalar values. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    struct TScalar4
    {
        Type x;
        Type y;
        Type z;
        Type w;

        /// Constructors.
        constexpr               TScalar4() = default; // Uninitialized intentionally for performance reasons.
        constexpr               TScalar4(const Type x, const Type y, const Type z, const Type w) : x(x), y(y), z(z), w(w) {}
        explicit constexpr      TScalar4(const Type uniformValue) : x(uniformValue), y(uniformValue), z(uniformValue), w(uniformValue) {}
        constexpr               TScalar4(const TScalar4& other) = default;
        TScalar4&               operator=(const TScalar4& other) = default;

        /// Index operators.
        Type                    operator[] (const size_t index) const   { NES_ASSERT(index < 4); return *(&x + index); }
        Type&                   operator[] (const size_t index)         { NES_ASSERT(index < 4); return *(&x + index); }

        /// Boolean operators
        constexpr bool          operator==(const TScalar4& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
        constexpr bool          operator!=(const TScalar4& other) const { return !(*this == other); }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Cast each element to another type.
        //----------------------------------------------------------------------------------------------------
        template <ScalarType Type2>
        TScalar4<Type2>     CastTo() const { return TScalar4<Type2>(static_cast<Type2>(x), static_cast<Type2>(y), static_cast<Type2>(z), static_cast<Type2>(w)); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a Scalar4 with all values set to zero.
        //----------------------------------------------------------------------------------------------------
        static constexpr TScalar4 Zero() { return TScalar4(0, 0, 0, 0); }
    };
    
    static_assert(std::is_trivial_v<TScalar4<float>>, "Scalar4 must be trivial!");

    //----------------------------------------------------------------------------------------------------
    /// @brief : Storage class for 4 floats. Convert to Vec4 to perform calculations. 
    //----------------------------------------------------------------------------------------------------
    using Float4 = TScalar4<float>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Storage class for 4 doubles. Convert to DVec4 to perform calculations. 
    //----------------------------------------------------------------------------------------------------
    using Double4 = TScalar4<double>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Storage class for 4 ints. Convert to IVec4 to perform calculations. 
    //----------------------------------------------------------------------------------------------------
    using Int4 = TScalar4<int>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Storage class for 4 32-bit unsigned integers. Convert to UVec4 to perform calculations. 
    //----------------------------------------------------------------------------------------------------
    using UInt4 = TScalar4<uint32>;
}
    
