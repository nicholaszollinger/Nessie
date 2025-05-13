// Float3.h
#pragma once
#include "Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //		
    /// @brief : Class that holds 3 floats. Used as a storage class. Convert to a Vector3 for calculations. 
    //----------------------------------------------------------------------------------------------------
    struct Float3
    {
        float x{};
        float y{};
        float z{};
        
        Float3() = default;
        Float3(const Float3& other) = default;
        Float3& operator=(const Float3& other) = default;
        constexpr Float3(const float x, const float y, const float z) : x(x), y(y), z(z) {}

        float operator[](const int index) const
        {
            NES_ASSERT(index >= 0 && index < 3);
            return *(&x + index);
        }

        bool operator==(const Float3& other) const
        {
            return x == other.x && y == other.y && z == other.z;            
        }

        bool operator!=(const Float3& other) const
        {
            return !(*this == other);
        }
    };
}
