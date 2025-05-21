// ActiveEdgeMode.h
#pragma once
#include <cstdint>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : How to treat active/inactive edges.
    ///     An active edge is an edge that either has no neighbouring edge or if the angle between
    ///     the two connecting faces is too large, see: ActiveEdges
    //----------------------------------------------------------------------------------------------------
    enum class ActiveEdgeMode : uint8_t
    {
        CollideOnlyWithActive,  /// Do not collide with inactive edges. For physics simulation, this gives less ghost collisions.
        CollideWithAll,         /// Collide with all edges. Use this when you're interested in all collisions.
    };
}