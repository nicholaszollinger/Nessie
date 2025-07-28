// VulkanQueue.cpp
#include "VulkanQueue.h"
#include "VulkanDevice.h"

namespace nes
{
    EGraphicsResult VulkanQueue::Create(const EQueueType type, const uint32 familyIndex, VkQueue handle)
    {
        m_queueType = type;
        m_familyIndex = familyIndex;
        m_handle = handle;
        return EGraphicsResult::Success;
    }

    // void VulkanQueue::SetDebugName(const char* name)
    // {
    //     m_device.SetDebugNameToTrivialObject(vk::ObjectType::eQueue, reinterpret_cast<uint64>(m_handle), name);
    // }
    //
    // void VulkanQueue::BeginAnnotation(const char* name, const uint32 brga)
    // {
    //     VkDebugUtilsLabelEXT info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT};
    //     info.pLabelName = name;
    //     // [TODO]: Color.
    //     info.color[3] = 1.f;
    //
    //     const auto& vk = m_device.GetDispatchTable();
    //     if (vk.QueueBeginDebugUtilsLabelEXT)
    //         vk.QueueBeginDebugUtilsLabelEXT(m_handle, &info);
    // }
    //
    // void VulkanQueue::EndAnnotation()
    // {
    //     const auto& vk = m_device.GetDispatchTable();
    //     if (vk.QueueEndDebugUtilsLabelEXT)
    //         vk.QueueEndDebugUtilsLabelEXT(m_handle);
    // }
    //
    // void VulkanQueue::Annotation(const char* name, const uint32 brga)
    // {
    //     VkDebugUtilsLabelEXT info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT};
    //     info.pLabelName = name;
    //     // [TODO]: Color.
    //     info.color[3] = 1.f;
    //
    //     const auto& vk = m_device.GetDispatchTable();
    //     if (vk.QueueInsertDebugUtilsLabelEXT)
    //         vk.QueueInsertDebugUtilsLabelEXT(m_handle, &info);
    // }

    EGraphicsResult VulkanQueue::WaitUntilIdle()
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        const auto& vk = m_device.GetDispatchTable();
        VkResult vkResult = vk.QueueWaitIdle(m_handle);
        NES_RETURN_ON_BAD_VKRESULT(m_device, vkResult, "QueueWaitIdle")
        
        return EGraphicsResult::Success;
    }
}
