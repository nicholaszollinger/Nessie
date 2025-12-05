// WorldBase.cpp
#include <ranges>

#include "Nessie/World.h"

namespace nes
{
    bool WorldBase::Init()
    {
        // Add all Component Systems to the world.
        AddComponentSystems();

        // Initialize all systems:
        for (auto& pSystem : m_systems)
        {
            if (!pSystem->Init())
            {
                NES_ERROR("Failed to initialize World! Failed to initialize ComponentSystems!");
                return false;
            }
        }
        
        return PostInit();
    }

    void WorldBase::BeginSimulation()
    {
        if (IsSimulating())
            return;
        
        m_simState = EWorldSimState::Playing;
        OnBeginSimulation();
    }

    void WorldBase::SetPaused(const bool shouldPause)
    {
        if (!IsSimulating() || IsPaused() == shouldPause)
            return;

        m_simState = shouldPause? EWorldSimState::Paused : EWorldSimState::Playing;
    }

    void WorldBase::EndSimulation()
    {
        if (!IsSimulating())
            return;

        m_simState = EWorldSimState::Stopped;
        OnEndSimulation();
    }

    void WorldBase::Destroy()
    {
        OnDestroy();
        DestroyAllEntities();
        
        // Shutdown all systems in reverse order.
        for (auto it = m_systems.rbegin(); it != m_systems.rend(); ++it)
        {
            (*it)->Shutdown();
        }
        m_systems.clear();
        m_systemMap.clear();
    }

    void WorldBase::DestroyAllEntities()
    {
        if (auto* pRegistry = GetEntityRegistry())
        {
            // Destroy all entities, allowing systems to respond.
            pRegistry->MarkAllEntitiesForDestruction();
            ProcessPendingDisable(*pRegistry);
            ProcessPendingDestruction(*pRegistry, true);
            pRegistry->Clear();
        }
    }

    void WorldBase::MergeWorld(WorldAsset& srcWorld)
    {
        auto* pRegistry = GetEntityRegistry();
        if (pRegistry == nullptr)
            return;
        
        auto& componentRegistry = ComponentRegistry::Get();
        auto componentTypes = componentRegistry.GetAllComponentTypes();
        auto& srcRegistry = srcWorld.GetEntityRegistry();
        auto& dstRegistry = *pRegistry;

        // Add Entities to this world while maintaining the current order.
        for (auto srcEntityID : srcWorld.GetRootEntities())
        {
            MergeEntityAndChildren(srcRegistry, dstRegistry, componentTypes, srcEntityID);
        }
    }

    void WorldBase::DestroyEntity(const EntityID entity)
    {
        if (auto* pRegistry = GetEntityRegistry())
        {
            const auto entityHandle = pRegistry->GetEntity(entity);
            if (entityHandle != kInvalidEntityHandle)
                DestroyEntity(entityHandle);    
        }
    }

    void WorldBase::ParentEntity(const EntityID entity, const EntityID parent)
    {
        auto* pRegistry = GetEntityRegistry();
        if (pRegistry == nullptr)
            return;
        
        const EntityHandle entityHandle = pRegistry->GetEntity(entity);
        const EntityHandle parentHandle = pRegistry->GetEntity(parent);
        ParentEntity(entityHandle, parentHandle);
    }

    void WorldBase::RemoveParent(const EntityID entity)
    {
        auto* pRegistry = GetEntityRegistry();
        if (pRegistry == nullptr)
            return;
        
        const EntityHandle entityHandle = pRegistry->GetEntity(entity);
        ParentEntity(entityHandle, kInvalidEntityHandle);
    }

    void WorldBase::RemoveParent(const EntityHandle entity)
    {
        ParentEntity(entity, kInvalidEntityHandle);
    }

    bool WorldBase::IsDescendantOf(const EntityID entity, const EntityID potentialAncestor) const
    {
        const auto* pRegistry = GetEntityRegistry();
        if (pRegistry == nullptr)
            return false;
        
        if (entity == potentialAncestor)
            return true;
        
        EntityID currentID = entity;

        // Walk up the parent chain:
        while (currentID != kInvalidEntityID)
        {
            const auto handle = pRegistry->GetEntity(currentID);
            if (handle == kInvalidEntityHandle)
                break;

            auto& nodeComp = pRegistry->GetComponent<NodeComponent>(handle);
            currentID = nodeComp.m_parentID;

            // Found an ancestor in the parent chain.
            if (currentID == potentialAncestor)
                return true; 
        }

        return false;
    }

