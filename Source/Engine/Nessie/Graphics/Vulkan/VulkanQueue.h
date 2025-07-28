// VulkanQueue.h
#pragma once
#include "Nessie/Core/Config.h"
#include "Nessie/Core/Thread/Mutex.h"
#include "Nessie/Graphics/GraphicsCommon.h"
#include "vulkan/vulkan_core.h"

namespace nes
{
    class VulkanDevice;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Wrapper for a VkQueue. 
    //----------------------------------------------------------------------------------------------------
    class VulkanQueue
    {
    public:
        VulkanQueue(VulkanDevice& device) : m_device(device) {}

        operator        VkQueue() const         { return m_handle; }
        
        EGraphicsResult Create(const EQueueType type, const uint32 familyIndex, VkQueue handle);
        // void            SetDebugName(const char* name);
        // void            BeginAnnotation(const char* name, const uint32 brga);
        // void            EndAnnotation();
        // void            Annotation(const char* name, const uint32 brga);
        // EGraphicsResult Submit(const QueueSubmitDesc& submitDesc, const Swapchain* pSwapchain);
        EGraphicsResult WaitUntilIdle();
        
        VulkanDevice&   GetDevice() const       { return m_device; }
        uint32          GetFamilyIndex() const  { return m_familyIndex; }
        EQueueType      GetQueueType() const    { return m_queueType; }
        Mutex&          GetMutex()              { return m_mutex; }

    private:
        VulkanDevice&   m_device;
        VkQueue         m_handle = nullptr;
        uint32          m_familyIndex = static_cast<uint32>(-1);
        EQueueType      m_queueType = EQueueType::MaxNum;
        Mutex           m_mutex;
    };
}
