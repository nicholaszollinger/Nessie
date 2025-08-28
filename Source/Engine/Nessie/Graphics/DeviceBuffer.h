// DeviceBuffer.h
#pragma once
#include "DeviceAsset.h"
#include "GraphicsCommon.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A device buffer is a region of memory used to store data on the GPU.
    /// It can be used to store vertex data, index data, uniform data, and other types of data.
    //----------------------------------------------------------------------------------------------------
    class DeviceBuffer : public DeviceAsset
    {
    public:
        explicit            DeviceBuffer(RenderDevice& device) : DeviceAsset(device) {}
        virtual             ~DeviceBuffer() override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Allocates a new buffer resource, and maps the memory if it is host visible.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     Init(const AllocateBufferDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Debug name for this DeviceBuffer. 
        //----------------------------------------------------------------------------------------------------
        virtual void        SetDebugName(const std::string& name) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the buffer's properties.
        //----------------------------------------------------------------------------------------------------
        const BufferDesc&   GetDesc() const                 { return m_desc; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan buffer handle.
        //----------------------------------------------------------------------------------------------------
        vk::Buffer          GetVkBuffer() const             { return m_buffer; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the address of the buffer in the shader.
        //----------------------------------------------------------------------------------------------------
        VkDeviceAddress     GetAddress() const              { return m_deviceAddress; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the size of the buffer, in bytes.
        //----------------------------------------------------------------------------------------------------
        uint64              GetSize() const                 { return m_desc.m_size; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Copy a block of data to the allocated buffer memory. Only valid for mappable memory.
        ///     This performs the Map(), memcpy(), UnMap() and Flush() functions all in one operation.
        /// @param data : Address of the data.
        /// @param offset : Offset in the buffer to begin copying into. By default, this is 0.
        /// @param size : Size of the data, in bytes. By default, it is a special value to denote the remaining size
        ///     from the offset.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     CopyToBuffer(const void* data, const size_t offset = 0, const size_t size = graphics::kUseRemaining);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Copy data from the buffer into pOutData.
        ///     This performs the Invalidate(), Map(), memcpy(), UnMap() functions automatically.
        ///	@param pOutData : Pointer that we are going to copy the data into.
        ///	@param srcOffset : Byte offset from the buffer to read from. By default, this is 0.
        ///	@param size : Number of bytes to copy. By default, it is a special value to denote the remaining size
        ///     from the source offset.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     CopyFromBuffer(void* pOutData, const size_t srcOffset = 0, const size_t size = graphics::kUseRemaining);
        
        // [TODO]:
        // - Map/Unmap()
        // - Flush/Invalidate()

    private:
        BufferDesc          m_desc{};                   // Buffer properties.
        vk::Buffer          m_buffer = nullptr;         // Vulkan handle.
        vk::DeviceAddress   m_deviceAddress = 0;        // Address of the buffer in the shader.
        uint8*              m_pMappedMemory = nullptr;  // CPU mapped memory. 
        VmaAllocation       m_allocation = nullptr;     // Memory associated with the buffer.
        bool                m_ownsNativeObjects = true;
    };
}
