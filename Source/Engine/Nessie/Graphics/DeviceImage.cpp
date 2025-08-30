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

    void DeviceImage::AllocateResource(const RenderDevice& device, const AllocateImageDesc& desc)
    {
        // Fill out the ImageCreateInfo object. 
        vk::ImageCreateInfo imageInfo{};
        device.FillCreateInfo(desc.m_desc, imageInfo);

        // Allocation Info:
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT | VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        allocCreateInfo.priority = desc.m_priority * 0.5f + 0.5f;
        allocCreateInfo.usage = IsHostMemory(desc.m_memoryLocation) ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        if (desc.m_isDedicated)
            allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        // Allocate the image.
        VkImage vkImage;
        VkImageCreateInfo& vkImageCreateInfo = imageInfo;
        NES_VK_MUST_PASS(device, vmaCreateImage(device, &vkImageCreateInfo, &allocCreateInfo, &vkImage, &m_allocation, nullptr));

        // We own this image.
        m_ownsNativeObjects = true;
        m_image = vkImage;
        m_desc = desc.m_desc;
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

    uint16 DeviceImage::GetSize(const uint16 dimensionIndex, const uint16 mip) const
    {
        NES_ASSERT(m_pDevice != nullptr && m_image != nullptr, "Attempted to get size of null image!");
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

    NativeVkObject DeviceImage::GetNativeVkObject() const
    {
        return NativeVkObject(m_image, vk::ObjectType::eImage);
    }
}
