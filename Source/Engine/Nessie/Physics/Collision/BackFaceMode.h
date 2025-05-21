// BackFaceMode.h
#pragma once
#include <cstdint>

namespace nes
{
    enum class BackFaceMode : uint8_t
    {
        IgnoreBackFaces,        /// Ignore collision with back facing surfaces/triangles.
        CollideWithBackFaces,   /// Collide with back facing surfaces/triangles.
    };
}