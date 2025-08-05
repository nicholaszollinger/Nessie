// ResourceAllocator.h
#pragma once
#include "GraphicsCore.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : This class is in charge of allocating device memory for DeviceBuffers and Textures.
    ///     It is a wrapper for the VMA Allocator library.
    //----------------------------------------------------------------------------------------------------
    class ResourceAllocator
    {
    public:
        explicit                ResourceAllocator(RenderDevice& device) : m_device(device) {}

        /// No Copy or Move
        /* Copy Constructor */  ResourceAllocator(const ResourceAllocator&) = delete;
        /* Move Constructor */  ResourceAllocator(ResourceAllocator&&) noexcept = delete;
        /* Copy Assignment */   ResourceAllocator& operator=(const ResourceAllocator&) = delete;
        /* Move Assignment */   ResourceAllocator& operator=(ResourceAllocator&&) noexcept = delete;
        /* Destructor */        ~ResourceAllocator() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates the VMA allocator object based on the Render Device capabilities. 
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         Init();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroys the VMA allocator object.
        //----------------------------------------------------------------------------------------------------
        void                    Destroy();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocate a buffer resource.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         AllocateBuffer(const AllocateBufferDesc& bufferDesc, DeviceBuffer& outBuffer);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free a buffer resource.
        //----------------------------------------------------------------------------------------------------
        void                    FreeBuffer(DeviceBuffer& buffer);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocate a texture resource.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         AllocateTexture(const AllocateTextureDesc& textureDesc, Texture& outTexture);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free a texture resource.
        //----------------------------------------------------------------------------------------------------
        void                    FreeTexture(Texture& texture);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the render device that owns this allocator.
        //----------------------------------------------------------------------------------------------------
        RenderDevice&           GetDevice() const       { return m_device; }
        
    private:
        RenderDevice&           m_device;
        VmaAllocator            m_vmaAllocator = nullptr;
        bool                    m_deviceAddressSupported = false;
    };
}
