// Texture.h
#pragma once
#include "GraphicsCommon.h"
#include "GraphicsResource.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : An image object; represents a multidimensional array of data (1D, 2D or 3D).
    //----------------------------------------------------------------------------------------------------
    class Texture
    {
        friend class ResourceAllocator;
        
    public:
        explicit            Texture(RenderDevice& device) : m_device(device) {}
        /* Destructor */    ~Texture();

        /// Operator to cast to Vulkan type.
        inline              operator VkImage() const { return m_handle; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a texture using an existing image. When this texture object is destroyed, the
        ///     image resource will not be destroyed. This is to be used for cases like the Swapchain.  
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     Init(const VkImage image, const TextureDesc& textureDesc);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Uses the RenderDevice's Resource Allocator to create the new texture.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     Init(const AllocateTextureDesc& textureDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this texture. 
        //----------------------------------------------------------------------------------------------------
        void                SetDebugName(const char* name);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the texture's properties.
        //----------------------------------------------------------------------------------------------------
        const TextureDesc&  GetDesc() const { return m_desc; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the extent of the image.
        //----------------------------------------------------------------------------------------------------
        VkExtent3D          GetExtent() const { return { m_desc.m_width, m_desc.m_height, m_desc.m_depth }; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan resource handle.
        //----------------------------------------------------------------------------------------------------
        VkImage             GetHandle() const { return m_handle; }

    private:
        RenderDevice&       m_device;
        VkImage             m_handle = nullptr;
        TextureDesc         m_desc{};
        VmaAllocation       m_allocation = nullptr;     // Device Memory associated with the Texture.
        bool                m_ownsNativeObjects = true; // If true, then on destruction the image will be freed.
    };
}

    