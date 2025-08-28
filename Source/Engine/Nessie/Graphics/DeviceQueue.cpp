// DeviceQueue.cpp
#include "DeviceQueue.h"
#include "RenderDevice.h"

namespace nes
{
    DeviceQueue::DeviceQueue(DeviceQueue&& other) noexcept
        : DeviceAsset(other.m_device)
        , m_queue(std::move(other.m_queue))
        , m_familyIndex(other.m_familyIndex)
        , m_queueIndex(other.m_queueIndex)
        , m_queueType(other.m_queueType)
    {
        //
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

    EGraphicsResult DeviceQueue::Init(const EQueueType type, const uint32 familyIndex, const uint32 queueIndex)
    {
        m_queueType = type;
        m_familyIndex = familyIndex;
        m_queueIndex = queueIndex;
        m_queue = vk::raii::Queue(m_device, familyIndex, queueIndex);
        return EGraphicsResult::Success;
    }

    void DeviceQueue::WaitUntilIdle()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.waitIdle();
    }

    void DeviceQueue::SetDebugName(const std::string& name)
    {
        vk::Queue queue = *m_queue;
        m_device.SetDebugNameToTrivialObject(queue, name.c_str());
    }
}
