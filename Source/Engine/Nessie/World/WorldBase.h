// WorldBase.h
#pragma once
#include "Nessie/Core/Events/Event.h"
#include "Nessie/Core/Memory/StrongPtr.h"
#include "WorldAsset.h"
#include "WorldRenderer.h"

namespace nes
{
    enum class EWorldSimState
    {
        Stopped,    // The world is not simulating.
        Playing,    // The world is simulating and will be ticked.
        Paused,     // The world is simulating and will be ticked, but delta time will be zero.
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : A World contains an EntityRegistry and a number of ComponentSystems to operate on those
    /// entities.
    //----------------------------------------------------------------------------------------------------
    class WorldBase
    {
    public:
        using SystemArray = std::vector<StrongPtr<ComponentSystem>>;
        
    public:
        WorldBase() = default;
        WorldBase(const WorldBase&) = delete;
        WorldBase(WorldBase&&) noexcept = delete;
        WorldBase& operator=(const WorldBase&) = delete;
        WorldBase& operator=(WorldBase&&) noexcept = delete;
        virtual ~WorldBase() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calls WorldBase::AddComponentSystems(), initializes each ComponentSystem, then calls PostInit().
        //----------------------------------------------------------------------------------------------------
        bool                        Init();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Handle incoming Application events.
        //----------------------------------------------------------------------------------------------------
        virtual void                OnEvent(Event& event) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Starts the simulation. All Component Systems will be notified.
        //----------------------------------------------------------------------------------------------------
        void                        BeginSimulation();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set whether the world should be paused, globally. IsSimulating() will still return true in
        ///     the paused state - you can check IsPaused() as well.
        //----------------------------------------------------------------------------------------------------
        void                        SetPaused(const bool shouldPause);

