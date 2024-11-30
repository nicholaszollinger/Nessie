// GenerationalID.cpp

#include "GenerationalID.h"
#include "Core/Log/Log.h"

namespace nes
{
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
    constexpr Type GenerationalID<Type>::GetID() const
    {
        // Mask out the upper half of bits.
        return m_value & kGenerationMask;
    }

    template <UnsignedIntegralType Type>
    constexpr Type GenerationalID<Type>::GetGeneration() const
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
        return GetGeneration() != 0;
    }

    template <UnsignedIntegralType Type>
    std::string GenerationalID<Type>::ToString() const
    {
        if (!IsValid())
            return "Invalid ID!";

        return CombineIntoString("ID: ", GetID(), " | Generation: ", GetGeneration());
    }

    template <UnsignedIntegralType Type>
    uint64_t GenerationalIDHasher<Type>::operator()(const GenerationalID<Type> id) const
    {
        static constexpr std::hash<Type> kHash;
        return kHash(id.m_value);
    }
}