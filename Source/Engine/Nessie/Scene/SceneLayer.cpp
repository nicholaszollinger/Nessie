// SceneLayer.cpp
#include "SceneLayer.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Begin destroying this Layer. 
    //----------------------------------------------------------------------------------------------------
    void SceneLayer::DestroyLayer()
    {
        m_isBeingDestroyed = true;
        OnLayerDestroyed();
    }
}