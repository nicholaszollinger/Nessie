// VkRenderCommandBuffer.h
#pragma once
#include "VulkanCore.h"
#include "Nessie/Graphics/RenderCommandBuffer.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Under development. This is a Vulkan Command Buffer. 
    //----------------------------------------------------------------------------------------------------
    class VulkanRenderCommandBuffer : public RenderCommandBuffer
    {
    public:
        VulkanRenderCommandBuffer(uint32 count = 0, std::string debugName = "");
        virtual ~VulkanRenderCommandBuffer() override;
        
        virtual void                    Begin() override;
        virtual void                    End() override;
        virtual void                    Submit() override;
        
        vk::CommandBuffer               GetCommandBuffer(const uint32 frameIndex) const;
        vk::CommandBuffer               GetActiveCommandBuffer() const { return m_pActiveCommandBuffer; }
    
    private:
        std::vector<vk::CommandBuffer>  m_commandBuffers;
        std::vector<vk::Fence>          m_waitFences;
        std::string                     m_debugName;
        vk::CommandPool                 m_pCommandPool = nullptr;
        vk::CommandBuffer               m_pActiveCommandBuffer = nullptr;
        bool                            m_ownedBySwapchain = false;
    };
}
