// GBuffer.cpp
#include "GBuffer.h"

#include <algorithm>

#include "Renderer.h"

namespace nes
{
    GBufferDesc& GBufferDesc::SetColors(const std::vector<EFormat>& formats)
    {
        m_colorFormats = formats;
        return *this;
    }

    GBufferDesc& GBufferDesc::SetDepth(EFormat depthFormat)
    {
        m_depthFormat = depthFormat;
        return *this;
    }

    GBufferDesc& GBufferDesc::SetMaxSampleCount()
    {
        RenderDevice& device = Renderer::GetDevice();
        
        uint32 maxSampleCount = 1;

        // Color formats
        for (auto& format : m_colorFormats)
        {
            const EFormatFeatureBits features = device.GetFormatFeatures(format);
            maxSampleCount = std::max(GetMaxSampleCount(features), maxSampleCount);
        }

        // Depth
        if (m_depthFormat != EFormat::Unknown)
        {
            const EFormatFeatureBits features = device.GetFormatFeatures(m_depthFormat);
            maxSampleCount = std::max(GetMaxSampleCount(features), maxSampleCount);
        }

        m_sampleCount = maxSampleCount;

        return *this;
    }

    GBuffer::GBuffer(GBuffer&& other) noexcept
        : m_colorImageAndViews(std::move(other.m_colorImageAndViews))
        , m_depthImageAndView(std::move(other.m_depthImageAndView))
        , m_size(other.m_size)
        , m_desc(std::move(other.m_desc))
    {
        other.m_size = UInt2::Zero();
    }

    GBuffer& GBuffer::operator=(std::nullptr_t)
    {
        Destroy();
        return *this;
    }

    GBuffer& GBuffer::operator=(GBuffer&& other) noexcept
    {
        if (this != &other)
        {
            Destroy();

            m_colorImageAndViews = std::move(other.m_colorImageAndViews);
            m_depthImageAndView = std::move(other.m_depthImageAndView);
            m_desc = std::move(other.m_desc);
            m_size = other.m_size;
            
            other.m_size = UInt2::Zero();
        }

        return *this;
    }

    GBuffer::~GBuffer()
    {
        Destroy();
    }

    GBuffer::GBuffer(const GBufferDesc& desc)
        : m_desc(desc)
    {
        m_colorImageAndViews.reserve(desc.m_colorFormats.size());
    }

    void GBuffer::Resize(RenderDevice& device, const uint32 width, const uint32 height)
    {
        FreeResources();
        m_colorImageAndViews.resize(m_desc.m_colorFormats.size());

        // Create the color resources:
        for (size_t i = 0; i < m_desc.m_colorFormats.size(); ++i)
        {
            // Create the image desc:
            nes::ImageDesc imageDesc{};
            imageDesc.m_mipCount = 1;
            imageDesc.m_format = m_desc.m_colorFormats[i];
            imageDesc.m_layerCount = 1;
            imageDesc.m_sampleCount = m_desc.m_sampleCount;
            imageDesc.m_type = nes::EImageType::Image2D;
            imageDesc.m_usage = nes::EImageUsageBits::ColorAttachment;
            imageDesc.m_width = width;
            imageDesc.m_height = height;
            imageDesc.m_depth = 1;

            // Allocate the image
            nes::AllocateImageDesc allocDesc;
            allocDesc.m_desc = imageDesc;
            allocDesc.m_memoryLocation = nes::EMemoryLocation::Device;
            m_colorImageAndViews[i].m_image = nes::DeviceImage(device, allocDesc);

            // Create the image descriptor (image view):
            nes::Image2DViewDesc imageViewDesc{};
            imageViewDesc.m_format = m_desc.m_colorFormats[i];
            imageViewDesc.m_pImage = &m_colorImageAndViews[i].m_image;
            imageViewDesc.m_viewType = nes::EImage2DViewType::ColorAttachment;
            m_colorImageAndViews[i].m_view = nes::Descriptor(device, imageViewDesc);
        }

        // Create the depth image
        if (m_desc.m_depthFormat != EFormat::Unknown)
        {
            // Depth Image:
            nes::ImageDesc imageDesc{};
            imageDesc.m_mipCount = 1;
            imageDesc.m_format = m_desc.m_depthFormat;
            imageDesc.m_layerCount = 1;
            imageDesc.m_sampleCount = m_desc.m_sampleCount;
            imageDesc.m_type = nes::EImageType::Image2D;
            imageDesc.m_usage = nes::EImageUsageBits::DepthStencilAttachment;
            imageDesc.m_width = width;
            imageDesc.m_height = height;
            imageDesc.m_depth = 1;

            // Allocate the image
            nes::AllocateImageDesc allocDesc;
            allocDesc.m_desc = imageDesc;
            allocDesc.m_memoryLocation = nes::EMemoryLocation::Device;
            m_depthImageAndView.m_image = nes::DeviceImage(device, allocDesc);

            // Create the image descriptor (image view):
            nes::Image2DViewDesc imageViewDesc{};
            imageViewDesc.m_format = m_desc.m_depthFormat;
            imageViewDesc.m_pImage = &m_depthImageAndView.m_image;
            imageViewDesc.m_viewType = nes::EImage2DViewType::DepthStencilAttachment;
            m_depthImageAndView.m_view = nes::Descriptor(device, imageViewDesc);
        }
    }

    DeviceImage& GBuffer::GetColorImage(uint32 colorIndex)
    {
        NES_ASSERT(colorIndex < m_colorImageAndViews.size());
        return m_colorImageAndViews[colorIndex].m_image;
    }

    Descriptor& GBuffer::GetColorImageView(const uint32 colorIndex)
    {
        NES_ASSERT(colorIndex < m_colorImageAndViews.size());
        return m_colorImageAndViews[colorIndex].m_view;
    }

    EFormat GBuffer::GetColorFormat(const uint32 colorIndex) const
    {
        NES_ASSERT(colorIndex < m_colorImageAndViews.size());
        return m_colorImageAndViews[colorIndex].m_image.GetDesc().m_format;
    }

    void GBuffer::FreeResources()
    {
        m_colorImageAndViews.clear();
        m_depthImageAndView = {};
    }

    void GBuffer::Destroy()
    {
        FreeResources();
        m_desc = GBufferDesc{};
        m_size = UInt2::Zero();
    }
}
