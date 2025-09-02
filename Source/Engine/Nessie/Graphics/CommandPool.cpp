// CommandPool.cpp
#include "CommandPool.h"

#include "CommandBuffer.h"
#include "DeviceQueue.h"
#include "RenderDevice.h"
#include "Renderer.h"

namespace nes
{
    CommandPool::CommandPool(CommandPool&& other) noexcept
        : m_pDevice(other.m_pDevice)
        , m_pool(std::move(other.m_pool))
        , m_queueType(other.m_queueType)
    {
        other.m_pDevice = nullptr;
        other.m_queueType = EQueueType::MaxNum;
    }

    CommandPool& CommandPool::operator=(std::nullptr_t)
    {
        FreePool();
        return *this;
    }

    CommandPool& CommandPool::operator=(CommandPool&& other) noexcept
    {
        if (this != &other)
        {
            FreePool();

            m_pool = std::move(other.m_pool);
            m_pDevice = other.m_pDevice;
            m_queueType = other.m_queueType;

            other.m_pDevice = nullptr;
            other.m_queueType = EQueueType::MaxNum;
        }

        return *this;
    }

    CommandPool::CommandPool(RenderDevice& device, const DeviceQueue& queue, const bool isTransient)
        : m_pDevice(&device)
    {
        // Default to the Reset Command Buffer Bit.
        vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo()
            .setFlags(isTransient? vk::CommandPoolCreateFlagBits::eTransient : vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
            .setQueueFamilyIndex(queue.GetFamilyIndex());

        m_pool = vk::raii::CommandPool(device, poolInfo);
        m_queueType = queue.GetQueueType();
    }
    
    CommandPool::~CommandPool()
    {
        FreePool();
    }

    void CommandPool::Reset()
    {
        std::lock_guard lock(m_mutex);
        m_pool.reset();
    }

    NativeVkObject CommandPool::GetNativeVkObject() const
    {
        return NativeVkObject(*m_pool, vk::ObjectType::eCommandPool);
    }

    void CommandPool::FreePool()
    {
        if (m_pool != nullptr)
        {
            Renderer::SubmitResourceFree([commandPool = std::move(m_pool)]() mutable
            {
                commandPool = nullptr;
            });
        }

        m_pDevice = nullptr;
    }

    void CommandPool::SetDebugName(const std::string& name)
    {
        NES_ASSERT(m_pDevice != nullptr);
        m_pDevice->SetDebugNameVkObject(GetNativeVkObject(), name);
    }

    CommandBuffer CommandPool::CreateCommandBuffer()
    {
        NES_ASSERT(m_pDevice != nullptr);
        
        std::lock_guard lock(m_mutex);

        vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo()
            .setCommandPool(m_pool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1);

        auto commandBuffer = std::move(vk::raii::CommandBuffers(*m_pDevice, allocInfo).front());
        return CommandBuffer(*m_pDevice, *this, std::move(commandBuffer));
    }
}
