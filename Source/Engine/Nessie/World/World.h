// World.h
#pragma once
#include "Entity3D.h"
#include "Scene/EntityLayer.h"
#include "Scene/EntityPool.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : A World manages the 3D space of a Scene. 
    //----------------------------------------------------------------------------------------------------
    class World final : public EntityLayer
    {
        NES_DEFINE_ENTITY_LAYER(World, Entity3D)
        using EntityPool = TEntityPool<Entity3D>;

        EntityPool m_entityPool;
        
    public:
        explicit World(Scene* pScene);
        StrongPtr<Entity3D> CreateEntity(const EntityID& id, const StringID& name);
        
        virtual void DestroyEntity(const LayerHandle& handle) override;
        [[nodiscard]] virtual bool IsValidNode(const LayerHandle& handle) const override;
        
    private:
        virtual bool InitializeLayer() override;
        virtual void OnSceneBegin() override;
        virtual void OnLayerDestroyed() override;
        virtual void Render(const Camera& worldCamera) override;
        virtual void Tick(const double deltaTime) override;
        virtual void OnEvent(Event& event) override;

        // TEMP:
        virtual void EditorRenderEntityHierarchy() override;
        virtual bool LoadLayer(YAML::Node& layerNode) override;
    };

    static_assert(EntityLayerType<World>, "World not correctly setup as an Entity Layer!!!");
}
