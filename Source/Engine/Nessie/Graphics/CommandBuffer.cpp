// CommandBuffer.cpp
#include "CommandBuffer.h"

#include "RenderDevice.h"
#include "CommandPool.h"
#include "Descriptor.h"
#include "DeviceImage.h"
#include "Pipeline.h"

namespace nes
{
    EGraphicsResult CommandBuffer::Init(CommandPool* pPool, vk::raii::CommandBuffer&& cmdBuffer)
    {
        NES_ASSERT(pPool != nullptr);
        
        m_pCommandPool = pPool;
        m_buffer = std::move(cmdBuffer);

        return EGraphicsResult::Success;
    }

    void CommandBuffer::SetDebugName(const std::string& name)
    {
        vk::CommandBuffer buffer = *m_buffer;
        m_device.SetDebugNameToTrivialObject(buffer, name);
    }

    void CommandBuffer::Begin()
    {
        constexpr vk::CommandBufferBeginInfo beginInfo = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
            .setPInheritanceInfo(nullptr); // Only used if this is a secondary command buffer.
        
        m_buffer.begin(beginInfo);

        m_pPipeline = nullptr;
        m_pPipelineLayout = nullptr;
    }

    void CommandBuffer::End()
    {
        m_buffer.end();
    }

    void CommandBuffer::BeginCommandLabel([[maybe_unused]] const std::string& label, [[maybe_unused]] const LinearColor& color)
    {
    #if NES_DEBUG
        vk::DebugUtilsLabelEXT debugLabel = vk::DebugUtilsLabelEXT()
            .setPLabelName(label.c_str())
            .setColor({color.r, color.g, color.b, color.a});
        
        m_buffer.beginDebugUtilsLabelEXT(debugLabel);
    #endif
    }

    void CommandBuffer::EndCommandLabel()
    {
    #if NES_DEBUG
        m_buffer.endDebugUtilsLabelEXT();
    #endif
    }

    void CommandBuffer::TransitionImageLayout(const vk::Image image, ImageMemoryBarrierDesc& barrierDesc) const
    {
        vk::ImageMemoryBarrier2 barrier = barrierDesc.CreateVkBarrier(image);

        vk::DependencyInfo dependencyInfo = vk::DependencyInfo()
            .setDependencyFlags({})
            .setImageMemoryBarrierCount(1)
            .setPImageMemoryBarriers(&barrier);

        m_buffer.pipelineBarrier2(dependencyInfo);
    }

    void CommandBuffer::BeginRendering(const RenderTargetsDesc& targetsDesc)
    {
        NES_ASSERT(targetsDesc.HasTargets());
        
        const DeviceDesc& deviceDesc = m_device.GetDesc();
        
        // From NRI:
        //  If there are no attachments, the render area is maxed.
        //  This is suboptimal, even on desktop. It's a no-go for tiled architectures.
        m_renderLayerCount = deviceDesc.m_dimensions.m_maxAttachmentLayerCount;
        m_renderWidth = deviceDesc.m_dimensions.m_maxDimensionAttachment;
        m_renderHeight = deviceDesc.m_dimensions.m_maxDimensionAttachment;

        // Color Attachments:
        std::vector<vk::RenderingAttachmentInfo> colors(targetsDesc.m_colors.size());
        for (uint32_t i = 0; i < targetsDesc.m_colors.size(); ++i)
        {
            const Descriptor& descriptor = *(*(targetsDesc.m_colors.begin() + i));
            const DescriptorImageDesc& desc = descriptor.GetImageDesc();

            vk::RenderingAttachmentInfo& color = colors[i];
            color.setImageView(descriptor.GetVkImageView())
                .setImageLayout(desc.m_imageLayout)
                .setResolveMode(vk::ResolveModeFlagBits::eNone)
                .setResolveImageView(nullptr)
                .setResolveImageLayout(vk::ImageLayout::eUndefined)
                .setLoadOp(vk::AttachmentLoadOp::eLoad)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setClearValue({});

            const uint16 width = desc.m_pImage->GetSize(0, desc.m_mipOffset);
            const uint16 height = desc.m_pImage->GetSize(1, desc.m_mipOffset);

            m_renderLayerCount = math::Min(m_renderLayerCount, desc.m_layerCount);
            m_renderWidth = math::Min(m_renderWidth, width);
            m_renderHeight = math::Min(m_renderHeight, height);
        }
        
        // Depth-Stencil Attachment:
        vk::RenderingAttachmentInfo depthStencil{};
        bool hasStencil = false;
        if (targetsDesc.m_pDepthStencil != nullptr)
        {
            const Descriptor& descriptor = *targetsDesc.m_pDepthStencil;
            const DescriptorImageDesc& desc = descriptor.GetImageDesc();

            depthStencil.setImageView(descriptor.GetVkImageView())
                .setImageLayout(desc.m_imageLayout)
                .setResolveMode(vk::ResolveModeFlagBits::eNone)
                .setResolveImageView(nullptr)
                .setResolveImageLayout(vk::ImageLayout::eUndefined)
                .setLoadOp(vk::AttachmentLoadOp::eLoad)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setClearValue({});

            const uint16 width = desc.m_pImage->GetSize(0, desc.m_mipOffset);
            const uint16 height = desc.m_pImage->GetSize(1, desc.m_mipOffset);
            
            m_renderLayerCount = math::Min(m_renderLayerCount, desc.m_layerCount);
            m_renderWidth = math::Min(m_renderWidth, width);
            m_renderHeight = math::Min(m_renderHeight, height);

            const FormatProps& props = GetFormatProps(desc.m_pImage->GetDesc().m_format);
            hasStencil = props.m_isStencil != 0;

            m_depthStencil = &descriptor;
        }
        else
        {
            m_depthStencil = nullptr;
        }
        
        // [TODO]: Shading Rate Attachment
        
        if (!targetsDesc.HasTargets())
            m_renderLayerCount = 1;

        vk::RenderingInfo renderingInfo = vk::RenderingInfo()
            .setFlags({}) // No flags.
            .setRenderArea({ {0, 0}, {m_renderWidth, m_renderHeight}})
            .setLayerCount(m_renderLayerCount)
            //.setViewMask(attachments.m_viewMask)
            .setColorAttachmentCount(targetsDesc.m_colors.size())
            .setPColorAttachments(colors.data())
            .setPDepthAttachment(targetsDesc.m_pDepthStencil ? &depthStencil : nullptr)
            .setPStencilAttachment(hasStencil ? &depthStencil : nullptr);

        // [TODO]: Add optional Shading Rate Attachment
        // if (attachments.pShadingRate)
        //     renderingInfo.pNext = &shadingRate;

        m_buffer.beginRendering(renderingInfo);

        // [TODO]: Set the view mask.
        //m_viewMask = attachments.m_viewMask; 
    }

