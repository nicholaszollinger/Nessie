// EntityLayer.cpp
#include "EntityLayer.h"

namespace nes
{
    void EntityLayer::RegisterTick(const TickFunction& function)
    {
        // [TODO]: Need better registration management.
        m_tickFunctions.emplace_back(function);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Begin destroying this Layer. 
    //----------------------------------------------------------------------------------------------------
    void EntityLayer::DestroyLayer()
    {
        m_isBeingDestroyed = true;
        m_tickFunctions.clear();
        OnLayerDestroyed();
    }
}