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

    EGraphicsResult DeviceQueue::SubmitSingleTimeCommands(const CommandBuffer& buffer)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Create the fence:
        constexpr VkFenceCreateInfo kFenceInfo
        {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
        };
        std::array<VkFence, 1> fences{};
        NES_VK_FAIL_RETURN(m_device, vkCreateFence(m_device, &kFenceInfo, m_device.GetVkAllocationCallbacks(), fences.data()));

        // Create the submission info:
        const VkCommandBufferSubmitInfo cmdBufferSubmitInfo
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .pNext = nullptr,
            .commandBuffer = buffer,
        };

        const std::array<VkSubmitInfo2, 1> submitInfo
        {
            {{ .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2, .commandBufferInfoCount = 1, .pCommandBufferInfos = &cmdBufferSubmitInfo}}
        };

        // Submit and Wait
        NES_VK_FAIL_RETURN(m_device, vkQueueSubmit2(m_handle, static_cast<uint32>(submitInfo.size()), submitInfo.data(), fences[0]));
        NES_VK_FAIL_RETURN(m_device, vkWaitForFences(m_device, static_cast<uint32>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX));

        // Cleanup
        vkDestroyFence(m_device, fences[0], m_device.GetVkAllocationCallbacks());
        return EGraphicsResult::Success;
    }

    void DeviceQueue::SetDebugName(const char* name)
    {
        m_device.SetDebugNameToTrivialObject(m_handle, name);
    }
}
