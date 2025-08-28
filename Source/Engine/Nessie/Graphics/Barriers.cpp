// Barriers.cpp
#include "Barriers.h"
#include "Vulkan/VulkanConversions.h"

namespace nes
{
    ImageMemoryBarrierDesc& ImageMemoryBarrierDesc::SetLayoutTransition(const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout)
    {
        m_oldLayout = oldLayout;
        m_newLayout = newLayout;
        return *this;
    }

    ImageMemoryBarrierDesc& ImageMemoryBarrierDesc::SetImageSubresourceRange(const vk::ImageAspectFlags aspectFlags, const uint32 mipLevelCount, const uint32 baseMipLevel, const uint32 layerCount, const uint32 baseLayer)
    {
        m_subresourceRange.setAspectMask(aspectFlags)
            .setBaseMipLevel(baseMipLevel)
            .setLevelCount(mipLevelCount)
            .setBaseArrayLayer(baseLayer)
            .setLayerCount(layerCount);

        return *this;
    }

    ImageMemoryBarrierDesc& ImageMemoryBarrierDesc::SetAccessFlags(const vk::AccessFlags2 srcAccess, const vk::AccessFlags2 dstAccess)
    {
        m_srcAccessMask = srcAccess;
        m_dstAccessMask = dstAccess;
        return *this;
    }

    ImageMemoryBarrierDesc& ImageMemoryBarrierDesc::SetStages(const vk::PipelineStageFlags2 srcStages, const vk::PipelineStageFlags2 dstStages)
    {
        m_srcStageMask = srcStages;
        m_dstStageMask = dstStages;
        return *this;
    }

    ImageMemoryBarrierDesc& ImageMemoryBarrierDesc::SetQueueFamilyIndexTransition(const uint32 srcQueueFamilyIndex, const uint32 dstQueueFamilyIndex)
    {
        m_srcQueueFamilyIndex = srcQueueFamilyIndex;
        m_dstQueueFamilyIndex = dstQueueFamilyIndex;
        return *this;
    }

    vk::ImageMemoryBarrier2 ImageMemoryBarrierDesc::CreateVkBarrier(const vk::Image image)
    {
        NES_ASSERT(image);
        
        if (m_srcStageMask == graphics::kInferPipelineStage && m_srcAccessMask == graphics::kInferAccess)
        {
            graphics::InferPipelineStageAccess(m_oldLayout, m_srcStageMask, m_srcAccessMask);
        }
        else if (m_srcAccessMask == graphics::kInferAccess)
        {
            NES_ASSERT(m_srcStageMask != graphics::kInferPipelineStage);
            m_srcAccessMask = graphics::InferAccessMaskFromStage(m_srcStageMask, false);
        }

        if (m_dstStageMask == graphics::kInferPipelineStage && m_dstAccessMask == graphics::kInferAccess)
        {
            graphics::InferPipelineStageAccess(m_newLayout, m_dstStageMask, m_dstAccessMask);
        }
        else if (m_dstAccessMask == graphics::kInferAccess)
        {
            NES_ASSERT(m_dstStageMask != graphics::kInferPipelineStage);
            m_dstAccessMask = graphics::InferAccessMaskFromStage(m_dstStageMask, false);
        }

        // [TODO]: Custom subresource range.
        constexpr vk::ImageSubresourceRange kDefaultSubresourceRange = vk::ImageSubresourceRange()
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1);
        
        // Set the Barrier params.
        const vk::ImageMemoryBarrier2 outBarrier = vk::ImageMemoryBarrier2()
            .setImage(image)
            .setSubresourceRange(kDefaultSubresourceRange)
            .setOldLayout(m_oldLayout)
            .setNewLayout(m_newLayout)
            .setSrcStageMask(m_srcStageMask)
            .setDstStageMask(m_dstStageMask)
            .setSrcAccessMask(m_srcAccessMask)
            .setDstAccessMask(m_dstAccessMask)
            .setSrcQueueFamilyIndex(m_srcQueueFamilyIndex)
            .setDstQueueFamilyIndex(m_dstQueueFamilyIndex);

        return outBarrier;
    }

    // BufferMemoryBarrierDesc& BufferMemoryBarrierDesc::SetBuffer(VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize size)
    // {
    //     m_buffer = buffer;
    //     m_offset = offset;
    //     m_size = size;
    //     return *this;
    // }
    //
    // BufferMemoryBarrierDesc& BufferMemoryBarrierDesc::SetAccessFlags(const VkAccessFlags2 srcAccess, const VkAccessFlags2 dstAccess)
    // {
    //     m_srcAccessMask = srcAccess;
    //     m_dstAccessMask = dstAccess;
    //     return *this;
    // }
    //
    // BufferMemoryBarrierDesc& BufferMemoryBarrierDesc::SetStages(const VkPipelineStageFlags2 srcStages, const VkPipelineStageFlags dstStages)
    // {
    //     m_srcStageMask = srcStages;
    //     m_dstStageMask = dstStages;
    //     return *this;
    // }
    //
    // BufferMemoryBarrierDesc& BufferMemoryBarrierDesc::SetQueueFamilyIndexTransition(const uint32 srcQueueFamilyIndex, const uint32 dstQueueFamilyIndex)
    // {
    //     m_srcQueueFamilyIndex = srcQueueFamilyIndex;
    //     m_dstQueueFamilyIndex = dstQueueFamilyIndex;
    //     return *this;
    // }
    //
    // EGraphicsResult BufferMemoryBarrierDesc::Resolve(VkBufferMemoryBarrier2& outBarrier)
    // {
    //     if (!m_buffer)
    //     {
    //         outBarrier = {};
    //         return EGraphicsResult::Failure;
    //     }
    //
    //     if (m_srcAccessMask == graphics::kInferBarrierParams)
    //     {
    //         m_srcAccessMask = graphics::InferAccessMaskFromStage(m_srcStageMask, false);
    //     }
    //     
    //     if (m_dstAccessMask == graphics::kInferBarrierParams)
    //     {
    //         m_dstAccessMask = graphics::InferAccessMaskFromStage(m_dstStageMask, true);
    //     }
    //
    //     outBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    //     outBarrier.pNext = nullptr;
    //     outBarrier.buffer = m_buffer;
    //     outBarrier.offset = m_offset;
    //     outBarrier.size = m_size;
    //     outBarrier.srcAccessMask = m_srcAccessMask;
    //     outBarrier.dstAccessMask = m_dstAccessMask;
    //     outBarrier.srcStageMask = m_srcStageMask;
    //     outBarrier.dstStageMask = m_dstStageMask;
    //     outBarrier.srcQueueFamilyIndex = m_srcQueueFamilyIndex;
    //     outBarrier.dstQueueFamilyIndex = m_dstQueueFamilyIndex;
    //     
    //     return EGraphicsResult::Success;
    // }
}
