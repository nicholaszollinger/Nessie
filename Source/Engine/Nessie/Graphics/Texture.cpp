// Texture.cpp
#include "Texture.h"

#include "RenderDevice.h"

namespace nes
{
    Texture::~Texture()
    {
        if (m_ownsNativeObjects)
        {
            // [TODO]: VMA allocation:
            // if (m_pVmaAllocation)
            //     DestroyVma();
            // else
                vkDestroyImage(m_device, m_handle, m_device.GetVkAllocationCallbacks());
        }
    }

    EGraphicsResult Texture::Init(const TextureDesc& textureDesc)
    {
        m_desc = textureDesc;
        m_desc.Validate();

        VkImageCreateInfo info{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        m_device.FillCreateInfo(m_desc, info);
        NES_VK_FAIL_RETURN(m_device, vkCreateImage(m_device, &info, m_device.GetVkAllocationCallbacks(), &m_handle));
        
        return EGraphicsResult::Success;
    }

    EGraphicsResult Texture::Init(const VkImage image, const TextureDesc& textureDesc)
    {
        if (image == nullptr)
            return EGraphicsResult::InvalidArgument;

        // We do not own the image.
        m_ownsNativeObjects = false;
        m_desc = textureDesc;
        m_desc.Validate();
        m_handle = image;
        
        return EGraphicsResult::Success;
    }

    void Texture::SetDebugName(const char* name)
    {
        m_device.SetDebugNameToTrivialObject(m_handle, name);
    }
}
