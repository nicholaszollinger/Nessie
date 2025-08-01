// Barriers.h
#pragma once
#include <vulkan/vulkan_core.h>
#include "GraphicsCore.h"

namespace nes
{
    namespace vulkan
    {
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
        constexpr VkAccessFlags2    InferAccessMaskFromStage(const VkPipelineStageFlags2 stage, const bool isRead)
        {
            VkAccessFlags2 access = 0;

            if ((stage & (VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT | VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT)) != 0)
            {
                access |= isRead ? VK_ACCESS_2_MEMORY_READ_BIT : VK_ACCESS_2_MEMORY_WRITE_BIT; 
            }

            // Shader Stages
            if ((stage &
                (VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT
                | VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT | VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT
                | VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT | VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT
                | VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT))
                != 0)
            {
                access |= isRead ? VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_UNIFORM_READ_BIT : VK_ACCESS_2_SHADER_WRITE_BIT;
            }

            // Host
            if ((stage & VK_PIPELINE_STAGE_2_HOST_BIT) != 0)
                access |= isRead ? VK_ACCESS_2_HOST_READ_BIT : VK_ACCESS_2_HOST_WRITE_BIT;

            // Transfer
            if ((stage & VK_PIPELINE_STAGE_2_TRANSFER_BIT) != 0)
                access |= isRead ? VK_ACCESS_2_TRANSFER_READ_BIT : VK_ACCESS_2_TRANSFER_WRITE_BIT;

            // Vertex Attribute Input
            if ((stage & VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT) != 0)
                access |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;

            // Index Input
            if ((stage & VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT) != 0)
                access |= VK_ACCESS_2_INDEX_READ_BIT;

            // Draw Indirect
            if ((stage & VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT) != 0)
                access |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;

            // Early/Late Fragment Tests (Depth Stencil)
            if ((stage & (VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT)) != 0)
                access |= isRead ? VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT : VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            // Color Attachment
            if ((stage & VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT) != 0)
                access |= isRead ? VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT : VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;

            // Command Preprocess
            if ((stage & VK_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_EXT) != 0)
                access |= isRead ? VK_ACCESS_2_COMMAND_PREPROCESS_READ_BIT_EXT : VK_ACCESS_2_COMMAND_PREPROCESS_WRITE_BIT_EXT;

            // Fragment Shading Rate Attachment
            if ((stage & VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) != 0)
                access |= VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;

            // Video Encode & Video Decode
            if ((stage & VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR) != 0)
                access |= isRead ? VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR : VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR;
            if ((stage & VK_PIPELINE_STAGE_2_VIDEO_ENCODE_BIT_KHR) != 0)
                access |= isRead ? VK_ACCESS_2_VIDEO_ENCODE_READ_BIT_KHR : VK_ACCESS_2_VIDEO_ENCODE_WRITE_BIT_KHR;

            // Acceleration Structure Build & Copy
            if ((stage & VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR) != 0)
                access |= isRead ? VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR : VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
            if ((stage & VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR) != 0)
                access |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;

            // Ray Tracing Shader
            if ((stage & VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR) != 0)
                access |= isRead ? VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR : 0;
            
            NES_ASSERT((access != 0 || stage == VK_PIPELINE_STAGE_2_NONE));
            return access;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Outputs a pipeline stage and access flags based on an image layout.
        ///	@param state : The Image layout that we are inferring from.
        //----------------------------------------------------------------------------------------------------
        constexpr void              InferPipelineStageAccess(const VkImageLayout state, VkPipelineStageFlags2& outStage, VkAccessFlags2& outAccess)
        {
            switch (state)
            {
                case VK_IMAGE_LAYOUT_UNDEFINED:
                {
                    outStage = VK_PIPELINE_STAGE_2_NONE;
                    outAccess = VK_ACCESS_2_NONE;
                    break;
                }

                case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                {
                    outStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                    outAccess = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
                    break;
                }

                case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                {
                    outStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
                                    | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT
                                    | VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT
                                    | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                    
                    outAccess = VK_ACCESS_2_SHADER_READ_BIT;
                    break;
                }
                
                case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                {
                    outStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                    outAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
                    break;
                }

                case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                {
                    outStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                    outAccess = VK_ACCESS_2_TRANSFER_READ_BIT;
                    break;
                }

                case VK_IMAGE_LAYOUT_GENERAL:
                {
                    outStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT
                                | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
                                | VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT
                                | VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
                                | VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                    
                    outAccess = VK_ACCESS_2_SHADER_READ_BIT
                                | VK_ACCESS_2_SHADER_WRITE_BIT
                                | VK_ACCESS_2_TRANSFER_WRITE_BIT;
                    break;
                }

                case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                {
                    outStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                    outAccess = VK_ACCESS_2_NONE;
                    break;
                }

                case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
                {
                    outStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
                    outAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    break;
                }

                default:
                {
                    NES_ASSERT(false, "Unsupported Layout Transition!");
                    outStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                    outAccess = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
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
        ImageMemoryBarrierDesc& SetLayoutTransition(const VkImageLayout oldLayout, const VkImageLayout newLayout);

        // [TODO]: 
        //ImageMemoryBarrierDesc& SetImageSubResourceRange();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the access flags that the image is starting from and ending with.
        ///     By default, these values will be inferred by the image layout if not set.
        ///	@param srcAccess : The access the image currently has.
        ///	@param dstAccess : The access the image will have after the transition.
        //----------------------------------------------------------------------------------------------------
        ImageMemoryBarrierDesc& SetAccessFlags(const VkAccessFlags2 srcAccess, const VkAccessFlags2 dstAccess);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the pipeline stages where the src access and old layout will be set, and the pipeline
        ///     stages where the properties should be updated.
        ///     By default, these values will be inferred by the image layout if not set.
        //----------------------------------------------------------------------------------------------------
        ImageMemoryBarrierDesc& SetStages(const VkPipelineStageFlags2 srcStages, const VkPipelineStageFlags dstStages);

        //----------------------------------------------------------------------------------------------------
        /// @brief : If set, then the image resource will have its ownership changed to the new queue family index.
        ///     By default, no transition is set.
        ///	@param srcQueueFamilyIndex : The Queue Family Index that this image belongs to.
        ///	@param dstQueueFamilyIndex : The Queue Family Index that will take over ownership of the image.
        //----------------------------------------------------------------------------------------------------
        ImageMemoryBarrierDesc& SetQueueFamilyIndexTransition(const uint32 srcQueueFamilyIndex, const uint32 dstQueueFamilyIndex);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Converts any stand-in inferred parameters to their appropriate values and outputs the
        ///     barrier object. Returns EGraphicsResult::Failure if the Image object was not set.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         CreateBarrier(const VkImage image, VkImageMemoryBarrier2& outBarrier);
        
        VkImageLayout           m_oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout           m_newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageSubresourceRange m_subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS};
        VkPipelineStageFlags2   m_srcStageMask = vulkan::kInferBarrierParams;
        VkPipelineStageFlags2   m_dstStageMask = vulkan::kInferBarrierParams;
        VkAccessFlags2          m_srcAccessMask = vulkan::kInferBarrierParams;
        VkAccessFlags2          m_dstAccessMask = vulkan::kInferBarrierParams;
        uint32                  m_srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        uint32                  m_dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Struct to set up a buffer synchronization step.
    ///     Particularly useful for compute/shader to transfer synchronization.
    //----------------------------------------------------------------------------------------------------
    struct BufferMemoryBarrierDesc
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the buffer object that we will be modifying. 
        ///	@param buffer : Buffer resource.
        ///	@param offset : Byte offset in the buffer. Default is 0.
        ///	@param size : Size of the buffer. Default is the entire size from the given offset.
        //----------------------------------------------------------------------------------------------------
        BufferMemoryBarrierDesc& SetBuffer(VkBuffer buffer, const VkDeviceSize offset = 0, const VkDeviceSize size = VK_WHOLE_SIZE);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the access flags that the image is starting from and ending with.
        ///     By default, these values will be inferred by the image layout if not set.
        ///	@param srcAccess : The access the image currently has.
        ///	@param dstAccess : The access the image will have after the transition.
        //----------------------------------------------------------------------------------------------------
        BufferMemoryBarrierDesc& SetAccessFlags(const VkAccessFlags2 srcAccess, const VkAccessFlags2 dstAccess);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the pipeline stages that will facilitate the transition.
        ///     By default, no Pipeline Stages are set.
        //----------------------------------------------------------------------------------------------------
        BufferMemoryBarrierDesc& SetStages(const VkPipelineStageFlags2 srcStages, const VkPipelineStageFlags dstStages);

        //----------------------------------------------------------------------------------------------------
        /// @brief : If set, then the image resource will have its ownership changed to the new queue family index.
        ///     By default, no transition is set.
        ///	@param srcQueueFamilyIndex : The Queue Family Index that this image belongs to.
        ///	@param dstQueueFamilyIndex : The Queue Family Index that will take over ownership of the image.
        //----------------------------------------------------------------------------------------------------
        BufferMemoryBarrierDesc& SetQueueFamilyIndexTransition(const uint32 srcQueueFamilyIndex, const uint32 dstQueueFamilyIndex);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Converts any stand-in inferred parameters to their appropriate values and outputs the
        ///     barrier object. Returns EGraphicsResult::Failure if the Buffer object was not set.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         Resolve(VkBufferMemoryBarrier2& outBarrier);
        
        VkBuffer                m_buffer = nullptr;
        VkDeviceSize            m_offset = 0;
        VkDeviceSize            m_size = VK_WHOLE_SIZE;
        VkPipelineStageFlags2   m_srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        VkPipelineStageFlags2   m_dstStageMask = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2          m_srcAccessMask = vulkan::kInferBarrierParams;
        VkAccessFlags2          m_dstAccessMask = vulkan::kInferBarrierParams;
        uint32                  m_srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        uint32                  m_dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };

    // [TODO]: Memory Barrier.
    
}