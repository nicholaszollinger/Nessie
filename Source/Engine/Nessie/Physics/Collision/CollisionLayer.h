// CollisionLayer.h
#pragma once
#include <cstdint>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Layer that Bodies can be in; determines which other Bodies that can collide with.  
    //----------------------------------------------------------------------------------------------------
    using CollisionLayer = uint16_t;
    static constexpr CollisionLayer kInvalidCollisionLayer = static_cast<CollisionLayer>(~static_cast<CollisionLayer>(0));

    // [TODO]: CollisionLayerFilter struct.
}