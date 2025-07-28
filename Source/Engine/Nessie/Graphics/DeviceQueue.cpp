// DeviceQueue.cpp
#include "DeviceQueue.h"
#include "volk.h"
#include "RenderDevice.h"

namespace nes
{
    EGraphicsResult DeviceQueue::Init(const EQueueType type, const uint32 familyIndex, const VkQueue handle)
    {
        m_queueType = type;
        m_familyIndex = familyIndex;
        m_handle = handle;
        return EGraphicsResult::Success;
    }

    EGraphicsResult DeviceQueue::WaitUntilIdle()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        NES_VK_FAIL_RETURN(m_device, vkQueueWaitIdle(m_handle));
        
        return EGraphicsResult::Success;
    }
}
