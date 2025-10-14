// Entity.h
#pragma once
#include "entt/entity/registry.hpp"
#include "Components/IDComponent.h"
#include "Components/LifetimeComponents.h"

namespace nes
{
    class Entity;
    class EntityRegistry;

    //----------------------------------------------------------------------------------------------------
    /// @brief : An entity handle is a runtime-only value that is used to access Components associated with
    ///     the Entity in the registry.
    //----------------------------------------------------------------------------------------------------
    using EntityHandle = entt::entity;
    static constexpr EntityHandle kInvalidEntityHandle = entt::null;

    //----------------------------------------------------------------------------------------------------
    /// @brief : An Entity contains a number of components. By default, all Entities are created with an
    ///     EntityPropsComponent, which contains lifetime data and a unique identifier.
    //----------------------------------------------------------------------------------------------------
    class Entity
    {
    public:
        Entity() = default;
        Entity(EntityRegistry& registry, EntityHandle handle) : m_pRegistry(&registry), m_handle(handle) {}
        Entity(const Entity&) = default;
        Entity(Entity&& other) noexcept = default;
        Entity& operator=(const Entity&) = default;
        Entity& operator=(Entity&& other) noexcept = default;
        virtual ~Entity() = default;
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Implicit conversion to the handle type. 
        //----------------------------------------------------------------------------------------------------
        operator EntityHandle() const { return m_handle; }
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the entity's unique id. Unlike the EntityHandle, this can be saved to disk.
        //----------------------------------------------------------------------------------------------------
        EntityID            GetID() const;
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : An Entity is invalid if it does not exist in the Registry, or its handle is invalid.
        //----------------------------------------------------------------------------------------------------
        bool                IsValid() const;
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the name of this entity. 
        //----------------------------------------------------------------------------------------------------
        void                SetName(const std::string& name);
        const std::string&  GetName() const;
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Entity's IDComponent, which contains its name, and unique identifier. 
        //----------------------------------------------------------------------------------------------------
        const IDComponent&  GetIDComponent() const;
        IDComponent&        GetIDComponent();
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Marks this Entity to be destroyed. It will actually be destroyed when the registry is processed. 
        //----------------------------------------------------------------------------------------------------
        void                Destroy();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if this entity is queued to be destroyed.
        //----------------------------------------------------------------------------------------------------
        bool                IsMarkedForDestruction() const;
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Adds or replaces a component of a particular type.
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type, typename...Args>
        Type&               AddComponent(Args&&...args);
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Removes and destroys a component of a particular type.
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        void                RemoveComponent();
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a component of a particular type. The entity must have this component. If it is unknown
        ///     whether it has it or not, use TryGetComponent().
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        Type&               GetComponent();
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a component of a particular type. The entity must have this component. If it is unknown
        ///     whether it has it or not, use TryGetComponent().
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        const Type&         GetComponent() const;
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Attempts to get a component of a particular type. If the entity does not have one, this
        ///     returns nullptr. 
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        Type*               TryGetComponent();
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Attempts to get a component of a particular type. If the entity does not have one, this
        ///     returns nullptr.
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        const Type*         TryGetComponent() const;
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check to see if an entity has a particular component.
        //----------------------------------------------------------------------------------------------------
        template <ComponentType Type>
        bool                HasComponent() const;
    
    protected:
        EntityRegistry*     m_pRegistry = nullptr;
        EntityHandle        m_handle = kInvalidEntityHandle;
    };
}