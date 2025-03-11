// AbstractLayer.h
#pragma once
#include "EntityLayer.h"

namespace nes
{
    class AbstractLayer final : public EntityLayer
    {
        NES_DEFINE_ENTITY_LAYER(AbstractLayer, Entity, EntityDomain::Abstract)
        EntityPool<Entity> m_entityPool;
        
    public:
        explicit AbstractLayer(Scene* pScene);
        virtual void DestroyEntity(const LayerHandle& handle) override;
        [[nodiscard]] virtual bool IsValidEntity(const LayerHandle& handle) const override;

    private:
        virtual bool InitializeLayer() override;
        virtual void OnSceneBegin() override;
        virtual void OnEvent([[maybe_unused]] Event& event) override {}
        virtual void Render([[maybe_unused]] const Camera& sceneCamera) override {}
        virtual void Tick([[maybe_unused]] const double deltaTime) override {}
        virtual void DestroyLayer() override;
        virtual bool LoadLayer(YAML::Node& layerNode) override;
        virtual void RenderEditorEntityHierarchy() override;

    public:
    };
}
