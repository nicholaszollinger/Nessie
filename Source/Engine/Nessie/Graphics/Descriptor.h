// Descriptor.h
#pragma once
#include "GraphicsCommon.h"
#include "DeviceObject.h"

namespace nes
{
    class Texture;

    struct DescriptorBufferDesc
    {
        const DeviceBuffer*         m_pBuffer = nullptr;
        uint64                      m_offset = 0;
        uint64                      m_size = 0;
        EBufferViewType             m_viewType = EBufferViewType::ShaderResource;
    };

    struct DescriptorImageDesc
    {
        const DeviceImage*          m_pImage = nullptr;
        vk::ImageLayout             m_imageLayout;
        vk::ImageAspectFlags        m_aspectFlags = vk::ImageAspectFlagBits::eColor;
        uint32                      m_layerOffset = 0;
        uint32                      m_layerCount = 1;
        uint32                      m_sliceOffset = 0;
        uint32                      m_sliceCount = 1;
        uint32                      m_mipOffset = 0;
        uint32                      m_mipCount = 1;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A Descriptor represents access to a resource (texture, buffer, sampler, etc.).
    //----------------------------------------------------------------------------------------------------
    class Descriptor
    {
    public:
        Descriptor(std::nullptr_t) {}
        Descriptor(const Descriptor&) = delete;
        Descriptor(Descriptor&& other) noexcept;
        Descriptor& operator=(std::nullptr_t);
        Descriptor& operator=(const Descriptor&) = delete;
        Descriptor& operator=(Descriptor&& other) noexcept;
        ~Descriptor();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a Buffer View Descriptor.
        //----------------------------------------------------------------------------------------------------
        Descriptor(RenderDevice& device, const BufferViewDesc& bufferViewDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a 1D Image View Descriptor.
        //----------------------------------------------------------------------------------------------------
        Descriptor(RenderDevice& device, const Image1DViewDesc& imageViewDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a 2D Image View Descriptor.
        //----------------------------------------------------------------------------------------------------
        Descriptor(RenderDevice& device, const Image2DViewDesc& imageViewDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a 3D Image View Descriptor.
        //----------------------------------------------------------------------------------------------------
        Descriptor(RenderDevice& device, const Image3DViewDesc& imageViewDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a Sampler Descriptor.
        //----------------------------------------------------------------------------------------------------
        Descriptor(RenderDevice& device, const SamplerDesc& samplerDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this Descriptor. 
        //----------------------------------------------------------------------------------------------------
        void                        SetDebugName(const std::string& name);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the resource type that this Descriptor represents. 
        //----------------------------------------------------------------------------------------------------
        EDescriptorType             GetType() const                                     { return m_type; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns whether the Descriptor was properly initialized.
        //----------------------------------------------------------------------------------------------------
        bool                        IsValid() const                                     { return m_type != EDescriptorType::None; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if this descriptor represents an Image resource.
        //----------------------------------------------------------------------------------------------------
        bool                        IsImageType() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if this resource represents a Buffer resource.
        //----------------------------------------------------------------------------------------------------
        bool                        IsBufferType() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns information about the Buffer resource.
        /// @note : Only valid if the Descriptor is a buffer type. 
        //----------------------------------------------------------------------------------------------------
        const DescriptorBufferDesc& GetBufferDesc() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns information about the image resource.
        /// @note : Only valid if the Descriptor is a texture type. 
        //----------------------------------------------------------------------------------------------------
        const DescriptorImageDesc&  GetImageDesc() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the vulkan image view handle.
        /// @note : Only valid if the Descriptor is a texture type. 
        //----------------------------------------------------------------------------------------------------
        vk::ImageView               GetVkImageView() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the vulkan sampler handle.
        /// @note : Only valid if the Descriptor is a sampler type. 
        //----------------------------------------------------------------------------------------------------
        vk::Sampler                 GetVkSampler() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the vulkan buffer view handle.
        /// @note : Only valid if the Descriptor is a buffer type. 
        //----------------------------------------------------------------------------------------------------
        vk::BufferView              GetVkBufferView() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the descriptor info for a Buffer.
        //----------------------------------------------------------------------------------------------------
        vk::DescriptorBufferInfo    GetVkBufferInfo() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the texture allows depth write operations.
        /// @note : Only valid if the Descriptor is a texture type. 
        //----------------------------------------------------------------------------------------------------
        bool                        IsDepthWritable() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the texture allows depth stencil write operations.
        /// @note : Only valid if the Descriptor is a texture type. 
        //----------------------------------------------------------------------------------------------------
        bool                        IsStencilWritable() const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the native vulkan object handle, and the type.
        //----------------------------------------------------------------------------------------------------
        NativeVkObject              GetNativeVkObject() const;
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Submits the Descriptor to the Renderer to be freed.  
        //----------------------------------------------------------------------------------------------------
        void                        FreeDescriptor();

    private:
        
        /// Vulkan Resource Handle.
        union
        {
            vk::raii::ImageView     m_imageView = nullptr;
            vk::raii::BufferView    m_bufferView;
            vk::raii::Sampler       m_sampler;
        };

        /// Descriptor Description.
        union
        {
            DescriptorImageDesc     m_imageDesc = {};
            DescriptorBufferDesc    m_bufferDesc;
        };

        RenderDevice*               m_pDevice = nullptr;
        EDescriptorType             m_type = EDescriptorType::None;
    };

    static_assert(DeviceObjectType<Descriptor>);
}
