// EditorWorld.cpp
#include "EditorWorld.h"

#include "Nessie/Core/MoveElement.h"
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

        ConnectRootEntityCallbacks(m_pCurrentWorldAsset->GetEntityRegistry());
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
    
    void EditorWorld::DestroyEntity(const EntityHandle entity)
    {
        GetEntityRegistry()->MarkEntityForDestruction(entity);
    }

    void EditorWorld::ParentEntity(const EntityHandle entity, const EntityHandle parent)
    {
        auto* pRegistry = GetEntityRegistry();
        NES_ASSERT(pRegistry != nullptr);

        // Update Root Entity status:
        auto& idComp = pRegistry->GetComponent<IDComponent>(entity);
        auto& nodeComp = pRegistry->GetComponent<NodeComponent>(entity);
        if (!nodeComp.HasParent() && parent != kInvalidEntityHandle)
        {
            // We are parenting a root entity to another entity, remove it from the array.
            RemoveRootEntity(idComp.GetID());
        }
        else if (nodeComp.HasParent() && parent == kInvalidEntityHandle)
        {
            // We are removing the parent entity, it is now a root.
            AddRootEntity(idComp.GetID());
        }
        
        NES_ASSERT(m_pRuntimeWorld != nullptr);
        m_pRuntimeWorld->ParentEntity(entity, parent);
    }

    const std::vector<EntityID>* EditorWorld::GetRootEntities() const
    {
        if (IsSimulating())
            return &m_runtimeRootEntities;
        
        if (m_pCurrentWorldAsset != nullptr)
            return &m_pCurrentWorldAsset->GetRootEntities();

        return nullptr;
    }

    void EditorWorld::AddRootEntity(const EntityID id)
    {
        if (auto* pRootEntities = GetMutableRootEntities())
        {
            if (std::ranges::find(*pRootEntities, id) == pRootEntities->end())
            {
                pRootEntities->emplace_back(id);
            }            
        }
    }

    void EditorWorld::ReorderRootEntity(const EntityID id, const EntityID target, const bool insertAfter)
    {
        if (auto* pRootEntities = GetMutableRootEntities())
        {
            auto targetPosition = std::ranges::find(*pRootEntities, target);
            auto currentPosition = std::ranges::find(*pRootEntities, id);
        
            if (std::ranges::find(*pRootEntities, id) == pRootEntities->end())
            {
                // The entity is not already a root, insert it.
                if (insertAfter && targetPosition != pRootEntities->end())
                    ++targetPosition;
            
                pRootEntities->insert(targetPosition, id);
            }
            else
            {
                MoveElement(*pRootEntities, currentPosition, targetPosition, insertAfter);
            }
        }
        
    }

    void EditorWorld::RemoveRootEntity(const EntityID id)
    {
        if (auto* pRootEntities = GetMutableRootEntities())
        {
            std::erase(*pRootEntities, id);  
        }
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
            RemoveRootEntityCallbacks(m_pCurrentWorldAsset->GetEntityRegistry());
            
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
            RemoveRootEntityCallbacks(m_pCurrentWorldAsset->GetEntityRegistry());

            // Beginning the simulation will set the IsSimulating flag, which will
            // instruct the runtime world to use its own entity registry rather than the Asset Version.
            m_pRuntimeWorld->BeginSimulation();

            // Destroy any entities that were in the runtime world.
            m_pRuntimeWorld->DestroyAllEntities();

            // Respond to root entity creation:
            ConnectRootEntityCallbacks(*m_pRuntimeWorld->GetEntityRegistry());
            
            // Merge the asset's entities into the runtime world.
            m_pRuntimeWorld->MergeWorld(*m_pCurrentWorldAsset);
        }
    }

    void EditorWorld::OnEndSimulation()
    {
        WorldBase::OnEndSimulation();
        
        if (m_pRuntimeWorld != nullptr)
        {
            // Remove the callbacks from the Runtime World's registry.
            RemoveRootEntityCallbacks(*m_pRuntimeWorld->GetEntityRegistry());
            m_pRuntimeWorld->EndSimulation();
            
            // [Note]
            // We don't need to clear the entities, since GetEntityRegistry will now return the asset's registry
            // instead of this one. All entities are cleaned up from the runtime registry when beginning the simulation.

            NES_ASSERT(m_pCurrentWorldAsset != nullptr);
            auto& assetEntityRegistry = m_pCurrentWorldAsset->GetEntityRegistry();
            
            ConnectRootEntityCallbacks(assetEntityRegistry);
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

    std::vector<EntityID>* EditorWorld::GetMutableRootEntities()
    {
        if (IsSimulating())
            return &m_runtimeRootEntities;
        
        if (m_pCurrentWorldAsset != nullptr)
            return &m_pCurrentWorldAsset->GetRootEntities();

        return nullptr;
    }

    void EditorWorld::ConnectRootEntityCallbacks(EntityRegistry& registry)
    {
        registry.OnComponentCreated<IDComponent>().connect<&EditorWorld::OnIDComponentAdded>(this);
        registry.OnComponentCreated<NodeComponent>().connect<&EditorWorld::OnNodeComponentAdded>(this);
        registry.OnComponentDestroyed<NodeComponent>().connect<&EditorWorld::OnNodeComponentRemoved>(this);
        registry.OnComponentDestroyed<IDComponent>().connect<&EditorWorld::OnIDComponentDestroyed>(this);
    }

    void EditorWorld::RemoveRootEntityCallbacks(EntityRegistry& registry)
    {
        registry.OnComponentCreated<IDComponent>().disconnect<&EditorWorld::OnIDComponentAdded>(this);
        registry.OnComponentCreated<NodeComponent>().disconnect<&EditorWorld::OnNodeComponentAdded>(this);
        registry.OnComponentDestroyed<NodeComponent>().disconnect<&EditorWorld::OnNodeComponentRemoved>(this);
        registry.OnComponentDestroyed<IDComponent>().disconnect<&EditorWorld::OnIDComponentDestroyed>(this);
    }

    void EditorWorld::OnIDComponentAdded(entt::registry& registry, entt::entity entity)
    {
        auto* pRootEntities = GetMutableRootEntities();
        NES_ASSERT(pRootEntities != nullptr);
        
        const auto id = registry.get<IDComponent>(entity).GetID();
        pRootEntities->emplace_back(id);
    }

    void EditorWorld::OnNodeComponentAdded(entt::registry& registry, entt::entity entity)
    {
        auto* pRootEntities = GetMutableRootEntities();
        NES_ASSERT(pRootEntities != nullptr);
        
        const auto id = registry.get<IDComponent>(entity).GetID();
        auto& nodeComp = registry.get<NodeComponent>(entity);

        // If it's not a root entity (no parent), remove it.
        // - The Entity will be set as a root entity to being with, when the IDComponent is added.
        if (nodeComp.m_parentID != kInvalidEntityID)
        {
            std::erase(*pRootEntities, id);
        }
    }

    void EditorWorld::OnNodeComponentRemoved(entt::registry& registry, entt::entity entity)
    {
        auto* pRootEntities = GetMutableRootEntities();
        NES_ASSERT(pRootEntities != nullptr);
        
        // Remove from the Root entities, if applicable.
        if (auto* pIDComponent = registry.try_get<IDComponent>(entity))
        {
            std::erase(*pRootEntities, pIDComponent->GetID());
        }
    }

    void EditorWorld::OnIDComponentDestroyed(entt::registry& registry, entt::entity entity)
    {
        auto* pRootEntities = GetMutableRootEntities();
        NES_ASSERT(pRootEntities != nullptr);
        
        const auto id = registry.get<IDComponent>(entity).GetID();
        std::erase(*pRootEntities, id);
    }
}
