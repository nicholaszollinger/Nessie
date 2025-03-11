// World.h
#pragma once
#include "Actor.h"
#include "Scene/EntityLayer.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A World manages the 3D space of a Scene. 
    //----------------------------------------------------------------------------------------------------
    class World final : public EntityLayer
    {
        NES_DEFINE_ENTITY_LAYER(World, Actor, EntityDomain::Physical3D)

        EntityPool<Actor> m_actorPool;
        
    public:
        explicit World(Scene* pScene);
        StrongPtr<Actor> CreateActor(const EntityID id, const StringID& name);

    public:
        virtual void DestroyEntity(const LayerHandle& handle) override;
        [[nodiscard]] virtual bool IsValidEntity(const LayerHandle& handle) const override;
        
    private:
        virtual bool InitializeLayer() override;
        virtual void OnSceneBegin() override;
        virtual void DestroyLayer() override;
        virtual void RenderEditorEntityHierarchy() override;
        virtual void Render(const Camera& worldCamera) override;
        virtual void Tick(const double deltaTime) override;
        virtual void OnEvent(Event& event) override;

        // TEMP:
        virtual bool LoadLayer(YAML::Node& layerNode) override;
        void LoadWorldComponentData(WorldComponent& worldComponent, YAML::Node& componentNode) const;
    };

    static_assert(EntityLayerType<World>, "World not correctly setup as Entity Layer!!!");
}
