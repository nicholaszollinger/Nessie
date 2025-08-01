// CommandPool.cpp
#include "CommandPool.h"

#include "CommandBuffer.h"
#include "DeviceQueue.h"
#include "RenderDevice.h"

namespace nes
{
    CommandPool::~CommandPool()
    {
        if (m_handle)
        {
            vkDestroyCommandPool(m_device, m_handle, m_device.GetVulkanAllocationCallbacks());
        }
    }

    EGraphicsResult CommandPool::Init(const DeviceQueue& queue)
    {
        // Default to the Reset Command Buffer Bit.
        const VkCommandPoolCreateInfo poolInfo
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queue.GetFamilyIndex(),
        };
        NES_VK_FAIL_RETURN(m_device, vkCreateCommandPool(m_device, &poolInfo, m_device.GetVulkanAllocationCallbacks(), &m_handle));

        m_queueType = queue.GetQueueType();   

        return EGraphicsResult::Success;
    }

    void CommandPool::Reset()
    {
        std::lock_guard lock(m_mutex);
        NES_VK_FAIL_RETURN_VOID(m_device, vkResetCommandPool(m_device, m_handle, VkCommandPoolResetFlags{}));
    }

    void CommandPool::SetDebugName(const char* name)
    {
        m_device.SetDebugNameToTrivialObject(m_handle, name);
    }

    EGraphicsResult CommandPool::CreateCommandBuffer(CommandBuffer*& pOutCommandBuffer)
    {
        NES_GRAPHICS_ASSERT(m_device, pOutCommandBuffer == nullptr);
        
        std::lock_guard lock(m_mutex);

        const VkCommandBufferAllocateInfo info
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_handle,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        VkCommandBuffer commandBufferHandle = VK_NULL_HANDLE;
        NES_VK_FAIL_RETURN(m_device, vkAllocateCommandBuffers(m_device, &info, &commandBufferHandle));

        pOutCommandBuffer = Allocate<CommandBuffer>(m_device.GetAllocationCallbacks(), m_device);
        pOutCommandBuffer->Init(this, commandBufferHandle);
        
        return EGraphicsResult::Success;
    }
}
