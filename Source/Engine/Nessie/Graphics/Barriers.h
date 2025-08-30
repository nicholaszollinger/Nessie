// Barriers.h
#pragma once
#include <vulkan/vulkan_core.h>
#include "GraphicsCore.h"

namespace nes
{
    namespace graphics
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Special value to infer a pipeline stage for a barrier.
        //----------------------------------------------------------------------------------------------------
        static constexpr vk::PipelineStageFlagBits2 kInferPipelineStage = static_cast<vk::PipelineStageFlagBits2>(std::numeric_limits<std::underlying_type_t<vk::PipelineStageFlagBits2>>::max());

        //----------------------------------------------------------------------------------------------------
        /// @brief : Special value to infer resource access for a barrier.
        //----------------------------------------------------------------------------------------------------
        static constexpr vk::AccessFlagBits2 kInferAccess = static_cast<vk::AccessFlagBits2>(std::numeric_limits<std::underlying_type_t<vk::AccessFlagBits2>>::max());
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Special value for BarrierDesc structs to automatically infer a parameter's value.
        //----------------------------------------------------------------------------------------------------
        static constexpr uint64 kInferBarrierParams = ~0ULL;
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the proper access flags based on the given pipeline stage and whether the result
        ///     should have read or write access.
        ///	@param stage : The pipeline stage that we are looking for access. 
        ///	@param isRead : Whether access should be read or write.
        //----------------------------------------------------------------------------------------------------
        constexpr vk::AccessFlags2 InferAccessMaskFromStage(const vk::PipelineStageFlags2 stage, const bool isRead)
        {
            constexpr vk::PipelineStageFlags2 kNoStage = vk::PipelineStageFlagBits2::eNone;
            constexpr vk::AccessFlags2 kNoAccess = vk::AccessFlagBits2::eNone;
        
            vk::AccessFlags2 access = {};

            if ((stage & (vk::PipelineStageFlagBits2::eAllCommands | vk::PipelineStageFlagBits2::eAllGraphics)) != kNoStage)
            {
                access |= isRead ? vk::AccessFlagBits2::eMemoryRead : vk::AccessFlagBits2::eMemoryWrite; 
            }

            // Shader Stages
            if ((stage &
                (vk::PipelineStageFlagBits2::eComputeShader | vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eVertexShader
                | vk::PipelineStageFlagBits2::eMeshShaderEXT | vk::PipelineStageFlagBits2::eRayTracingShaderKHR | vk::PipelineStageFlagBits2::eTaskShaderEXT
                | vk::PipelineStageFlagBits2::ePreRasterizationShaders | vk::PipelineStageFlagBits2::eTessellationControlShader
                | vk::PipelineStageFlagBits2::eTessellationEvaluationShader | vk::PipelineStageFlagBits2::eGeometryShader))
                != kNoStage)
            {
                access |= isRead ? vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eUniformRead : vk::AccessFlagBits2::eShaderWrite;
            }

            // Host
            if ((stage & vk::PipelineStageFlagBits2::eHost) != kNoStage)
                access |= isRead ? vk::AccessFlagBits2::eHostRead : vk::AccessFlagBits2::eHostWrite;

            // Transfer
            if ((stage & vk::PipelineStageFlagBits2::eTransfer) != kNoStage)
                access |= isRead ? vk::AccessFlagBits2::eTransferRead : vk::AccessFlagBits2::eTransferWrite;

            // Vertex Attribute Input
            if ((stage & vk::PipelineStageFlagBits2::eVertexAttributeInput) != kNoStage)
                access |= vk::AccessFlagBits2::eVertexAttributeRead;

            // Index Input
            if ((stage & vk::PipelineStageFlagBits2::eIndexInput) != kNoStage)
                access |= vk::AccessFlagBits2::eIndexRead;

            // Draw Indirect
            if ((stage & vk::PipelineStageFlagBits2::eDrawIndirect) != kNoStage)
                access |= vk::AccessFlagBits2::eIndirectCommandRead;

            // Early/Late Fragment Tests (Depth Stencil)
            if ((stage & (vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests)) != kNoStage)
                access |= isRead ? vk::AccessFlagBits2::eDepthStencilAttachmentRead : vk::AccessFlagBits2::eDepthStencilAttachmentWrite;

            // Color Attachment
            if ((stage & vk::PipelineStageFlagBits2::eColorAttachmentOutput) != kNoStage)
                access |= isRead ? vk::AccessFlagBits2::eColorAttachmentRead : vk::AccessFlagBits2::eColorAttachmentWrite;

            // Command Preprocess
            if ((stage & vk::PipelineStageFlagBits2::eCommandPreprocessEXT) != kNoStage)
                access |= isRead ? vk::AccessFlagBits2::eCommandPreprocessReadEXT : vk::AccessFlagBits2::eCommandPreprocessWriteEXT;

            // Fragment Shading Rate Attachment
            if ((stage & vk::PipelineStageFlagBits2::eFragmentShadingRateAttachmentKHR) != kNoStage)
                access |= vk::AccessFlagBits2::eFragmentShadingRateAttachmentReadKHR;

            // Video Encode & Video Decode
            if ((stage & vk::PipelineStageFlagBits2::eVideoDecodeKHR) != kNoStage)
                access |= isRead ? vk::AccessFlagBits2::eVideoDecodeReadKHR : vk::AccessFlagBits2::eVideoDecodeWriteKHR;
            if ((stage & vk::PipelineStageFlagBits2::eVideoEncodeKHR) != kNoStage)
                access |= isRead ? vk::AccessFlagBits2::eVideoEncodeReadKHR : vk::AccessFlagBits2::eVideoEncodeWriteKHR;

            // Acceleration Structure Build & Copy
            if ((stage & vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR) != kNoStage)
                access |= isRead ? vk::AccessFlagBits2::eAccelerationStructureReadKHR : vk::AccessFlagBits2::eAccelerationStructureWriteKHR;
            if ((stage & vk::PipelineStageFlagBits2::eAccelerationStructureCopyKHR) != kNoStage)
                access |= vk::AccessFlagBits2::eIndirectCommandRead;

            // Ray Tracing Shader
            if ((stage & vk::PipelineStageFlagBits2::eRayTracingShaderKHR) != kNoStage)
                access |= isRead ? vk::AccessFlagBits2::eAccelerationStructureReadKHR : kNoAccess;
        
            NES_ASSERT((access != kNoAccess || stage == kNoStage));
            return access;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Outputs a pipeline stage and access flags based on an image layout.
        ///	@param layout : The Image layout that we are inferring from.
        //----------------------------------------------------------------------------------------------------
        constexpr void InferPipelineStageAccess(const vk::ImageLayout layout, vk::PipelineStageFlags2& outStage, vk::AccessFlags2& outAccess)
        {
            switch (layout)
            {
                case vk::ImageLayout::eUndefined:
                {
                    outStage = vk::PipelineStageFlagBits2::eNone;
                    outAccess = vk::AccessFlagBits2::eNone;
                    break;
                }

                case vk::ImageLayout::eColorAttachmentOptimal:
                {
                    outStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
                    outAccess = vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite;
                    break;
                }

                case vk::ImageLayout::eShaderReadOnlyOptimal:
                {
                    outStage = vk::PipelineStageFlagBits2::eFragmentShader
                                    | vk::PipelineStageFlagBits2::eComputeShader
                                    | vk::PipelineStageFlagBits2::ePreRasterizationShaders
                                    | vk::PipelineStageFlagBits2::eAllCommands;
                
                    outAccess = vk::AccessFlagBits2::eShaderRead;
                    break;
                }
            
                case vk::ImageLayout::eTransferDstOptimal:
                {
                    outStage = vk::PipelineStageFlagBits2::eTransfer;
                    outAccess = vk::AccessFlagBits2::eTransferWrite;
                    break;
                }

                case vk::ImageLayout::eTransferSrcOptimal:
                {
                    outStage = vk::PipelineStageFlagBits2::eTransfer;
                    outAccess = vk::AccessFlagBits2::eTransferRead;
                    break;
                }

                case vk::ImageLayout::eGeneral:
                {
                    outStage = vk::PipelineStageFlagBits2::eComputeShader
                                | vk::PipelineStageFlagBits2::eFragmentShader
                                | vk::PipelineStageFlagBits2::ePreRasterizationShaders
                                | vk::PipelineStageFlagBits2::eAllCommands
                                | vk::PipelineStageFlagBits2::eTransfer;
                
                    outAccess = vk::AccessFlagBits2::eShaderRead
                                | vk::AccessFlagBits2::eShaderWrite
                                | vk::AccessFlagBits2::eTransferWrite;
                    break;
                }

                case vk::ImageLayout::ePresentSrcKHR:
                {
                    outStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
                    outAccess = vk::AccessFlagBits2::eNone;
                    break;
                }

                case vk::ImageLayout::eDepthAttachmentOptimal:
                case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                case vk::ImageLayout::eDepthReadOnlyOptimal:
                {
                    outStage = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
                    outAccess = vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
                    break;
                }

                default:
                {
                    NES_ASSERT(false, "Unsupported Layout Transition!");
                    outStage = vk::PipelineStageFlagBits2::eAllCommands;
                    outAccess = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite;
                }
            }
        }
    }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Barrier description for transitioning an Image's properties. 
    //----------------------------------------------------------------------------------------------------
    struct ImageMemoryBarrierDesc
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the image that we are modifying, and the src and destination layouts.
        ///	@param oldLayout : The layout the image should be in at the time of transition.
        ///	@param newLayout : The layout that the image should be transitioned to.
        //----------------------------------------------------------------------------------------------------
        ImageMemoryBarrierDesc&     SetLayoutTransition(const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a subregion of the image resource that we are applying the barrier to.
        ///	@param aspectFlags : Which parts of the image to access.
        ///	@param mipLevelCount : Number of mip levels.
        ///	@param baseMipLevel : First mip level index.
        ///	@param layerCount : Number of image layers.
        ///	@param baseLayer : First array layer index.
        //----------------------------------------------------------------------------------------------------
        ImageMemoryBarrierDesc&     SetImageSubresourceRange(const vk::ImageAspectFlags aspectFlags, const uint32 mipLevelCount = 1, const uint32 baseMipLevel = 0, const uint32 layerCount = 1, const uint32 baseLayer = 0);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the access flags that the image is starting from and ending with.
        ///     By default, these values will be inferred by the image layouts if not set.
        ///	@param srcAccess : The access the image currently has.
        ///	@param dstAccess : The access the image will have after the transition.
        //----------------------------------------------------------------------------------------------------
        ImageMemoryBarrierDesc&     SetAccessFlags(const vk::AccessFlags2 srcAccess, const vk::AccessFlags2 dstAccess);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the pipeline stages where the src access and old layout will be set, and the pipeline
        ///     stages where the properties should be updated.
        ///     By default, these values will be inferred by the image layouts if not set.
        //----------------------------------------------------------------------------------------------------
        ImageMemoryBarrierDesc&     SetStages(const vk::PipelineStageFlags2 srcStages, const vk::PipelineStageFlags2 dstStages);

        //----------------------------------------------------------------------------------------------------
        /// @brief : If set, then the image resource will have its ownership changed to the new queue family index.
        ///     By default, no transition is set.
        ///	@param srcQueueFamilyIndex : The Queue Family Index that this image belongs to.
        ///	@param dstQueueFamilyIndex : The Queue Family Index that will take over ownership of the image.
        //----------------------------------------------------------------------------------------------------
        ImageMemoryBarrierDesc&     SetQueueFamilyIndexTransition(const uint32 srcQueueFamilyIndex, const uint32 dstQueueFamilyIndex);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Converts any stand-in inferred parameters to their appropriate values and outputs the
        ///     barrier object.
        //----------------------------------------------------------------------------------------------------
        vk::ImageMemoryBarrier2     CreateVkBarrier(const vk::Image image);
        
        vk::ImageLayout             m_oldLayout = vk::ImageLayout::eUndefined;
        vk::ImageLayout             m_newLayout = vk::ImageLayout::eUndefined;
        vk::ImageSubresourceRange   m_subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        vk::PipelineStageFlags2     m_srcStageMask = graphics::kInferPipelineStage;
        vk::PipelineStageFlags2     m_dstStageMask = graphics::kInferPipelineStage;
        vk::AccessFlags2            m_srcAccessMask = graphics::kInferAccess;
        vk::AccessFlags2            m_dstAccessMask = graphics::kInferAccess;
        uint32                      m_srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        uint32                      m_dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };

