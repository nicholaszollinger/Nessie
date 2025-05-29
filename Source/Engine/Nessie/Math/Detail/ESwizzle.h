// Swizzle.h
#pragma once
#include <cstdint>

namespace nes
{
    enum class ESwizzle : uint8_t
    {
        X = 0,      /// Use the X Component
        Y = 1,      /// Use the Y Component
        Z = 2,      /// Use the Z Component
        W = 3,      /// Use the W Component
        Unused = 2, /// Always use the Z component when we don't initialize a value. This is for the Vector3 class when using registers. 
    };
}