    StrongPtr<ComponentSystem> WorldBase::GetSystem(const entt::id_type typeID) const
    {
        if (auto it = m_systemMap.find(typeID); it != m_systemMap.end())
        {
            NES_ASSERT(it->second < m_systems.size());
            return m_systems[it->second];
        }

        return nullptr;
    }

    const EntityRegistry* WorldBase::GetEntityRegistry() const
    {
        // Call the non-const version:
        return const_cast<WorldBase*>(this)->GetEntityRegistry();
    }

    void WorldBase::ProcessEntityLifecycle()
    {
        auto* pRegistry = GetEntityRegistry();
        if (pRegistry == nullptr)
            return;
        
        ProcessPendingInitialization(*pRegistry);
        ProcessPendingEnable(*pRegistry);
        ProcessPendingDisable(*pRegistry);
        ProcessPendingDestruction(*pRegistry);
    }

    void WorldBase::OnBeginSimulation()
    {
        for (auto& pSystem : m_systems)
        {
            pSystem->OnBeginSimulation();
        }
    }

    void WorldBase::OnEndSimulation()
    {
        for (auto& pSystem : m_systems)
        {
            pSystem->OnEndSimulation();
        }
    }

    void WorldBase::MergeEntityAndChildren(EntityRegistry& srcRegistry, EntityRegistry& dstRegistry, const std::vector<ComponentTypeDesc>& componentTypes, EntityID srcEntityID)
    {
        const auto srcEntity = srcRegistry.GetEntity(srcEntityID);
        if (srcEntity == kInvalidEntityHandle)
            return;
        
        auto& idComp = srcRegistry.GetComponent<IDComponent>(srcEntity);
            
        // Check if an Entity with that ID already exists.
        EntityHandle dstEntity = dstRegistry.GetEntity(idComp.GetID());

        // If not, create a new entity.
        if (dstEntity == kInvalidEntityHandle)
            dstEntity = dstRegistry.CreateEntity(idComp.GetID(), idComp.GetName());

        // Add all registered components that exist in the Registry.
        // - If the entity already exists, this will update its current components.
        for (const auto& desc : componentTypes)
        {
            NES_ASSERT(desc.m_copyFunction != nullptr);
            desc.m_copyFunction(srcRegistry, dstRegistry, srcEntity, dstEntity);
        }

        // Add Pending Initialization and Enable methods.
        dstRegistry.AddComponent<PendingInitialization>(dstEntity);

        if (auto* pSrcNodeComp = srcRegistry.TryGetComponent<NodeComponent>(srcEntity))
        {
            for (const auto srcChildID : pSrcNodeComp->m_childrenIDs)
            {
                MergeEntityAndChildren(srcRegistry, dstRegistry, componentTypes, srcChildID);
            }
        }
    }

    void WorldBase::ProcessPendingInitialization(EntityRegistry& registry) const
    {
        // Check if we have entities to initialize.
        auto view = registry.GetAllEntitiesWith<PendingInitialization>();
        if (view.empty())
            return;

        for (auto& pSystem : m_systems)
        {
            pSystem->ProcessNewEntities();
        }

        // Clear all Pending Initialization components from the registry.
        registry.ClearAllComponentsOfType<PendingInitialization>();
    }

    void WorldBase::ProcessPendingEnable(EntityRegistry& registry) const
    {
        auto view = registry.GetAllEntitiesWith<PendingEnable>();
        if (view.empty())
            return;

        for (auto& pSystem : m_systems)
        {
            pSystem->ProcessEnabledEntities();
        }

        // Remove DisabledComponents for all entities in the view.
        registry.RemoveComponentFromAll<DisabledComponent>(view);

        // Clear all pending components.
        registry.ClearAllComponentsOfType<PendingEnable>();
    }

    void WorldBase::ProcessPendingDisable(EntityRegistry& registry) const
    {
        auto view = registry.GetAllEntitiesWith<PendingDisable>();
        if (view.empty())
            return;

        for (auto& pSystem : m_systems)
        {
            pSystem->ProcessDisabledEntities();
        }

        // Add DisabledComponents for all entities in the view.
        registry.AddComponentToAll<DisabledComponent>(view);

        // Clear all pending components.
        registry.ClearAllComponentsOfType<PendingDisable>();
    }

    void WorldBase::ProcessPendingDestruction(EntityRegistry& registry, const bool destroyingAllEntities) const
    {
        // Check if we have entities that need to be destroyed.
        auto view = registry.GetAllEntitiesWith<PendingDestruction>();
        if (view.empty())
            return;

        for (auto& pSystem : m_systems)
        {
            pSystem->ProcessDestroyedEntities(destroyingAllEntities);
        }

        registry.DestroyEntities(view);
    }
}
