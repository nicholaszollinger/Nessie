// CommandPool.cpp
#include "CommandPool.h"

#include "CommandBuffer.h"
#include "DeviceQueue.h"
#include "RenderDevice.h"
#include "Renderer.h"

namespace nes
{
    CommandPool::~CommandPool()
    {
        Renderer::SubmitResourceFree([commandPool = std::move(m_pool)]() mutable
        {
            commandPool = nullptr;
        });
    }

    EGraphicsResult CommandPool::Init(const DeviceQueue& queue)
    {
        NES_ASSERT(m_pool == nullptr);
        
        // Default to the Reset Command Buffer Bit.
        vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo()
            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
            .setQueueFamilyIndex(queue.GetFamilyIndex());

        m_pool = vk::raii::CommandPool(m_device, poolInfo);
        m_queueType = queue.GetQueueType();

        return EGraphicsResult::Success;
    }

    void CommandPool::Reset()
    {
        std::lock_guard lock(m_mutex);
        m_pool.reset();
    }

    void CommandPool::SetDebugName(const std::string& name)
    {
        vk::CommandPool pool = *m_pool;
        m_device.SetDebugNameToTrivialObject(pool, name);
    }

    EGraphicsResult CommandPool::CreateCommandBuffer(CommandBuffer*& pOutCommandBuffer)
    {
        NES_GRAPHICS_ASSERT(m_device, pOutCommandBuffer == nullptr);
        
        std::lock_guard lock(m_mutex);

        vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(m_pool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);

        auto commandBuffer = std::move(vk::raii::CommandBuffers(m_device, allocInfo).front());

        pOutCommandBuffer = Allocate<CommandBuffer>(m_device.GetAllocationCallbacks(), m_device);
        pOutCommandBuffer->Init(this, std::move(commandBuffer));
        
        return EGraphicsResult::Success;
    }
}
