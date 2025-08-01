// CommandPool.h
#pragma once
#include "GraphicsCommon.h"
#include "GraphicsResource.h"
#include "Nessie/Core/Thread/Mutex.h"

namespace nes
{
    class CommandBuffer;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : An object used to allocate Command Buffers.
    ///     A Command Pool is created with a specific queue family index. All CommandBuffers created
    ///     with this pool must be submitted to a DeviceQueue of the same queue family index.
    //----------------------------------------------------------------------------------------------------
    class CommandPool final : public GraphicsResource
    {
    public:
        explicit                CommandPool(RenderDevice& device) : GraphicsResource(device) {}
        virtual                 ~CommandPool() override;

        /// Operator to cast the Vulkan Type.
        operator                VkCommandPool() const { return m_handle; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the Command Pool object. This will allocate the Vulkan resource.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         Init(const DeviceQueue& queue);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this command pool. 
        //----------------------------------------------------------------------------------------------------
        virtual void            SetDebugName(const char* name) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a new command buffer.
        /// VK: This creates a primary command buffer. 
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         CreateCommandBuffer(CommandBuffer*& pOutCommandBuffer);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Resetting a command pool recycles all resources from all command buffers
        ///     allocated from the command pool back to the command pool.
        ///     All command buffers that have been allocated from the command pool are put in the initial state.
        //----------------------------------------------------------------------------------------------------
        void                    Reset();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the type of Queue that command buffers should be submitted to.  
        //----------------------------------------------------------------------------------------------------
        const EQueueType&       GetQueueType() const { return m_queueType; }

    private:
        VkCommandPool           m_handle{};
        EQueueType              m_queueType{}; /// What Queue Type should all commands be submitted to?
        Mutex                   m_mutex;
    };
}