    void CommandBuffer::EndRendering()
    {
        m_buffer.endRendering();
        m_depthStencil = nullptr;
    }

    void CommandBuffer::ClearRenderTargets(const vk::ArrayProxy<ClearDesc>& clearDescs, const vk::ArrayProxy<vk::Rect2D>& clearRegions)
    {
        if (clearDescs.empty())
            return;

        // Create the array of clear attachments.
        uint32 attachmentCount = 0;
        std::vector<vk::ClearAttachment> attachments(clearDescs.size());

        for (auto& clearDesc : clearDescs)
        {
            // If depth aspect set, ensure the depth-stencil attachment is writeable.
            if (clearDesc.m_aspect & vk::ImageAspectFlagBits::eDepth && !m_depthStencil->IsDepthWritable())
                continue;

            // If stencil aspect set, ensure the depth-stencil attachment is writeable.
            if (clearDesc.m_aspect & vk::ImageAspectFlagBits::eStencil && !m_depthStencil->IsStencilWritable())
                continue;

            vk::ClearAttachment& clearAttachment = attachments[attachmentCount++];
            clearAttachment.setAspectMask(clearDesc.m_aspect)
                .setColorAttachment(clearDesc.m_colorAttachmentIndex)
                .setClearValue(clearDesc.m_clearValue);
        }

        // No valid attachments found.
        if (attachmentCount == 0)
            return;

        // Create the array of rect regions to clear.
        uint32 rectCount = clearRegions.size();
        const bool hasRects = rectCount != 0;
        if (!hasRects)
            rectCount = 1;

        const vk::Rect2D fullRect = vk::Rect2D({0, 0}, {m_renderWidth, m_renderHeight});

        std::vector<vk::ClearRect> clearRects(rectCount);
        for (uint32 i = 0; i < rectCount; ++i)
        {
            const vk::Rect2D region = hasRects? *(clearRegions.begin() + i) : fullRect;
            
            vk::ClearRect& clearRect = clearRects[i];
            clearRect = vk::ClearRect()
                .setBaseArrayLayer(0)
                .setLayerCount(m_renderLayerCount)
                .setRect(region);
        }
        
        m_buffer.clearAttachments(attachments, clearRects);
    }

    void CommandBuffer::BindPipeline(const Pipeline& pipeline)
    {
        m_pPipeline = &pipeline;
        m_buffer.bindPipeline(pipeline.GetBindPoint(), pipeline.GetVkPipeline());
    }

    void CommandBuffer::BindPipelineLayout(const PipelineLayout& pipelineLayout)
    {
        m_pPipelineLayout = &pipelineLayout;
    }

    void CommandBuffer::SetViewports(const vk::ArrayProxy<Viewport>& viewports)
    {
        std::vector<vk::Viewport> vkViewports(viewports.size());
        for (uint32 i = 0; i < viewports.size(); ++i)
        {
            vkViewports[i] = *(viewports.begin() + i);
        }
        
        m_buffer.setViewport(0, vkViewports);
    }

    void CommandBuffer::SetScissors(const vk::ArrayProxy<Scissor>& scissors)
    {
        std::vector<vk::Rect2D> vkScissors(scissors.size());
        for (uint32 i = 0; i < scissors.size(); ++i)
        {
            vkScissors[i] = *(scissors.begin() + i);
        }
        
        m_buffer.setScissor(0, vkScissors);
    }

    void CommandBuffer::Draw(const DrawDesc& draw)
    {
        m_buffer.draw(draw.m_vertexCount, draw.m_instanceCount, draw.m_firstVertex, draw.m_firstInstance);
    }

    void CommandBuffer::DrawIndexed(const DrawIndexedDesc& draw)
    {
        m_buffer.drawIndexed(draw.m_indexCount, draw.m_instanceCount, draw.m_firstIndex, static_cast<int32>(draw.m_firstVertex), draw.m_firstInstance);
    }

    CommandBuffer::ScopedCommandLabel::ScopedCommandLabel(CommandBuffer& buffer, const std::string& label, const LinearColor& color)
        : m_commandBuffer(&buffer)
    {
        m_commandBuffer->BeginCommandLabel(label, color);
    }

    CommandBuffer::ScopedCommandLabel::~ScopedCommandLabel()
    {
        m_commandBuffer->EndCommandLabel();
    }
}
