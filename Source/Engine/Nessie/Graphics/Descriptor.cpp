// Descriptor.cpp
#include "Descriptor.h"

#include "Texture.h"
#include "DeviceBuffer.h"
#include "RenderDevice.h"
#include "Renderer.h"

namespace nes
{
    Descriptor::Descriptor(Descriptor&& other) noexcept
        : m_pDevice(other.m_pDevice)
        , m_type(other.m_type)
    {
        if (other.IsBufferType())
        {
            m_bufferView = std::move(other.m_bufferView);
            m_bufferDesc = other.m_bufferDesc;
        }

        else if (other.IsImageType())
        {
            m_imageView = std::move(other.m_imageView);
            m_imageDesc = other.m_imageDesc;
        }

        else if (other.m_type == EDescriptorType::Sampler)
        {
            m_sampler = std::move(other.m_sampler);
        }

        other.m_type = EDescriptorType::None;
        other.m_pDevice = nullptr;
    }

    Descriptor& Descriptor::operator=(std::nullptr_t)
    {
        FreeDescriptor();
        return *this;
    }

    Descriptor& Descriptor::operator=(Descriptor&& other) noexcept
    {
        if (this != &other)
        {
            FreeDescriptor();
            
            m_pDevice = other.m_pDevice;
            m_type = other.m_type;

            if (other.IsBufferType())
            {
                m_bufferView = std::move(other.m_bufferView);
                m_bufferDesc = other.m_bufferDesc;
            }

            else if (other.IsImageType())
            {
                m_imageView = std::move(other.m_imageView);
                m_imageDesc = other.m_imageDesc;
            }

            else if (other.m_type == EDescriptorType::Sampler)
            {
                m_sampler = std::move(other.m_sampler);
            }

            other.m_type = EDescriptorType::None;
            other.m_pDevice = nullptr;
        }

        return *this;
    }

    Descriptor::Descriptor(RenderDevice& device, const BufferViewDesc& bufferViewDesc)
        : m_pDevice(&device)
    {
        NES_ASSERT(bufferViewDesc.m_pBuffer != nullptr);
        
        const DeviceBuffer& buffer = *bufferViewDesc.m_pBuffer;
        const BufferDesc& bufferDesc = buffer.GetDesc();

        m_type = EDescriptorType::Buffer;
        m_bufferDesc.m_offset = bufferViewDesc.m_offset;
        m_bufferDesc.m_size = (bufferViewDesc.m_size == graphics::kUseRemaining) ? bufferDesc.m_size : bufferViewDesc.m_size;
        m_bufferDesc.m_pBuffer = bufferViewDesc.m_pBuffer;
        m_bufferDesc.m_viewType = bufferViewDesc.m_viewType;

        if (bufferViewDesc.m_format == EFormat::Unknown)
            return;

        vk::BufferViewCreateInfo info = vk::BufferViewCreateInfo()
            .setBuffer(buffer.GetVkBuffer())
            .setFormat(GetVkFormat(bufferViewDesc.m_format))
            .setOffset(bufferViewDesc.m_offset)
            .setRange(m_bufferDesc.m_size);

        auto& vkDevice = device.GetVkDevice();
        m_bufferView = vkDevice.createBufferView(info, device.GetVkAllocationCallbacks());
    }

