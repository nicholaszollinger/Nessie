// World.cpp
#include "World.h"
#include <yaml-cpp/yaml.h>
#include "Entity3D.h"
#include "Nessie/Application/Application.h"
#include "Nessie/World/Components/CameraComponent.h"
#include "Nessie/World/Components/FreeCamMovementComponent.h"
#include "Nessie/Physics/Collision/Shapes/BoxShape.h"
#include "Nessie/Physics/Collision/Shapes/EmptyShape.h"
#include "Nessie/Scene/TickManager.h"

// Hack to test stb_image
#include "stb_image.h"
#include "Nessie/Physics/Collision/CollisionSolver.h"

namespace nes
{
    //-----------------------------------------------------------------------------------------------
    // [TEMP]: Physics System Config Variables 
    //-----------------------------------------------------------------------------------------------
    static constexpr unsigned kNumBodies                = 10240;
    static constexpr unsigned kNumBodyMutexes           = 0; // Autodetect
    static constexpr unsigned kMaxBodyPairs             = 65636;
    static constexpr unsigned kMaxContactConstraints    = 20480;

    Body& CreateFloor(BodyInterface& bodyInterface, const float size = 200.f, const float worldScale = 1.f)
    {
        Body& floor = *bodyInterface.CreateBody(BodyCreateInfo(new BoxShape(worldScale * Vec3(0.5f * size, 1.f, 0.5f * size), 0.f), RVec3(worldScale * Vec3(0.f, -1.f, 0.f)), Quat::Identity(), EBodyMotionType::Static, PhysicsLayers::kNonMoving));
        bodyInterface.AddBody(floor.GetID(), EBodyActivationMode::DontActivate);
        return floor;
    }
    
    World::World(Scene* pScene)
        : EntityLayer(pScene)
        , m_entityPool(this)
        , m_prePhysicsTickGroup(ETickStage::PrePhysics)
        , m_physicsTickGroup(ETickStage::Physics)
        , m_postPhysicsTickGroup(ETickStage::PostPhysics)
        , m_lateTickGroup(ETickStage::Late)
    {
        m_prePhysicsTickGroup.SetDebugName("World PrePhysics Tick");
        m_physicsTickGroup.SetDebugName("World Physics Tick");
        m_postPhysicsTickGroup.SetDebugName("World PostPhysics Tick");
        m_lateTickGroup.SetDebugName("World Late Tick");

        m_pPhysicsAllocator = NES_NEW(StackAllocator(static_cast<size_t>(32 * 1024 * 1024)));
        m_pJobSystem = NES_NEW(JobSystemThreadPool(physics::kMaxPhysicsJobs, physics::kMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1));
    }

    StrongPtr<Entity3D> World::CreateEntity(const EntityID& id, const StringID& name)
    {
        StrongPtr<Entity3D> pActor = m_entityPool.CreateEntity(id, name);
        return pActor;
    }

    void World::RegisterTickToWorldTickGroup(TickFunction* pFunction, const ETickStage stage)
    {
        switch (stage)
        {
            case ETickStage::PrePhysics:
                pFunction->RegisterTick(&m_prePhysicsTickGroup);
                break;
            
            case ETickStage::Physics:
                pFunction->RegisterTick(&m_physicsTickGroup);
                break;
            
            case ETickStage::PostPhysics:
                pFunction->RegisterTick(&m_postPhysicsTickGroup);
                break;
            
            case ETickStage::Late:
                pFunction->RegisterTick(&m_lateTickGroup);
                break;
            
            default:
                NES_ERROR(kWorldLogTag, "Attempted to register Tick to invalid World Tick Group!");
                break;
        }
    }

