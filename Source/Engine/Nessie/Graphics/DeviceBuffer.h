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
        /// @brief : Allocates a new buffer resource.
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

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns whether you can copy data into the buffer directly from the CPU side. The buffer
        ///     would have to be allocated with the Memory Location = EMemoryLocation::HostUpload.
        //----------------------------------------------------------------------------------------------------
        bool                    IsHostMappable() const          { return m_pMappedMemory != nullptr; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : If this buffer is 'Host Mappable', this will copy the data into the CPU addressable pointer.
        /// @param pData : Pointer to the data that will be copied into the Device Buffer.
        /// @param offset : Byte offset in the device buffer. By default, this is 0.
        /// @param size : Size of the source data. If set to nes::graphics::kWholeSize, the remaining size from
        ///     the offset will be used.
        /// @see : DeviceBuffer::IsHostMappable() and nes::EMemoryLocation.
        //----------------------------------------------------------------------------------------------------
        void                    CopyToMappedMemory(void* pData, const uint64 offset = 0, const uint64 size = graphics::kWholeSize);

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
        friend class DeviceBufferRange;
        
        RenderDevice*           m_pDevice = nullptr;
        BufferDesc              m_desc{};                   // Buffer properties.
        vk::Buffer              m_buffer = nullptr;         // Vulkan handle.
        vk::DeviceAddress       m_deviceAddress = 0;        // Address of the buffer in the shader.
        uint8*                  m_pMappedMemory = nullptr;  // CPU mapped memory. 
        VmaAllocation           m_allocation = nullptr;     // Memory associated with the buffer.
    };
    
    static_assert(DeviceObjectType<DeviceBuffer>);
}
