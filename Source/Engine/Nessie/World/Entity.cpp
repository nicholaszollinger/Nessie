// Entity.cpp

#include "Entity.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Returns whether this Entity is valid within the Entity Registry.
    //----------------------------------------------------------------------------------------------------
    bool Entity::IsValid() const
    {
        return m_pRegistry && m_pRegistry->IsValidEntity(m_handle);
    }
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Destroys all entities and their components.
    //----------------------------------------------------------------------------------------------------
    void EntityRegistry::Clear()
    {
        m_registry.clear();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Create an Empty Entity, that has no components.
    //----------------------------------------------------------------------------------------------------
    Entity EntityRegistry::CreateEntity()
    {
        const EntityRegistryHandle handle = m_registry.create();

        Entity entity;
        entity.m_handle = handle;
        entity.m_pRegistry = this;
        return entity;
    }

    //----------------------------------------------------------------------------------------------------
    //      NOTES:
    //
    ///		@brief : Destroy an Entity and all its components.
    ///		@param entity : Entity to destroy.
    //----------------------------------------------------------------------------------------------------
    void EntityRegistry::DestroyEntity(Entity& entity)
    {
        // This returns a Version of the Recycled Entity. I don't have a use for it right now,
        // but something to keep in mind.
        const entt::entity handle = entity.m_handle;

        // Remove all Components
        // TODO: Do I have to do this with this version of EnTT?
        //m_registry.(id);

        // Destroy the Entity (recycles the identifier)
        m_registry.destroy(handle);

        entity.m_handle = internal::kNullEntity;

        // Do I want to keep this reference?
        //entity.m_pSystem = nullptr;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Returns whether this EntityRegistryHandle is a valid Entity identifier in the registry.
    ///		@param handle : EntityRegistryHandle to check.
    //----------------------------------------------------------------------------------------------------
    bool EntityRegistry::IsValidEntity(const EntityRegistryHandle handle) const
    {
        return m_registry.valid(handle);
    }
}