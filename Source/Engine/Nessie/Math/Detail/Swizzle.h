// Swizzle.h
#pragma once
#include <cstdint>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    // I made this a non-class enum just so that I don't have to cast to uin32 values everywhere.
    /// @brief : Enum indicating which component to use when swizzling a vector's components. 
    //----------------------------------------------------------------------------------------------------
    enum
    {
        ESwizzleX       = 0, /// Use the X Component
        ESwizzleY       = 1, /// Use the Y Component
        ESwizzleZ       = 2, /// Use the Z Component
        ESwizzleW       = 3, /// Use the W Component
        ESwizzleUnused  = 2, /// Always use the Z component when we don't initialize a value. This is for the Vector3 class when using registers. 
    };
}