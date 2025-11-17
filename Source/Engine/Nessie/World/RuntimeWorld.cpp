// RuntimeWorld.cpp
#include "RuntimeWorld.h"

namespace nes
{
    EntityHandle World::CreateEntity(const std::string& newName)
    {
        auto* pRegistry = GetEntityRegistry();
        NES_ASSERT(pRegistry != nullptr);
        const auto newEntity = pRegistry->CreateEntity(newName);
        OnNewEntityCreated(*pRegistry, newEntity);
        return newEntity;
    }

    void World::DestroyEntity(EntityHandle entity)
    {
        GetEntityRegistry()->MarkEntityForDestruction(entity);
    }

    EntityRegistry* World::GetEntityRegistry()
    {
        if (m_pEntityRegistryOverride && !IsSimulating())
            return m_pEntityRegistryOverride;

        return &m_entityRegistry;
    }

    void World::OnBeginSimulation()
    {
        if (m_pEntityRegistryOverride != nullptr)
        {
            // Now that we are simulating, our GetEntityRegistry will now return our
            // own entity registry. Notify all Systems that the entity registry has changed:
            for (auto& pSystem : m_systems)
            {
                pSystem->OnEntityRegistryChanged(&m_entityRegistry, m_pEntityRegistryOverride);
            }
        }

        WorldBase::OnBeginSimulation();
    }

    void World::OnEndSimulation()
    {
        if (m_pEntityRegistryOverride != nullptr)
        {
            // Now that we are *not* simulating, our GetEntityRegistry will now return the
            // registry override. Notify all Systems that the entity registry has changed:
            for (auto& pSystem : m_systems)
            {
                pSystem->OnEntityRegistryChanged(m_pEntityRegistryOverride, &m_entityRegistry);
            }
        }
        
        WorldBase::OnEndSimulation();
    }
}