    // [TODO]: Update to c++ API.
    // //----------------------------------------------------------------------------------------------------
    // /// @brief : Struct to set up a buffer synchronization step.
    // ///     Particularly useful for compute/shader to transfer synchronization.
    // //----------------------------------------------------------------------------------------------------
    //  struct BufferMemoryBarrierDesc
    //  {
    //      //----------------------------------------------------------------------------------------------------
    //      /// @brief : Set the buffer object that we will be modifying. 
    //      ///	@param buffer : Buffer resource.
    //      ///	@param offset : Byte offset in the buffer. Default is 0.
    //      ///	@param size : Size of the buffer. Default is the entire size from the given offset.
    //      //----------------------------------------------------------------------------------------------------
    //      BufferMemoryBarrierDesc& SetBuffer(VkBuffer buffer, const VkDeviceSize offset = 0, const VkDeviceSize size = VK_WHOLE_SIZE);
    //      
    //      //----------------------------------------------------------------------------------------------------
    //      /// @brief : Set the access flags that the image is starting from and ending with.
    //      ///     By default, these values will be inferred by the image layout if not set.
    //      ///	@param srcAccess : The access the image currently has.
    //      ///	@param dstAccess : The access the image will have after the transition.
    //      //----------------------------------------------------------------------------------------------------
    //      BufferMemoryBarrierDesc& SetAccessFlags(const VkAccessFlags2 srcAccess, const VkAccessFlags2 dstAccess);
    //
    //      //----------------------------------------------------------------------------------------------------
    //      /// @brief : Set the pipeline stages that will facilitate the transition.
    //      ///     By default, no Pipeline Stages are set.
    //      //----------------------------------------------------------------------------------------------------
    //      BufferMemoryBarrierDesc& SetStages(const VkPipelineStageFlags2 srcStages, const VkPipelineStageFlags dstStages);
    //
    //      //----------------------------------------------------------------------------------------------------
    //      /// @brief : If set, then the image resource will have its ownership changed to the new queue family index.
    //      ///     By default, no transition is set.
    //      ///	@param srcQueueFamilyIndex : The Queue Family Index that this image belongs to.
    //      ///	@param dstQueueFamilyIndex : The Queue Family Index that will take over ownership of the image.
    //      //----------------------------------------------------------------------------------------------------
    //      BufferMemoryBarrierDesc& SetQueueFamilyIndexTransition(const uint32 srcQueueFamilyIndex, const uint32 dstQueueFamilyIndex);
    //
    //      //----------------------------------------------------------------------------------------------------
    //      /// @brief : Converts any stand-in inferred parameters to their appropriate values and outputs the
    //      ///     barrier object. Returns EGraphicsResult::Failure if the Buffer object was not set.
    //      //----------------------------------------------------------------------------------------------------
    //      EGraphicsResult         Resolve(VkBufferMemoryBarrier2& outBarrier);
    //      
    //      VkBuffer                m_buffer = nullptr;
    //      VkDeviceSize            m_offset = 0;
    //      VkDeviceSize            m_size = VK_WHOLE_SIZE;
    //      VkPipelineStageFlags2   m_srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    //      VkPipelineStageFlags2   m_dstStageMask = VK_PIPELINE_STAGE_2_NONE;
    //      VkAccessFlags2          m_srcAccessMask = graphics::kInferBarrierParams;
    //      VkAccessFlags2          m_dstAccessMask = graphics::kInferBarrierParams;
    //      uint32                  m_srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //      uint32                  m_dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //  };

    // [TODO]: Memory Barrier.
    
}