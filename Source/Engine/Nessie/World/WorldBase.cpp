// WorldBase.cpp
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

    void WorldBase::Destroy()
    {
        OnDestroy();

        // Destroy all entities, allowing systems to respond.
        m_entityRegistry.MarkAllEntitiesForDestruction();
        ProcessPendingDisable();
        ProcessPendingDestruction(true);
        m_entityRegistry.Clear();

        // Shutdown all systems in reverse order.
        for (auto it = m_systems.rbegin(); it != m_systems.rend(); ++it)
        {
            (*it)->Shutdown();
        }
        m_systems.clear();
    }

    void WorldBase::MergeWorld(WorldAsset& srcWorld)
    {
        auto& componentRegistry = ComponentRegistry::Get();
        auto componentTypes = componentRegistry.GetAllComponentTypes();
        auto& srcRegistry = srcWorld.GetRegistry();
        
        // All Entities must have an IDComponent, so this is equivalent to getting all entities. 
        auto view = srcRegistry.GetAllEntitiesWith<IDComponent>();
        for (auto srcEntity : view)
        {
            auto& idComp = view.get<IDComponent>(srcEntity);
            
            // Check if an Entity with that ID already exists.
            EntityHandle dstEntity = m_entityRegistry.GetEntity(idComp.GetID());

            // If not, create a new entity.
            if (dstEntity == kInvalidEntityHandle)
                dstEntity = m_entityRegistry.CreateEntity(idComp.GetID(), idComp.GetName());

            // Add all registered components that exist in the Registry.
            // - If the entity already exists, this will update its current components.
            for (const auto& desc : componentTypes)
            {
                NES_ASSERT(desc.m_copyFunction != nullptr);
                desc.m_copyFunction(srcRegistry, m_entityRegistry, srcEntity, dstEntity);
            }

            // Add Pending Initialization and Enable methods.
            m_entityRegistry.AddComponent<PendingInitialization>(dstEntity);
        }
    }

    void WorldBase::ExportToAsset(WorldAsset& dstAsset)
    {
        auto& componentRegistry = ComponentRegistry::Get();
        auto componentTypes = componentRegistry.GetAllComponentTypes();
        auto& srcRegistry = m_entityRegistry;

        // Clear the current asset registry:
        auto& dstRegistry = dstAsset.GetRegistry();
        dstRegistry.Clear();

        // All Entities must have an IDComponent, so this is equivalent to getting all entities. 
        auto view = srcRegistry.GetAllEntitiesWith<IDComponent>();
        for (auto srcEntity : view)
        {
            auto& idComp = view.get<IDComponent>(srcEntity);
            
            EntityHandle dstEntity = dstRegistry.CreateEntity(idComp.GetID(), idComp.GetName());

            // Add all registered components that exist in the Registry.
            // - If the entity already exists, this will update its current components.
            for (const auto& desc : componentTypes)
            {
                NES_ASSERT(desc.m_copyFunction != nullptr);
                desc.m_copyFunction(srcRegistry, dstRegistry, srcEntity, dstEntity);
            }
        }
    }

    EntityHandle WorldBase::CreateEntity(const std::string& newName)
    {
        const auto newEntity = m_entityRegistry.CreateEntity(newName);
        OnNewEntityCreated(newEntity);
        return newEntity;
    }

    void WorldBase::DestroyEntity(const EntityID entity)
    {
        const auto entityHandle = m_entityRegistry.GetEntity(entity);
        if (entityHandle != kInvalidEntityHandle)
            DestroyEntity(entityHandle);
    }

    void WorldBase::DestroyEntity(const EntityHandle entity)
    {
        m_entityRegistry.MarkEntityForDestruction(entity);
    }

    void WorldBase::ParentEntity(const EntityID entity, const EntityID parent)
    {
        const EntityHandle entityHandle = m_entityRegistry.GetEntity(entity);
        const EntityHandle parentHandle = m_entityRegistry.GetEntity(parent);
        ParentEntity(entityHandle, parentHandle);
    }

    void WorldBase::RemoveParent(const EntityID entity)
    {
        const EntityHandle entityHandle = m_entityRegistry.GetEntity(entity);
        ParentEntity(entityHandle, kInvalidEntityHandle);
    }

    void WorldBase::RemoveParent(const EntityHandle entity)
    {
        ParentEntity(entity, kInvalidEntityHandle);
    }

    bool WorldBase::IsDescendantOf(const EntityID entity, const EntityID potentialAncestor) const
    {
        if (entity == potentialAncestor)
            return true;
        
        EntityID currentID = entity;

        // Walk up the parent chain:
        while (currentID != kInvalidEntityID)
        {
            const auto handle = m_entityRegistry.GetEntity(currentID);
            if (handle == kInvalidEntityHandle)
                break;

            auto& nodeComp = m_entityRegistry.GetComponent<NodeComponent>(handle);
            currentID = nodeComp.m_parentID;

            // Found an ancestor in the parent chain.
            if (currentID == potentialAncestor)
                return true; 
        }

        return false;
    }

    void WorldBase::ProcessEntityLifecycle()
    {
        ProcessPendingInitialization();
        ProcessPendingEnable();
        ProcessPendingDisable();
        ProcessPendingDestruction();
    }

    void WorldBase::ProcessPendingInitialization()
    {
        // Check if we have entities to initialize.
        auto view = m_entityRegistry.GetAllEntitiesWith<PendingInitialization>();
        if (view.empty())
            return;

        for (auto& pSystem : m_systems)
        {
            pSystem->ProcessNewEntities();
        }

        // Clear all Pending Initialization components from the registry.
        m_entityRegistry.ClearAllComponentsOfType<PendingInitialization>();
    }

    void WorldBase::ProcessPendingEnable()
    {
        auto view = m_entityRegistry.GetAllEntitiesWith<PendingEnable>();
        if (view.empty())
            return;

        for (auto& pSystem : m_systems)
        {
            pSystem->ProcessEnabledEntities();
        }

        // Remove DisabledComponents for all entities in the view.
        m_entityRegistry.RemoveComponentFromAll<DisabledComponent>(view);

        // Clear all pending components.
        m_entityRegistry.ClearAllComponentsOfType<PendingEnable>();
    }

    void WorldBase::ProcessPendingDisable()
    {
        auto view = m_entityRegistry.GetAllEntitiesWith<PendingDisable>();
        if (view.empty())
            return;

        for (auto& pSystem : m_systems)
        {
            pSystem->ProcessDisabledEntities();
        }

        // Add DisabledComponents for all entities in the view.
        m_entityRegistry.AddComponentToAll<DisabledComponent>(view);

        // Clear all pending components.
        m_entityRegistry.ClearAllComponentsOfType<PendingDisable>();
    }

    void WorldBase::ProcessPendingDestruction(const bool destroyingWorld)
    {
        // Check if we have entities that need to be destroyed.
        auto view = m_entityRegistry.GetAllEntitiesWith<PendingDestruction>();
        if (view.empty())
            return;

        for (auto& pSystem : m_systems)
        {
            pSystem->ProcessDestroyedEntities(destroyingWorld);
        }

        m_entityRegistry.DestroyEntities(view);
    }
}
