// RenderTarget.cpp
#include "RenderTarget.h"
#include "RenderDevice.h"

namespace nes
{
    RenderTarget::RenderTarget(RenderTarget&& other) noexcept
        : m_pDevice(other.m_pDevice)
        , m_image(std::move(other.m_image))
        , m_view(std::move(other.m_view))
        , m_desc(std::move(other.m_desc))
    {
        // The image pointer is invalidated due to the move above.
        // We have to reset that pointer.
        m_view.GetImageDesc().m_pImage = &m_image;
    }

    RenderTarget& RenderTarget::operator=(std::nullptr_t)
    {
        Destroy();
        return *this;
    }

    RenderTarget& RenderTarget::operator=(RenderTarget&& other) noexcept
    {
        if (this != &other)
        {
            Destroy();

            m_pDevice = other.m_pDevice;
            m_image = std::move(other.m_image);
            m_view = std::move(other.m_view);

            // The image pointer is invalidated due to the move above.
            // We have to reset that pointer.
            m_view.GetImageDesc().m_pImage = &m_image;
            
            m_desc = std::move(other.m_desc);

            other.m_pDevice = nullptr;
        }

        return *this;
    }

    RenderTarget::~RenderTarget()
    {
        Destroy();
    }

    RenderTarget::RenderTarget(RenderDevice& device, const RenderTargetDesc& desc)
        : m_pDevice(&device)
        , m_desc(desc)
    {
        // Ensure that the sample count is valid.
        if (desc.m_sampleCount == 0)
            SetMaxSupportedSampleCount();
        else
            SetSampleCount(desc.m_sampleCount);
        
        // Resize the Target to the given size.
        Resize(desc.m_size.x, desc.m_size.y);
    }

    void RenderTarget::Resize(const uint32 width, const uint32 height)
    {
        FreeResources();

        const bool isColor = (m_desc.m_planes & EImagePlaneBits::Color) != 0;
        
        // Create the image desc:
        nes::ImageDesc imageDesc{};
        imageDesc.m_mipCount = 1;
        imageDesc.m_format = m_desc.m_format;
        imageDesc.m_layerCount = 1;
        imageDesc.m_sampleCount = m_desc.m_sampleCount;
        imageDesc.m_type = nes::EImageType::Image2D;
        imageDesc.m_usage = m_desc.m_usage;
        imageDesc.m_width = width;
        imageDesc.m_height = height;
        imageDesc.m_depth = 1;

        // Allocate the image
        nes::AllocateImageDesc allocDesc;
        allocDesc.m_imageDesc = imageDesc;
        allocDesc.m_memoryLocation = nes::EMemoryLocation::Device;
        m_image = nes::DeviceImage(*m_pDevice, allocDesc);

        // Create the attachment view.
        nes::Image2DViewDesc imageViewDesc{};
        imageViewDesc.m_format = m_desc.m_format;
        imageViewDesc.m_pImage = &m_image;
        imageViewDesc.m_viewType = isColor? EImage2DViewType::ColorAttachment : EImage2DViewType::DepthStencilAttachment;
        m_view = nes::Descriptor(*m_pDevice, imageViewDesc);

        // Set Debug Names
        m_image.SetDebugName(m_desc.m_name + " Image");
        m_view.SetDebugName(m_desc.m_name + " View");

        m_desc.m_size.x = width;
        m_desc.m_size.y = height;
    }

    float RenderTarget::GetAspectRatio() const
    {
        return (static_cast<float>(m_desc.m_size.x) / static_cast<float>(m_desc.m_size.y));
    }

    void RenderTarget::SetMaxSupportedSampleCount()
    {
        const EFormatFeatureBits features = m_pDevice->GetFormatFeatures(m_desc.m_format);
        m_desc.m_sampleCount = GetMaxSampleCount(features);
    }

    void RenderTarget::SetSampleCount(const uint32 sampleCount)
    {
        const EFormatFeatureBits features = m_pDevice->GetFormatFeatures(m_desc.m_format);
        m_desc.m_sampleCount = std::min(GetMaxSampleCount(features), sampleCount);
    }

    bool RenderTarget::IsColorTarget() const
    {
        return (m_desc.m_planes & EImagePlaneBits::Color) != 0;
    }

    bool RenderTarget::IsDepthTarget() const
    {
        return (m_desc.m_planes & EImagePlaneBits::Depth) != 0;
    }

    bool RenderTarget::IsStencilTarget() const
    {
        return (m_desc.m_planes & EImagePlaneBits::Stencil) != 0;
    }

    void RenderTarget::FreeResources()
    {
        m_image = nullptr;
        m_view = nullptr;
    }

    void RenderTarget::Destroy()
    {
        FreeResources();
        m_desc = {};
    }

    uint32 GetMaxSampleCountForTargets(const std::vector<RenderTarget*>& targets)
    {
        uint32 maxSamples = 1;
        for (auto& pTarget : targets)
        {
            if (pTarget)
                maxSamples = math::Max(maxSamples, pTarget->GetSampleCount());
        }
        return maxSamples;
    }
}
