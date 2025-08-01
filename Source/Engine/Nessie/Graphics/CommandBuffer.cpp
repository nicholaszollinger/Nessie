// CommandBuffer.cpp
#include "CommandBuffer.h"

#include "RenderDevice.h"
#include "CommandPool.h"

namespace nes
{
    CommandBuffer::~CommandBuffer()
    {
        if (m_pCommandPool != nullptr)
        {
            vkFreeCommandBuffers(m_device, *m_pCommandPool, 1, &m_handle);
        }
    }

    EGraphicsResult CommandBuffer::Init(CommandPool* pPool, VkCommandBuffer handle)
    {
        NES_GRAPHICS_ASSERT(m_device, pPool != nullptr);
        NES_GRAPHICS_ASSERT(m_device, handle != nullptr);
        
        m_pCommandPool = pPool;
        m_handle = handle;

        return EGraphicsResult::Success;
    }

    void CommandBuffer::SetDebugName(const char* name)
    {
        m_device.SetDebugNameToTrivialObject(m_handle, name);
    }

    EGraphicsResult CommandBuffer::Begin()
    {
        static constexpr VkCommandBufferBeginInfo kBeginInfo
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr,
        };
        NES_VK_FAIL_RETURN(m_device, vkBeginCommandBuffer(m_handle, &kBeginInfo));

        // [TODO]: 
        // Reset temp bindings.
        
        return EGraphicsResult::Success;
    }

    EGraphicsResult CommandBuffer::End()
    {
        NES_VK_FAIL_RETURN(m_device, vkEndCommandBuffer(m_handle));
        return EGraphicsResult::Success;
    }

    EGraphicsResult CommandBuffer::TransitionImageLayout(const VkImage image, ImageMemoryBarrierDesc& barrierDesc) const
    {
        VkImageMemoryBarrier2 barrier{};
        const EGraphicsResult result = barrierDesc.CreateBarrier(image, barrier);
        if (result != EGraphicsResult::Success)
            return result;

        const VkDependencyInfo depInfo
        {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = nullptr,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        };
        
        vkCmdPipelineBarrier2(m_handle, &depInfo);
        return EGraphicsResult::Success;
    }
}
