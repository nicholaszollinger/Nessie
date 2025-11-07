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

class TestWorld final : public nes::WorldBase
{
public:
    virtual void    OnEvent(nes::Event&) override {}
    virtual void    Tick(const float deltaTime) override;
    void            Render(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context) const;
    virtual void    ParentEntity(const nes::EntityHandle entity, const nes::EntityHandle parent) override;

protected:
    virtual void    AddComponentSystems() override;
    virtual bool    PostInit() override;
    virtual void    OnDestroy() override;

    nes::StrongPtr<nes::TransformSystem>    m_pTransformSystem = nullptr;
    nes::StrongPtr<SimpleRenderer>          m_pSimpleRenderer = nullptr;
};
