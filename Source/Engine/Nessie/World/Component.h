// Component.h
#pragma once
#include "Nessie/Core/Config.h"
#include "Nessie/Core/Concepts.h"
#include "yaml-cpp/yaml.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// A Component is treated as just data. It must be both Copyable and Moveable.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    concept ComponentType = std::is_copy_constructible_v<Type>
        && std::is_move_constructible_v<Type>
        && std::is_copy_assignable_v<Type>
        && std::is_move_assignable_v<Type>
        && !std::is_empty_v<Type>;
    
    class EntityRegistry;

    template <typename Type>
    concept SerializableComponent = ComponentType<Type> && requires(Type component, YAML::Emitter& emitter, const YAML::Node& node)
    {
        { Type::Serialize(emitter, component) } -> std::same_as<void>;
        { Type::Deserialize(node, component) } -> std::same_as<void>;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Derive from this Component if all you need is to add a 'marker' to an entity.
    ///
    /// For example, when an Entity is going to be destroyed, a PendingDestruction component is added.
    /// Component Systems can use this fact to query all Entities that are going to be destroyed and perform
    /// any cleanup logic.
    ///
    /// The reason that this class contains a single unused byte is because entt needs to have some sort of
    /// storage size.
    //----------------------------------------------------------------------------------------------------
    struct MarkerComponentBase
    {
        uint8 m_unused = 0;
    };
}