// Barriers.cpp
#include "Barriers.h"

namespace nes
{
    ImageMemoryBarrierDesc& ImageMemoryBarrierDesc::SetLayoutTransition(const VkImageLayout oldLayout, const VkImageLayout newLayout)
    {
        m_oldLayout = oldLayout;
        m_newLayout = newLayout;
        return *this;
    }

    ImageMemoryBarrierDesc& ImageMemoryBarrierDesc::SetAccessFlags(const VkAccessFlags2 srcAccess, const VkAccessFlags2 dstAccess)
    {
        m_srcAccessMask = srcAccess;
        m_dstAccessMask = dstAccess;
        return *this;
    }

    ImageMemoryBarrierDesc& ImageMemoryBarrierDesc::SetStages(const VkPipelineStageFlags2 srcStages, const VkPipelineStageFlags dstStages)
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

    EGraphicsResult ImageMemoryBarrierDesc::CreateBarrier(const VkImage image, VkImageMemoryBarrier2& outBarrier)
    {
        if (!image)
        {
            outBarrier = {};
            return EGraphicsResult::Failure;
        }
        
        if (m_srcStageMask == vulkan::kInferBarrierParams && m_srcAccessMask == vulkan::kInferBarrierParams)
        {
            vulkan::InferPipelineStageAccess(m_oldLayout, m_srcStageMask, m_srcAccessMask);
        }
        else if (m_srcAccessMask == vulkan::kInferBarrierParams)
        {
            NES_ASSERT(m_srcStageMask != vulkan::kInferBarrierParams);
            m_srcAccessMask = vulkan::InferAccessMaskFromStage(m_srcStageMask, false);
        }

        if (m_dstStageMask == vulkan::kInferBarrierParams && m_dstAccessMask == vulkan::kInferBarrierParams)
        {
            vulkan::InferPipelineStageAccess(m_newLayout, m_dstStageMask, m_dstAccessMask);
        }
        else if (m_dstAccessMask == vulkan::kInferBarrierParams)
        {
            NES_ASSERT(m_dstStageMask != vulkan::kInferBarrierParams);
            m_dstAccessMask = vulkan::InferAccessMaskFromStage(m_dstStageMask, false);
        }
        
        outBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        outBarrier.pNext = nullptr;
        outBarrier.image = image;
        outBarrier.oldLayout = m_oldLayout;
        outBarrier.newLayout = m_newLayout;
        outBarrier.subresourceRange = m_subresourceRange;
        outBarrier.srcAccessMask = m_srcAccessMask;
        outBarrier.dstAccessMask = m_dstAccessMask;
        outBarrier.srcStageMask = m_srcStageMask;
        outBarrier.dstStageMask = m_dstStageMask;
        outBarrier.srcQueueFamilyIndex = m_srcQueueFamilyIndex;
        outBarrier.dstQueueFamilyIndex = m_dstQueueFamilyIndex;
        
        return EGraphicsResult::Success;
    }

    BufferMemoryBarrierDesc& BufferMemoryBarrierDesc::SetBuffer(VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize size)
    {
        m_buffer = buffer;
        m_offset = offset;
        m_size = size;
        return *this;
    }

    BufferMemoryBarrierDesc& BufferMemoryBarrierDesc::SetAccessFlags(const VkAccessFlags2 srcAccess, const VkAccessFlags2 dstAccess)
    {
        m_srcAccessMask = srcAccess;
        m_dstAccessMask = dstAccess;
        return *this;
    }

    BufferMemoryBarrierDesc& BufferMemoryBarrierDesc::SetStages(const VkPipelineStageFlags2 srcStages, const VkPipelineStageFlags dstStages)
    {
        m_srcStageMask = srcStages;
        m_dstStageMask = dstStages;
        return *this;
    }

    BufferMemoryBarrierDesc& BufferMemoryBarrierDesc::SetQueueFamilyIndexTransition(const uint32 srcQueueFamilyIndex, const uint32 dstQueueFamilyIndex)
    {
        m_srcQueueFamilyIndex = srcQueueFamilyIndex;
        m_dstQueueFamilyIndex = dstQueueFamilyIndex;
        return *this;
    }

    EGraphicsResult BufferMemoryBarrierDesc::Resolve(VkBufferMemoryBarrier2& outBarrier)
    {
        if (!m_buffer)
        {
            outBarrier = {};
            return EGraphicsResult::Failure;
        }

        if (m_srcAccessMask == vulkan::kInferBarrierParams)
        {
            m_srcAccessMask = vulkan::InferAccessMaskFromStage(m_srcStageMask, false);
        }
        
        if (m_dstAccessMask == vulkan::kInferBarrierParams)
        {
            m_dstAccessMask = vulkan::InferAccessMaskFromStage(m_dstStageMask, true);
        }

        outBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        outBarrier.pNext = nullptr;
        outBarrier.buffer = m_buffer;
        outBarrier.offset = m_offset;
        outBarrier.size = m_size;
        outBarrier.srcAccessMask = m_srcAccessMask;
        outBarrier.dstAccessMask = m_dstAccessMask;
        outBarrier.srcStageMask = m_srcStageMask;
        outBarrier.dstStageMask = m_dstStageMask;
        outBarrier.srcQueueFamilyIndex = m_srcQueueFamilyIndex;
        outBarrier.dstQueueFamilyIndex = m_dstQueueFamilyIndex;
        
        return EGraphicsResult::Success;
    }
}
