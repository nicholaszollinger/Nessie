#pragma once
// Entity.h
#include <entt/entt.hpp>
#include "Component.h"
#include "Debug/Assert.h"

namespace nes
{
    class EntityRegistry;
    using EntityRegistryHandle = entt::entity;

    namespace internal
    {
        constexpr EntityRegistryHandle kNullEntity = entt::null;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : An Entity is essentially a handle into owning EntityRegistry object. This class
    ///              provides an API to Add, Remove, and Get Components from the Entity.
    //----------------------------------------------------------------------------------------------------
    class Entity
    {
        friend EntityRegistry;

        EntityRegistry* m_pRegistry = nullptr;

        // Handle into the Registry Object used to access the Entity's Components.
        // This handle is always determined at runtime.
        EntityRegistryHandle m_handle = internal::kNullEntity;

        // Registry should be the only class that can create an Entity.
        Entity() = default;

    public:
        Entity(const Entity& other) = delete;
        Entity& operator=(const Entity& other) = delete;
        Entity(Entity&& other) noexcept = default;
        Entity& operator=(Entity&& other) noexcept = default;
        ~Entity() = default;

        template<ComponentType Type, typename...Params>
        Type& AddComponent(Params&&...params);

        template<ComponentType Type>
        void RemoveComponent();

        template<ComponentType Type>
        [[nodiscard]] Type& GetComponent() const;

        template<ComponentType Type>
        [[nodiscard]] Type* TryGetComponent() const;

        template<ComponentType Type>
        [[nodiscard]] bool Has() const;

        template<ComponentType...ComponentTypes>
        [[nodiscard]] bool HasAllOf() const;

        template<ComponentType...ComponentTypes>
        [[nodiscard]] bool HasAnyOf() const;

        [[nodiscard]] bool IsValid() const;
    };

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      For now, this is going to essentially be a wrapper around the EnTT library.
    //      It has all the features that I want to have in my ECS system, I just don't have time at the
    //      moment to (try to) implement it myself.
    //      - Outside the Registry, this library has some other features that I should study as well.
    //      - Wiki: https://github.com/skypjack/entt/wiki
    //		
    ///		@brief : This class manages Entities and their Components.
    //----------------------------------------------------------------------------------------------------
    class EntityRegistry
    {
        entt::registry m_registry;

    public:
        EntityRegistry() = default;
        EntityRegistry(EntityRegistry&& right) noexcept = default;
        EntityRegistry& operator=(EntityRegistry&& right) noexcept = default;

        // No Copying
        EntityRegistry(const EntityRegistry& right) = delete;
        EntityRegistry& operator=(const EntityRegistry& right) = delete;

    public:
        Entity CreateEntity();
        void DestroyEntity(Entity& entity);
        [[nodiscard]] bool IsValidEntity(const EntityRegistryHandle handle) const;
        void Clear();

        template <ComponentType Type, typename ... Params>
        Type& AddComponent(const EntityRegistryHandle handle, Params&&...params);

        template <ComponentType Type>
        Type& GetComponent(const EntityRegistryHandle handle);

        template <ComponentType Type>
        Type* TryGetComponent(const EntityRegistryHandle handle);
        
        template <ComponentType Type>
        void RemoveComponent(const EntityRegistryHandle handle);

        template <ComponentType Type>
        [[nodiscard]] bool Has(const EntityRegistryHandle handle) const;

        template <ComponentType...Components>
        [[nodiscard]] bool HasAllOf(const EntityRegistryHandle handle) const;

        template <ComponentType...Components>
        [[nodiscard]] bool HasAnyOf(const EntityRegistryHandle handle) const;

        template <ComponentType...Owned>
        auto GetGroup();

        template <ComponentType ...Components>
        auto GetEntitiesWith();

