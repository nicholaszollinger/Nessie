// EntityLayer.cpp
#include "EntityLayer.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Begin destroying this Layer. 
    //----------------------------------------------------------------------------------------------------
    void EntityLayer::DestroyLayer()
    {
        m_isBeingDestroyed = true;
        OnLayerDestroyed();
    }
}