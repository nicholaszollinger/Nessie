// DeviceImage.cpp
#include "DeviceImage.h"
#include "RenderDevice.h"
#include "Renderer.h"
#include "Vulkan/VmaUsage.h"

namespace nes
{
    DeviceImage::~DeviceImage()
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

                m_image = nullptr;
                m_allocation = nullptr;
            }
        }
    }

    EGraphicsResult DeviceImage::Init(const vk::Image image, const ImageDesc& imageDesc)
    {
        NES_ASSERT(m_image == nullptr);
        
        if (image == nullptr)
            return EGraphicsResult::InvalidArgument;

        // We do not own the image.
        m_ownsNativeObjects = false;
        m_image = image;
        m_desc = imageDesc;
        m_desc.Validate();
        
        return EGraphicsResult::Success;
    }

    EGraphicsResult DeviceImage::Init(const AllocateImageDesc& allocDesc)
    {
        NES_ASSERT(m_image == nullptr);
        
        // Fill out the ImageCreateInfo object. 
        vk::ImageCreateInfo imageInfo{};
        m_device.FillCreateInfo(allocDesc.m_desc, imageInfo);

        // Allocation Info:
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT | VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        allocCreateInfo.priority = allocDesc.m_priority * 0.5f + 0.5f;
        allocCreateInfo.usage = IsHostMemory(allocDesc.m_memoryLocation) ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        if (allocDesc.m_isDedicated)
           allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        // Allocate the image.
        VkImage vkImage;
        VkImageCreateInfo& vkImageCreateInfo = imageInfo;
        NES_VK_FAIL_RETURN(m_device, vmaCreateImage(m_device, &vkImageCreateInfo, &allocCreateInfo, &vkImage, &m_allocation, nullptr));

        m_image = vkImage;
        m_desc = allocDesc.m_desc;
        m_desc.Validate();

        return EGraphicsResult::Success;
    }

    void DeviceImage::SetDebugName(const std::string& name)
    {
        m_device.SetDebugNameToTrivialObject(m_image, name);
    }

    uint16 DeviceImage::GetSize(const uint16 dimensionIndex, const uint16 mip) const
    {
        NES_ASSERT(dimensionIndex < 3);

        uint16 size = static_cast<uint16>(m_desc.m_depth);
        if (dimensionIndex == 0)
            size = static_cast<uint16>(m_desc.m_width);
        if (dimensionIndex == 1)
            size = static_cast<uint16>(m_desc.m_height);

        // Set the size of the particular mip level:
        size = static_cast<uint16>(math::Max(size >> mip, 1));

        // Align the value to the format's block width.
        size = math::AlignUp(size, dimensionIndex < 2 ? GetFormatProps(m_desc.m_format).m_blockWidth : 1);

        return size;
    }
}
