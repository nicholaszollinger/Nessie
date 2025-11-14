// EntityRegistry.h
#pragma once
#include "Nessie/Core/Config.h"
#include "Nessie/Random/Rng.h"
#include "Entity.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : An Entity Registry manages the lifetime of Entities and their Components.
    //----------------------------------------------------------------------------------------------------
    class EntityRegistry
    {
    public:
        EntityRegistry();
        EntityRegistry(const EntityRegistry&) = delete;
        EntityRegistry(EntityRegistry&& other) noexcept;
        EntityRegistry& operator=(const EntityRegistry&) = delete;
        EntityRegistry& operator=(EntityRegistry&& other) noexcept;
        ~EntityRegistry();

        // Return Type of calling EntityRegistry::GetAllEntitiesWith<PendingDestruction>(). Used to destroy a batch of entities.
        using EntitiesPendingDestructionView = entt::basic_view<entt::get_t<entt::basic_sigh_mixin<entt::basic_storage<PendingDestruction>, entt::basic_registry<>>>, entt::exclude_t<>>;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Clears the registry, destroying all entities and their components, and removing any
        /// construction/destroy callbacks. *No callbacks will be called using this*, they will be removed first.
        ///
        /// Generally, you should call MarkAllEntitiesForDestruction(), handle cleanup, then call DestroyAllEntities().
        //----------------------------------------------------------------------------------------------------
        void                        Clear();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a new entity with an optional name.
        //----------------------------------------------------------------------------------------------------
        EntityHandle                CreateEntity(const std::string& name = {});

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates an entity with a given ID and optional name.
        //----------------------------------------------------------------------------------------------------
        EntityHandle                CreateEntity(const EntityID id, const std::string& name = {});

        //----------------------------------------------------------------------------------------------------
        /// @brief : Marks an Entity for destruction by adding a PendingDestruction component.
        //----------------------------------------------------------------------------------------------------
        void                        MarkEntityForDestruction(const EntityHandle entity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroys an entity, all of its Components, and removes it from the Entity Map.
        //----------------------------------------------------------------------------------------------------
        void                        DestroyEntity(const EntityHandle entity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Marks a view of entities for destruction by adding a PendingDestruction component.
        //----------------------------------------------------------------------------------------------------
        void                        MarkEntitiesForDestruction(auto& view);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroys a view of entities that are pending destruction.
        //----------------------------------------------------------------------------------------------------
        void                        DestroyEntities(const EntitiesPendingDestructionView& view);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Marks all Entities for destruction, ensuring they all have the Pending Destruction Component.
        //----------------------------------------------------------------------------------------------------
        void                        MarkAllEntitiesForDestruction();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroy all entities within the registry.
        //----------------------------------------------------------------------------------------------------
        void                        DestroyAllEntities();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Try to get an Entity by its unique ID. If none is found, the resulting Entity will be
        /// invalid.
        //----------------------------------------------------------------------------------------------------
        EntityHandle                GetEntity(const EntityID id) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : If an Entity is disabled, this will add a PendingEnable component. 
        //----------------------------------------------------------------------------------------------------
        void                        TryEnableEntity(const EntityHandle entity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : If an Entity is enabled, this will add a PendingDisable component.
        //----------------------------------------------------------------------------------------------------
        void                        TryDisableEntity(const EntityHandle entity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Adds or replaces a component of a particular type.
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type, typename...Args>
        Type&                       AddComponent(const EntityHandle entity, Args&&...args) { return m_registry.emplace_or_replace<Type>(entity, std::forward<Args>(args)...); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Adds or replaces a component of a particular type to all entities in a view.
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        void                        AddComponentToAll(auto& view, const Type& value = {}) { return m_registry.insert<Type>(view.begin(), view.end(), value); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Removes and destroys a component of a particular type.
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        void                        RemoveComponent(const EntityHandle entity) { m_registry.remove<Type>(entity); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Removes and destroys a component if the entity has it, using a type ID.
        //----------------------------------------------------------------------------------------------------
        void                        RemoveComponent(const entt::id_type componentTypeID, const EntityHandle handle);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Removes and destroys a component of a particular type from all entities within a view.
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        void                        RemoveComponentFromAll(auto& view) { m_registry.remove<Type>(view.begin(), view.end()); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a component of a particular type. The entity must have this component. If it is unknown
        ///     whether it has it or not, use TryGetComponent().
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        Type&                       GetComponent(const EntityHandle entity) { return m_registry.get<Type>(entity); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a component of a particular type. The entity must have this component. If it is unknown
        ///     whether it has it or not, use TryGetComponent().
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        const Type&                 GetComponent(const EntityHandle entity) const { return m_registry.get<Type>(entity); }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Attempts to get a component of a particular type. If the entity does not have one, this
        ///     returns nullptr. 
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        Type*                       TryGetComponent(const EntityHandle entity) { return m_registry.try_get<Type>(entity); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Attempts to get a component of a particular type. If the entity does not have one, this
        ///     returns nullptr. 
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        const Type*                 TryGetComponent(const EntityHandle entity) const { return m_registry.try_get<Type>(entity); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Returns a pointer to the component memory, or nullptr if the entity does not have it.
        //----------------------------------------------------------------------------------------------------
        void*                       TryGetComponentRaw(const entt::id_type componentTypeID, const EntityHandle entity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check to see if an entity has a particular component.
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        bool                        HasComponent(const EntityHandle entity) const { return m_registry.all_of<Type>(entity); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check to see if an entity has a particular component, using a component type ID.
        //----------------------------------------------------------------------------------------------------
        bool                        HasComponent(const entt::id_type componentTypeID, const EntityHandle entity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check to see if an entity has all the given components.
        //----------------------------------------------------------------------------------------------------
        template <ComponentType...Types>
        bool                        HasAllComponents(const EntityHandle entity) const { return m_registry.all_of<Types...>(entity); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the entity has one or more of the given components.
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        bool                        HasAnyComponents(const EntityHandle entity) const { return m_registry.any_of<Type>(entity); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns a "View" object that contains all entities with each of the given components that
        ///     can be iterated through.
        ///
        /// <code>
        ///  // Example Usage:
        ///  auto view = registry.GetAllEntitiesWith<Position, Velocity, Renderable>();
        ///
        ///  for (auto entity: view)
        ///  {
        ///      // a component at a time ...
        ///      auto& position = view.get<Position>(entity);
        ///      auto& velocity = view.get<Velocity>(entity);
        ///
        ///      // ... multiple components ...
        ///      auto [pos, vel] = view.get<Position, Velocity>(entity);
        ///
        ///      // ... all components at once
        ///      auto [pos, vel, rend] = view.get(entity);
        ///  }
        ///  </code>
        //----------------------------------------------------------------------------------------------------
        template <ComponentType...Types, ComponentType...Exclude>
        auto                        GetAllEntitiesWith(entt::exclude_t<Exclude...> = entt::exclude_t{}) { return m_registry.view<Types...>(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Removes and destroys all components of a single type.
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        void                        ClearAllComponentsOfType() { m_registry.clear<Type>(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the event that is invoked when a Component of the given type is created.
        /// You can connect a lambda or member function to respond to the event. Must be in the form
        /// <code>
        ///     void(entt::registry& registry, entt::entity entity);
        /// </code> 
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        auto                        OnComponentCreated();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the event that is invoked *before* a Component of the given type is destroyed. 
        /// You can connect a lambda or member function to respond to the event. Must be in the form:
        /// <code>
        ///     void(entt::registry& registry, entt::entity entity);
        /// </code> 
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        auto                        OnComponentDestroyed();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks to see if the Entity exists in the registry, and contains an IDComponent.
        //----------------------------------------------------------------------------------------------------
        bool                        IsValidEntity(const EntityHandle entity) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks to see if the Entity exists in the registry, and contains an IDComponent.
        //----------------------------------------------------------------------------------------------------
        bool                        IsValidEntity(const EntityID id) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of entities in the registry.
        //----------------------------------------------------------------------------------------------------
        size_t                      GetNumEntities() const { return m_entityMap.size(); }
    
    private:
        friend class ComponentRegistry;
        using EntityMap = std::unordered_map<uint64, EntityHandle>;
        
        EntityMap                   m_entityMap{};
        entt::registry              m_registry{};
    };
}