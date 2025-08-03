// Texture.h
#pragma once
#include "GraphicsCommon.h"
#include "GraphicsResource.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    // UNDER DEVELOPMENT. VMA NOT IMPLEMENTED YET.
    //----------------------------------------------------------------------------------------------------
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : An image object; represents a multidimensional array of data (1D, 2D or 3D).
    //----------------------------------------------------------------------------------------------------
    class Texture final : public GraphicsResource
    {
    public:
        explicit            Texture(RenderDevice& device) : GraphicsResource(device) {}
        virtual             ~Texture() override;

        /// Operator to cast to Vulkan type.
        inline              operator VkImage() const { return m_handle; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a new image resource based on the given description. When this texture is
        ///     destroyed, so will the image resource.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     Init(const TextureDesc& textureDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a texture using an existing image. When this texture object is destroyed, the
        ///     image resource will not be destroyed. This is to be used for cases like the Swapchain.  
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     Init(const VkImage image, const TextureDesc& textureDesc);
        
        // [TODO]: VMA
        //EGraphicsResult     Init(const AllocateTextureDesc& textureDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this texture. 
        //----------------------------------------------------------------------------------------------------
        virtual void        SetDebugName(const char* name) override;
        
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

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroy the image resource allocated with VMA.
        //----------------------------------------------------------------------------------------------------
        //void                DestroyVma();

    private:
        VkImage             m_handle = nullptr;
        TextureDesc         m_desc{};
        // VMA Allocation*
        bool                m_ownsNativeObjects = true;
    };
}

    