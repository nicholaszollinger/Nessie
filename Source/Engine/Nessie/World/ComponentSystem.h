// ComponentSystem.h
#pragma once
#include "Nessie/Core/Memory/StrongPtr.h"

namespace nes
{
    class EntityRegistry;
    class WorldBase;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Component System processes a subset of entities with specific components. You can have a
    ///     system that processes a physics simulation, manage a render update, etc.
    //----------------------------------------------------------------------------------------------------
    class ComponentSystem
    {
        friend class WorldBase;
        
    protected:
        ComponentSystem(WorldBase& world) : m_pWorld(&world) {}
        
    public:
        ComponentSystem(const ComponentSystem&) = delete;
        ComponentSystem(ComponentSystem&&) noexcept = delete;
        ComponentSystem& operator=(const ComponentSystem&) = delete;
        ComponentSystem& operator=(ComponentSystem&&) noexcept = delete;
        virtual ~ComponentSystem() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Init is called after all Component Systems have been added to the world.
        //----------------------------------------------------------------------------------------------------
        virtual bool        Init() { return true; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sets the World reference. 
        //----------------------------------------------------------------------------------------------------
        void                SetWorld(WorldBase& world);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Shutdown is called when the world is being destroyed, but *before* all Systems have
        ///     been removed from the world.
        //----------------------------------------------------------------------------------------------------
        virtual void        Shutdown() {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called when added to the World. For each component that this system uses, use the following
        /// macros to ensure that every Component can be properly saved/loaded. You don't need to worry about duplicate
        /// calls, that is handled safely.
        /// <code>
        ///     NES_REGISTER_COMPONENT(Type);
        /// </code>
        //----------------------------------------------------------------------------------------------------
        virtual void       RegisterComponentTypes() = 0;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Override if necessary. When entities are created, they are given a PendingInitialization
        /// component. Systems can grab all entities with a set of components that includes PendingInitialization
        /// to process the subset of entities that need to be initialized.
        //----------------------------------------------------------------------------------------------------
        virtual void        ProcessNewEntities() {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Override if necessary. When an Entity is set to be destroyed, a PendingDestruction component
        /// will be added. Systems can grab all entities with a set of components that includes PendingDestruction
        /// to process the entities that need to cleaned up before actually being destroyed.
        /// @param destroyingAllEntities : If true, *all* entities are going to be destroyed.
        /// Can be checked to skip complex cleanup operations, if applicable.
        //----------------------------------------------------------------------------------------------------
        virtual void        ProcessDestroyedEntities([[maybe_unused]] const bool destroyingAllEntities = false) {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Override if necessary. When an Entity is enabled from a disabled state, a PendingEnable component
        /// will be added. Systems can grab all entities with a set of components that includes PendingEnable
        /// to handle any enable logic.
        //----------------------------------------------------------------------------------------------------
        virtual void        ProcessEnabledEntities() {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Override if necessary. When an Entity is disabled from an enabled state, a PendingDisable component
        /// will be added. Systems can grab all entities with a set of components that includes PendingDisable
        /// to handle any enable logic.
        //----------------------------------------------------------------------------------------------------
        virtual void        ProcessDisabledEntities() {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called any time the Entity Registry used by the World has changed.
        /// - Component Systems that maintain entity handle references will be invalidated.
        /// - Component Systems that listen for Component changes for a registry will be invalid and must be disconnected from
        ///     the old registry and connected to the new registry.
        ///	@param pNewRegistry : The New Registry that the world is using.
        ///	@param pOldRegistry : The Old Registry that was previously being used.
        //----------------------------------------------------------------------------------------------------
        virtual void        OnEntityRegistryChanged([[maybe_unused]] EntityRegistry* pNewRegistry, [[maybe_unused]] EntityRegistry* pOldRegistry) {}
        
    protected:
        EntityRegistry*     GetEntityRegistry() const;
        WorldBase&          GetWorld() const;

        //----------------------------------------------------------------------------------------------------
        // [TODO]: Have a OnPause, OnResume()?
        /// @brief : Called when the world begins simulating.
        //----------------------------------------------------------------------------------------------------
        virtual void        OnBeginSimulation() {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called when the world is no longer simulating.
        //----------------------------------------------------------------------------------------------------
        virtual void        OnEndSimulation() {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called after the World reference has been set.
        //----------------------------------------------------------------------------------------------------
        virtual void        OnWorldSet() {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called before the World Reference is removed.
        //----------------------------------------------------------------------------------------------------
        virtual void        OnWorldRemoved() {}
        
    private:
        WorldBase*          m_pWorld = nullptr;
    };

    template <typename Type>
    concept ComponentSystemType = nes::TypeIsDerivedFrom<Type, ComponentSystem>;
}
