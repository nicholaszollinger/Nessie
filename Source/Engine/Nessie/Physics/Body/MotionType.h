// MotionType.h
#pragma once
#include <cstdint>

namespace nes
{
    enum class EBodyMotionType : uint8_t
    {
        Static,         /// Non-movable.
        Kinematic,      /// Movable using velocities only - does not respond to any forces.
        Dynamic         /// Responds to forces.
    };
}
