// DeviceQueue.cpp
#include "DeviceQueue.h"
#include "RenderDevice.h"

namespace nes
{
    DeviceQueue::DeviceQueue(RenderDevice& device, const EQueueType type, const uint32 familyIndex, const uint32 queueIndex)
        : m_pDevice(&device)
        , m_familyIndex(familyIndex)
        , m_queueIndex(queueIndex)
        , m_queueType(type)
    {
        // Create the queue:
        m_queue = vk::raii::Queue(*m_pDevice, familyIndex, queueIndex);
    }

    DeviceQueue& DeviceQueue::operator=(std::nullptr_t)
    {
        *this = DeviceQueue(nullptr);
        return *this;
    }

    DeviceQueue::DeviceQueue(DeviceQueue&& other) noexcept
        : m_pDevice(other.m_pDevice)
        , m_queue(std::move(other.m_queue))
        , m_familyIndex(other.m_familyIndex)
        , m_queueIndex(other.m_queueIndex)
        , m_queueType(other.m_queueType)
    {
        other.m_pDevice = nullptr;
    }

    DeviceQueue& DeviceQueue::operator=(DeviceQueue&& other) noexcept
    {
        if (this != &other)
        {
            m_queue = std::move(other.m_queue);
            m_queueType = other.m_queueType;
            m_familyIndex = other.m_familyIndex;
            m_queueType = other.m_queueType;
        }

        return *this;
    }

    void DeviceQueue::WaitUntilIdle()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.waitIdle();
    }

    void DeviceQueue::SetDebugName(const std::string& name)
    {
        m_pDevice->SetDebugNameVkObject(GetNativeVkObject(), name);
    }

    NativeVkObject DeviceQueue::GetNativeVkObject() const
    {
        return NativeVkObject(*m_queue, vk::ObjectType::eQueue);
    }
}
