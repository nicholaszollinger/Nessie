// DeviceQueue.h
#pragma once
#include "GraphicsCommon.h"
#include "Nessie/Core/Thread/Mutex.h"
#include "CommandBuffer.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Device Queue is a logical queue that has access to a hardware queue. The queue is used to submit
    ///     command buffers to the GPU.
    ///     - The Queue's family index is used to identify the type of queue (graphics, compute, transfer, ...).
    ///     - The Queue's index is used to identify the specific queue in the family - multiple queues can be in the same family.
    //----------------------------------------------------------------------------------------------------
    class DeviceQueue
    {
    public:
        DeviceQueue(std::nullptr_t) {}
        DeviceQueue(const DeviceQueue&) = delete;
        DeviceQueue(DeviceQueue&& other) noexcept;
        DeviceQueue& operator=(std::nullptr_t);
        DeviceQueue& operator=(const DeviceQueue&) = delete;
        DeviceQueue& operator=(DeviceQueue&& other) noexcept;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Wait until this queue has finished all command submissions.
        //----------------------------------------------------------------------------------------------------
        void                WaitUntilIdle();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the debug name for this device queue. 
        //----------------------------------------------------------------------------------------------------
        void                SetDebugName(const std::string& name);

        //----------------------------------------------------------------------------------------------------
        /// @brief : The Queue's family index is used to identify the type of queue (graphics, compute, transfer, ...).
        //----------------------------------------------------------------------------------------------------
        uint32              GetFamilyIndex() const  { return m_familyIndex; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the index of the queue in the family.
        //----------------------------------------------------------------------------------------------------
        uint32              GetQueueIndex() const  { return m_queueIndex; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : This is the type of Queue (graphics, compute, transfer, ...).
        //----------------------------------------------------------------------------------------------------
        EQueueType          GetQueueType() const    { return m_queueType; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Queue's mutex that is locked when waiting or submitting.
        //----------------------------------------------------------------------------------------------------
        Mutex&              GetMutex()              { return m_mutex; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan Queue object.
        //----------------------------------------------------------------------------------------------------
        vk::raii::Queue&    GetVkQueue()          { return m_queue; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the native vulkan object handle, and the type.
        //----------------------------------------------------------------------------------------------------
        NativeVkObject      GetNativeVkObject() const;
        
    private:
        friend class RenderDevice;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Private Ctor only for the Render Device. Device Queues are created during logical device
        ///     construction and are tied to the lifetime of the Render Device. 
        //----------------------------------------------------------------------------------------------------
        DeviceQueue(RenderDevice& device, const EQueueType type, const uint32 familyIndex, const uint32 queueIndex);
        
        RenderDevice*       m_pDevice       = nullptr;
        vk::raii::Queue     m_queue         = nullptr;
        Mutex               m_mutex{};
        uint32              m_familyIndex   = std::numeric_limits<uint32>::max();
        uint32              m_queueIndex    = std::numeric_limits<uint32>::max();
        EQueueType          m_queueType     = EQueueType::MaxNum;
    };
    
    static_assert(DeviceObjectType<DeviceQueue>);
}