// ActorComponent.h
#pragma once
#include "Scene/Component.h"

namespace nes
{
    class Entity3D;
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Components that can be attached to an Entity that exist in 3D space. 
    //----------------------------------------------------------------------------------------------------
    using Entity3DComponent = TComponent<Entity3D>;
}
