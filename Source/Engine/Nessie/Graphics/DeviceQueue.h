// DeviceQueue.h
#pragma once
#include "GraphicsCommon.h"
#include "Nessie/Core/Thread/Mutex.h"
#include "Vulkan/VulkanCore.h"

namespace nes
{
    class RenderDevice;

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Device Queue is a logical queue that has access to a hardware queue. More broadly,
    ///     a queue is a sequence of commands that are executed in order. The queue is used to submit
    ///     command buffers to the GPU.
    ///     - The Queue's family index is used to identify the type of queue (graphics, compute, transfer, ...).
    ///     - The Queue's index is used to identify the specific queue in the family - multiple queues can be in the same family.
    //----------------------------------------------------------------------------------------------------
    class DeviceQueue
    {
    public:
        DeviceQueue(RenderDevice& device) : m_device(device) {}

        /// Operator to cast to the Vulkan type.
        operator        VkQueue() const         { return m_handle; }

        EGraphicsResult Init(const EQueueType type, const uint32 familyIndex, VkQueue handle);
        EGraphicsResult WaitUntilIdle();

        // [TODO]: 
        // void            SetDebugName(const char* name);
        // void            BeginAnnotation(const char* name, const uint32 brga);
        // void            EndAnnotation();
        // void            Annotation(const char* name, const uint32 brga);
        //EGraphicsResult Submit(const QueueSubmitDesc& submitDesc, const Swapchain* pSwapchain);
        
        RenderDevice&   GetDevice() const       { return m_device; }
        uint32          GetFamilyIndex() const  { return m_familyIndex; }
        EQueueType      GetQueueType() const    { return m_queueType; }
        Mutex&          GetMutex()              { return m_mutex; }
    
    private:
        RenderDevice&   m_device;
        VkQueue         m_handle = nullptr;
        uint32          m_familyIndex = static_cast<uint32>(-1);
        EQueueType      m_queueType = EQueueType::MaxNum;
        Mutex           m_mutex;
    };
}