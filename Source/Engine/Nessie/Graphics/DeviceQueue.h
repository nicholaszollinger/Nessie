// DeviceQueue.h
#pragma once
#include "GraphicsCommon.h"
#include "Nessie/Core/Thread/Mutex.h"
#include "CommandBuffer.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Device Queue is a logical queue that has access to a hardware queue. More broadly,
    ///     a queue is a sequence of commands that are executed in order. The queue is used to submit
    ///     command buffers to the GPU.
    ///     - The Queue's family index is used to identify the type of queue (graphics, compute, transfer, ...).
    ///     - The Queue's index is used to identify the specific queue in the family - multiple queues can be in the same family.
    //----------------------------------------------------------------------------------------------------
    class DeviceQueue final : public GraphicsResource
    {
    public:
        DeviceQueue(RenderDevice& device) : GraphicsResource(device) {}

        /// Operator to cast to the Vulkan type.
        operator        VkQueue() const         { return m_handle; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialization function for the Device Queue. This will be called by the Render Device. 
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult Init(const EQueueType type, const uint32 familyIndex, VkQueue handle);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Wait until this queue has finished all command submissions.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult WaitUntilIdle();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Submits the commands and blocks until complete. CommandBuffer::End() must be called before
        ///     this function!
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult SubmitSingleTimeCommands(const CommandBuffer& buffer);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this Queue. 
        //----------------------------------------------------------------------------------------------------
        virtual void    SetDebugName(const char* name) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : The Queue's family index is used to identify the type of queue (graphics, compute, transfer, ...).
        //----------------------------------------------------------------------------------------------------
        uint32          GetFamilyIndex() const  { return m_familyIndex; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : This is the type of Queue (graphics, compute, transfer, ...).
        //----------------------------------------------------------------------------------------------------
        EQueueType      GetQueueType() const    { return m_queueType; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Queue's mutex that is locked when waiting or submitting.
        //----------------------------------------------------------------------------------------------------
        Mutex&          GetMutex()              { return m_mutex; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan handle.  
        //----------------------------------------------------------------------------------------------------
        VkQueue         GetHandle() const       { return m_handle; }   
    
    private:
        VkQueue         m_handle = nullptr;
        uint32          m_familyIndex = static_cast<uint32>(-1);
        EQueueType      m_queueType = EQueueType::MaxNum;
        Mutex           m_mutex;
    };
}