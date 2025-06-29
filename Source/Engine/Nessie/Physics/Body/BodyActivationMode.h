// BodyActivationMode.h
#pragma once
#include <cstdint>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Enum defining how bodies active state should be handled when adding to a Physics Scene. 
    //----------------------------------------------------------------------------------------------------
    enum class EBodyActivationMode : uint8_t
    {   
        Activate,    /// Activate the Body, making it part of the simulation.
        DontActivate,   /// Leave the activation state as it is. This will not deactivate an active body!
    };
}
