// WorldRenderer.cpp
#include "WorldRenderer.h"

namespace nes
{
    void WorldRenderer::RenderWorld(CommandBuffer& commandBuffer, const RenderFrameContext& context)
    {
        WorldCamera camera = GetActiveCamera();
        RenderWorldWithCamera(camera, commandBuffer, context);
    }
}
