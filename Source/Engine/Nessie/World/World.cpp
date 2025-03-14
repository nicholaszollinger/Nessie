// World.cpp
#include "World.h"
#include <yaml-cpp/yaml.h>
#include "Entity3D.h"
#include "Components/CameraComponent.h"
#include "Components/FreeCamMovementComponent.h"

namespace nes
{
    World::World(Scene* pScene)
        : EntityLayer(pScene)
        , m_entityPool(this)
    {
        //
    }

    StrongPtr<Entity3D> World::CreateEntity(const EntityID& id, const StringID& name)
    {
        StrongPtr<Entity3D> pActor = m_entityPool.CreateEntity(id, name);
        return pActor;
    }

    // [TODO]: Can these be pushed up to the base layer?
    void World::DestroyEntity(const LayerHandle& handle)
    {
        m_entityPool.QueueDestroyEntity(handle);
    }

    // [TODO]: Can these be pushed up to the base layer? 
    bool World::IsValidNode(const LayerHandle& handle) const
    {
        return m_entityPool.IsValidEntity(handle);
    }

    bool World::InitializeLayer()
    {
        for (auto& entity : m_entityPool)
        {
            if (!entity.Init())
            {
                NES_ERRORV("World", "Failed to initialize World! Failed to initialize Entity: ", entity.GetName().CStr());
                return false;
            }
        }
        
        return true;
    }

    void World::OnSceneBegin()
    {
        // [TODO]:
    }

    void World::OnLayerDestroyed()
    {
        m_entityPool.ClearPool();
    }

    void World::OnEvent([[maybe_unused]] Event& event)
    {
        // [TODO]: 
    }
    
    void World::EditorRenderEntityHierarchy()
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
        //m_actorPool.ProcessDestroyedEntities();
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
            StrongPtr<Entity3D> pActor = CreateEntity(actorID, actorName);

            // Load Actor Data:
            {
                // IsEnabled:
                const bool isEnabled = actorNode["IsEnabled"].as<bool>();
                pActor->SetEnabled(isEnabled);
                
                // Parent:
                auto parentNode = actorNode["Parent"];
                if (!parentNode.IsNull())
                {
                    // [TODO]: Save the EntityID of the parent, and
                    // perform a parenting step at the end.
                }
                
                // Location
                Vector3 location;
                const auto locationNode = actorNode["Location"];
                {
                    location.x = locationNode[0].as<float>();
                    location.y = locationNode[1].as<float>();
                    location.z = locationNode[2].as<float>();
                }
                
                // Orientation
                Quat orientation;
                const auto orientationNode = actorNode["Orientation"];
                {
                    Vector3 eulerAngles;
                    eulerAngles.x = orientationNode[0].as<float>();
                    eulerAngles.y = orientationNode[1].as<float>();
                    eulerAngles.z = orientationNode[2].as<float>();
                    orientation = Quat::MakeFromEuler(eulerAngles);
                }
                
                // Scale
                Vector3 scale;
                const auto scaleNode = actorNode["Scale"];
                {
                    scale.x = scaleNode[0].as<float>();
                    scale.y = scaleNode[1].as<float>();
                    scale.z = scaleNode[2].as<float>();
                }
                pActor->SetLocalTransform(location, orientation, scale);
            }
            
            auto componentsNode = actorNode["Components"];
            for (auto componentNode : componentsNode)
            {
                const StringID componentName = componentNode.first.as<std::string>();

                // [HACK]: Just checking for specific components for now. 
                // [TODO]: Loading Components should be done systematically, through
                // some Factory or Serialize function.

                // Camera
                if (componentName == CameraComponent::GetStaticTypename())
                {
                    auto cameraNode = componentNode.second;
                    const StringID name = cameraNode["Name"].as<std::string>();
                    StrongPtr<CameraComponent> pCamera = pActor->AddComponent<CameraComponent>(name);
                    //LoadWorldComponentData(*pCamera, cameraNode);

                    const bool setActiveOnEnable = cameraNode["SetActiveOnEnabled"].as<bool>(true);
                    pCamera->SetActiveOnEnabled(setActiveOnEnable);

                    // Camera Data:
                    auto& camera = pCamera->GetCamera();
                    
                    // Perspective Params
                    float value = cameraNode["PerspectiveFOV"].as<float>();
                    camera.SetPerspectiveFOV(value);

                    value = cameraNode["PerspectiveNear"].as<float>();
                    camera.SetPerspectiveNearPlane(value);

                    value = cameraNode["PerspectiveFar"].as<float>();
                    camera.SetPerspectiveFarPlane(value);

                    // Orthographic Params
                    value = cameraNode["OrthographicSize"].as<float>();
                    camera.SetOrthographicSize(value);
                    
                    value = cameraNode["OrthographicNear"].as<float>();
                    camera.SetOrthographicNearPlane(value);

                    value = cameraNode["OrthographicFar"].as<float>();
                    camera.SetOrthographicFarPlane(value);

                    // ProjectionType
                    const auto projectionType = static_cast<Camera::ProjectionType>(cameraNode["ProjectionType"].as<uint8_t>());
                    camera.SetProjectionType(projectionType);
                }

                // Free Cam
                if (componentName == FreeCamMovementComponent::GetStaticTypename())
                {
                    auto freeCamNode = componentNode.second;
                    const StringID name = freeCamNode["Name"].as<std::string>();
                    StrongPtr<FreeCamMovementComponent> pFreeCam = pActor->AddComponent<FreeCamMovementComponent>(name);

                    float value = freeCamNode["MoveSpeed"].as<float>();
                    pFreeCam->SetMoveSpeed(value);

                    value = freeCamNode["TurnSpeedYaw"].as<float>();
                    pFreeCam->SetTurnSpeedYaw(value);

                    value = freeCamNode["TurnSpeedPitch"].as<float>();
                    pFreeCam->SetTurnSpeedPitch(value);

                    const bool isEnabled = freeCamNode["IsEnabled"].as<bool>(true);
                    pFreeCam->SetEnabled(isEnabled);
                }
            }
        }

        return true;
    }
}
