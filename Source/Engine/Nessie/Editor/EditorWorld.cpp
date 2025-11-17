// EditorWorld.cpp
#include "EditorWorld.h"
#include "Nessie/World/RuntimeWorld.h"
#include "Nessie/World/ComponentSystems/TransformSystem.h"

namespace nes
{
    void EditorWorld::SetWorldAsset(const nes::AssetID worldAssetID)
    {
        // [TODO]: At the moment, this will only be called once. I need to handle loading different worlds later.
        NES_ASSERT(m_pCurrentWorldAsset == nullptr);
        
        m_pCurrentWorldAsset = AssetManager::GetAsset<WorldAsset>(worldAssetID);
        NES_ASSERT(m_pCurrentWorldAsset != nullptr, "World Asset is not loaded!");

        // Runtime world uses the asset's registry when not simulating:
        m_pRuntimeWorld->SetEntityRegistryOverride(&m_pCurrentWorldAsset->GetEntityRegistry());
        
        // Override all Runtime Component Systems to use the Editor World as a reference.
        // This is to make sure that they are using the right Entity Registry.
        // They can still access the runtime world's systems.
        for (auto system : m_pRuntimeWorld->GetSystems())
        {
            system->SetWorld(*this);
        }
    }

    void EditorWorld::SetRuntimeWorld(const StrongPtr<World>& pWorld)
    {
        // At the moment, this should only be called once.
        NES_ASSERT(pWorld != nullptr);
        NES_ASSERT(m_pRuntimeWorld == nullptr);
        
        m_pRuntimeWorld = pWorld;
        if (!m_pRuntimeWorld->Init())
        {
            NES_ERROR("Failed to initialize Runtime World!");
        }
    }

    void EditorWorld::Tick(const float deltaTime)
    {
        if (m_pRuntimeWorld)
            m_pRuntimeWorld->Tick(deltaTime);
    }

    void EditorWorld::OnEvent(Event& event)
    {
        if (IsSimulating() && !IsPaused())
        {
            NES_ASSERT(m_pRuntimeWorld != nullptr);
            m_pRuntimeWorld->OnEvent(event);
        }
    }
    
    EntityHandle EditorWorld::CreateEntity(const std::string& name)
    {
        NES_ASSERT(m_pRuntimeWorld != nullptr);
        return m_pRuntimeWorld->CreateEntity(name);
    }

    // [TODO]: Do I need this?
    // - I already get the correct registry through GetEntityRegistry()
    void EditorWorld::DestroyEntity(const EntityHandle entity)
    {
        GetEntityRegistry()->DestroyEntity(entity);
    }

    void EditorWorld::ParentEntity(const EntityHandle entity, const EntityHandle parent)
    {
        NES_ASSERT(m_pRuntimeWorld != nullptr);
        m_pRuntimeWorld->ParentEntity(entity, parent);
    }

    EntityRegistry* EditorWorld::GetEntityRegistry()
    {
        if (IsSimulating() && m_pRuntimeWorld)
        {
            return m_pRuntimeWorld->GetEntityRegistry();
        }

        if (m_pCurrentWorldAsset)
        {
            return &m_pCurrentWorldAsset->GetEntityRegistry();
        }
        
        return nullptr;
    }

    StrongPtr<WorldRenderer> EditorWorld::GetRenderer() const
    {
        NES_ASSERT(m_pRuntimeWorld != nullptr);
        return m_pRuntimeWorld->GetRenderer();
    }

    StrongPtr<ComponentSystem> EditorWorld::GetSystem(const entt::id_type typeID) const
    {
        if (auto pEditorSystem = WorldBase::GetSystem(typeID))
        {
            return pEditorSystem;
        }

        if (m_pRuntimeWorld)
        {
            return m_pRuntimeWorld->GetSystem(typeID);
        }

        return nullptr;
    }

    void EditorWorld::AddComponentSystems()
    {
        // [TODO]: Add Editor Only Component Systems?
    }

    bool EditorWorld::PostInit()
    {
        return true;
    }

    void EditorWorld::OnDestroy()
    {
        NES_ASSERT(IsSimulating() == false, "Destroying Editor World while simulation is occuring! You need to End the Simulation first!");
        
        if (m_pCurrentWorldAsset != nullptr)
        {
            // Save on close:
            // [TODO:Later]: Have a prompt for saving unsaved changes.
            AssetManager::SaveAssetSync(m_pCurrentWorldAsset->GetAssetID());
        }

        // Destroy the Runtime World object.
        if (m_pRuntimeWorld != nullptr)
        {
            m_pRuntimeWorld->Destroy();
            m_pRuntimeWorld = nullptr;
        }
    }

    void EditorWorld::OnBeginSimulation()
    {
        WorldBase::OnBeginSimulation();
        
        if (m_pRuntimeWorld != nullptr)
        {
            NES_ASSERT(m_pCurrentWorldAsset != nullptr);

            // Beginning the simulation will set the IsSimulating flag, which will
            // instruct the runtime world to use its own entity registry rather than the Asset Version.
            m_pRuntimeWorld->BeginSimulation();

            // Destroy any entities that were in the runtime world.
            m_pRuntimeWorld->DestroyAllEntities();

            // Merge the asset's entities into the runtime world.
            m_pRuntimeWorld->MergeWorld(*m_pCurrentWorldAsset);
        }
    }

    void EditorWorld::OnEndSimulation()
    {
        WorldBase::OnEndSimulation();
        
        if (m_pRuntimeWorld != nullptr)
        {
            m_pRuntimeWorld->EndSimulation();

            // [Note]
            // We don't need to clear the entities, since GetEntityRegistry will now return the asset's registry
            // instead of this one. All entities are cleaned up from the runtime registry when beginning the simulation.

            NES_ASSERT(m_pCurrentWorldAsset != nullptr);
            auto& assetEntityRegistry = m_pCurrentWorldAsset->GetEntityRegistry();

            auto view = assetEntityRegistry.GetAllEntitiesWith<IDComponent>();
            for (auto entity : view)
            {
                // Re-add the Pending Initialization Component
                assetEntityRegistry.AddComponent<PendingInitialization>(entity);

                // If not disabled by default, re-add the Pending Enable Component.
                if (!assetEntityRegistry.HasComponent<DisabledComponent>(entity))
                    assetEntityRegistry.AddComponent<PendingEnable>(entity);
            }
        }
    }
}