        //----------------------------------------------------------------------------------------------------
        /// @brief : End the world simulation. All component systems will be notified.
        //----------------------------------------------------------------------------------------------------
        void                        EndSimulation();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called every frame. Delta time is in seconds.
        //----------------------------------------------------------------------------------------------------
        virtual void                Tick(const float deltaTime) = 0;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Calls WorldBase::OnDestroy(), destroys all entities and components, then shuts down each ComponentSystem. 
        //----------------------------------------------------------------------------------------------------
        void                        Destroy();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroys all Entities and their components.
        //----------------------------------------------------------------------------------------------------
        void                        DestroyAllEntities();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Copies all entities from the World Asset into the World.
        //----------------------------------------------------------------------------------------------------
        void                        MergeWorld(WorldAsset& srcWorld);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Export the current entity information to the WorldAsset.
        //----------------------------------------------------------------------------------------------------
        void                        ExportToAsset(WorldAsset& dstAsset);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a new entity in the world. 
        //----------------------------------------------------------------------------------------------------
        virtual EntityHandle        CreateEntity(const std::string& newName) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Marks an Entity to be destroyed. The Entity will actually be destroyed on the next call
        ///     to ProcessEntityLifecycle().
        //----------------------------------------------------------------------------------------------------
        void                        DestroyEntity(const EntityID entity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Marks an Entity to be destroyed. The Entity will actually be destroyed on the next call
        ///     to ProcessEntityLifecycle().
        //----------------------------------------------------------------------------------------------------
        virtual void                DestroyEntity(const EntityHandle entity) = 0;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Parent an Entity to another. 
        //----------------------------------------------------------------------------------------------------
        void                        ParentEntity(const EntityID entity, const EntityID parent);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Parent an Entity to another. 
        //----------------------------------------------------------------------------------------------------
        virtual void                ParentEntity(const EntityHandle entity, const EntityHandle parent) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Unparent an entity. 
        //----------------------------------------------------------------------------------------------------
        void                        RemoveParent(const EntityID entity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Unparent an entity. 
        //----------------------------------------------------------------------------------------------------
        void                        RemoveParent(const EntityHandle entity);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the ids are equal or the entity is a child, grand-child etc. of the potential ancestor.
        //----------------------------------------------------------------------------------------------------
        bool                        IsDescendantOf(const EntityID entity, const EntityID potentialAncestor) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a Component System by Type. Can be nullptr if the System was not correctly added to the
        /// World with AddComponentSystem().
        //----------------------------------------------------------------------------------------------------
        template <ComponentSystemType Type>
        StrongPtr<Type>             GetSystem() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a ComponentSystem using a TypeID. Can be nullptr if the System was not correctly added to the
        /// World with AddComponentSystem().
        //----------------------------------------------------------------------------------------------------
        virtual StrongPtr<ComponentSystem> GetSystem(const entt::id_type typeID) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the Renderer System for the world.
        //----------------------------------------------------------------------------------------------------
        virtual StrongPtr<WorldRenderer> GetRenderer() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Entity Registry for the world. The EntityRegistry contains all Entities and their
        /// Components. It has the interface for Adding and Removing Components directly.
        //----------------------------------------------------------------------------------------------------
        virtual EntityRegistry*     GetEntityRegistry() = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Entity Registry for the world. The EntityRegistry contains all Entities and their
        /// Components. It has the interface for Adding and Removing Components directly.
        //----------------------------------------------------------------------------------------------------
        const EntityRegistry*       GetEntityRegistry() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : The World is "Simulating" if it is Playing or Paused.
        /// When simulating, Tick() will be called every frame. When paused, the delta time will be 0 per tick.
        //----------------------------------------------------------------------------------------------------
        bool                        IsSimulating() const        { return m_simState == EWorldSimState::Playing || m_simState == EWorldSimState::Paused; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if the World is paused, meaning that delta time will always be zero is the Tick() function.
        //----------------------------------------------------------------------------------------------------
        bool                        IsPaused() const            { return m_simState == EWorldSimState::Paused; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the current simulation state of the world. See EWorldSimState for more details.
        //----------------------------------------------------------------------------------------------------
        EWorldSimState              GetSimState() const         { return m_simState; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced Use. Get the array of systems for the world.
        //----------------------------------------------------------------------------------------------------
        const SystemArray&          GetSystems() const { return m_systems; }

    protected:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Add all Component Systems that will be used in the world. Higher priority systems should
        /// be added first.
        /// - After all ComponentSystems are added, they will be initialized from the first added to last.
        /// - When destroying the world, ComponentSystems are shutdown in the reverse order they were added.
        //----------------------------------------------------------------------------------------------------
        virtual void                AddComponentSystems() = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called after all Component Systems have been initialized.
        //----------------------------------------------------------------------------------------------------
        virtual bool                PostInit() { return true; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called before all entities have been destroyed, and before all ComponentSystems have been
        /// shutdown.
        //----------------------------------------------------------------------------------------------------
        virtual void                OnDestroy() {}
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a new Component System of a given type, calls RegisterComponentTypes(), and adds
        /// it to the end of the array of m_systems.
        ///
        /// When processing Entities that need to be initialized or cleaned up, the Systems are notified from
        /// front to back of the array. So critical systems should be added first.
        //----------------------------------------------------------------------------------------------------
        template <ComponentSystemType Type>
        StrongPtr<Type>             AddComponentSystem();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Processes any entities that need to be initialized, enabled, disable, or destroyed.
        //----------------------------------------------------------------------------------------------------
        void                        ProcessEntityLifecycle();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called once when the Simulation has started, in the call to BeginSimulation.
        /// The base implementation calls ComponentSystem::OnBeginSimulation() for all component systems.
        //----------------------------------------------------------------------------------------------------
        virtual void                OnBeginSimulation();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called once, when the Simulation has ended.
        /// The base implementation calls ComponentSystem::OnEndSimulation() for all component systems.
        //----------------------------------------------------------------------------------------------------
        virtual void                OnEndSimulation();
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Allows systems to initialize entities that need to be initialized. 
        //----------------------------------------------------------------------------------------------------
        void                        ProcessPendingInitialization(EntityRegistry& registry) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allows systems to process all entities that need to be enabled.
        //----------------------------------------------------------------------------------------------------
        void                        ProcessPendingEnable(EntityRegistry& registry) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allows systems to process all entities that need to be disabled.
        //----------------------------------------------------------------------------------------------------
        void                        ProcessPendingDisable(EntityRegistry& registry) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allows systems to clean up entities that are going to be destroyed.
        //----------------------------------------------------------------------------------------------------
        void                        ProcessPendingDestruction(EntityRegistry& registry, const bool destroyingAllEntities = false) const;
    
    protected:
        using SystemMap = std::unordered_map<entt::id_type, size_t>;
        
        SystemMap                   m_systemMap{};    
        SystemArray                 m_systems{};
        EWorldSimState              m_simState = EWorldSimState::Stopped;
    };
}