    Descriptor::Descriptor(RenderDevice& device, const Image1DViewDesc& imageViewDesc)
        : m_pDevice(&device)
    {
        NES_ASSERT(imageViewDesc.m_pImage);

        const DeviceImage& image = *imageViewDesc.m_pImage;
        const ImageDesc& imageDesc = image.GetDesc();
        const uint32 remainingMips = imageDesc.m_mipCount - imageViewDesc.m_baseMipLevel;
        const uint32 remainingLayers = imageDesc.m_layerCount - imageViewDesc.m_baseLayer;

        vk::ImageViewUsageCreateInfo usageInfo = vk::ImageViewUsageCreateInfo()
            .setUsage(GetVkImageViewUsage(imageViewDesc.m_viewType));

        const vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange()
            .setAspectMask(GetVkImageAspectFlags(imageViewDesc.m_format))
            .setBaseMipLevel(imageViewDesc.m_baseMipLevel)
            .setBaseArrayLayer(imageViewDesc.m_baseLayer)
            .setLevelCount(imageViewDesc.m_mipCount == graphics::kUseRemaining ? remainingMips : imageViewDesc.m_mipCount)
            .setLayerCount(imageViewDesc.m_layerCount == graphics::kUseRemaining ? remainingLayers : imageViewDesc.m_layerCount);
        
        vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
            .setPNext(&usageInfo)
            .setViewType(GetVkImageViewType(imageViewDesc.m_viewType, subresourceRange.layerCount))
            .setImage(image.GetVkImage())
            .setSubresourceRange(subresourceRange)
            .setFormat(GetVkFormat(imageViewDesc.m_format));

        // Create the image view
        auto& vkDevice = device.GetVkDevice();
        m_imageView = vkDevice.createImageView(viewInfo, device.GetVkAllocationCallbacks());

        // Set description values.
        m_type = GetDescriptorType(imageViewDesc.m_viewType);
        m_imageDesc.m_pImage = imageViewDesc.m_pImage;
        m_imageDesc.m_imageLayout = GetVkImageViewLayout(imageViewDesc.m_viewType);
        m_imageDesc.m_aspectFlags = GetVkImageAspectFlags(imageViewDesc.m_format);
        m_imageDesc.m_layerOffset = imageViewDesc.m_baseLayer;
        m_imageDesc.m_layerCount = subresourceRange.layerCount;
        m_imageDesc.m_sliceOffset = 0;
        m_imageDesc.m_sliceCount = 1;
        m_imageDesc.m_mipOffset = imageViewDesc.m_baseMipLevel;
        m_imageDesc.m_mipCount = subresourceRange.levelCount;
    }

    Descriptor::Descriptor(RenderDevice& device, const Image2DViewDesc& imageViewDesc)
        : m_pDevice(&device)
    {
        NES_ASSERT(imageViewDesc.m_pImage);

        const DeviceImage& image = *imageViewDesc.m_pImage;
        const ImageDesc& imageDesc = image.GetDesc();
        const uint32 remainingMips = imageDesc.m_mipCount - imageViewDesc.m_baseMipLevel;
        const uint32 remainingLayers = imageDesc.m_layerCount - imageViewDesc.m_baseLayer;

        vk::ImageViewUsageCreateInfo usageInfo = vk::ImageViewUsageCreateInfo()
            .setUsage(GetVkImageViewUsage(imageViewDesc.m_viewType));

        const vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange()
            .setAspectMask(GetVkImageAspectFlags(imageViewDesc.m_format))
            .setBaseMipLevel(imageViewDesc.m_baseMipLevel)
            .setBaseArrayLayer(imageViewDesc.m_baseLayer)
            .setLevelCount(imageViewDesc.m_mipCount == graphics::kUseRemaining ? remainingMips : imageViewDesc.m_mipCount)
            .setLayerCount(imageViewDesc.m_layerCount == graphics::kUseRemaining ? remainingLayers : imageViewDesc.m_layerCount);
        
        vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
            .setPNext(&usageInfo)
            .setViewType(GetVkImageViewType(imageViewDesc.m_viewType, subresourceRange.layerCount))
            .setImage(image.GetVkImage())
            .setSubresourceRange(subresourceRange)
            .setFormat(GetVkFormat(imageViewDesc.m_format));

        // Create the image view
        auto& vkDevice = device.GetVkDevice();
        m_imageView = vkDevice.createImageView(viewInfo, device.GetVkAllocationCallbacks());
        
        // Set description values.
        m_type = GetDescriptorType(imageViewDesc.m_viewType);
        m_imageDesc.m_pImage = imageViewDesc.m_pImage;
        m_imageDesc.m_imageLayout = GetVkImageViewLayout(imageViewDesc.m_viewType);
        m_imageDesc.m_aspectFlags = GetVkImageAspectFlags(imageViewDesc.m_format);
        m_imageDesc.m_layerOffset = imageViewDesc.m_baseLayer;
        m_imageDesc.m_layerCount = subresourceRange.layerCount;
        m_imageDesc.m_sliceOffset = 0;
        m_imageDesc.m_sliceCount = 1;
        m_imageDesc.m_mipOffset = imageViewDesc.m_baseMipLevel;
        m_imageDesc.m_mipCount = subresourceRange.levelCount;
    }

