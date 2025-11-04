// WorldBase.h
#pragma once
#include "Nessie/Core/Events/Event.h"
#include "Nessie/Core/Memory/StrongPtr.h"
#include "WorldAsset.h"
#include "WorldRenderer.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A World contains an EntityRegistry and a number of ComponentSystems to operate on those
    /// entities.
    //----------------------------------------------------------------------------------------------------
    class WorldBase
    {
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
        bool                Init();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Handle incoming Application events.
        //----------------------------------------------------------------------------------------------------
        virtual void        OnEvent(Event& event) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called every frame. Delta time is in seconds.
        //----------------------------------------------------------------------------------------------------
        virtual void        Tick(const float deltaTime) = 0;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Calls WorldBase::OnDestroy(), destroys all entities and components, then shuts down each ComponentSystem. 
        //----------------------------------------------------------------------------------------------------
        void                Destroy();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Copies all entities from the World Asset into the World.
        //----------------------------------------------------------------------------------------------------
        void                MergeWorld(WorldAsset& srcWorld);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Marks an Entity to be destroyed. The Entity will actually be destroyed on the next call
        ///     to ProcessEntityLifecycle().
        //----------------------------------------------------------------------------------------------------
        void                DestroyEntity(const EntityHandle entity);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a Component System by Type. Can be nullptr if the System was not correctly added to the
        /// World with EmplaceComponentSystem().
        //----------------------------------------------------------------------------------------------------
        template <ComponentSystemType Type>
        StrongPtr<Type>     GetSystem();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the Renderer object for the world.
        //----------------------------------------------------------------------------------------------------
        StrongPtr<WorldRenderer> GetRenderer() const { return m_pRenderer; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Entity Registry for the world.
        //----------------------------------------------------------------------------------------------------
        EntityRegistry&     GetRegistry() { return m_entityRegistry; }

    protected:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Add all Component Systems that will be used in the world. Higher priority systems should
        /// be added first.
        /// - After all ComponentSystems are added, they will be initialized from the first added to last.
        /// - When destroying the world, ComponentSystems are shutdown in the reverse order they were added.
        //----------------------------------------------------------------------------------------------------
        virtual void        AddComponentSystems() = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called after all Component Systems have been initialized.
        //----------------------------------------------------------------------------------------------------
        virtual bool        PostInit() { return true; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called before all entities have been destroyed, and before all ComponentSystems have been
        /// shutdown.
        //----------------------------------------------------------------------------------------------------
        virtual void        OnDestroy() {}   
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a new Component System of a given type, calls RegisterComponentTypes(), and adds
        /// it to the end of the array of m_systems.
        ///
        /// When processing Entities that need to be initialized or cleaned up, the Systems are notified from
        /// front to back of the array. So critical systems should be added first.
        //----------------------------------------------------------------------------------------------------
        template <ComponentSystemType Type>
        StrongPtr<Type>     AddComponentSystem();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Processes any entities that need to be initialized, enabled, disable, or destroyed.
        //----------------------------------------------------------------------------------------------------
        void                ProcessEntityLifecycle();
        
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Allows systems to initialize entities that need to be initialized. 
        //----------------------------------------------------------------------------------------------------
        void                ProcessPendingInitialization();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allows systems to process all entities that need to be enabled.
        //----------------------------------------------------------------------------------------------------
        void                ProcessPendingEnable();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allows systems to process all entities that need to be disabled.
        //----------------------------------------------------------------------------------------------------
        void                ProcessPendingDisable();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allows systems to clean up entities that are going to be destroyed.
        //----------------------------------------------------------------------------------------------------
        void                ProcessPendingDestruction(const bool destroyingWorld = false);
    
    protected:
        using SystemArray = std::vector<StrongPtr<ComponentSystem>>;
        
        EntityRegistry              m_entityRegistry{};
        SystemArray                 m_systems{};
        StrongPtr<WorldRenderer>    m_pRenderer = nullptr;
    };
}
