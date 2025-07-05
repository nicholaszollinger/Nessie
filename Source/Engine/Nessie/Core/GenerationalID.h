// GenerationalID.hpp
#pragma once
#include <string>
#include "Nessie/Core/Concepts.h"
#include "Nessie/Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Generational ID tracks a unique ID and a Generation Value in a single unsigned
    ///     integral type. The ID is stored in the lower half of the bits, and the Generation value is
    ///     stored in the upper half of the bits.
    /// @note : The ID value must less than the maximum value of the lower half of the bits!
    ///     So if you are using a 64-bit unsigned integral type, the maximum ID value is
    ///     2^32 = 4,294,967,296 = <code>std::numeric_limits<uint32_t>::max()</code>.
    ///	@tparam Type : Unsigned integral Type to use for the Generational ID.
    //----------------------------------------------------------------------------------------------------
    template <UnsignedIntegralType Type>
    class GenerationalID
    {
    public:
        static constexpr uint8 kHalfSize = (sizeof(Type) / 2) * 8;
        static constexpr Type kInitialGeneration = static_cast<Type>(1) << kHalfSize;   // Start at Generation 1.
        static constexpr Type kGenerationMask = kInitialGeneration - 1;                 // All Bits of the lower half set to 1.

    public:
        // Default initializes to an invalid ID.
        constexpr GenerationalID() = default;
        constexpr GenerationalID(const IntegralType auto id);

        constexpr bool operator==(const GenerationalID& other) const = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Increments the internal generation value - this will cause this and another GenerationalID
        ///     with the same ID to not be equal.
        //----------------------------------------------------------------------------------------------------
        void                    IncrementGeneration();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the value of this ID.
        //----------------------------------------------------------------------------------------------------
        constexpr Type          GetValue() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the generation value. 
        //----------------------------------------------------------------------------------------------------
        constexpr Type          Generation() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Whether this ID has been initialized with a value (generation value > 0).
        //----------------------------------------------------------------------------------------------------
        constexpr bool          IsValid() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the max value that can be used with this ID.
        //----------------------------------------------------------------------------------------------------
        static constexpr Type   MaxValue() { return kInitialGeneration - 1; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a string representation of the ID. 
        //----------------------------------------------------------------------------------------------------
        std::string             ToString() const;

    private:
        /// Upper half of bits are the generation value, Lower half of bits are the index value.
        Type                    m_value = 0;
    };

    using GenerationalIndex = GenerationalID<uint64>;
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Hash struct for GenerationalIDs to use for std containers.
    //----------------------------------------------------------------------------------------------------
    template <UnsignedIntegralType Type>
    struct GenerationalIDHasher
    {
        uint64 operator()(const GenerationalID<Type> id) const;
    };
    
    template <UnsignedIntegralType Type>
    constexpr GenerationalID<Type>::GenerationalID(const IntegralType auto id)
    {
        NES_ASSERT(id >= 0 && id <= kGenerationMask, "Attempted to construct a GenerationalID with ID value out of range! Value must"
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
    constexpr Type GenerationalID<Type>::GetValue() const
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

        return CombineIntoString("ID: ", GetValue(), " | Generation: ", Generation());
    }
    
    template <UnsignedIntegralType Type>
    uint64_t GenerationalIDHasher<Type>::operator()(const GenerationalID<Type> id) const
    {
        static constexpr std::hash<Type> kHash;
        return kHash(id.GetValue());
    }
}