    Descriptor::Descriptor(RenderDevice& device, const Image3DViewDesc& imageViewDesc)
        : m_pDevice(&device)
    {
        NES_ASSERT(imageViewDesc.m_pImage);

        const DeviceImage& image = *imageViewDesc.m_pImage;
        const ImageDesc& imageDesc = image.GetDesc();
        const uint32 remainingMips = imageDesc.m_mipCount - imageViewDesc.m_baseMipLevel;
        const uint32 remainingLayers = imageDesc.m_layerCount - imageViewDesc.m_baseSlice;

        vk::ImageViewSlicedCreateInfoEXT slicesInfo = vk::ImageViewSlicedCreateInfoEXT()
            .setSliceCount(imageViewDesc.m_sliceCount == graphics::kUseRemaining? remainingLayers : imageViewDesc.m_sliceCount)
            .setSliceOffset(imageViewDesc.m_baseSlice);

        vk::ImageViewUsageCreateInfo usageInfo = vk::ImageViewUsageCreateInfo()
            .setUsage(GetVkImageViewUsage(imageViewDesc.m_viewType));

        if (device.GetDesc().m_features.m_imageSlicedView)
            usageInfo.setPNext(&slicesInfo);

        const vk::ImageSubresourceRange subresourceRange = vk::ImageSubresourceRange()
            .setAspectMask(GetVkImageAspectFlags(imageViewDesc.m_format))
            .setBaseMipLevel(imageViewDesc.m_baseMipLevel)
            .setLevelCount(imageViewDesc.m_mipCount == graphics::kUseRemaining ? remainingMips : imageViewDesc.m_mipCount)
            .setBaseArrayLayer(0)
            .setLayerCount(1);
        
        vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
            .setPNext(&usageInfo)
            .setViewType(GetVkImageViewType(imageViewDesc.m_viewType, subresourceRange.layerCount))
            .setImage(image.GetVkImage())
            .setSubresourceRange(subresourceRange)
            .setFormat(GetVkFormat(imageViewDesc.m_format));

        // Create the image view
        auto& vkDevice = device.GetVkDevice();
        m_imageView = vkDevice.createImageView(viewInfo, device.GetVkAllocationCallbacks());

        // Set description values.
        m_type = GetDescriptorType(imageViewDesc.m_viewType);
        m_imageDesc.m_pImage = imageViewDesc.m_pImage;
        m_imageDesc.m_imageLayout = GetVkImageViewLayout(imageViewDesc.m_viewType);
        m_imageDesc.m_aspectFlags = GetVkImageAspectFlags(imageViewDesc.m_format);
        m_imageDesc.m_layerOffset = 0;
        m_imageDesc.m_layerCount = 1;
        m_imageDesc.m_sliceOffset = imageViewDesc.m_baseSlice;
        m_imageDesc.m_sliceCount = imageViewDesc.m_sliceCount;
        m_imageDesc.m_mipOffset = imageViewDesc.m_baseMipLevel;
        m_imageDesc.m_mipCount = subresourceRange.levelCount;
    }

    Descriptor::Descriptor(RenderDevice& device, const SamplerDesc& samplerDesc)
        : m_pDevice(&device)
    {
        vk::SamplerCreateInfo info = vk::SamplerCreateInfo()
            .setMagFilter(GetVkFilterType(samplerDesc.m_filters.m_mag))
            .setMinFilter(GetVkFilterType(samplerDesc.m_filters.m_min))
            .setMipmapMode(GetVkSamplerMipMode(samplerDesc.m_filters.m_mip))
            .setAddressModeU(GetVkSamplerAddressMode(samplerDesc.m_addressModes.u))
            .setAddressModeV(GetVkSamplerAddressMode(samplerDesc.m_addressModes.v))
            .setAddressModeW(GetVkSamplerAddressMode(samplerDesc.m_addressModes.w))
            .setMipLodBias(samplerDesc.m_mipBias)
            .setAnisotropyEnable(static_cast<float>(samplerDesc.m_anisotropy) > 1.f)
            .setMaxAnisotropy(samplerDesc.m_anisotropy)
            .setCompareEnable(samplerDesc.m_compareOp != ECompareOp::None)
            .setMinLod(samplerDesc.m_mipMin)
            .setMaxLod(samplerDesc.m_mipMax);

        const void** pTail = &info.pNext; 

        vk::SamplerReductionModeCreateInfo reductionModeInfo = vk::SamplerReductionModeCreateInfo();
        if (device.GetDesc().m_features.m_textureFilterMinMax)
        {
            reductionModeInfo.setReductionMode(GetVkSamplerReductionMode(samplerDesc.m_filters.m_reduction));
            *pTail = &reductionModeInfo;
            pTail = &reductionModeInfo.pNext;
        }

        vk::SamplerCustomBorderColorCreateInfoEXT borderColorInfo = vk::SamplerCustomBorderColorCreateInfoEXT();
        if (device.GetDesc().m_features.m_customBorderColor)
        {
            info.setBorderColor(samplerDesc.m_isInteger ? vk::BorderColor::eIntCustomEXT : vk::BorderColor::eFloatCustomEXT);
            static_assert(sizeof(vk::ClearColorValue) == sizeof(samplerDesc.m_borderColor), "Unexpected sizeof");
            memcpy(&borderColorInfo.customBorderColor, &samplerDesc.m_borderColor, sizeof(borderColorInfo.customBorderColor));

            *pTail = &borderColorInfo;
        }

        auto& vkDevice = device.GetVkDevice();
        m_sampler = vkDevice.createSampler(info, device.GetVkAllocationCallbacks());
        m_type = EDescriptorType::Sampler;
    }

