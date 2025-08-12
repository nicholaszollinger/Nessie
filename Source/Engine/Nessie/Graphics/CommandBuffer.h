// CommandBuffer.h
#pragma once
#include "Barriers.h"
#include "DeviceAsset.h"

namespace nes
{
    class CommandPool;

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Command Buffer is used to record commands that are then submitted to a DeviceQueue.
    //----------------------------------------------------------------------------------------------------
    class CommandBuffer final : public DeviceAsset
    {
    public:
        explicit        CommandBuffer(RenderDevice& device) : DeviceAsset(device) {}
        virtual         ~CommandBuffer() override;
        
        /// Operator to cast to the Vulkan Type.
        operator        VkCommandBuffer() const { return m_handle; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Do not call directly; this will be called by CommandPool::CreateCommandBuffer().
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult Init(CommandPool* pPool, VkCommandBuffer handle);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this command buffer. 
        //----------------------------------------------------------------------------------------------------
        virtual void    SetDebugName(const char* name) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Begin recording commands to this command buffer.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult Begin();

        //----------------------------------------------------------------------------------------------------
        /// @brief : End recording commands to the buffer. The Command Buffer is now ready to be submitted to
        ///     a device queue. If there was an error during recording, this will return the invalid result
        ///     here and the Command Buffer will return to the invalid state (before Begin()).
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult End();

        // [TODO]: Refactor with better desc and nes::Texture
        //----------------------------------------------------------------------------------------------------
        /// @brief : Transition an image from one layout to another. In the pipeline, the image must be in
        ///     the correct layout to be used.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult TransitionImageLayout(VkImage image, ImageMemoryBarrierDesc& barrierDesc) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : [TODO]: This is for dynamic rendering. We need: AttachmentsDesc, Descriptor, ...
        //----------------------------------------------------------------------------------------------------
        //EGraphicsResult BeginRendering();
    
    private:
        VkCommandBuffer m_handle{};
        CommandPool*    m_pCommandPool = nullptr;
    };
}
