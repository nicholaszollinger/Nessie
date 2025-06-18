// CollectFacesMode.h
#pragma once
#include <cstdint>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Whether or not to collect faces, used by CastShape and CollideShape.
    //----------------------------------------------------------------------------------------------------
    enum class ECollectFacesMode : uint8_t
    {
        CollectFaces,       /// m_shape1/2Face is desired
        NoFaces,            /// m_shape1/2Face is not desired.
    };
}
