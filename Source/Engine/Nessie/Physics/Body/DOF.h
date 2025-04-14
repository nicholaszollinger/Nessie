// DOF.h
#pragma once
#include <cstdint>
#include "Core/Generic/Bitmask.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Determines the allowable "degrees of freedom" for moving a Body. 
    //----------------------------------------------------------------------------------------------------
    enum class AllowedDOFs : uint8_t
    {
        None            = 0b000000,                                 /// No degrees of freedom are allowed.
        All             = 0b111111,                                 /// All degrees of freedom are allowed.
        TranslationX    = 0b000001,                                 /// Body can move on the World X-Axis.
        TranslationY    = 0b000010,                                 /// Body can move on the World Y-Axis.
        TranslationZ    = 0b000100,                                 /// Body can move on the World Z-Axis.
        RotationX       = 0b001000,                                 /// Body can rotate on the World X-Axis.
        RotationY       = 0b010000,                                 /// Body can rotate on the World Y-Axis.
        RotationZ       = 0b100000,                                 /// Body can rotate on the World Z-Axis.
        Plane2D         = TranslationX | TranslationY | RotationZ,  /// Body can move in the World XY plane, and rotate on the Z Axis.
    };

    NES_DEFINE_BIT_OPERTATIONS_FOR_ENUM(AllowedDOFs);
}
