#pragma once
// GenerationalID.hpp

#include <string>
#include "Core/Generic/Concepts.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///     @brief : 
    ///     A Generational ID tracks a unique ID and a Generation Value in a single unsigned
    ///     integral type. The ID is stored in the lower half of the bits, and the Generation value is
    ///     stored in the upper half of the bits.
    ///
    ///     IMPORTANT: The ID value must less than the maximum value of the lower half of the bits!
    ///     So if you are using a 64-bit unsigned integral type, the maximum ID value is
    ///     2^32 = 4294967296 = <code>std::numeric_limits<uint32_t>::max()</code>.
    ///
    ///		@tparam Type : Unsigned integral Type to use for the Generational ID.
    //----------------------------------------------------------------------------------------------------
    template <UnsignedIntegralType Type = uint64_t>
    class GenerationalID
    {
    public:
        static constexpr uint8_t kHalfSize = (sizeof(Type) / 2) * 8;
        static constexpr Type kInitialGeneration = static_cast<Type>(1) << kHalfSize;   // Start at Generation 1.
        static constexpr Type kGenerationMask = kInitialGeneration - 1;                 // All Bits of the lower half set to 1.

        // Upper half of bits are the generation value,
        // Lower half of bits are the index value.
        Type m_value = 0;

    public:
        // Default initializes to an invalid ID.
        constexpr GenerationalID() = default;
        constexpr GenerationalID(const std::integral auto id);

        constexpr bool operator==(const GenerationalID& other) const = default;

        void IncrementGeneration();
        constexpr Type GetID() const;
        constexpr Type GetGeneration() const;
        constexpr bool IsValid() const;

        std::string ToString() const;
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Hash struct for GenerationalIDs to use for std containers.
    //----------------------------------------------------------------------------------------------------
    template <UnsignedIntegralType Type = uint64_t>
    struct GenerationalIDHasher
    {
        uint64_t operator()(const GenerationalID<Type> id) const;
    };

    using GenerationalIndex = GenerationalID<uint64_t>;
}