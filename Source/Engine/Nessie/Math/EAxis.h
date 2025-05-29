// Axis.h
#pragma once
#include <cstdint>

namespace nes
{
    enum class EAxis : uint8_t
    {
        None    = 0,
        X       = 1,
        Y       = 2,
        Z       = 4,
        W       = 8,
    };
}
