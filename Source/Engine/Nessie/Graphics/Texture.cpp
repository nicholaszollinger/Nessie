// Texture.cpp
#include "Texture.h"
#include "RenderDevice.h"
#include "Vulkan/VmaUsage.h"

namespace nes
{
    Texture::~Texture()
    {
        if (m_ownsNativeObjects)
        {
            if (m_allocation)
            {
                vmaDestroyImage(m_device, m_handle, m_allocation);
                m_handle = nullptr;
                m_allocation = nullptr;
            }
        }
    }
    
    EGraphicsResult Texture::Init(const VkImage image, const TextureDesc& textureDesc)
    {
        NES_ASSERT(m_handle == nullptr);
        
        if (image == nullptr)
            return EGraphicsResult::InvalidArgument;

        // We do not own the image.
        m_ownsNativeObjects = false;
        m_desc = textureDesc;
        m_desc.Validate();
        m_handle = image;
        
        return EGraphicsResult::Success;
    }

    EGraphicsResult Texture::Init(const AllocateTextureDesc& textureDesc)
    {
        NES_ASSERT(m_handle == nullptr);
        
        // Fill out the ImageCreateInfo object. 
        VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        m_device.FillCreateInfo(textureDesc.m_desc, imageInfo);

        // Allocation Info:
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT | VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        allocCreateInfo.priority = textureDesc.m_priority * 0.5f + 0.5f;
        allocCreateInfo.usage = IsHostMemory(textureDesc.m_memoryLocation) ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        // Dedicated flag:
        if (textureDesc.m_isDedicated)
            allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        NES_VK_FAIL_RETURN(m_device, vmaCreateImage(m_device, &imageInfo, &allocCreateInfo, &m_handle, &m_allocation, nullptr));

        m_desc = textureDesc.m_desc;
        m_desc.Validate();

        return EGraphicsResult::Success;
    }

    void Texture::SetDebugName(const char* name)
    {
        m_device.SetDebugNameToTrivialObject(m_handle, name);
    }
}
