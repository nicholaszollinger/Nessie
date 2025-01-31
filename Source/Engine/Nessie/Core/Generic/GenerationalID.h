#pragma once
// GenerationalID.hpp

#include <string>
#include "Core/Generic/Concepts.h"
#include "Debug/Assert.h"

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
        constexpr Type ID() const;
        constexpr Type Generation() const;
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


    template <UnsignedIntegralType Type>
    constexpr GenerationalID<Type>::GenerationalID(const std::integral auto id)
    {
        NES_ASSERTV(id >= 0 && id <= kGenerationMask, "Attempted to construct a GenerationalID with ID value out of range! Value must"
                                           "be greater than 0 and less than ", kGenerationMask);
        m_value = id + kInitialGeneration;
    }

    template <UnsignedIntegralType Type>
    void GenerationalID<Type>::IncrementGeneration()
    {
        // Increment the generation value by 1.
        m_value += kInitialGeneration;
    }

    template <UnsignedIntegralType Type>
    constexpr Type GenerationalID<Type>::ID() const
    {
        // Mask out the upper half of bits.
        return m_value & kGenerationMask;
    }

    template <UnsignedIntegralType Type>
    constexpr Type GenerationalID<Type>::Generation() const
    {
        // Shift the value to the right by half of the total bits
        // to isolate the upper half bits.
        return m_value >> kHalfSize;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : A Generational Index is valid if the Generation value is greater than 0.
    //----------------------------------------------------------------------------------------------------
    template <UnsignedIntegralType Type>
    constexpr bool GenerationalID<Type>::IsValid() const
    {
        return Generation() != 0;
    }

    template <UnsignedIntegralType Type>
    std::string GenerationalID<Type>::ToString() const
    {
        if (!IsValid())
            return "Invalid ID!";

        return CombineIntoString("ID: ", ID(), " | Generation: ", Generation());
    }
    
    template <UnsignedIntegralType Type>
    uint64_t GenerationalIDHasher<Type>::operator()(const GenerationalID<Type> id) const
    {
        static constexpr std::hash<Type> kHash;
        return kHash(id.m_value);
    }
}