// ComponentSystem.cpp
#include "Nessie/World.h"

namespace nes
{
    void ComponentSystem::SetWorld(WorldBase& world)
    {
        if (m_pWorld != nullptr)
            OnWorldRemoved();
        
        m_pWorld = &world;
        OnWorldSet();
    }

    EntityRegistry* ComponentSystem::GetEntityRegistry() const
    {
        return GetWorld().GetEntityRegistry();
    }
    
    WorldBase& ComponentSystem::GetWorld() const
    {
        NES_ASSERT(m_pWorld != nullptr);
        return *m_pWorld;
    }
}
