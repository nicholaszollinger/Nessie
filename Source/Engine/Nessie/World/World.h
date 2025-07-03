// World.h
#pragma once
#include "Entity3D.h"
#include "PhysicsLayers.h"
#include "Components/MeshComponent.h"
#include "Nessie/Core/Color.h"
#include "Nessie/Core/Jobs/JobSystemThreadPool.h"
#include "Nessie/Physics/PhysicsScene.h"
#include "Nessie/Scene/EntityLayer.h"
#include "Nessie/Scene/EntityPool.h"
#include "Nessie/Scene/TickGroup.h"
#include "Nessie/Graphics/Renderer.h"

// [TEMP]: 
#include "Nessie/Physics/Body/BodyActivationListener.h"

namespace nes
{
    struct Material;

    NES_DEFINE_LOG_TAG(kWorldLogTag, "World", Info);

    class BodyActivateListenerTest final : public BodyActivationListener
    {
    public:
        virtual void OnBodyActivated([[maybe_unused]] const BodyID& bodyID, [[maybe_unused]] uint64_t bodyUserData) override
        {
            NES_LOG("Body {} activated: ", bodyID.GetIndex());
        }

        virtual void OnBodyDeactivated([[maybe_unused]] const BodyID& bodyID, [[maybe_unused]] uint64_t bodyUserData) override
        {
            NES_LOG("Body {} deactivated: ", bodyID.GetIndex());
        }
    };

    enum class EWorldRenderMode : uint8_t
    {
        Fill = 0,
        Wireframe,
        Num, // Only the above two are supported atm.
        Point,
        FillRectangleNV,
    };
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A World manages the 3D space of a Scene. 
    //----------------------------------------------------------------------------------------------------
    class World final : public EntityLayer
    {
        NES_DEFINE_ENTITY_LAYER(World, Entity3D)
        
        struct SceneCameraUniforms
        {
            static constexpr uint32_t kBinding = 0;

            Mat44 m_projectionMatrix = Mat44::Identity();
            Mat44 m_viewMatrix = Mat44::Identity();
        };

        struct PhysicsTick final : public TickFunction
        {
            PhysicsScene*   m_pPhysicsScene     = nullptr;
            StackAllocator* m_pAllocator        = nullptr;
            JobSystem*      m_pJobSystem        = nullptr;
            int             m_collisionSteps    = 1;
        
            virtual void ExecuteTick(const TickDeltaTime& deltaTime) override
            {
                m_pPhysicsScene->Update(deltaTime.m_deltaTime, m_collisionSteps, m_pAllocator, m_pJobSystem);
            }
        };
        
    public:
        struct GeometryPushConstants
        {
            Mat44 m_objectMatrix = Mat44::Identity();
            // Note: This is only here because the Material is trivial for now.
            // a full material might not make sense as a push constant and should be
            // moved to a Uniform Buffer.
            LinearColor m_baseColor = LinearColor::White();
        };
        
    public:
        explicit            World(Scene* pScene);
        StrongPtr<Entity3D> CreateEntity(const EntityID& id, const StringID& name);

        void                RegisterTickToWorldTickGroup(TickFunction* pFunction, const ETickStage stage);
        TickGroup*          GetTickGroup(const ETickStage stage);
        
        virtual void        DestroyEntity(const LayerHandle& handle) override;
        virtual bool        IsValidNode(const LayerHandle& handle) const override;

        void                RegisterEventHandler(const EventHandler& handler);
        void                RegisterMesh(MeshComponent* pMesh);
        GraphicsPipelinePtr GetDefaultMeshRenderPipeline() const;
        
    private:
        virtual bool        InitializeLayer() override;
        virtual void        OnSceneBegin() override;
        virtual void        OnLayerDestroyed() override;
        virtual void        PreRender(const Camera& sceneCamera) override;
        virtual void        Render(const Camera& worldCamera) override;
        virtual void        OnEvent(Event& event) override;
        virtual bool        LoadLayer(YAML::Node& layerNode) override;
        virtual void        OnPostTick() override;
        
        virtual void        EditorRenderEntityHierarchy() override;
        void                EditorDrawInspector();
        void                EditorDrawEntityNode(Entity3D& entity);
        void                EditorDrawComponentNode(StrongPtr<Entity3DComponent>& component);
        void                EditorDrawComponentProperties(StrongPtr<Entity3DComponent>& component);
        bool                EditorDrawPropertyVec3(const char* pLabel, Vec3& value);
        bool                EditorDrawPropertyRotation(const char* pLabel, Rotation& value);
        bool                EditorDrawPropertyFloat(const char* pLabel, float& value);
        bool                EditorDrawPropertyBool(const char* pLabel, bool& value);
        bool                EditorDrawPropertyTransform(const char* pLabel, Vec3& location, Rotation& rotation, Vec3& scale);
        bool                EditorDrawPropertyLinearColor(const char* pLabel, LinearColor& value);

        void                CreateRenderResources();
        void                FreeRenderResources();
        void                RenderSkybox();
        void                RenderGrid();

    private:
        using EntityPool = TEntityPool<Entity3D>;
        EntityPool                              m_entityPool;
        std::vector<EventHandler>               m_eventHandlers{};
        
        // Tick Groups
        TickGroup                               m_prePhysicsTickGroup;
        TickGroup                               m_physicsTickGroup;
        TickGroup                               m_postPhysicsTickGroup;
        TickGroup                               m_lateTickGroup;

        // Physics
        PhysicsScene*                           m_pPhysicsScene     = nullptr;
        PhysicsSettings                         m_physicsSettings;
        PhysicsTick                             m_physicsTick;
        StackAllocator*                         m_pPhysicsAllocator = nullptr;
        JobSystem*                              m_pJobSystem        = nullptr;
        BroadPhaseLayerInterfaceTest            m_broadPhaseLayerInterface{};
        CollisionVsBroadPhaseLayerFilterTest    m_layerVsBroadPhaseFilter{};
        CollisionLayerPairFilterTest            m_layerPairFilter{};
        BodyActivateListenerTest                m_bodyActivationListener{};

        // [TEMP]: 
        BodyID                                  m_testID;

        // Render Resources:
        std::vector<MeshComponent*>             m_transparentMeshes;
        std::vector<MeshComponent*>             m_opaqueMeshes;
        std::vector<GraphicsPipelinePtr>        m_defaultMeshPipelines{};
        GraphicsPipelinePtr                     m_gridPipeline = nullptr;
        GraphicsPipelinePtr                     m_skyboxPipeline = nullptr;
        std::vector<std::shared_ptr<Mesh>>      m_meshAssets;
        std::vector<std::shared_ptr<Material>>  m_materialAssets;
        
        RendererContext::ShaderUniform          m_cameraUniforms;
        vk::Buffer                              m_cameraUniformBuffer;
        
        RendererContext::ShaderUniform          m_skyboxUniforms;
        vk::Image                               m_skyboxCubeImage;
        vk::ImageView                           m_skyboxCubeImageView;
        vk::Sampler                             m_skyboxCubeSampler;
        
        EWorldRenderMode                        m_currentRenderMode = EWorldRenderMode::Fill;

        // TEMP Editor Data:
        Entity3D*                               m_pSelectedEntity = nullptr;
        StrongPtr<Entity3DComponent>            m_pSelectedComponent = nullptr;
    };

    static_assert(EntityLayerType<World>, "World not correctly setup as an Entity Layer!!!");
}
