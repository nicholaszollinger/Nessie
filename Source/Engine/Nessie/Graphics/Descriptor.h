// Descriptor.h
#pragma once
#include "GraphicsCommon.h"
#include "GraphicsResource.h"

namespace nes
{
    class Texture;

    struct DescriptorBufferDesc
    {
        VkBuffer        m_handle;
        uint64          m_offset;
        uint64          m_size;
        EBufferViewType m_viewType;
    };
        
    struct DescriptorTextureDesc
    {
        const Texture*  m_pTexture = nullptr;
        VkImageLayout   m_imageLayout;
        VkImageAspectFlags m_aspectFlags;
        uint32          m_layerOffset = 0;
        uint32          m_layerCount;
        uint32          m_sliceOffset = 0;
        uint32          m_sliceCount;
        uint32          m_mipOffset = 0;
        uint32          m_mipCount;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Descriptor represents access to a resource (texture, buffer, sampler, etc.).
    //----------------------------------------------------------------------------------------------------
    class Descriptor final : public GraphicsResource
    {
    public:
        explicit            Descriptor(RenderDevice& device) : GraphicsResource(device) {}
        virtual             ~Descriptor() override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this Descriptor. 
        //----------------------------------------------------------------------------------------------------
        virtual void        SetDebugName(const char* name) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a Buffer View Descriptor.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     Init(const BufferViewDesc& bufferViewDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a 1D Texture View Descriptor.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     Init(const Texture1DViewDesc& textureViewDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a 2D Texture View Descriptor.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     Init(const Texture2DViewDesc& textureViewDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a 3D Texture View Descriptor.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     Init(const Texture3DViewDesc& textureViewDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a Sampler Descriptor.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     Init(const SamplerDesc& samplerDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the resource type that this Descriptor represents. 
        //----------------------------------------------------------------------------------------------------
        EDescriptorType     GetType() const                                     { return m_type; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns whether the Descriptor was properly initialized.
        //----------------------------------------------------------------------------------------------------
        bool                IsValid() const                                     { return m_type != EDescriptorType::None; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if this descriptor represents a Texture resource.
        //----------------------------------------------------------------------------------------------------
        bool                IsTextureType() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if this resource represents a Buffer resource.
        //----------------------------------------------------------------------------------------------------
        bool                IsBufferType() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns information about the Buffer resource.
        /// @note : Only valid if the Descriptor is a buffer type. 
        //----------------------------------------------------------------------------------------------------
        const DescriptorBufferDesc&   GetBufferDesc() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns information about the Texture resource.
        /// @note : Only valid if the Descriptor is a texture type. 
        //----------------------------------------------------------------------------------------------------
        const DescriptorTextureDesc&  GetTextureDesc() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the vulkan image view handle.
        /// @note : Only valid if the Descriptor is a texture type. 
        //----------------------------------------------------------------------------------------------------
        VkImageView         GetVulkanImageView() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the vulkan sampler handle.
        /// @note : Only valid if the Descriptor is a sampler type. 
        //----------------------------------------------------------------------------------------------------
        VkSampler           GetVulkanSampler() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the vulkan buffer view handle.
        /// @note : Only valid if the Descriptor is a buffer type. 
        //----------------------------------------------------------------------------------------------------
        VkBufferView        GetVulkanBufferView() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the texture allows depth write operations.
        /// @note : Only valid if the Descriptor is a texture type. 
        //----------------------------------------------------------------------------------------------------
        bool                IsDepthWritable() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the texture allows depth stencil write operations.
        /// @note : Only valid if the Descriptor is a texture type. 
        //----------------------------------------------------------------------------------------------------
        bool                IsStencilWritable() const;
    
    private:
        /// Vulkan Resource Handle.
        union
        {
            VkImageView     m_imageView = nullptr;
            VkBufferView    m_bufferView;
            VkSampler       m_sampler;
        };

        /// Descriptor Description.
        union
        {
            DescriptorTextureDesc m_textureDesc = {};
            DescriptorBufferDesc  m_bufferDesc;
        };
        
        EDescriptorType     m_type = EDescriptorType::None;
    };
}
