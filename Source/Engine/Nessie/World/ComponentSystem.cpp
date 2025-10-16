// ComponentSystem.cpp
#include "Nessie/World.h"

namespace nes
{
    EntityRegistry& ComponentSystem::GetRegistry() const
    {
        return GetWorld().GetRegistry();
    }
    
    WorldBase& ComponentSystem::GetWorld() const
    {
        NES_ASSERT(m_pWorld != nullptr);
        return *m_pWorld;
    }
}
