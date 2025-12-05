// EntityRegistry.cpp
#include "Nessie/World.h"

namespace nes
{
    EntityRegistry::EntityRegistry()
    {
        // Register IDComponent and Lifetime Component Types.
        NES_REGISTER_COMPONENT(nes::IDComponent);
        NES_REGISTER_COMPONENT(nes::PendingInitialization);
        NES_REGISTER_COMPONENT(nes::PendingEnable);
        NES_REGISTER_COMPONENT(nes::PendingDisable);
        NES_REGISTER_COMPONENT(nes::DisabledComponent);
        NES_REGISTER_COMPONENT(nes::PendingDestruction);
    }

    EntityRegistry::EntityRegistry(EntityRegistry&& other) noexcept
        : m_entityMap(std::move(other.m_entityMap))
        , m_registry(std::move(other.m_registry))
    {
        //
    }

    EntityRegistry& EntityRegistry::operator=(EntityRegistry&& other) noexcept
    {
        if (this != &other)
        {
            m_entityMap = std::move(other.m_entityMap);
            m_registry = std::move(other.m_registry);
        }

        return *this;
    }

    EntityRegistry::~EntityRegistry()
    {
        Clear();
    }

    void EntityRegistry::Clear()
    {
        m_entityMap.clear();
        m_registry.clear();
    }

    EntityHandle EntityRegistry::CreateEntity(const std::string& name)
    {
        // Create a new entity, and add the ID Component.
        auto handle = m_registry.create();
        auto& idComp = m_registry.emplace<IDComponent>(handle, name);
        m_registry.emplace<PendingInitialization>(handle);
        m_registry.emplace<PendingEnable>(handle);

        // Add to the map:
        m_entityMap.emplace(idComp.GetID(), handle);
        
        return handle;
    }

    EntityHandle EntityRegistry::CreateEntity(const uint64 id, const std::string& name)
    {
        // Create a new entity, and add the ID Component.
        auto handle = m_registry.create();
        m_registry.emplace<IDComponent>(handle, id, name);
        m_registry.emplace<PendingInitialization>(handle);
        m_registry.emplace<PendingEnable>(handle);
        
        // Add to the map:
        m_entityMap.emplace(id, handle);

        return handle;
    }

    void EntityRegistry::MarkEntityForDestruction(const EntityHandle entity)
    {
        if (entity == kInvalidEntityHandle)
            return;

        m_registry.remove<PendingInitialization>(entity);

        // Disable on Destruction as well.
        TryDisableEntity(entity);
        m_registry.emplace_or_replace<PendingDestruction>(entity);
    }

    void EntityRegistry::DestroyEntity(const EntityHandle entity)
    {
        // Remove from the map:
        auto& idComp = m_registry.get<IDComponent>(entity);
        NES_ASSERT(m_entityMap.contains(idComp.GetID()));
        m_entityMap.erase(idComp.GetID());

        // Destroy the entity.
        m_registry.destroy(entity);
    }

    void EntityRegistry::DestroyEntities(const EntitiesPendingDestructionView& view)
    {
        // Unregister from the Entity Map:
        for (auto entity : view)
        {
            auto& idComp = m_registry.get<IDComponent>(entity);
            NES_ASSERT(m_entityMap.contains(idComp.GetID()));
            m_entityMap.erase(idComp.GetID());
        }

        // Destroy all entities that are marked to destroy.
        m_registry.destroy(view.begin(), view.end());
    }

    void EntityRegistry::MarkAllEntitiesForDestruction()
    {
        // Add a pending disable component to enabled entities
        {
            auto view = m_registry.view<entt::entity>(entt::exclude<DisabledComponent>);
            m_registry.insert<PendingDisable>(view.begin(), view.end());
        }
        
        // Add a Pending Destruction Component to all entities without one.
        {
            auto view = m_registry.view<entt::entity>(entt::exclude<PendingDestruction>);
            m_registry.insert<PendingDestruction>(view.begin(), view.end());
        }
    }

    void EntityRegistry::DestroyAllEntities()
    {
        // Clear our map and registry.
        m_entityMap.clear();
        m_registry.clear();
    }

    EntityHandle EntityRegistry::GetEntity(const EntityID id) const
    {
        if (auto it = m_entityMap.find(id); it != m_entityMap.end())
        {
            return it->second;
        }

        return kInvalidEntityHandle;
    }

    void EntityRegistry::TryEnableEntity(const EntityHandle entity)
    {
        if (entity == kInvalidEntityHandle)
            return;

        // Add Pending Enable only if disabled:
        if (m_registry.all_of<DisabledComponent>(entity))
        {
            m_registry.emplace<PendingEnable>(entity);
        }
    }

    void EntityRegistry::TryDisableEntity(const EntityHandle entity)
    {
        if (entity == kInvalidEntityHandle)
            return;

        // Add Pending Disable only if enabled.
        if (!m_registry.all_of<DisabledComponent>(entity))
        {
            m_registry.emplace<PendingDisable>(entity);
        }
    }

    void EntityRegistry::RemoveComponent(const entt::id_type componentTypeID, const EntityHandle handle)
    {
        // Get the storage pool for this component type
        auto* storage = m_registry.storage(componentTypeID);
        if (storage && storage->contains(handle))
        {
            storage->erase(handle);
        }
    }

    void* EntityRegistry::TryGetComponentRaw(const entt::id_type componentTypeID, const EntityHandle entity)
    {
        // Get the storage pool for this component type
        auto* storage = m_registry.storage(componentTypeID);

        if (!storage || !storage->contains(entity))
            return nullptr;
        
        // Get raw pointer to the component data
        return storage->value(entity);
    }

    bool EntityRegistry::HasComponent(const entt::id_type componentTypeID, const EntityHandle entity)
    {
        // Get the storage pool for this component type
        const auto* storage = m_registry.storage(componentTypeID);

        // Check if storage exists and contains the entity
        return storage && storage->contains(entity);
    }

    bool EntityRegistry::IsValidEntity(const EntityHandle entity) const
    {
        if (!m_registry.valid(entity))
            return false;
        
        const IDComponent* pProps = m_registry.try_get<IDComponent>(entity);
        return pProps != nullptr && m_entityMap.contains(pProps->GetID());
    }

    bool EntityRegistry::IsValidEntity(const EntityID id) const
    {
        auto handle = GetEntity(id);
        if (handle == kInvalidEntityHandle)
            return false;

        return IsValidEntity(handle);
    }
}
