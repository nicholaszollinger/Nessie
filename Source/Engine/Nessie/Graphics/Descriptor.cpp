// Descriptor.cpp
#include "Descriptor.h"
#include "Texture.h"
#include "DeviceBuffer.h"
#include "RenderDevice.h"

namespace nes
{
    Descriptor::~Descriptor()
    {
        if (IsBufferType())
        {
            if (m_bufferView)
                vkDestroyBufferView(m_device, m_bufferView, m_device.GetVkAllocationCallbacks());
        }

        else if (IsTextureType())
        {
            if (m_imageView)
                vkDestroyImageView(m_device, m_imageView, m_device.GetVkAllocationCallbacks());
        }

        else if (m_type == EDescriptorType::Sampler)
        {
            if (m_sampler)
                vkDestroySampler(m_device, m_sampler, m_device.GetVkAllocationCallbacks());
        }
    }

    void Descriptor::SetDebugName(const char* name)
    {
        if (IsBufferType())
        {
            m_device.SetDebugNameToTrivialObject(m_bufferView, name);
        }

        else if (IsTextureType())
        {
            m_device.SetDebugNameToTrivialObject(m_imageView, name);
        }

        else if (m_type == EDescriptorType::Sampler)
        {
            m_device.SetDebugNameToTrivialObject(m_sampler, name);
        }
    }

    EGraphicsResult Descriptor::Init(const BufferViewDesc& bufferViewDesc)
    {
        const DeviceBuffer& buffer = *bufferViewDesc.m_pBuffer;
        const BufferDesc& bufferDesc = buffer.GetDesc();

        m_type = GetDescriptorType(bufferViewDesc.m_viewType);
        m_bufferDesc.m_offset = bufferViewDesc.m_offset;
        m_bufferDesc.m_size = (bufferViewDesc.m_size == graphics::kUseRemaining) ? bufferDesc.m_size : bufferViewDesc.m_size;
        m_bufferDesc.m_handle = buffer.GetHandle();
        m_bufferDesc.m_viewType = bufferViewDesc.m_viewType;

        if (bufferViewDesc.m_format == EFormat::Unknown)
            return EGraphicsResult::Success;

        VkBufferViewCreateInfo info = { VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO };
        info.flags = static_cast<VkBufferViewCreateFlags>(0);
        info.buffer = buffer.GetHandle();
        info.format = GetVkFormat(bufferViewDesc.m_format);
        info.offset = bufferViewDesc.m_offset;
        info.range = m_bufferDesc.m_size;
        NES_VK_FAIL_RETURN(m_device, vkCreateBufferView(m_device, &info, m_device.GetVkAllocationCallbacks(), &m_bufferView));
        
        return EGraphicsResult::Success;
    }

    EGraphicsResult Descriptor::Init(const Texture1DViewDesc& textureViewDesc)
    {
        NES_ASSERT(textureViewDesc.m_pTexture);
        const Texture& texture = *textureViewDesc.m_pTexture;
        const TextureDesc& textureDesc = texture.GetDesc();
        const uint32 remainingMips = textureDesc.m_mipCount - textureViewDesc.m_mipOffset;
        const uint32 remainingLayers = textureDesc.m_layerCount - textureViewDesc.m_layerOffset;
        
        VkImageViewUsageCreateInfo usageInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO };
        usageInfo.usage = GetVkImageViewUsage(textureViewDesc.m_viewType);

        const VkImageSubresourceRange subresource =
        {
            .aspectMask = GetVkImageAspectFlags(textureViewDesc.m_format),
            .baseMipLevel = textureViewDesc.m_mipOffset,
            .levelCount = textureViewDesc.m_mipCount == graphics::kUseRemaining ? remainingMips : textureViewDesc.m_mipCount,
            .baseArrayLayer = textureViewDesc.m_layerOffset,
            .layerCount = textureViewDesc.m_layerCount == graphics::kUseRemaining ? remainingLayers : textureViewDesc.m_layerCount,
        };
        
        VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        createInfo.pNext = &usageInfo;
        createInfo.viewType = GetVkImageViewType(textureViewDesc.m_viewType, subresource.layerCount);
        createInfo.format = GetVkFormat(textureViewDesc.m_format);
        createInfo.subresourceRange = subresource;
        createInfo.image = texture.GetHandle();

        NES_VK_FAIL_RETURN(m_device, vkCreateImageView(m_device, &createInfo, m_device.GetVkAllocationCallbacks(), &m_imageView));

        m_type = GetDescriptorType(textureViewDesc.m_viewType);
        m_textureDesc.m_pTexture = textureViewDesc.m_pTexture;
        m_textureDesc.m_imageLayout = GetVkImageViewLayout(textureViewDesc.m_viewType);
        m_textureDesc.m_aspectFlags = GetVkImageAspectFlags(textureViewDesc.m_format);
        m_textureDesc.m_layerOffset = textureViewDesc.m_layerOffset;
        m_textureDesc.m_layerCount = subresource.layerCount;
        m_textureDesc.m_sliceOffset = 0;
        m_textureDesc.m_sliceCount = 1;
        m_textureDesc.m_mipOffset = textureViewDesc.m_mipOffset;
        m_textureDesc.m_mipCount = subresource.levelCount;
        
