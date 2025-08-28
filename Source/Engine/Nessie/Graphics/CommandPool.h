// CommandPool.h
#pragma once
#include "GraphicsCommon.h"
#include "DeviceAsset.h"
#include "Nessie/Core/Thread/Mutex.h"

namespace nes
{
    class CommandBuffer;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : An object used to allocate Command Buffers.
    ///     A Command Pool is created with a specific queue family index. All CommandBuffers created
    ///     with this pool must be submitted to a DeviceQueue of the same queue family index.
    //----------------------------------------------------------------------------------------------------
    class CommandPool final : public DeviceAsset
    {
    public:
        explicit                CommandPool(RenderDevice& device) : DeviceAsset(device) {}
        virtual                 ~CommandPool() override;

        /// Operator to cast the Vulkan Type.
        operator                const vk::raii::CommandPool&() const { return m_pool; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the Command Pool object. This will allocate the Vulkan resource.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         Init(const DeviceQueue& queue);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this command pool. 
        //----------------------------------------------------------------------------------------------------
        virtual void            SetDebugName(const std::string& name) override;

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
        vk::raii::CommandPool   m_pool = nullptr;
        EQueueType              m_queueType{}; /// What Queue Type should all commands be submitted to?
        Mutex                   m_mutex;
    };
}
