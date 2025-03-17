// Material.h
#pragma once
#include "Core/Generic/Color.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A Material defines a set of parameters to give to the Mesh Shaders. 
    //----------------------------------------------------------------------------------------------------
    struct Material
    {
        LinearColor m_baseColor;
        
        inline bool IsTransparent() const
        {
            return m_baseColor.a < 1.f;
        }
    };
}
