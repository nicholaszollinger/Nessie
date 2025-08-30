// AABoxCast.h
#pragma once
#include "Nessie/Core/Memory/Memory.h"
#include "Nessie/Geometry/AABox.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Structure that holds an AABox moving linearly through 3D space. 
    //----------------------------------------------------------------------------------------------------
    struct AABoxCast
    {
        NES_OVERRIDE_NEW_DELETE

        AABox   m_box;          /// Axis aligned box at the starting location.
        Vec3    m_direction;    /// Direction and length of the cast (anything beyond this will not be reported as a hit).
    };
}