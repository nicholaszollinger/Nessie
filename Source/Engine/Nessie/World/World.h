// World.h
#pragma once
#include "Actor.h"
#include "Scene/SceneLayer.h"
#include "Scene/SceneNodePool.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A World manages the 3D space of a Scene. 
    //----------------------------------------------------------------------------------------------------
    class World final : public SceneLayer
    {
        NES_DEFINE_NODE_LAYER(World, Actor)
        using ActorPool = TSceneNodePool<Actor>;

        ActorPool m_actorPool;
        
    public:
        explicit World(Scene* pScene);
        StrongPtr<Actor> CreateActor(const NodeID& id, const StringID& name);
        
        virtual void DestroyNode(const LayerHandle& handle) override;
        [[nodiscard]] virtual bool IsValidNode(const LayerHandle& handle) const override;
        
    private:
        virtual bool InitializeLayer() override;
        virtual void OnSceneBegin() override;
        virtual void OnLayerDestroyed() override;
        virtual void Render(const Camera& worldCamera) override;
        virtual void Tick(const double deltaTime) override;
        virtual void OnEvent(Event& event) override;

        // TEMP:
        virtual void EditorRenderNodeHierarchy() override;
        virtual bool LoadLayer(YAML::Node& layerNode) override;
        void LoadWorldComponentData(WorldComponent& worldComponent, YAML::Node& componentNode) const;
    };

    static_assert(NodeLayerType<World>, "World not correctly setup as Node Layer!!!");
}
