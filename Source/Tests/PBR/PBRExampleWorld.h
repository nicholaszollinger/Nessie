// PBRExampleWorld.h
#pragma once
#include "Nessie/World.h"

// [TODO]: Figure out how to fix the forward declarations with StrongPtr.
#include "Nessie/World/ComponentSystems/TransformSystem.h"
#include "Nessie/World/ComponentSystems/FreeCamSystem.h"
#include "ComponentSystems/PBRSceneRenderer.h"
#include "ComponentSystems/DayNightSystem.h"

namespace nes
{
    class CommandBuffer;
    class RenderFrameContext;
    class TransformSystem;
    class FreeCamSystem;
}

namespace pbr
{
    class DayNightSystem;
    class PBRSceneRenderer;
    
    class PBRExampleWorld final : public nes::WorldBase
    {
    public:
        virtual bool    PostInit() override;
        virtual void    OnDestroy() override;
        virtual void    OnEvent(nes::Event& event) override;
        virtual void    Tick(const float deltaTime) override;
        void            OnResize(const uint32 width, const uint32 height);
        void            Render(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context);
        virtual void    ParentEntity(const nes::EntityHandle entity, const nes::EntityHandle parent) override;

    private:
        virtual void    OnNewEntityCreated(const nes::EntityHandle newEntity) override;
        virtual void    AddComponentSystems() override;

    private:
        nes::StrongPtr<nes::TransformSystem>    m_pTransformSystem = nullptr;
        nes::StrongPtr<nes::FreeCamSystem>      m_pFreeCamSystem = nullptr;
        nes::StrongPtr<PBRSceneRenderer>        m_pSceneRenderer = nullptr;
        nes::StrongPtr<DayNightSystem>          m_pDayNightSystem = nullptr;
    };
}
