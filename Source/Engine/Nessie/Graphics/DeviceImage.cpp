// DeviceImage.cpp
#include "DeviceImage.h"
#include "RenderDevice.h"
#include "Renderer.h"
#include "Vulkan/VmaUsage.h"

namespace nes
{
    DeviceImage::DeviceImage(RenderDevice& device, const vk::Image image, const ImageDesc& imageDesc)
        : m_pDevice(&device)
    {
        NES_ASSERT(image != nullptr);

        // We do not own the image.
        m_ownsNativeObjects = false;
        m_image = image;
        m_desc = imageDesc;
        m_desc.Validate();
    }

    DeviceImage::DeviceImage(RenderDevice& device, const AllocateImageDesc& allocDesc)
        : m_pDevice(&device)
        , m_ownsNativeObjects(true)
    {
        AllocateResource(*m_pDevice, allocDesc);
    }

    DeviceImage::DeviceImage(DeviceImage&& other) noexcept
        : m_pDevice(other.m_pDevice)
        , m_image(other.m_image)
        , m_desc(other.m_desc)
        , m_allocation(other.m_allocation)
        , m_ownsNativeObjects(other.m_ownsNativeObjects)
    {
        other.m_pDevice = nullptr;
        other.m_image = nullptr;
        other.m_allocation = nullptr;
        other.m_ownsNativeObjects = false;
    }

    DeviceImage& DeviceImage::operator=(std::nullptr_t)
    {
        *this = DeviceImage(nullptr);
        return *this;
    }

    DeviceImage& DeviceImage::operator=(DeviceImage&& other) noexcept
    {
        if (this != &other)
        {
            FreeImage();
            
            m_pDevice = other.m_pDevice;
            m_desc = other.m_desc;
            m_image = other.m_image;
            m_allocation = other.m_allocation;
            m_ownsNativeObjects = other.m_ownsNativeObjects;

            other.m_pDevice = nullptr;
            other.m_image = nullptr;
            other.m_allocation = nullptr;
            other.m_ownsNativeObjects = false;
        }

        return *this;
    }

    DeviceImage::~DeviceImage()
    {
        FreeImage();
    }

    void DeviceImage::SetDebugName(const std::string& name)
    {
        m_pDevice->SetDebugNameVkObject(GetNativeVkObject(), name);
    }

    void DeviceImage::AllocateResource(const RenderDevice& device, const AllocateImageDesc& allocDesc)
    {
        // Fill out the ImageCreateInfo object. 
        const auto& imageDesc = allocDesc.m_desc;
        
        vk::ImageCreateFlags flags = vk::ImageCreateFlagBits::eMutableFormat | vk::ImageCreateFlagBits::eExtendedUsage;
        
        const FormatProps& formatProps = GetFormatProps(imageDesc.m_format);
        if (formatProps.m_blockWidth > 1 && (imageDesc.m_usage & EImageUsageBits::ShaderResourceStorage))
            flags |= vk::ImageCreateFlagBits::eBlockTexelViewCompatible; // Format can be used to create a view with an uncompressed format (1 texel covers 1 block)
        if (imageDesc.m_layerCount >= 6 && imageDesc.m_width == imageDesc.m_height)
            flags |= vk::ImageCreateFlagBits::eCubeCompatible; // Allow cube maps
        if (imageDesc.m_type == EImageType::Image3D)
            flags |= vk::ImageCreateFlagBits::e2DArrayCompatible; // allow 3D demotion to a set of layers

        // [TODO]: 
        //if (m_desc.m_tieredFeatures.m_sampleLocations && formatProps.m_isDepth)
        // flags |= vk::ImageCreateFlagBits::eSampleLocationsCompatibleDepthEXT;
        
        vk::ImageCreateInfo createInfo = vk::ImageCreateInfo()
            .setFlags(flags)
            .setImageType(GetVkImageType(imageDesc.m_type))
            .setFormat(GetVkFormat(imageDesc.m_format))
            .setExtent({imageDesc.m_width, math::Max(imageDesc.m_height, 1U), math::Max(imageDesc.m_depth, 1U)})
            .setMipLevels(math::Max(imageDesc.m_mipCount, 1U))
            .setArrayLayers(math::Max(imageDesc.m_layerCount, 1U))
            .setTiling(vk::ImageTiling::eOptimal)
            .setSamples(static_cast<vk::SampleCountFlagBits>(math::Max(imageDesc.m_sampleCount, 1U)))
            .setUsage(GetVkImageUsageFlags(imageDesc.m_usage))
            .setSharingMode(allocDesc.m_queueFamilyIndices.empty()? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent)
            .setQueueFamilyIndexCount(static_cast<uint32>(allocDesc.m_queueFamilyIndices.size()))
            .setPQueueFamilyIndices(allocDesc.m_queueFamilyIndices.data())
            .setInitialLayout(vk::ImageLayout::eUndefined);

        // Allocation Info:
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT | VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        //allocCreateInfo.priority = desc.m_priority * 0.5f + 0.5f;
        allocCreateInfo.usage = IsHostMemory(allocDesc.m_memoryLocation) ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        if (allocDesc.m_isDedicated)
            allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        // Allocate the image.
        VkImage vkImage;
        VkImageCreateInfo& vkImageCreateInfo = createInfo;
        NES_VK_MUST_PASS(device, vmaCreateImage(device, &vkImageCreateInfo, &allocCreateInfo, &vkImage, &m_allocation, nullptr));

        // We own this image.
        m_ownsNativeObjects = true;
        m_image = vkImage;
        m_desc = allocDesc.m_desc;
        m_desc.Validate();
    }

