// WorldRenderer.h
#pragma once
#include "ComponentSystem.h"
#include "WorldCamera.h"
#include "Components/IDComponent.h"
#include "Nessie/Graphics/Renderer.h"

namespace nes
{
    class WorldRenderer : public ComponentSystem
    {
    public:
        WorldRenderer(WorldBase& world) : ComponentSystem(world) {}

        void                    RenderWorld(CommandBuffer& commandBuffer, const RenderFrameContext& context);
        virtual void            RenderWorldWithCamera(const WorldCamera& worldCamera, CommandBuffer& commandBuffer, const RenderFrameContext& context) = 0;
        virtual RenderTarget*   GetFinalColorTarget() = 0;
        virtual RenderTarget*   GetFinalDepthTarget() = 0;
        virtual WorldCamera     GetActiveCamera() = 0;
        virtual void            SetActiveCameraEntity(const nes::EntityID& id) = 0;
        virtual void            OnViewportResize(const uint32 width, const uint32 height) = 0;
    };
}
