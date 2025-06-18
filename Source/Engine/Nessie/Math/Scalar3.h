// Scalar3.h
#pragma once
#include "Core/Generic/Concepts.h"
#include "Math/Generic.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Generic storage class for 3 scalar values. 
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    struct TScalar3
    {
        Type x;
        Type y;
        Type z;

        /// Constructors.
        constexpr               TScalar3() = default; // Uninitialized intentionally for performance reasons.
        constexpr               TScalar3(const Type x, const Type y, const Type z) : x(x), y(y), z(z) {}
        explicit constexpr      TScalar3(const Type uniformValue) : x(uniformValue), y(uniformValue), z(uniformValue) {} 
        constexpr               TScalar3(const TScalar3& other) = default;
        TScalar3&               operator=(const TScalar3& other) = default;

        /// Index operators.
        Type                    operator[] (const size_t index) const   { NES_ASSERT(index < 3); return *(&x + index); }
        Type&                   operator[] (const size_t index)         { NES_ASSERT(index < 3); return *(&x + index); }

        /// Boolean operators
        constexpr bool          operator==(const TScalar3& other) const { return x == other.x && y == other.y && z == other.z; }
        constexpr bool          operator!=(const TScalar3& other) const { return !(*this == other); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Cast each element to another type.
        //----------------------------------------------------------------------------------------------------
        template <ScalarType Type2>
        TScalar3<Type2>     CastTo() const { return TScalar3<Type2>(static_cast<Type2>(x), static_cast<Type2>(y), static_cast<Type2>(z)); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a Scalar3 with all values set to zero.
        //----------------------------------------------------------------------------------------------------
        static constexpr TScalar3 Zero() { return TScalar3(0, 0, 0); }

    };
    
    static_assert(std::is_trivial_v<TScalar3<float>>, "Scalar3 must be trivial!");

    //----------------------------------------------------------------------------------------------------
    /// @brief : Storage class for 3 floats. Convert to Vec3 to perform calculations. 
    //----------------------------------------------------------------------------------------------------
    using Float3 = TScalar3<float>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Storage class for 3 doubles. Convert to DVec3 to perform calculations. 
    //----------------------------------------------------------------------------------------------------
    using Double3 = TScalar3<double>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Storage class for 3 ints. Convert to IVec3 to perform calculations. 
    //----------------------------------------------------------------------------------------------------
    using Int3 = TScalar3<int>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Storage class for 32-bit unsigned integers. Convert to UVec3 to perform calculations. 
    //----------------------------------------------------------------------------------------------------
    using UInt3 = TScalar3<uint32>;
}