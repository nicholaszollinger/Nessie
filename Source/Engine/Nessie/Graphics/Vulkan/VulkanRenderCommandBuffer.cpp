// Vulkan_RenderCommandBuffer.cpp
#include "VulkanRenderCommandBuffer.h"
#include "VulkanDevice.h"

namespace nes
{
    VulkanRenderCommandBuffer::VulkanRenderCommandBuffer([[maybe_unused]] uint32 count, std::string debugName)
        : m_debugName(std::move(debugName))
    {
        // auto device = VulkanDevice::Get();
        // NES_ASSERT(device != nullptr);
        //
        // if (count == 0)
        // {
        //     // 0 means 1 per frame in flight.
        //     count = Renderer::GetConfig().m_framesInFlight;
        // }
        //
        // NES_ASSERT(count > 0);
        //
        // // Create the command pool object
        // vk::CommandPoolCreateInfo commandPoolInfo = {};
        // commandPoolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        // commandPoolInfo.queueFamilyIndex = context.GetQueueFamilyIndices().m_graphics;
        // m_pCommandPool = device.createCommandPool(commandPoolInfo);
        // NES_ASSERT(m_pCommandPool != nullptr);
        // vulkan::SetDebugObjectName(device, vk::ObjectType::eCommandPool, m_debugName, m_pCommandPool);
        //
        // // Allocate the command buffers
        // vk::CommandBufferAllocateInfo commandBufferAllocateInfo = {};
        // commandBufferAllocateInfo.commandPool = m_pCommandPool;
        // commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
        // commandBufferAllocateInfo.commandBufferCount = count;
        // m_commandBuffers.resize(count);
        // NES_VULKAN_MUST_PASS(device.allocateCommandBuffers(&commandBufferAllocateInfo, m_commandBuffers.data()));
        //
        // for (uint32 i = 0; i < count; ++i)
        // {
        //     vulkan::SetDebugObjectName(device, vk::ObjectType::eCommandBuffer, std::format("{} (frame in flight: {})", m_debugName, i), m_commandBuffers[i]);
        // }
        //
        // // Allocate the wait fences
        // vk::FenceCreateInfo fenceCreateInfo = {};
        // fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
        // m_waitFences.resize(count);
        // for (uint32 i = 0; i < count; ++i)
        // {
        //     NES_VULKAN_MUST_PASS(device.createFence(&fenceCreateInfo, nullptr, &m_waitFences[i]));
        //     vulkan::SetDebugObjectName(device, vk::ObjectType::eFence, std::format("{} (frame in flight: {}) fence", m_debugName, i), m_waitFences[i]);
        // }
    }

    VulkanRenderCommandBuffer::~VulkanRenderCommandBuffer()
    {
        // if (m_ownedBySwapchain)
        //     return;
        //
        // vk::CommandPool pool = m_pCommandPool;
        // Renderer::SubmitResourceFree([pool]()
        // {
        //     auto device = Vulkan_Context::Get().GetDevice();
        //     device.destroyCommandPool(pool);
        // });
    }

    void VulkanRenderCommandBuffer::Begin()
    {
        // StrongPtr<Vulkan_RenderCommandBuffer> instance = this;
        // Renderer::Submit([instance]() mutable
        // {
        //     uint32 commandBufferIndex = Renderer::RT_GetCurrentFrameIndex();
        //
        //     vk::CommandBufferBeginInfo beginInfo = {};
        //     beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        //     beginInfo.pNext = nullptr;
        //
        //     vk::CommandBuffer commandBuffer = nullptr;
        //     if (instance->m_ownedBySwapchain)
        //     {
        //         // [TODO]: 
        //     }
        //     else
        //     {
        //         commandBufferIndex %= instance->m_commandBuffers.size();
        //         commandBuffer = instance->m_commandBuffers[commandBufferIndex];
        //     }
        //
        //     instance->m_pActiveCommandBuffer = commandBuffer;
        //     NES_VULKAN_MUST_PASS(commandBuffer.begin(&beginInfo));
        // });
    }

    void VulkanRenderCommandBuffer::End()
    {
        // StrongPtr<Vulkan_RenderCommandBuffer> instance = this;
        // Renderer::Submit([instance]() mutable
        // {
        //     // [TODO]: Use the following for timestamp queries.
        //     /*
        //     auto& swapchain = Vulkan_Renderer::Get()->GetSwapchain();
        //     uint32 commandBufferIndex = swapchain.GetCurrentFrameIndex();
        //     if (!instance->m_ownedBySwapchain)
        //         commandBufferIndex %= instance->m_commandBuffers.size();
        //     vk::CommandBuffer commandBuffer = instance->m_pActiveCommandBuffer;
        //     commandBuffer.writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe, instance->m_timestampQueryPools[commandBufferIndex], 1);
        //     commandBuffer.endQuery(instance->m_pipelineStatisticsQueryPools[commandBufferIndex], 0);
        //     */
        //     
        //     vk::CommandBuffer commandBuffer = instance->m_pActiveCommandBuffer;
        //     commandBuffer.end();
        //     
        //     instance->m_pActiveCommandBuffer = nullptr;
        // });
    }

    void VulkanRenderCommandBuffer::Submit()
    {
        // if (m_ownedBySwapchain)
        //     return;
        //
        // StrongPtr<Vulkan_RenderCommandBuffer> instance = this;
        // Renderer::Submit([instance]() mutable
        // {
        //     auto& context = Vulkan_Context::Get();
        //     auto device = context.GetDevice();
        //     const uint32 commandBufferIndex = Renderer::RT_GetCurrentFrameIndex() % instance->m_commandBuffers.size();
        //
        //     vk::SubmitInfo submitInfo = {};
        //     submitInfo.commandBufferCount = 1;
        //     submitInfo.pCommandBuffers = &instance->m_commandBuffers[commandBufferIndex];
        //     submitInfo.pNext = nullptr;
        //
        //     NES_VULKAN_MUST_PASS(device.waitForFences(1, &instance->m_waitFences[commandBufferIndex], true, UINT64_MAX));
        //     NES_VULKAN_MUST_PASS(device.resetFences(1, &instance->m_waitFences[commandBufferIndex]));
        //     
        //     context.LockGraphicsQueue();
        //     context.GetGraphicsQueue().submit(submitInfo, instance->m_waitFences[commandBufferIndex]);
        //     context.UnlockGraphicsQueue();
        // });
    }

    vk::CommandBuffer VulkanRenderCommandBuffer::GetCommandBuffer([[maybe_unused]] const uint32 frameIndex) const
    {
        // [TODO]: 
        // NES_ASSERT(frameIndex < m_commandBuffers.size());
        // return m_commandBuffers[frameIndex];
        NES_ASSERT(false);
        return vk::CommandBuffer();
    }
}
