// DeviceBuffer.h
#pragma once
#include "GraphicsResource.h"
#include "GraphicsCommon.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A device buffer is a region of memory used to store data.
    /// It can be used to store vertex data, index data, uniform data, and other types of data.
    //----------------------------------------------------------------------------------------------------
    class DeviceBuffer
    {
        friend class ResourceAllocator;
        
    public:
        explicit            DeviceBuffer(RenderDevice& device) : m_device(device) {}
        /* Destructor */    ~DeviceBuffer();

        /// Operator to cast to Vulkan Type.
        inline              operator VkBuffer() const       { return m_handle; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Uses the RenderDevice's Resource Allocator to create the buffer.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     Init(const AllocateBufferDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Debug name for this DeviceBuffer. 
        //----------------------------------------------------------------------------------------------------
        void                SetDebugName(const char* name);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the buffer's properties.
        //----------------------------------------------------------------------------------------------------
        const BufferDesc&   GetDesc() const                 { return m_desc; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan buffer handle.
        //----------------------------------------------------------------------------------------------------
        VkBuffer            GetHandle() const               { return m_handle; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the address of the buffer in the shader.
        //----------------------------------------------------------------------------------------------------
        VkDeviceAddress     GetAddress() const              { return m_deviceAddress; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the size of the buffer, in bytes.
        //----------------------------------------------------------------------------------------------------
        uint64              GetSize() const                 { return m_desc.m_size; }
        
        // [TODO]:
        // - Map/Unmap()
        // - Flush/Invalidate()

    private:
        RenderDevice&       m_device;                   
        BufferDesc          m_desc{};                   // Buffer properties.
        VkBuffer            m_handle = nullptr;         // Vulkan handle.
        VkDeviceAddress     m_deviceAddress = 0;        // Address of the buffer in the shader.
        uint8*              m_pMappedMemory = nullptr;  
        VmaAllocation       m_allocation = nullptr;     // Memory associated with the buffer.
        bool                m_ownsNativeObjects = true;
    };
}