    TickGroup* World::GetTickGroup(const ETickStage stage)
    {
        switch (stage)
        {
            case ETickStage::PrePhysics:     return &m_prePhysicsTickGroup;
            case ETickStage::Physics:        return &m_physicsTickGroup;
            case ETickStage::PostPhysics:    return &m_postPhysicsTickGroup;
            case ETickStage::Late:           return &m_lateTickGroup;

            default:
                NES_ERROR(kWorldLogTag, "Attempted to get invalid World Tick Group!");
                return nullptr;
        }
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

    void World::RegisterEventHandler(const EventHandler& handler)
    {
        // [TODO]: Need better registration management.
        m_eventHandlers.push_back(handler);
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      [TODO]: This should return a handle. 
    //		
    ///		@brief : Register a Mesh for drawing. 
    //----------------------------------------------------------------------------------------------------
    void World::RegisterMesh(MeshComponent* pMesh)
    {
        NES_ASSERT(pMesh != nullptr);

        auto pMaterial = pMesh->GetMaterial();
        if (!pMaterial)
        {
            NES_WARN(kWorldLogTag, "Attempted to register a Mesh with an invalid Material!");
            return;
        }

        // Register the Mesh to the appropriate array.
        if (pMaterial->IsTransparent())
            m_transparentMeshes.push_back(pMesh);
        else
            m_opaqueMeshes.push_back(pMesh);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the current Default Geometry Pipeline.  
    //----------------------------------------------------------------------------------------------------
    std::shared_ptr<nes::RendererContext::GraphicsPipeline> World::GetDefaultMeshRenderPipeline() const
    {
        const size_t pipelineIndex = static_cast<size_t>(m_currentRenderMode);
        NES_ASSERT(pipelineIndex < m_defaultMeshPipelines.size());
        auto& pCurrentPipeline = m_defaultMeshPipelines[pipelineIndex];
        return pCurrentPipeline;
    }

    bool World::InitializeLayer()
    {
        // [TODO]: This should be moved.
        // Register Shape functions
        CollisionSolver::Internal_Init();
        ConvexShape::Register();
        BoxShape::Register();
        EmptyShape::Register();
        
        // Add Tick Groups:
        auto& tickManager = TickManager::Get();
        tickManager.RegisterTickGroup(&m_prePhysicsTickGroup);
        tickManager.RegisterTickGroup(&m_physicsTickGroup);
        tickManager.RegisterTickGroup(&m_postPhysicsTickGroup);
        tickManager.RegisterTickGroup(&m_lateTickGroup);

        // Create the Physics Scene
        m_pPhysicsScene = NES_NEW(PhysicsScene());
        PhysicsScene::CreateInfo physicsCreateInfo;
        physicsCreateInfo.m_maxBodies = kNumBodies;
        physicsCreateInfo.m_numBodyMutexes = kNumBodyMutexes;
        physicsCreateInfo.m_maxNumBodyPairs = kMaxBodyPairs;
        physicsCreateInfo.m_maxNumContactConstraints = kMaxContactConstraints;
        physicsCreateInfo.m_pCollisionLayerPairFilter = &m_layerPairFilter;
        physicsCreateInfo.m_pCollisionVsBroadPhaseLayerFilter = &m_layerVsBroadPhaseFilter;
        physicsCreateInfo.m_pLayerInterface = &m_broadPhaseLayerInterface;
        m_pPhysicsScene->Init(physicsCreateInfo);
        
        m_pPhysicsScene->SetSettings(m_physicsSettings);
        m_pPhysicsScene->SetBodyActivationListener(&m_bodyActivationListener);
        
        // Set up the Physics Tick
        m_physicsTick.SetTickInterval(1.f / 60.f);
        m_physicsTick.m_pAllocator = m_pPhysicsAllocator;
        m_physicsTick.m_pPhysicsScene = m_pPhysicsScene;
        m_physicsTick.m_pJobSystem = m_pJobSystem;
        m_physicsTick.m_collisionSteps = 1;
        m_physicsTick.RegisterTick(&m_physicsTickGroup);

        // [TEMP]: Simple test for the Physics System.
        auto& bodyInterface = m_pPhysicsScene->GetBodyInterface();
        CreateFloor(bodyInterface);
        ConstStrongPtr<Shape> boxShape = NES_NEW(BoxShape(Vec3(0.5f, 1.f, 2.f)));
        
        // Dynamic Body 1
        m_testID = bodyInterface.CreateAndAddBody(BodyCreateInfo(boxShape, RVec3(0, 10, 0), Quat::Identity(), EBodyMotionType::Dynamic, PhysicsLayers::kMoving), EBodyActivationMode::Activate);
        
        // Dynamic Body 2
        const auto id2 = bodyInterface.CreateAndAddBody(BodyCreateInfo(boxShape, RVec3(5, 10, 0), Quat::FromAxisAngle(Vec3::AxisX(), 0.25f * math::Pi()), EBodyMotionType::Dynamic, PhysicsLayers::kMoving), EBodyActivationMode::Activate);
        
        // // Dynamic Body 3
        bodyInterface.CreateAndAddBody(BodyCreateInfo(boxShape, RVec3(10, 10, 0), Quat::FromAxisAngle(Vec3::AxisX(), 0.25f * math::Pi()), EBodyMotionType::Dynamic, PhysicsLayers::kMoving), EBodyActivationMode::Activate);
        
        for (auto& entity : m_entityPool)
        {
            if (!entity.Init())
            {
                NES_ERROR(kWorldLogTag, "Failed to initialize World! Failed to initialize Entity: {}", entity.GetName().CStr());
                return false;
            }
        }
        
        return true;
    }

    void World::OnSceneBegin()
    {
        // [TODO]: Begin Physics?
    }

    void World::OnLayerDestroyed()
    {
        // Unregister Tick Groups:
        auto& tickManager = TickManager::Get();
        tickManager.UnregisterTickGroup(&m_prePhysicsTickGroup);
        tickManager.UnregisterTickGroup(&m_physicsTickGroup);
        tickManager.UnregisterTickGroup(&m_postPhysicsTickGroup);
        tickManager.UnregisterTickGroup(&m_lateTickGroup);

        // Shutdown Physics
        if (m_pPhysicsScene)
        {
            auto& bodyInterface = m_pPhysicsScene->GetBodyInterface();
            bodyInterface.RemoveBody(m_testID);
            bodyInterface.DestroyBody(m_testID);

            // Remove the body activate listener.
            m_pPhysicsScene->SetBodyActivationListener(nullptr);
            
            NES_DELETE(m_pPhysicsScene);
        }
        
        m_entityPool.ClearPool();
        FreeRenderResources();

        NES_DELETE(m_pJobSystem);
        NES_DELETE(m_pPhysicsAllocator);
    }

    void World::PreRender(const Camera& sceneCamera)
    {
        // Update Camera Uniforms:
        SceneCameraUniforms cameraUniforms;
        cameraUniforms.m_projectionMatrix = sceneCamera.GetProjectionMatrix();
        cameraUniforms.m_viewMatrix = sceneCamera.GetViewMatrix();
        Renderer::UpdateBuffer(m_cameraUniformBuffer, 0, sizeof(SceneCameraUniforms), &cameraUniforms);

        const Vec3 cameraWorldLocation = sceneCamera.CameraViewLocation();
        
        // Sort Meshes based on Camera position:
        // Sort Opaque Meshes so that the closest meshes are drawn first.
        std::ranges::sort(m_opaqueMeshes, [&cameraWorldLocation](const MeshComponent* pMeshA, const MeshComponent* pMeshB)
        {
            return Vec3::DistanceSqr(cameraWorldLocation, pMeshA->GetOwner()->GetLocation()) < Vec3::DistanceSqr(cameraWorldLocation, pMeshB->GetOwner()->GetLocation());
        });

        // Sort Transparent Meshes so that furthest meshes are drawn first.
        std::ranges::sort(m_transparentMeshes, [&cameraWorldLocation](const MeshComponent* pMeshA, const MeshComponent* pMeshB)
        {
            return Vec3::DistanceSqr(cameraWorldLocation, pMeshA->GetOwner()->GetLocation()) > Vec3::DistanceSqr(cameraWorldLocation, pMeshB->GetOwner()->GetLocation());
        });
    }
    
    void World::Render([[maybe_unused]] const Camera& worldCamera)
    {
        // TODO: This should be part of a RenderPass object.
        static constexpr vk::ClearValue kClearValues[] =
        {
            vk::ClearColorValue({ 0.02f, 0.02f, 0.02f, 1.0f }),
            vk::ClearDepthStencilValue(1.0f, 0),
        };

        // Full screen:
        const auto windowExtent = Application::Get().GetWindow().GetExtent();
        const vk::Rect2D renderArea = {{0, 0}, {windowExtent.m_width, windowExtent.m_height}};

        Renderer::BeginRenderPass(renderArea, kClearValues, _countof(kClearValues));
        {
            RenderSkybox();
            
            // Render all registered Renderables:
            auto pPipeline = GetDefaultMeshRenderPipeline();

            // Render Opaque Meshes:
            for (size_t i = 0; i < m_opaqueMeshes.size(); i++)
            {
                Renderer::BindDescriptorSets(pPipeline, vk::PipelineBindPoint::eGraphics, { m_cameraUniforms });
                m_opaqueMeshes[i]->Render();
            }

            // Render Transparent Meshes:
            for (size_t i = 0; i < m_transparentMeshes.size(); i++)
            {
                Renderer::BindDescriptorSets(pPipeline, vk::PipelineBindPoint::eGraphics, { m_cameraUniforms });
                m_transparentMeshes[i]->Render();
            }

            RenderGrid();
            // [TODO]: I am manually rendering the Editor stuff here. It will be moved once I have time to implement
            // a RenderPass object. Right now, there is some issues with how they are setup in the RendererContext.
            EditorRenderEntityHierarchy();
        }
        Renderer::EndRenderPass();
    }

    void World::OnEvent(Event& event)
    {
        for (const auto& [m_callback] : m_eventHandlers)
        {
            m_callback(event);
            if (event.IsHandled())
                break;
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : 
    //----------------------------------------------------------------------------------------------------
    void World::OnPostTick()
    {
        m_entityPool.ProcessDestroyedEntities();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Load the World Layer
    //----------------------------------------------------------------------------------------------------
    bool World::LoadLayer(YAML::Node& layerNode)
    {
        CreateRenderResources();

        auto entities = layerNode["Entities"];
        if (!entities)
        {
            NES_ERROR(kWorldLogTag, "Failed to load World Layer! No Entities node found!");
            return false;
        }

        std::unordered_map<uint64_t, std::vector<EntityID>> entitiesThatNeedParent;

        for (auto entityNode : entities)
        {
            const uint64_t entityID = entityNode["Entity"].as<uint64_t>(); 
            const StringID entityName = entityNode["Name"].as<std::string>();
            StrongPtr<Entity3D> pEntity = CreateEntity(entityID, entityName);

            // Load Actor Data:
            {
                // IsEnabled:
                const bool isEnabled = entityNode["IsEnabled"].as<bool>();
                pEntity->SetEnabled(isEnabled);
                
                // Parent:
                auto parentNode = entityNode["Parent"];
                if (!parentNode.IsNull())
                {
                    // [TODO]: Save the EntityID of the parent, and
                    // perform a parenting step at the end.
                    const uint64_t parentID = parentNode.as<uint64_t>();

                    // If the Parent is loaded already, set the Parent:
                    if (m_entityPool.IsValidEntity(parentID))
                        pEntity->SetParent(m_entityPool.GetEntity(parentID).Get());
                    
                    // Otherwise save until the parent is loaded.
                    else
                        entitiesThatNeedParent[parentID].push_back(entityID);
                }

                // Set Parent for any entities that are waiting.
                if (auto it = entitiesThatNeedParent.find(entityID); it != entitiesThatNeedParent.end())
                {
                    for (const auto& childID : it->second)
                    {
                        pEntity->AddChild(m_entityPool.GetEntity(childID).Get());
                    }

                    // Remove the array.
                    entitiesThatNeedParent.erase(it);
                }
                
                // Location
                Vec3 location;
                const auto locationNode = entityNode["Location"];
                {
                    location.x = locationNode[0].as<float>();
                    location.y = locationNode[1].as<float>();
                    location.z = locationNode[2].as<float>();
                }
                
                // Rotation
                Rotation rotation;
                const auto orientationNode = entityNode["Rotation"];
                {
                    rotation.m_pitch = orientationNode[0].as<float>();
                    rotation.m_yaw   = orientationNode[1].as<float>();
                    rotation.m_roll  = orientationNode[2].as<float>();
                }
                
                // Scale
                Vec3 scale;
                const auto scaleNode = entityNode["Scale"];
                {
                    scale.x = scaleNode[0].as<float>();
                    scale.y = scaleNode[1].as<float>();
                    scale.z = scaleNode[2].as<float>();
                }
                pEntity->SetLocalTransform(location, rotation, scale);
            }
            
            auto componentsNode = entityNode["Components"];
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
                    StrongPtr<CameraComponent> pCamera = pEntity->AddComponent<CameraComponent>(name);

                    const bool setActiveOnEnable = cameraNode["SetActiveOnEnabled"].as<bool>(true);
                    pCamera->SetActiveOnEnabled(setActiveOnEnable);

                    // Camera Data:
                    auto& camera = pCamera->GetCamera();
                    
                    // Perspective Params
                    float value = cameraNode["PerspectiveFOV"].as<float>();
                    camera.SetPerspectiveFOV(math::ToRadians(value));

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
                    const auto projectionType = static_cast<Camera::EProjectionType>(cameraNode["ProjectionType"].as<uint8_t>());
                    camera.SetProjectionType(projectionType);
                }

                // Free Cam
                if (componentName == FreeCamMovementComponent::GetStaticTypename())
                {
                    auto freeCamNode = componentNode.second;
                    const StringID name = freeCamNode["Name"].as<std::string>();
                    StrongPtr<FreeCamMovementComponent> pFreeCam = pEntity->AddComponent<FreeCamMovementComponent>(name);

                    float value = freeCamNode["MoveSpeed"].as<float>();
                    pFreeCam->SetMoveSpeed(value);

                    value = freeCamNode["TurnSpeedYaw"].as<float>();
                    pFreeCam->SetTurnSpeedYaw(value);

                    value = freeCamNode["TurnSpeedPitch"].as<float>();
                    pFreeCam->SetTurnSpeedPitch(value);

                    const bool isEnabled = freeCamNode["IsEnabled"].as<bool>(true);
                    pFreeCam->SetEnabled(isEnabled);
                }

                // MeshComponent
                if (componentName == MeshComponent::GetStaticTypename())
                {
                    auto meshNode = componentNode.second;
                    const StringID name = meshNode["Name"].as<std::string>();
                    StrongPtr<MeshComponent> pMeshComponent = pEntity->AddComponent<MeshComponent>(name);

                    // [Hack] Setting the default pipeline for now.
                    pMeshComponent->SetPipeline(GetDefaultMeshRenderPipeline());

                    // [TODO]: Set the mesh from an index/id.
                    pMeshComponent->SetMesh(m_meshAssets[0]);
                    
                    // [TODO]: Set the material from an index/id.
                    pMeshComponent->SetMaterial(m_materialAssets[0]);
                }
            }
        }

        // Set any remaining parent/child relationships.
        for (auto& [parentID, children] : entitiesThatNeedParent)
        {
            auto pParent = m_entityPool.GetEntity(parentID);

            for (const auto& childID : children)
            {
                pParent->AddChild(m_entityPool.GetEntity(childID).Get());
            }
        }

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      [TODO]: This is 'fit for purpose' for now, as I am debugging things.
    //      This should be an EditorPanel with a draw function. I want there to be tabs for each scene layer
    //      as well as an "all" view.
    //		
    ///		@brief : Draw an Entity tree node. 
    //----------------------------------------------------------------------------------------------------
    void World::EditorRenderEntityHierarchy()
    {
        // This is thrown in so that I can debug some issues. This function should not
        // handle Renderer::BeginImGui and Renderer::EndImGui. That should be in the Scene.
        Renderer::BeginImGui();
        if (ImGui::Begin("World"))
        {
            ImGui::SeparatorText("Hierarchy");
            if (ImGui::BeginChild("##HierarchyTree", ImVec2(0, 0), ImGuiChildFlags_ResizeY | ImGuiChildFlags_Border | ImGuiChildFlags_NavFlattened))
            {
                if (ImGui::BeginTable("##bg", 1))
                {
                    for (auto& entity : m_entityPool)
                    {
                        // Only draw entity nodes of the root level.
                        if (entity.GetParent() != nullptr)
                            continue;

                        EditorDrawEntityNode(entity);
                    }
                    ImGui::EndTable();
                }
            
                ImGui::EndChild();
            }

            // Inspector
            EditorDrawInspector();

            ImGui::End();
        }
        Renderer::EndImGui();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      [TODO]: This is 'fit for purpose' for now, as I am debugging things. This should be an 'EditorPanel'
    //      with a virtual draw function.
    //		
    ///		@brief : Draw the currently selected Entity's information. 
    //----------------------------------------------------------------------------------------------------
    void World::EditorDrawInspector()
    {
        ImGui::SeparatorText("Inspector");
        if (ImGui::BeginChild("##InspectorView", ImVec2(0, 0), ImGuiChildFlags_ResizeY | ImGuiChildFlags_Border | ImGuiChildFlags_NavFlattened))
        {
            if (m_pSelectedEntity != nullptr)
            {
                // Render the Entity Tranform information:
                auto location = m_pSelectedEntity->GetLocalLocation();
                auto rotation = m_pSelectedEntity->GetLocalRotation();
                auto scale = m_pSelectedEntity->GetLocalScale();
                if (EditorDrawPropertyTransform("Transform", location, rotation, scale))
                {
                    rotation.Normalize();
                    m_pSelectedEntity->SetLocalTransform(location, rotation, scale);
                }

                // Render Components:
                ImGui::SeparatorText("Components");
                if (ImGui::BeginTable("##components", 1, ImGuiTableFlags_ScrollY))
                {
                    for (auto& pConstComp : m_pSelectedEntity->GetComponents())
                    {
                        StrongPtr<Entity3DComponent> pComponent = Cast<Entity3DComponent>(pConstComp);
                        EditorDrawComponentNode(pComponent);
                    }

                    ImGui::EndTable();
                }
            }
            
            ImGui::EndChild();
        }
        
        // Render Selected Component Properties:
        ImGui::SeparatorText("Properties");
        if (ImGui::BeginChild("##PropertiesView", ImVec2(0, 0), ImGuiChildFlags_ResizeY | ImGuiChildFlags_Border | ImGuiChildFlags_NavFlattened))
        {
            if (m_pSelectedComponent != nullptr)
            {
                EditorDrawComponentProperties(m_pSelectedComponent);   
            }

            ImGui::EndChild();
        }
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      [TODO]: This is 'fit for purpose' for now, as I am debugging things. I need to make a formal,
    //      systematic process for rendering Entity trees.
    //		
    ///		@brief : Draw an Entity tree node. 
    //----------------------------------------------------------------------------------------------------
    void World::EditorDrawEntityNode(Entity3D& entity)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::PushID(entity.GetName().CStr());
        ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_None;
        treeFlags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        treeFlags |= ImGuiTreeNodeFlags_NavLeftJumpsBackHere;

        if (m_pSelectedEntity == &entity)
            treeFlags |= ImGuiTreeNodeFlags_Selected;

        const auto& children = entity.GetChildren();
        if (children.empty())
            treeFlags |= ImGuiTreeNodeFlags_Leaf;

        bool nodeOpen = ImGui::TreeNodeEx("", treeFlags, "%s", entity.GetName().CStr());
        if (ImGui::IsItemFocused())
        {
            m_pSelectedEntity = &entity;
            m_pSelectedComponent = nullptr;

            // Select the first component if available.
            auto& components = entity.GetComponents();
            if (!components.empty())
                m_pSelectedComponent = Cast<Entity3DComponent>(components[0]);
        }

        if (nodeOpen)
        {
            for (auto* pChild : children)
            {
                EditorDrawEntityNode(*pChild);
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      [TODO]: This is 'fit for purpose' for now, as I am debugging things. I need to make a formal,
    //      systematic process for rendering Component lists.
    //		
    ///		@brief : Draw a Component node. 
    //----------------------------------------------------------------------------------------------------
    void World::EditorDrawComponentNode(StrongPtr<Entity3DComponent>& pComponent)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::PushID(pComponent->GetName().CStr());
        ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_None;
        treeFlags |= ImGuiTreeNodeFlags_Leaf;
        
        if (pComponent == m_pSelectedComponent)
            treeFlags |= ImGuiTreeNodeFlags_Selected;
        
        ImGui::TreeNodeEx("", treeFlags, "%s", pComponent->GetName().CStr());
        if (ImGui::IsItemFocused())
        {
            m_pSelectedComponent = pComponent;
        }
        
        ImGui::TreePop();
        ImGui::PopID();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      [TODO]: This is 'fit for purpose' for now, as I am debugging things. I need to make a formal,
    //      systematic process for rendering properties.
    //		
    ///		@brief : Draw the currently selected Component's properties. 
    //----------------------------------------------------------------------------------------------------
    void World::EditorDrawComponentProperties(StrongPtr<Entity3DComponent>& pComponent)
    {
        ImGui::Text("%s", pComponent->GetTypename());
        ImGui::Separator();

        if (ImGui::BeginTable("##Component", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 2.0f); // Default twice larger

            const TypeID componentTypeID = pComponent->GetTypeID();

            // [TODO] This would be a factory function with Components implementing their version. 
            switch (componentTypeID)
            {
                case CameraComponent::GetStaticTypeID():
                {
                    auto pCameraComp = Cast<CameraComponent>(pComponent);
                    auto& camera = pCameraComp->GetCamera();
                
                    // Projection Type
                    const auto projectionType = camera.GetProjectionType();
                    // [TODO]: Handle changing type with Enum property.
                    bool cameraNeedsUpdate = false;

                    if (projectionType == Camera::EProjectionType::Perspective)
                    {
                        float value = camera.GetPerspectiveFOV() * math::RadiansToDegrees();
                        
                        if (EditorDrawPropertyFloat("FOV", value))
                        {
                            value = math::Max(0.1f, value);
                            camera.SetPerspectiveFOV(value * math::DegreesToRadians());
                            cameraNeedsUpdate = true;
                        }

                        value = camera.GetPerspectiveNear();
                        if (EditorDrawPropertyFloat("Near", value))
                        {
                            value = math::Max(0.1f, value);
                            camera.SetPerspectiveNearPlane(value);
                            cameraNeedsUpdate = true;
                        }

                        value = camera.GetPerspectiveFar();
                        if (EditorDrawPropertyFloat("Far", value))
                        {
                            value = math::Max(0.1f, value);
                            camera.SetPerspectiveFarPlane(value);
                            cameraNeedsUpdate = true;
                        }
                    }
                    
                    // [TODO]: Same for orthographic if set as projection type.                    

                    // Update camera if necessary:
                    if (cameraNeedsUpdate)
                    {
                        const auto extent = Application::Get().GetWindow().GetExtent();
                        camera.UpdateViewport(extent.m_width, extent.m_height);
                    }
                    
                    break;
                }

                case FreeCamMovementComponent::GetStaticTypeID():
                {
                    auto pFreeCamMovement = Cast<FreeCamMovementComponent>(pComponent);
                    float speed = pFreeCamMovement->GetMoveSpeed();
                    if (EditorDrawPropertyFloat("Move Speed", speed))
                        pFreeCamMovement->SetMoveSpeed(speed);

                    speed = pFreeCamMovement->GetTurnSpeedYaw();
                    if (EditorDrawPropertyFloat("Turn Speed Yaw", speed))
                        pFreeCamMovement->SetTurnSpeedYaw(speed);

                    speed = pFreeCamMovement->GetTurnSpeedPitch();
                    if (EditorDrawPropertyFloat("Turn Speed Pitch", speed))
                        pFreeCamMovement->SetTurnSpeedPitch(speed);
                    break;
                }

                case MeshComponent::GetStaticTypeID():
                {
                    auto pMeshComp = Cast<MeshComponent>(pComponent);
                    auto pMaterial = pMeshComp->GetMaterial();

                    // Just Base Color for now.
                    LinearColor baseColor = pMaterial->m_baseColor;
                    if (EditorDrawPropertyLinearColor("Base Color", baseColor))
                        pMaterial->m_baseColor = baseColor;

                    break;
                }
            
                default:
                {
                    NES_ERROR(kWorldLogTag, "Unhandled Component type!: {}", pComponent->GetTypename());
                }
            }
            
            ImGui::EndTable();
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Draw an editable Vec3 in the Inspector. 
    //----------------------------------------------------------------------------------------------------
    bool World::EditorDrawPropertyVec3(const char* pLabel, Vec3& value)
    {
        ImGui::TableNextRow();
        ImGui::PushID(pLabel);
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(pLabel);
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        bool wasChanged = ImGui::DragFloat3("##Editor", reinterpret_cast<float*>(&value));
        ImGui::PopID();
        
        return wasChanged;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Draw an editable Rotation in the Inspector. 
    //----------------------------------------------------------------------------------------------------
    bool World::EditorDrawPropertyRotation(const char* pLabel, Rotation& value)
    {
        ImGui::TableNextRow();
        ImGui::PushID(pLabel);
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(pLabel);
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        bool wasChanged = ImGui::DragFloat3("##Editor", reinterpret_cast<float*>(&value));
        ImGui::PopID();
        return wasChanged;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Draw an editable Float in the Inspector. 
    //----------------------------------------------------------------------------------------------------
    bool World::EditorDrawPropertyFloat(const char* pLabel, float& value)
    {
        ImGui::TableNextRow();
        ImGui::PushID(pLabel);
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(pLabel);
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        bool wasChanged = ImGui::DragFloat("##Editor", &value);
        ImGui::PopID();

        return wasChanged;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Draw an editable Bool in the Inspector. 
    //----------------------------------------------------------------------------------------------------
    bool World::EditorDrawPropertyBool(const char* pLabel, bool& value)
    {
        ImGui::TableNextRow();
        ImGui::PushID(pLabel);
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(pLabel);
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        bool wasChanged = ImGui::Checkbox("##Editor", &value);
        ImGui::PopID();

        return wasChanged;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Draw an editable Transform in the Inspector. 
    //----------------------------------------------------------------------------------------------------
    bool World::EditorDrawPropertyTransform(const char* pLabel, Vec3& location, Rotation& rotation, Vec3& scale)
    {
        ImGui::SeparatorText(pLabel);
        bool wasChanged = false;

        if (ImGui::BeginTable("##EntityTransform", 2))
        {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 2.0f); // Default twice larger
            
            wasChanged |= EditorDrawPropertyVec3("Location", location);
            wasChanged |= EditorDrawPropertyRotation("Rotation", rotation);
            wasChanged |= EditorDrawPropertyVec3("Scale", scale);
            ImGui::EndTable();
        }
        
        return wasChanged;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Draw an editable LinearColor in the Inspector. 
    //----------------------------------------------------------------------------------------------------
    bool World::EditorDrawPropertyLinearColor(const char* pLabel, LinearColor& value)
    {
        ImGui::TableNextRow();
        ImGui::PushID(pLabel);
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(pLabel);
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        
        static constexpr ImGuiColorEditFlags kFlags = ImGuiColorEditFlags_Float;
        const bool wasChanged = ImGui::ColorEdit4(pLabel, reinterpret_cast<float*>(&value), kFlags);

        ImGui::PopID();
        return wasChanged;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Create the default render resources. 
    //----------------------------------------------------------------------------------------------------
    void World::CreateRenderResources()
    {
        // Camera Uniforms:
        m_cameraUniformBuffer = Renderer::CreateUniformBuffer(sizeof(SceneCameraUniforms));
        m_cameraUniforms = Renderer::CreateUniformForBuffer(SceneCameraUniforms::kBinding, m_cameraUniformBuffer, sizeof(SceneCameraUniforms));
        
         // Create Geometry3D pipelines:
        nes::GraphicsPipelineConfig pipelineConfig =
        {
            .m_vertexBindings =
            {
                vk::VertexInputBindingDescription()
                    .setBinding(0)
                    .setInputRate(vk::VertexInputRate::eVertex)
                    .setStride(sizeof(nes::Vec3)),
            },

            .m_vertexAttributes =
            {
                vk::VertexInputAttributeDescription()
                    .setLocation(0)
                    .setBinding(0)
                    .setFormat(vk::Format::eR32G32B32Sfloat)
                    .setOffset(0),
            },

            .m_shaderPushConstants =
            {
                vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(GeometryPushConstants)),
            },

            .m_shaderUniforms =
            {
                    m_cameraUniforms,
            },
            
            .m_shaderStages =
            {
                vk::PipelineShaderStageCreateInfo()
                    .setStage(vk::ShaderStageFlagBits::eVertex)
                    .setPName("main")
                    .setModule(Renderer::GetShader("Geometry3D.vert")),
                vk::PipelineShaderStageCreateInfo()
                    .setStage(vk::ShaderStageFlagBits::eFragment)
                    .setPName("main")
                    .setModule(Renderer::GetShader("Geometry3D.frag")),
            },

            .m_colorBlendStates =
            {
                vk::PipelineColorBlendAttachmentState()
                    .setBlendEnable(true)
                    .setColorBlendOp(vk::BlendOp::eAdd)
                    .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                    .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                    .setAlphaBlendOp(vk::BlendOp::eAdd)
                    .setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
                    .setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                    .setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
            },
        };

        // Fill
        pipelineConfig.m_polygonMode = vk::PolygonMode::eFill;
        pipelineConfig.m_cullMode = vk::CullModeFlagBits::eBack;
        pipelineConfig.m_frontFace = vk::FrontFace::eCounterClockwise;
        pipelineConfig.m_depthTestEnable = true;
        pipelineConfig.m_depthCompareOp = vk::CompareOp::eLess;
        pipelineConfig.m_depthWriteEnable = true;
        m_defaultMeshPipelines.emplace_back(Renderer::CreatePipeline(pipelineConfig));
        
        // Wireframe
        pipelineConfig.m_polygonMode = vk::PolygonMode::eLine;
        pipelineConfig.m_cullMode = vk::CullModeFlagBits::eNone;
        pipelineConfig.m_frontFace = vk::FrontFace::eCounterClockwise;
        pipelineConfig.m_depthTestEnable = false;
        pipelineConfig.m_depthCompareOp = vk::CompareOp::eNever;
        pipelineConfig.m_depthWriteEnable = false;
        m_defaultMeshPipelines.emplace_back(Renderer::CreatePipeline(pipelineConfig));
        
        // Create the Grid Pipeline
        nes::GraphicsPipelineConfig gridPipelineConfig =
        {
            .m_shaderUniforms =
            {
                m_cameraUniforms,
            },
            
            .m_shaderStages =
            {
                    vk::PipelineShaderStageCreateInfo()
                        .setStage(vk::ShaderStageFlagBits::eVertex)
                        .setPName("main")
                        .setModule(Renderer::GetShader("Grid.vert")),
                    vk::PipelineShaderStageCreateInfo()
                        .setStage(vk::ShaderStageFlagBits::eFragment)
                        .setPName("main")
                        .setModule(Renderer::GetShader("Grid.frag")),
                },

            .m_polygonMode = vk::PolygonMode::eFill,
            .m_cullMode = vk::CullModeFlagBits::eNone,
            .m_frontFace = vk::FrontFace::eCounterClockwise,
            .m_depthTestEnable = true,
            .m_depthCompareOp = vk::CompareOp::eLess,
            .m_depthWriteEnable = false,
            .m_colorBlendStates =
                {
                vk::PipelineColorBlendAttachmentState()
                    .setBlendEnable(true)
                    .setColorBlendOp(vk::BlendOp::eAdd)
                    .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                    .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrc1Alpha)
                    .setAlphaBlendOp(vk::BlendOp::eAdd)
                    .setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
                    .setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                    .setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
            },
        };
        m_gridPipeline = Renderer::CreatePipeline(gridPipelineConfig);

        // Create the Skybox Assets
        auto& context = Renderer::GetContext();
        
        m_skyboxCubeSampler = context.GetDevice().createSampler(
            vk::SamplerCreateInfo()
                .setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
                .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
                .setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
                .setMinFilter(vk::Filter::eLinear)
                .setMagFilter(vk::Filter::eLinear));

        
        constexpr const char* kSkyboxPaths[] =
        {
            "miramar_ft.png", // Front
            "miramar_bk.png", // Back
            "miramar_up.png", // Up
            "miramar_dn.png", // Down
            "miramar_rt.png", // Right
            "miramar_lf.png", // Left
        };

        std::vector<uint8_t> cubeMapBytes{};
        int width = 1024;
        int height = 1024;
        std::array<void*, 6> imagePointers{};
        int i = 0;
        
        for (const auto& path : kSkyboxPaths)
        {
            std::string fullPath = NES_CONTENT_DIR;
            fullPath += path;
            
            int channels;
            stbi_uc* pBytes = stbi_load(fullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

            imagePointers[i] = pBytes;
            
            const uint8_t* pStart = static_cast<const uint8_t*>(pBytes);
            const uint8_t* pEnd = &pStart[static_cast<uint32_t>(width * height * STBI_rgb_alpha)];
            cubeMapBytes.insert(cubeMapBytes.end(), pStart, pEnd);
            ++i;
        }

        std::tie(m_skyboxCubeImage, m_skyboxCubeImageView) = context.CreateCubemapImageAndView(
            {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}
            , vk::Format::eR8G8B8A8Unorm // TODO: Check how Hazel handles this.
            , cubeMapBytes.data(), cubeMapBytes.size());

        m_skyboxUniforms = context.CreateUniformForImage(3, m_skyboxCubeImageView, m_skyboxCubeSampler);

        // Free the image data.
        for (auto* image : imagePointers)
        {
            stbi_image_free(image);
        }
        
        // Skybox Pipeline
        nes::GraphicsPipelineConfig skyboxPipelineConfig =
        {
            .m_vertexBindings =
            {
                vk::VertexInputBindingDescription()
                    .setBinding(0)
                    .setInputRate(vk::VertexInputRate::eVertex)
                    .setStride(sizeof(nes::Vec3)),
            },

            .m_vertexAttributes =
            {
                vk::VertexInputAttributeDescription()
                    .setLocation(0)
                    .setBinding(0)
                    .setFormat(vk::Format::eR32G32B32Sfloat)
                    .setOffset(0),
            },
            
            .m_shaderUniforms =
            {
                m_cameraUniforms,
                m_skyboxUniforms,
            },
            
            .m_shaderStages =
            {
                vk::PipelineShaderStageCreateInfo()
                    .setStage(vk::ShaderStageFlagBits::eVertex)
                    .setPName("main")
                    .setModule(Renderer::GetShader("Skybox.vert")),
                vk::PipelineShaderStageCreateInfo()
                    .setStage(vk::ShaderStageFlagBits::eFragment)
                    .setPName("main")
                    .setModule(Renderer::GetShader("Skybox.frag")),
            },

        .m_polygonMode = vk::PolygonMode::eFill,
        };
        m_skyboxPipeline = Renderer::CreatePipeline(skyboxPipelineConfig);
        

        // Create a default Cube Mesh.
        static Vec3 vertices[] =
        {
            Vec3(-0.5,  0.5, -0.5),
            Vec3(0.5,  0.5, -0.5),
            Vec3(0.5,  -0.5, -0.5),
            Vec3(-0.5,  -0.5, -0.5),
            Vec3(-0.5,  0.5, 0.5),
            Vec3(0.5,  0.5, 0.5),
            Vec3(0.5,  -0.5, 0.5),
            Vec3(-0.5,  -0.5, 0.5),
        };

        static constexpr uint32_t indices[] =
        {
            0, 3, 2, 0, 2, 1, // Front
            4, 5, 7, 5, 6, 7, // Rear
            1, 2, 6, 5, 1, 6, // Right
            0, 4, 7, 0, 7, 3, // Left
            5, 4, 0, 5, 0, 1, // Top
            7, 6, 2, 7, 2, 3, // Bottom
        };
        
        m_meshAssets.emplace_back(Mesh::Create(
            vertices
            , sizeof(Vec3)
            , _countof(vertices)
            , indices
            , sizeof(uint32_t)
            , _countof(indices)));
        
        // Create a default Material.
        auto pMaterial = std::make_shared<Material>();
        pMaterial->m_baseColor = LinearColor::White();
        m_materialAssets.emplace_back(pMaterial);
    }

    void World::FreeRenderResources()
    {
        m_materialAssets.clear();
        for (auto& pMesh : m_meshAssets)
        {
            Mesh::Free(*pMesh);
            pMesh = nullptr;
        }
        
        auto& context = Renderer::GetContext();
        context.DestroyImageAndView(m_skyboxCubeImage, m_skyboxCubeImageView);
        context.GetDevice().destroySampler(m_skyboxCubeSampler);
        
        for (auto& pPipeline : m_defaultMeshPipelines)
        {
            Renderer::DestroyPipeline(pPipeline);
        }
        Renderer::DestroyPipeline(m_gridPipeline);
        Renderer::DestroyPipeline(m_skyboxPipeline);

        Renderer::DestroyBuffer(m_cameraUniformBuffer);
        Renderer::DestroyUniform(m_cameraUniforms);
        Renderer::DestroyUniform(m_skyboxUniforms);
    }

    void World::RenderSkybox()
    {
        Renderer::BindDescriptorSets(m_skyboxPipeline, vk::PipelineBindPoint::eGraphics, { m_cameraUniforms, m_skyboxUniforms });
        Renderer::BindGraphicsPipeline(m_skyboxPipeline);
        Renderer::DrawIndexed(m_meshAssets[0]->GetVertexBuffer(), m_meshAssets[0]->GetIndexBuffer(), m_meshAssets[0]->GetIndexCount());
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      This would only be in the "Editor version" of the World.
    //		
    ///		@brief : TEMPORARY: Renders a grid plane in the World. 
    //----------------------------------------------------------------------------------------------------
    void World::RenderGrid()
    {
        // static GeometryPushConstants pushConstant
        // {
        //     .m_objectMatrix = Mat44::Identity(),
        //     .m_baseColor = LinearColor::White()
        // };
        
        //Renderer::PushShaderConstant(m_gridPipeline, vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(GeometryPushConstants), &pushConstant);
        Renderer::BindDescriptorSets(m_gridPipeline, vk::PipelineBindPoint::eGraphics, { m_cameraUniforms });
        Renderer::BindGraphicsPipeline(m_gridPipeline);
        Renderer::Draw(6, 1, 0, 0);
    }
}
