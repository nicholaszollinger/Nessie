#pragma once
// Bit.h
#include "Core/Generic/Concepts.h"

namespace nes::math
{
    constexpr auto BitVal(const std::integral auto n)
    {
        return 1 << n;
    }

    constexpr bool IsBitSet(const std::integral auto number, const std::integral auto n)
    {
        return number & (1 << n);
    }

    constexpr void SetBit(std::integral auto& number, const std::integral auto n)
    {
        number |= (1 << n);
    }

    constexpr void ClearBit(std::integral auto& number, const std::integral auto n)
    {
        number &= ~(1 << n);
    }

    constexpr void ToggleBit(std::integral auto& number, const std::integral auto n)
    {
        number ^= (1 << n);
    }
}