        return EGraphicsResult::Success;
    }

    EGraphicsResult Descriptor::Init(const Texture2DViewDesc& textureViewDesc)
    {
        NES_ASSERT(textureViewDesc.m_pTexture);
        const Texture& texture = *textureViewDesc.m_pTexture;
        const TextureDesc& textureDesc = texture.GetDesc();
        const uint32 remainingMips = textureDesc.m_mipCount - textureViewDesc.m_mipOffset;
        const uint32 remainingLayers = textureDesc.m_layerCount - textureViewDesc.m_layerOffset;
        
        VkImageViewUsageCreateInfo usageInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO };
        usageInfo.usage = GetVkImageViewUsage(textureViewDesc.m_viewType);

        const VkImageSubresourceRange subresource =
        {
            .aspectMask = GetVkImageAspectFlags(textureViewDesc.m_format),
            .baseMipLevel = textureViewDesc.m_mipOffset,
            .levelCount = textureViewDesc.m_mipCount == graphics::kUseRemaining ? remainingMips : textureViewDesc.m_mipCount,
            .baseArrayLayer = textureViewDesc.m_layerOffset,
            .layerCount = textureViewDesc.m_layerCount == graphics::kUseRemaining ? remainingLayers : textureViewDesc.m_layerCount,
        };
        
        VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        createInfo.pNext = &usageInfo;
        createInfo.viewType = GetVkImageViewType(textureViewDesc.m_viewType, subresource.layerCount);
        createInfo.format = GetVkFormat(textureViewDesc.m_format);
        createInfo.subresourceRange = subresource;
        createInfo.image = texture.GetHandle();

        NES_VK_FAIL_RETURN(m_device, vkCreateImageView(m_device, &createInfo, m_device.GetVkAllocationCallbacks(), &m_imageView));

        m_type = GetDescriptorType(textureViewDesc.m_viewType);
        m_textureDesc.m_pTexture = textureViewDesc.m_pTexture;
        m_textureDesc.m_imageLayout = GetVkImageViewLayout(textureViewDesc.m_viewType);
        m_textureDesc.m_aspectFlags = GetVkImageAspectFlags(textureViewDesc.m_format);
        m_textureDesc.m_layerOffset = textureViewDesc.m_layerOffset;
        m_textureDesc.m_layerCount = subresource.layerCount;
        m_textureDesc.m_sliceOffset = 0;
        m_textureDesc.m_sliceCount = 1;
        m_textureDesc.m_mipOffset = textureViewDesc.m_mipOffset;
        m_textureDesc.m_mipCount = subresource.levelCount;
        return EGraphicsResult::Success;
    }

    EGraphicsResult Descriptor::Init(const Texture3DViewDesc& textureViewDesc)
    {
        NES_ASSERT(textureViewDesc.m_pTexture);
        const Texture& texture = *textureViewDesc.m_pTexture;
        const TextureDesc& textureDesc = texture.GetDesc();
        const uint32 remainingMips = textureDesc.m_mipCount - textureViewDesc.m_mipOffset;
        const uint32 remainingLayers = textureDesc.m_layerCount - textureViewDesc.m_sliceOffset;

        VkImageViewSlicedCreateInfoEXT slicesInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_SLICED_CREATE_INFO_EXT};
        slicesInfo.sliceOffset = textureViewDesc.m_sliceOffset;
        slicesInfo.sliceCount = textureViewDesc.m_sliceCount == graphics::kUseRemaining ? remainingLayers : textureViewDesc.m_sliceCount;
        
        VkImageViewUsageCreateInfo usageInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO };
        usageInfo.usage = GetVkImageViewUsage(textureViewDesc.m_viewType);

        const VkImageSubresourceRange subresource =
        {
            .aspectMask = GetVkImageAspectFlags(textureViewDesc.m_format),
            .baseMipLevel = textureViewDesc.m_mipOffset,
            .levelCount = textureViewDesc.m_mipCount == graphics::kUseRemaining ? remainingMips : textureViewDesc.m_mipCount,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
        
        VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        createInfo.pNext = &usageInfo;
        createInfo.viewType = GetVkImageViewType(textureViewDesc.m_viewType, subresource.layerCount);
        createInfo.format = GetVkFormat(textureViewDesc.m_format);
        createInfo.subresourceRange = subresource;
        createInfo.image = texture.GetHandle();

        NES_VK_FAIL_RETURN(m_device, vkCreateImageView(m_device, &createInfo, m_device.GetVkAllocationCallbacks(), &m_imageView));

        m_type = GetDescriptorType(textureViewDesc.m_viewType);
        m_textureDesc.m_pTexture = textureViewDesc.m_pTexture;
        m_textureDesc.m_imageLayout = GetVkImageViewLayout(textureViewDesc.m_viewType);
        m_textureDesc.m_aspectFlags = GetVkImageAspectFlags(textureViewDesc.m_format);
        m_textureDesc.m_layerOffset = 0;
        m_textureDesc.m_layerCount = 1;
        m_textureDesc.m_sliceOffset = textureViewDesc.m_sliceOffset;
        m_textureDesc.m_sliceCount = textureViewDesc.m_sliceCount;
        m_textureDesc.m_mipOffset = textureViewDesc.m_mipOffset;
        m_textureDesc.m_mipCount = subresource.levelCount;
        return EGraphicsResult::Success;
    }

    EGraphicsResult Descriptor::Init(const SamplerDesc& samplerDesc)
    {
        VkSamplerCreateInfo info = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        info.flags = static_cast<VkSamplerCreateFlags>(0);
        info.magFilter = GetVkFilterType(samplerDesc.m_filters.m_mag);
        info.minFilter = GetVkFilterType(samplerDesc.m_filters.m_min);
        info.mipmapMode = GetVkSamplerMipMode(samplerDesc.m_filters.m_mip);
        info.addressModeU = GetVkSamplerAddressMode(samplerDesc.m_addressModes.u);
        info.addressModeV = GetVkSamplerAddressMode(samplerDesc.m_addressModes.v);
        info.addressModeW = GetVkSamplerAddressMode(samplerDesc.m_addressModes.w);
        info.mipLodBias = samplerDesc.m_mipBias;
        info.anisotropyEnable = static_cast<VkBool32>(static_cast<float>(samplerDesc.m_anisotropy) > 1.f);
        info.maxAnisotropy = static_cast<float>(samplerDesc.m_anisotropy);
        info.compareEnable = static_cast<VkBool32>(samplerDesc.m_compareOp != ECompareOp::None);
        info.compareOp = GetVkCompareOp(samplerDesc.m_compareOp);
        info.minLod = samplerDesc.m_mipMin;
        info.maxLod = samplerDesc.m_mipMax;
        
        const void** pTail = &info.pNext;
        VkSamplerReductionModeCreateInfo reductionModeInfo = {VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO};
        if (m_device.GetDesc().m_features.m_textureFilterMinMax)
        {
            reductionModeInfo.reductionMode = GetVkSamplerReductionMode(samplerDesc.m_filters.m_reduction);
            *pTail = &reductionModeInfo;
            pTail = &reductionModeInfo.pNext;
        }
        
        VkSamplerCustomBorderColorCreateInfoEXT borderColorInfo = {VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT};
        if (m_device.GetDesc().m_features.m_customBorderColor)
        {
            info.borderColor = samplerDesc.m_isInteger ? VK_BORDER_COLOR_INT_CUSTOM_EXT : VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;
            static_assert(sizeof(VkClearColorValue) == sizeof(samplerDesc.m_borderColor), "Unexpected sizeof");
            memcpy(&borderColorInfo.customBorderColor, &samplerDesc.m_borderColor, sizeof(borderColorInfo.customBorderColor));
            *pTail = &borderColorInfo;
            pTail = &borderColorInfo.pNext;
        }

        VkResult result = vkCreateSampler(m_device, &info, m_device.GetVkAllocationCallbacks(), &m_sampler);
        NES_VK_FAIL_RETURN(m_device, result);

        m_type = EDescriptorType::Sampler;
        return EGraphicsResult::Success;
    }

    bool Descriptor::IsTextureType() const
    {
        return DescriptorIsTextureType(m_type);
    }

    bool Descriptor::IsBufferType() const
    {
        return DescriptorIsBufferType(m_type);
    }

    const DescriptorBufferDesc& Descriptor::GetBufferDesc() const
    {
        NES_ASSERT(IsTextureType());
        return m_bufferDesc;
    }

    const DescriptorTextureDesc& Descriptor::GetTextureDesc() const
    {
        NES_ASSERT(IsBufferType());
        return m_textureDesc;
    }

    VkImageView Descriptor::GetVulkanImageView() const
    {
        NES_ASSERT(IsTextureType());
        return m_imageView;
    }

    VkSampler Descriptor::GetVulkanSampler() const
    {
        NES_ASSERT(m_type == EDescriptorType::Sampler);
        return m_sampler;
    }

    VkBufferView Descriptor::GetVulkanBufferView() const
    {
        NES_ASSERT(IsBufferType());
        return m_bufferView;
    }

    bool Descriptor::IsDepthWritable() const
    {
        NES_ASSERT(IsTextureType());
        return m_textureDesc.m_imageLayout != VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
            && m_textureDesc.m_imageLayout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    }

    bool Descriptor::IsStencilWritable() const
    {
        NES_ASSERT(IsTextureType());
        return m_textureDesc.m_imageLayout != VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL
            && m_textureDesc.m_imageLayout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    }
}
