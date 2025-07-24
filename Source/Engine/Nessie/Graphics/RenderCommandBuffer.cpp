//// RenderCommandBuffer.cpp
//#include "RenderCommandBuffer.h"
//
//#if defined(NES_RENDER_API_VULKAN)
//#include "Nessie/Graphics/Vulkan/VulkanRenderCommandBuffer.h"
//#endif
//
//namespace nes
//{
//    StrongPtr<RenderCommandBuffer> RenderCommandBuffer::Create(uint32 count, const std::string& debugName)
//    {
//    #if defined(NES_RENDER_API_VULKAN)
//        const StrongPtr<VulkanRenderCommandBuffer> pVulkanCommandBuffer = NES_NEW(VulkanRenderCommandBuffer(count, debugName));
//        return Cast<RenderCommandBuffer>(pVulkanCommandBuffer);
//        //return Cast<RenderCommandBuffer>(Create<Vulkan_RenderCommandBuffer>(count, debugName));
//    #else
//        #error "Missing RenderCommandBuffer Create for current Render API!"
//    #endif
//    }
//
//}