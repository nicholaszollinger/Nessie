// World.cpp
#include "World.h"
#include <yaml-cpp/yaml.h>
#include "Actor.h"

namespace nes
{
    World::World(Scene* pScene)
        : EntityLayer(pScene)
        , m_actorPool(this)
    {
        //
    }

    StrongPtr<Actor> World::CreateActor(const EntityID id, const StringID& name)
    {
        StrongPtr<Actor> pActor = m_actorPool.CreateEntity(id, name);
        return pActor;
    }

    void World::DestroyEntity(const LayerHandle& handle)
    {
        m_actorPool.QueueDestroyEntity(handle);
    }

    bool World::IsValidEntity(const LayerHandle& handle) const
    {
        return m_actorPool.IsValidEntity(handle);
    }

    bool World::InitializeLayer()
    {
        for (auto& actor : m_actorPool)
        {
            if (!actor.Init())
            {
                NES_ERRORV("World", "Failed to initialize World! Failed to initialize Actor: ", actor.GetName().CStr());
                return false;
            }
        }
        
        return true;
    }

    void World::OnSceneBegin()
    {
        // [TODO]:
    }

    void World::DestroyLayer()
    {
        m_actorPool.ClearPool();
    }

    void World::OnEvent([[maybe_unused]] Event& event)
    {
        // [TODO]: 
    }
    
    void World::RenderEditorEntityHierarchy()
    {
        // [TODO]: 
    }

    void World::Render([[maybe_unused]] const Camera& worldCamera)
    {
        // [TODO]: 
        // Push the Camera uniforms for the Mesh Rendering.
    }

    void World::Tick([[maybe_unused]] const double deltaTime)
    {
        // [TODO]:
        // Tick the Collision System...
        m_actorPool.ProcessDestroyedEntities();
    }

    bool World::LoadLayer(YAML::Node& layerNode)
    {
        auto actors = layerNode["Actors"];
        if (!actors)
        {
            NES_ERROR("Failed to load World Layer! No Actors node found!");
            return false;
        }

        for (auto actorNode : actors)
        {
            const uint64_t actorID = actorNode["Actor"].as<uint64_t>(); 
            const StringID actorName = actorNode["Name"].as<std::string>();
            StrongPtr<Actor> pActor = CreateActor(actorID, actorName);
            
            auto componentsNode = actorNode["Components"];
            for (auto componentNode : componentsNode)
            {
                const StringID componentName = componentNode.first.as<std::string>();

                // [HACK]: Just checking for specific components for now. 
                // [TODO]: Loading Components should be done systematically, through
                // some Factory or Serialize function.
                if (componentName == WorldComponent::GetStaticTypename())
                {
                    auto worldComponentNode = componentNode.second; 
                    const StringID name = worldComponentNode["Name"].as<std::string>();
                    auto* pWorldComponent = pActor->AddComponent<WorldComponent>(name);
                    LoadWorldComponentData(*pWorldComponent, worldComponentNode);

                    // [HACK]: Just auto setting the root if necessary.
                    // Entity should probably have something like OnComponentAdded(Component*);
                    if (pActor->GetRootComponent() == nullptr)
                        pActor->SetRootComponent(pWorldComponent);
                }
            }
        }

        return true;
    }

    void World::LoadWorldComponentData(WorldComponent& worldComponent, YAML::Node& componentNode) const
    {
        // IsEnabled:
        const bool isEnabled = componentNode["IsEnabled"].as<bool>();
        worldComponent.SetEnabled(isEnabled);
        
        // Parent:
        auto parentNode = componentNode["Parent"];
        if (!parentNode.IsNull())
        {
            // [TODO]: 
        }
        
        // If the node has no parent, then set the World Root as the parent.
        // else
        // {
        //     worldComponent.SetParent(m_pWorldRoot);            
        // }

        // Location
        Vector3 location;
        const auto locationNode = componentNode["Location"];
        {
            location.x = locationNode[0].as<float>();
            location.y = locationNode[1].as<float>();
            location.z = locationNode[2].as<float>();
        }

        // Orientation
        Quat orientation;
        const auto orientationNode = componentNode["Orientation"];
        {
            Vector3 eulerAngles;
            eulerAngles.x = orientationNode[0].as<float>();
            eulerAngles.y = orientationNode[1].as<float>();
            eulerAngles.z = orientationNode[2].as<float>();
            orientation = Quat::MakeFromEuler(eulerAngles);
        }

        // Scale
        Vector3 scale;
        const auto scaleNode = componentNode["Scale"];
        {
            scale.x = scaleNode[0].as<float>();
            scale.y = scaleNode[1].as<float>();
            scale.z = scaleNode[2].as<float>();
        }
        worldComponent.SetLocalTransform(location, orientation, scale);
    }
}
