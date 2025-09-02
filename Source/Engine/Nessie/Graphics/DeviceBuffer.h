// DeviceBuffer.h
#pragma once
#include "DeviceObject.h"
#include "GraphicsCommon.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A device buffer is a region of memory used to store data on the GPU.
    /// It can be used to store vertex data, index data, uniform data, and other types of data.
    //----------------------------------------------------------------------------------------------------
    class DeviceBuffer
    {
    public:
        DeviceBuffer(std::nullptr_t) {}
        DeviceBuffer(const DeviceBuffer&) = delete;
        DeviceBuffer(DeviceBuffer&& other) noexcept;
        DeviceBuffer& operator=(std::nullptr_t);
        DeviceBuffer& operator=(const DeviceBuffer&) = delete;
        DeviceBuffer& operator=(DeviceBuffer&& other) noexcept;
        ~DeviceBuffer();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocates a new buffer resource, and maps the memory if it is host visible.
        //----------------------------------------------------------------------------------------------------
        DeviceBuffer(RenderDevice& device, const AllocateBufferDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Debug name for this DeviceBuffer. 
        //----------------------------------------------------------------------------------------------------
        void                    SetDebugName(const std::string& name);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the buffer's properties.
        //----------------------------------------------------------------------------------------------------
        const BufferDesc&       GetDesc() const                 { return m_desc; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan buffer handle.
        //----------------------------------------------------------------------------------------------------
        vk::Buffer              GetVkBuffer() const             { return m_buffer; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the address of the buffer in the shader.
        //----------------------------------------------------------------------------------------------------
        vk::DeviceAddress       GetAddress() const              { return m_deviceAddress; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the size of the buffer, in bytes.
        //----------------------------------------------------------------------------------------------------
        uint64                  GetSize() const                 { return m_desc.m_size; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the native vulkan object handle, and the type.
        //----------------------------------------------------------------------------------------------------
        NativeVkObject          GetNativeVkObject() const;

    private:
        friend class DataUploader;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocates the Buffer. 
        //----------------------------------------------------------------------------------------------------
        void                    AllocateBuffer(const RenderDevice& device, const AllocateBufferDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Submits the resource to the Renderer to be freed.
        //----------------------------------------------------------------------------------------------------
        void                    FreeBuffer();

    private:
        RenderDevice*           m_pDevice = nullptr;
        BufferDesc              m_desc{};                   // Buffer properties.
        vk::Buffer              m_buffer = nullptr;         // Vulkan handle.
        vk::DeviceAddress       m_deviceAddress = 0;        // Address of the buffer in the shader.
        uint8*                  m_pMappedMemory = nullptr;  // CPU mapped memory. 
        VmaAllocation           m_allocation = nullptr;     // Memory associated with the buffer.
    };

    static_assert(DeviceObjectType<DeviceBuffer>);
}
