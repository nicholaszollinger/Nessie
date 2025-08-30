// CommandPool.h
#pragma once
#include "GraphicsCommon.h"
#include "DeviceObject.h"
#include "Nessie/Core/Thread/Mutex.h"

namespace nes
{
    class CommandBuffer;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : An object used to allocate Command Buffers.
    ///     A Command Pool is created with a specific queue family index. All CommandBuffers created
    ///     with this pool must be submitted to a DeviceQueue of the same queue family index.
    //----------------------------------------------------------------------------------------------------
    class CommandPool
    {
    public:
        CommandPool(std::nullptr_t) {}
        CommandPool(const CommandPool&) = delete;
        CommandPool(CommandPool&& other) noexcept;
        CommandPool& operator=(std::nullptr_t);
        CommandPool& operator=(const CommandPool&) = delete;
        CommandPool& operator=(CommandPool&& other) noexcept;
        ~CommandPool();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the Command Pool object for the given queue.
        //----------------------------------------------------------------------------------------------------
        CommandPool(RenderDevice& device, const DeviceQueue& queue);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this command pool. 
        //----------------------------------------------------------------------------------------------------
        void                    SetDebugName(const std::string& name);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a new primary command buffer.
        //----------------------------------------------------------------------------------------------------
        CommandBuffer           CreateCommandBuffer();

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

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the native vulkan object handle, and the type.
        //----------------------------------------------------------------------------------------------------
        NativeVkObject          GetNativeVkObject() const;
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Submits the CommandPool to the Renderer to be freed. 
        //----------------------------------------------------------------------------------------------------
        void                    FreePool();
        
    private:
        RenderDevice*           m_pDevice = nullptr;
        vk::raii::CommandPool   m_pool = nullptr;
        EQueueType              m_queueType{}; /// What Queue Type should all commands be submitted to?
        Mutex                   m_mutex;
    };
}
