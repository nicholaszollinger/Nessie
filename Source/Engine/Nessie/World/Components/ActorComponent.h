// ActorComponent.h
#pragma once
#include "Scene/Component.h"

namespace nes
{
    class Actor;
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Components that can be attached to an Actor. 
    //----------------------------------------------------------------------------------------------------
    using ActorComponent = TComponent<Actor>;
}
