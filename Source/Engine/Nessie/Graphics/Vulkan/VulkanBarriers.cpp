// VulkanBarriers.cpp
#include "VulkanConversions.h"
#include "Nessie/Graphics/DeviceImage.h"
#include "Nessie/Graphics/DeviceQueue.h"

namespace nes
{
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
    
    vk::ImageMemoryBarrier2 CreateVkImageMemoryBarrier(const ImageBarrierDesc& desc)
    {
        NES_ASSERT(desc.m_pImage);

        // Resource range
        const vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange()
            .setAspectMask(GetVkImageAspectFlags(desc.m_planes))
            .setBaseMipLevel(desc.m_baseMip)
            .setLevelCount(desc.m_mipCount)
            .setBaseArrayLayer(desc.m_baseLayer)
            .setLayerCount(desc.m_layerCount);

        vk::ImageMemoryBarrier2 result = vk::ImageMemoryBarrier2()
            .setImage(desc.m_pImage->GetVkImage())
            .setOldLayout(GetVkImageLayout(desc.m_before.m_layout))
            .setNewLayout(GetVkImageLayout(desc.m_after.m_layout))
            .setSubresourceRange(subresourceRange);

        // Queue Transfer:
        if (desc.m_pSrcQueue)
        {
            NES_ASSERT(desc.m_pDstQueue);
            NES_ASSERT(desc.m_queueOp != EBarrierQueueOp::None, "Must have a valid EBarrerQueueOp for queue transfer operation!");
            result.setSrcQueueFamilyIndex(desc.m_pSrcQueue->GetFamilyIndex());
            result.setDstQueueFamilyIndex(desc.m_pDstQueue->GetFamilyIndex());

            if (desc.m_queueOp == EBarrierQueueOp::Release)
            {
                // Release: use m_before for src, ignore dst
                result.setSrcStageMask(GetVkPipelineStageFlags(desc.m_before.m_stages));
                result.setSrcAccessMask(GetVkAccessFlags(desc.m_before.m_access));

                if (desc.m_after.m_stages == graphics::kInferPipelineStage)
                {
                    vk::PipelineStageFlags2 inferredStage;
                    vk::AccessFlags2 inferredAccess;
                    InferPipelineStageAccess(result.newLayout, inferredStage, inferredAccess);
                    result.setDstStageMask(inferredStage);
                }
                else
                {
                    result.setDstStageMask(GetVkPipelineStageFlags(desc.m_after.m_stages));
                }

                // Can be zero for
                result.setDstAccessMask(vk::AccessFlagBits2::eNone);
            }
            else if (desc.m_queueOp == EBarrierQueueOp::Acquire)
            {
                // Acquire: ignore src, use m_after for dst
                result.setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe);
                result.setSrcAccessMask(vk::AccessFlagBits2::eNone);
                result.setDstStageMask(GetVkPipelineStageFlags(desc.m_after.m_stages));
                result.setDstAccessMask(GetVkAccessFlags(desc.m_after.m_access));
            }
        }
        else
        {
            result.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            result.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

            // Source Access:
            if (desc.m_before.m_stages == graphics::kInferPipelineStage && desc.m_before.m_access == graphics::kInferAccess)
            {
                InferPipelineStageAccess(result.oldLayout, result.srcStageMask, result.srcAccessMask);
            }
            else if (desc.m_before.m_access == graphics::kInferAccess)
            {
                NES_ASSERT(desc.m_before.m_stages != graphics::kInferPipelineStage);
                result.srcAccessMask = InferAccessMaskFromStage(GetVkPipelineStageFlags(desc.m_before.m_stages), true);
            }
        
            // Destination Access:
            if (desc.m_after.m_stages == graphics::kInferPipelineStage && desc.m_after.m_access == graphics::kInferAccess)
            {
                InferPipelineStageAccess(result.newLayout, result.dstStageMask, result.dstAccessMask);
            }
            else if (desc.m_after.m_access == graphics::kInferAccess)
            {
                NES_ASSERT(desc.m_after.m_stages != graphics::kInferPipelineStage);
                result.dstAccessMask = InferAccessMaskFromStage(GetVkPipelineStageFlags(desc.m_after.m_stages), false);
            }
        }

        // if (result.srcQueueFamilyIndex != VK_QUEUE_FAMILY_IGNORED)
        // {
        //     // At the end, before returning:
        //     NES_LOG("Created barrier:");
        //     NES_LOG("  oldLayout={}, newLayout={}", static_cast<int>(result.oldLayout), static_cast<int>(result.newLayout));
        //     NES_LOG("  srcStage=0x{:x}, dstStage=0x{:x}", static_cast<uint64_t>(result.srcStageMask), static_cast<uint64_t>(result.dstStageMask));
        //     NES_LOG("  srcAccess=0x{:x}, dstAccess=0x{:x}", static_cast<uint64_t>(result.srcAccessMask), static_cast<uint64_t>(result.dstAccessMask));
        //     NES_LOG("  srcQueue={}, dstQueue={}", result.srcQueueFamilyIndex, result.dstQueueFamilyIndex);
        //     NES_LOG("  subresource: baseMip={}, mipCount={}, baseLayer={}, layerCount={}", 
        //      result.subresourceRange.baseMipLevel,
        //      result.subresourceRange.levelCount,
        //      result.subresourceRange.baseArrayLayer,
        //      result.subresourceRange.layerCount);
        // }
        
        return result;
    }
}