        auto GetAllEntities();
    };
}

//----------------------------------------------------------------------------------------------------
// Entity Implementation
//----------------------------------------------------------------------------------------------------
namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Add a Component to this Entity.
    ///		@tparam Type : Type of Component to Add.
    ///		@tparam Params : Any Parameter Types to construct the Component.
    ///		@param params : Constructor params.
    ///		@returns : Reference to the new Component.
    //----------------------------------------------------------------------------------------------------
    template<ComponentType Type, typename...Params>
    Type& Entity::AddComponent(Params&&...params)
    {
        NES_ASSERT(m_pRegistry);

        if (m_pRegistry->Has<Type>(m_handle))
        {
            NES_WARNV("ECS", "Attempted to add a component that already exists on the Entity!");
            return m_pRegistry->GetComponent<Type>(m_handle);
        }

        Type& component = m_pRegistry->AddComponent<Type>(m_handle, std::forward<Params>(params)...);
        return component;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      I know this *can* be a const function, but I think it is clearer to say that it is
    //      changing the state of the underlying entity.
    //		
    ///		@brief : Remove a component from this Entity.
    ///		@tparam Type : Type of Component to Remove.
    //----------------------------------------------------------------------------------------------------
    template<ComponentType Type>
    void Entity::RemoveComponent()
    {
        NES_ASSERT(m_pRegistry);
        m_pRegistry->RemoveComponent<Type>(m_handle);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Returns a reference to the Component Type. This can fail! If you don't know if the
    ///              Component exists, use TryGetComponent.
    ///		@tparam Type : Type of Component you are looking for.
    //----------------------------------------------------------------------------------------------------
    template <ComponentType Type>
    Type& Entity::GetComponent() const
    {
        NES_ASSERT(m_pRegistry);
        return m_pRegistry->GetComponent<Type>(m_handle);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Returns a pointer to the Component of Type. If the Entity does not have the Component,
    ////              this will return nullptr
    ///		@tparam Type : Type of Component you are looking for.
    //----------------------------------------------------------------------------------------------------
    template <ComponentType Type>
    Type* Entity::TryGetComponent() const
    {
        NES_ASSERT(m_pRegistry);
        return m_pRegistry->TryGetComponent<Type>(m_handle);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Returns true if the Entity has a Component of ComponentType.
    ///		@tparam Type : Type of Component you are looking for.
    //----------------------------------------------------------------------------------------------------
    template <ComponentType Type>
    bool Entity::Has() const
    {
        NES_ASSERT(m_pRegistry);
        return m_pRegistry->Has<Type>(m_handle);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Returns true if the Entity has *all* the Components queried for.
    ///		@tparam ComponentTypes : Types of Components you are looking for.
    //----------------------------------------------------------------------------------------------------
    template <ComponentType... ComponentTypes>
    bool Entity::HasAllOf() const
    {
        NES_ASSERT(m_pRegistry);
        return m_pRegistry->HasAllOf<ComponentTypes...>(m_handle);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Returns true if the Entity has *any* single one of the Components queried for.
    ///		@tparam ComponentTypes : Types of Components you are looking for.
    //----------------------------------------------------------------------------------------------------
    template <ComponentType... ComponentTypes>
    bool Entity::HasAnyOf() const
    {
        NES_ASSERT(m_pRegistry);
        return m_pRegistry->HasAnyOf<ComponentTypes...>(m_handle);
    }
}

//----------------------------------------------------------------------------------------------------
// Entity Registry Implementation
//----------------------------------------------------------------------------------------------------
namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Creates and adds a component to the entity. If the component already exists on the Entity,
    ///              it will be replaced with the new one. This can fail if the Entity is not valid!
    ///		@tparam Type : Type of Component to Add.
    ///		@tparam Params : Constructor params for the Component, or an Initializer List.
    ///		@param handle : Handle of the Entity to add them to.
    ///		@param params : Constructor params for the Component, or an Initializer List.
    ///		@returns : Reference to the newly created Component.
    //----------------------------------------------------------------------------------------------------
    template <ComponentType Type, typename... Params>
    Type& EntityRegistry::AddComponent(const EntityRegistryHandle handle, Params&&... params)
    {
        if (Has<Type>(handle))
        {
            NES_WARNV("ECS", "Attempted to add a component that already exists on the Entity! Replacing Component...");
            return m_registry.get<Type>(handle);
        }

        return m_registry.emplace<Type>(handle, std::forward<Params>(params)...);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Get a reference to a Component of Type from the Entity.
    ///		@tparam Type : Type of Component.
    ///		@param handle : Handle of the Entity that owns the Component.
    //----------------------------------------------------------------------------------------------------
    template <ComponentType Type>
    Type& EntityRegistry::GetComponent(const EntityRegistryHandle handle)
    {
        return m_registry.get<Type>(handle);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Try to get a Component of Type from the Entity.
    ///		@tparam Type : Type of Component.
    ///		@param handle : Handle of the Entity that owns the Component.
    ///		@returns : A Pointer to the Component Type. If the Component doesn't exist, it will be nullptr.
    //----------------------------------------------------------------------------------------------------
    template <ComponentType Type>
    Type* EntityRegistry::TryGetComponent(const EntityRegistryHandle handle)
    {
        return m_registry.try_get<Type>(handle);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      Entt has "Erase", which can fail, and "Remove", which is guaranteed to succeed.
    //		
    ///		@brief : Remove a Component of Type from the Entity.
    ///		@tparam Type : Type of Component you are trying to remove. 
    ///		@param handle : Handle of the Entity.
    //----------------------------------------------------------------------------------------------------
    template <ComponentType Type>
    void EntityRegistry::RemoveComponent(const EntityRegistryHandle handle)
    {
        [[maybe_unused]] const auto numRemoved = m_registry.remove<Type>(handle);

#if !NES_RELEASE
        if (numRemoved == 0)
        {
            NES_WARNV("ECS", "Attempted to remove a Component of Type: from an Entity that doesn't have one!");
        }
#endif
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns true if the Entity has the specified Component Type.
    ///		@tparam Type : Type of Component.
    ///		@param handle : Handle of the Entity that we are querying.
    //----------------------------------------------------------------------------------------------------
    template <ComponentType Type>
    bool EntityRegistry::Has(const EntityRegistryHandle handle) const
    {
        return m_registry.has<Type>(handle);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns true if the Entity has all the specified Component Types.
    ///		@tparam Components : Types of Components.
    ///		@param handle : Handle of the Entity that we are querying.
    //----------------------------------------------------------------------------------------------------
    template <ComponentType ... Components>
    bool EntityRegistry::HasAllOf(const EntityRegistryHandle handle) const
    {
        return m_registry.has<Components...>(handle);
    }

    //----------------------------------------------------------------------------------------------------//		
    ///		@brief : Returns true if the Entity has any of the specified Component Types.
    ///		@tparam Components : Types of Components.
    ///		@param handle : Handle of the Entity that we are querying.
    //----------------------------------------------------------------------------------------------------
    template <ComponentType ... Components>
    bool EntityRegistry::HasAnyOf(const EntityRegistryHandle handle) const
    {
        return m_registry.any<Components...>(handle);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      TODO: Component Filtering options.
    //		Excluded : Component Types that the entities should not own.
    //		
    ///		@brief : Get a Group of Entities that have the specified Components.
    ///		@tparam Owned : Component Types that the entities should own.
    ///		@returns : 
    //----------------------------------------------------------------------------------------------------
    template <ComponentType ... Owned>
    auto EntityRegistry::GetGroup()
    {
        return m_registry.group<Owned...>();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Get a Group of Entities that have the specified Components.
    ///		@tparam Components : Components that the entities should own.
    ///		@returns : A View object that can be used to iterate over the entities.
    ///             - Ex:
    //----------------------------------------------------------------------------------------------------
    template <ComponentType ...Components>
    auto EntityRegistry::GetEntitiesWith()
    {
        return m_registry.view<Components...>();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      // [TODO]: Make this 
    //		
    ///		@brief : Get all entities in the registry.
    //----------------------------------------------------------------------------------------------------
    inline auto EntityRegistry::GetAllEntities()
    {
        auto view = m_registry.view<entt::entity>();
        return view;
    }
}