    void DeviceImage::FreeImage()
    {
        if (m_ownsNativeObjects)
        {
            if (m_allocation)
            {
                Renderer::SubmitResourceFree([image = m_image, allocation = m_allocation]() mutable
                {
                    auto& device = Renderer::GetDevice();
                    vmaDestroyImage(device, image, allocation);
                });
            }
        }

        m_pDevice = nullptr;
        m_image = nullptr;
        m_allocation = nullptr;
        m_ownsNativeObjects = false;
    }

    uint64 DeviceImage::GetPixelCount(const uint16 mipLevel) const
    {
        return static_cast<uint64>(GetDimensionSize(0, mipLevel)) * GetDimensionSize(1, mipLevel) * GetDimensionSize(2, mipLevel);
    }

    uint16 DeviceImage::GetDimensionSize(const uint16 dimensionIndex, const uint32 mipLevel) const
    {
        NES_ASSERT(m_pDevice != nullptr && m_image != nullptr, "Attempted to get size of null image!");
        NES_ASSERT(dimensionIndex < 3);

        uint16 size = static_cast<uint16>(m_desc.m_depth);
        if (dimensionIndex == 0)
            size = static_cast<uint16>(m_desc.m_width);
        if (dimensionIndex == 1)
            size = static_cast<uint16>(m_desc.m_height);

        // Set the size of the particular mip level:
        size = static_cast<uint16>(math::Max(size >> mipLevel, 1));

        // Align the value to the format's block width.
        size = math::AlignUp(size, dimensionIndex < 2 ? GetFormatProps(m_desc.m_format).m_blockWidth : 1);

        return size;
    }

    uint64 DeviceImage::GetPixelSize() const
    {
        return GetFormatProps(m_desc.m_format).m_stride;
    }

