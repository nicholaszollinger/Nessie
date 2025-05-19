// World.h
#pragma once
#include "Entity3D.h"
#include "PhysicsLayers.h"
#include "Components/MeshComponent.h"
#include "Core/Generic/Color.h"
#include "Core/Jobs/JobSystemThreadPool.h"
#include "Physics/PhysicsScene.h"
#include "Scene/EntityLayer.h"
#include "Scene/EntityPool.h"
#include "Scene/TickGroup.h"

namespace nes
{
    struct Material;

    enum class WorldRenderMode : uint8_t
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

            Mat4 m_projectionMatrix = Mat4::Identity();
            Mat4 m_viewMatrix = Mat4::Identity();
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
            Mat4 m_objectMatrix = Mat4::Identity();
            // Note: This is only here because the Material is trivial for now.
            // a full material might not make sense as a push constant and should be
            // moved to a Uniform Buffer.
            LinearColor m_baseColor = LinearColor::White();
        };

    private:
        using EntityPool = TEntityPool<Entity3D>;
        EntityPool m_entityPool;
        std::vector<EventHandler> m_eventHandlers{};
        
        // Tick Groups
        TickGroup m_prePhysicsTickGroup;
        TickGroup m_physicsTickGroup;
        TickGroup m_postPhysicsTickGroup;
        TickGroup m_lateTickGroup;

        // Physics
        PhysicsScene*       m_pPhysicsScene     = nullptr;
        PhysicsSettings     m_physicsSettings;
        PhysicsTick         m_physicsTick;
        StackAllocator*     m_pPhysicsAllocator = nullptr;
        JobSystem*          m_pJobSystem        = nullptr;
        BroadPhaseLayerInterfaceTest m_broadPhaseLayerInterface{};
        CollisionVsBroadPhaseLayerFilterTest m_layerVsBroadPhaseFilter{};
        CollisionLayerPairFilterTest m_layerPairFilter{};

        // [TEMP]: 
        BodyID m_testID;

        // Render Resources:
        std::vector<MeshComponent*> m_transparentMeshes;
        std::vector<MeshComponent*> m_opaqueMeshes;
        std::vector<std::shared_ptr<nes::RendererContext::GraphicsPipeline>> m_defaultMeshPipelines{};
        std::shared_ptr<nes::RendererContext::GraphicsPipeline> m_gridPipeline = nullptr;
        std::shared_ptr<nes::RendererContext::GraphicsPipeline> m_skyboxPipeline = nullptr;
        std::vector<std::shared_ptr<Mesh>> m_meshAssets;
        std::vector<std::shared_ptr<Material>> m_materialAssets;
        
        RendererContext::ShaderUniform m_cameraUniforms;
        vk::Buffer m_cameraUniformBuffer;
        
        RendererContext::ShaderUniform m_skyboxUniforms;
        vk::Image m_skyboxCubeImage;
        vk::ImageView m_skyboxCubeImageView;
        vk::Sampler m_skyboxCubeSampler;
        
        WorldRenderMode m_currentRenderMode = WorldRenderMode::Fill;

        // TEMP Editor Data:
        Entity3D* m_pSelectedEntity = nullptr;
        StrongPtr<Entity3DComponent> m_pSelectedComponent = nullptr;
        
    public:
        explicit World(Scene* pScene);
        StrongPtr<Entity3D> CreateEntity(const EntityID& id, const StringID& name);

        void RegisterTickToWorldTickGroup(TickFunction* pFunction, const TickStage stage);
        [[nodiscard]] TickGroup* GetTickGroup(const TickStage stage);
        
        virtual void DestroyEntity(const LayerHandle& handle) override;
        [[nodiscard]] virtual bool IsValidNode(const LayerHandle& handle) const override;

        void RegisterEventHandler(const EventHandler& handler);
        void RegisterMesh(MeshComponent* pMesh);
        [[nodiscard]] std::shared_ptr<nes::RendererContext::GraphicsPipeline> GetDefaultMeshRenderPipeline() const;
        
    private:
        virtual bool InitializeLayer() override;
        virtual void OnSceneBegin() override;
        virtual void OnLayerDestroyed() override;
        virtual void PreRender(const Camera& sceneCamera) override;
        virtual void Render(const Camera& worldCamera) override;
        virtual void OnEvent(Event& event) override;
        virtual bool LoadLayer(YAML::Node& layerNode) override;
        virtual void OnPostTick() override;

        // TEMP:
        virtual void EditorRenderEntityHierarchy() override;

        // TEMP
        // [TODO]: Make a Property Drawer class, that updates an internal property value.
        void EditorDrawInspector();
        void EditorDrawEntityNode(Entity3D& entity);
        void EditorDrawComponentNode(StrongPtr<Entity3DComponent>& component);
        void EditorDrawComponentProperties(StrongPtr<Entity3DComponent>& component);
        bool EditorDrawPropertyVector3(const char* pLabel, Vector3& value);
        bool EditorDrawPropertyRotation(const char* pLabel, Rotation& value);
        bool EditorDrawPropertyFloat(const char* pLabel, float& value);
        bool EditorDrawPropertyBool(const char* pLabel, bool& value);
        bool EditorDrawPropertyTransform(const char* pLabel, Vector3& location, Rotation& rotation, Vector3& scale);
        bool EditorDrawPropertyLinearColor(const char* pLabel, LinearColor& value);

        void CreateRenderResources();
        void FreeRenderResources();
        void RenderSkybox();
        void RenderGrid();
    };

    static_assert(EntityLayerType<World>, "World not correctly setup as an Entity Layer!!!");
}
