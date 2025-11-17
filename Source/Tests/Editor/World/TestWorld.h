// TestWorld.h
#pragma once
#include "Nessie/World.h"
#include "Nessie/World/ComponentSystems/TransformSystem.h"
#include "World/SimpleRenderer.h"

namespace nes
{
    class CommandBuffer;
    class RenderFrameContext;
    class TransformSystem;
    class FreeCamSystem;
}

class TestWorld final : public nes::World
{
public:
    virtual void    OnEvent(nes::Event&) override {}
    virtual void    Tick(const float deltaTime) override;
    void            Render(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context) const;
    virtual void    ParentEntity(const nes::EntityHandle entity, const nes::EntityHandle parent) override;
    virtual nes::StrongPtr<nes::WorldRenderer> GetRenderer() const override { return m_pSimpleRenderer; }

protected:
    virtual void    AddComponentSystems() override;
    virtual bool    PostInit() override;
    virtual void    OnNewEntityCreated(nes::EntityRegistry& registry, const nes::EntityHandle newEntity) override;
    virtual void    OnDestroy() override;

protected:
    nes::StrongPtr<nes::TransformSystem>    m_pTransformSystem = nullptr;
    nes::StrongPtr<SimpleRenderer>          m_pSimpleRenderer = nullptr;
};
