// Texture.cpp
#include "Texture.h"
#include "RenderDevice.h"

namespace nes
{
    Texture::~Texture()
    {
        if (m_ownsNativeObjects)
        {
            if (m_allocation)
            {
                auto& allocator = m_device.GetResourceAllocator();
                allocator.FreeTexture(*this);
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
        
        auto& allocator = m_device.GetResourceAllocator();
        return allocator.AllocateTexture(textureDesc, *this);
    }

    void Texture::SetDebugName(const char* name)
    {
        m_device.SetDebugNameToTrivialObject(m_handle, name);
    }
}
