#pragma once
// ComponentSystem.h
#include "Entity.h"

namespace nes
{
    class World;

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      This is the main class that I want to use to inject behavior into the Application.
    //      - I need some more context added to the ComponentSystem, like the Application itself so
    //        that I can access the Window, Time info, other Core Systems like Graphics, etc.
    //		
    ///		@brief : A ComponentSystem is a class that processes Components. Component Systems are attached
    ///              to the Application's update...
    //----------------------------------------------------------------------------------------------------
    class ComponentSystem
    {
        EntityRegistry& m_registry;
        World& m_world;

    public:
        ComponentSystem(EntityRegistry& registry, World& world);
        virtual ~ComponentSystem() = default;

        // No move or copy
        ComponentSystem(const ComponentSystem&) = delete;
        ComponentSystem& operator=(const ComponentSystem&) = delete;
        ComponentSystem(ComponentSystem&&) noexcept = delete;
        ComponentSystem& operator=(ComponentSystem&&) noexcept = delete;

        virtual void Process() = 0;

        [[nodiscard]] EntityRegistry& GetRegistry() { return m_registry; }
        [[nodiscard]] const EntityRegistry& GetRegistry() const { return m_registry; }
        [[nodiscard]] World& GetWorld() { return m_world; }
        [[nodiscard]] const World& GetWorld() const { return m_world; }
    };
}