    Descriptor::~Descriptor()
    {
        FreeDescriptor();
    }

    void Descriptor::SetDebugName(const std::string& name)
    {
        m_pDevice->SetDebugNameVkObject(GetNativeVkObject(), name);
    }

    bool Descriptor::IsImageType() const
    {
        return DescriptorIsTextureType(m_type);
    }

    bool Descriptor::IsBufferType() const
    {
        return DescriptorIsBufferType(m_type);
    }

    const DescriptorBufferDesc& Descriptor::GetBufferDesc() const
    {
        NES_ASSERT(IsImageType());
        return m_bufferDesc;
    }

    const DescriptorImageDesc& Descriptor::GetImageDesc() const
    {
        NES_ASSERT(IsImageType());
        return m_imageDesc;
    }

    vk::ImageView Descriptor::GetVkImageView() const
    {
        NES_ASSERT(IsImageType());
        return *m_imageView;
    }

    vk::Sampler Descriptor::GetVkSampler() const
    {
        NES_ASSERT(m_type == EDescriptorType::Sampler);
        return *m_sampler;
    }

    vk::BufferView Descriptor::GetVkBufferView() const
    {
        NES_ASSERT(IsBufferType());
        return *m_bufferView;
    }

    vk::DescriptorBufferInfo Descriptor::GetVkBufferInfo() const
    {
        NES_ASSERT(IsBufferType());

        vk::DescriptorBufferInfo info = vk::DescriptorBufferInfo()
            .setBuffer(m_bufferDesc.m_pBuffer->GetVkBuffer())
            .setOffset(m_bufferDesc.m_offset)
            .setRange(m_bufferDesc.m_size);

        return info;
    }

    bool Descriptor::IsDepthWritable() const
    {
        NES_ASSERT(IsImageType());
        return m_imageDesc.m_imageLayout != vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal
            && m_imageDesc.m_imageLayout != vk::ImageLayout::eDepthStencilReadOnlyOptimal;
    }

    bool Descriptor::IsStencilWritable() const
    {
        NES_ASSERT(IsImageType());
        return m_imageDesc.m_imageLayout != vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal
            && m_imageDesc.m_imageLayout != vk::ImageLayout::eDepthStencilReadOnlyOptimal;
    }

    NativeVkObject Descriptor::GetNativeVkObject() const
    {
        if (IsImageType())
        {
            return NativeVkObject(*m_imageView, vk::ObjectType::eImageView);
        }

        else if (IsBufferType())
        {
            return NativeVkObject(*m_bufferView, vk::ObjectType::eBufferView);
        }

        else if (m_type == EDescriptorType::Sampler)
        {
            return NativeVkObject(*m_sampler, vk::ObjectType::eSampler);
        }

        return NativeVkObject();
    }

    void Descriptor::FreeDescriptor()
    {
        if (IsBufferType())
        {
            Renderer::SubmitResourceFree([bufferView = std::move(m_bufferView)]() mutable
            {
                bufferView = nullptr; 
            });
        }

        else if (IsImageType())
        {
            Renderer::SubmitResourceFree([imageView = std::move(m_imageView)]() mutable
            {
                imageView = nullptr; 
            });
        }

        else if (m_type == EDescriptorType::Sampler)
        {
            Renderer::SubmitResourceFree([sampler = std::move(m_sampler)]() mutable
            {
                sampler = nullptr; 
            });
        }

        m_type = EDescriptorType::None;
        m_pDevice = nullptr;
    }
}