    NativeVkObject DeviceImage::GetNativeVkObject() const
    {
        return NativeVkObject(m_image, vk::ObjectType::eImage);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : DEPRECATED - Should be removed when I do the following:
    ///
    /// Turns out, this function that I started with isn't the best option - I should use a stb_image_resize2.h to
    /// create the mip map levels that I need instead of relying on the blit image. Not all formats are
    /// supported by blit image, and the command has to be submitted on a device queue with graphics capabilities.
    /// - When importing a texture into the Engine Application, mip maps should be generated and stored in a single file, with
    ///     a base level and count at the beginning of the binary file.
    //----------------------------------------------------------------------------------------------------
    [[maybe_unused]]
    static void GenerateMipmaps(CommandBuffer& buffer, DeviceImage& image, const uint32 numMips, const uint32 numLayers, const EImageLayout currentLayout)
    {
        // No mips to generate:
        if (numMips == 0)
            return;
        
        // Transition the top mip level to Copy Source
        ImageBarrierDesc imageBarrierDesc{};
        imageBarrierDesc.m_pImage = &image;
        imageBarrierDesc.m_before.m_layout = currentLayout;
        imageBarrierDesc.m_after.m_layout = EImageLayout::CopySource;
        imageBarrierDesc.m_planes = EImagePlaneBits::Color;
        imageBarrierDesc.m_baseMip = 0;
        imageBarrierDesc.m_mipCount = 1;
        imageBarrierDesc.m_baseLayer = 0;
        imageBarrierDesc.m_layerCount = numLayers;

        BarrierGroupDesc groupDesc({ imageBarrierDesc });
        buffer.SetBarriers(groupDesc);
        
        // Transition remaining mips to Copy Destination
        imageBarrierDesc.m_baseMip = 1;
        imageBarrierDesc.m_mipCount = numMips - 1;
        imageBarrierDesc.m_after.m_layout = EImageLayout::CopyDestination;
        groupDesc.m_imageBarriers[0] = imageBarrierDesc;
        buffer.SetBarriers(groupDesc);
        
        // Blit Regions:
        int32 srcWidth = image.GetDimensionSize(0);
        int32 srcHeight = image.GetDimensionSize(1);
        int32 srcDepth = image.GetDimensionSize(2);
        
        vk::ImageBlit2 blitRegion = vk::ImageBlit2()
            .setSrcSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, {}, {}, numLayers))
            .setDstSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, {}, {}, numLayers));
        
        vk::BlitImageInfo2 blitImageInfo = vk::BlitImageInfo2()
            .setSrcImage(image.GetVkImage())
            .setSrcImageLayout(vk::ImageLayout::eTransferSrcOptimal)
            .setDstImage(image.GetVkImage())
            .setDstImageLayout(vk::ImageLayout::eTransferDstOptimal)
            .setRegionCount(1)
            .setPRegions(&blitRegion)
            .setFilter(vk::Filter::eLinear);
        
        // Generate Mip Maps:
        for (uint32 i = 1; i < numMips; ++i)
        {
            const int32 dstWidth = image.GetDimensionSize(0, i);
            const int32 dstHeight = image.GetDimensionSize(1, i);
            const int32 dstDepth = image.GetDimensionSize(2, i);
            
            // Blit from current dimension to half of it.
            blitRegion.srcSubresource.mipLevel = i - 1;
            blitRegion.srcOffsets[1] = vk::Offset3D{ srcWidth, srcHeight, srcDepth };
            blitRegion.dstSubresource.mipLevel = i;
            blitRegion.dstOffsets[1] = vk::Offset3D{ dstWidth, dstHeight, dstDepth };
            buffer.GetVkCommandBuffer().blitImage2(blitImageInfo);

            // Transition the current mip-level to copy source, to be used for the next mip level.
            imageBarrierDesc.m_baseMip = i;
            imageBarrierDesc.m_mipCount = 1;
            imageBarrierDesc.m_before = { .m_access = EAccessBits::CopyDestination, .m_layout = EImageLayout::CopyDestination, .m_stages = EPipelineStageBits::Copy };
            imageBarrierDesc.m_after = { .m_access = EAccessBits::CopySource, .m_layout = EImageLayout::CopySource, .m_stages = EPipelineStageBits::Copy };
            groupDesc.m_imageBarriers[0] = imageBarrierDesc;
            buffer.SetBarriers(groupDesc);

            // Update our source dimensions:
            srcWidth = dstWidth;
            srcHeight = dstHeight;
            srcDepth = dstDepth;
        }

        // Transition all mip-levels back to current layout.
        imageBarrierDesc.m_baseMip = 0;
        imageBarrierDesc.m_mipCount = numMips;
        imageBarrierDesc.m_before = { .m_access = EAccessBits::CopySource, .m_layout = EImageLayout::CopySource, .m_stages = EPipelineStageBits::Copy };
        imageBarrierDesc.m_after = { .m_access = graphics::kInferAccess, .m_layout = currentLayout, .m_stages = graphics::kInferPipelineStage };
        groupDesc.m_imageBarriers[0] = imageBarrierDesc;
        buffer.SetBarriers(groupDesc);
    }
}
