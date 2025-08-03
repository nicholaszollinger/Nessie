// DeviceBuffer.h
#pragma once
#include "GraphicsResource.h"
#include "GraphicsCommon.h"

namespace nes
{
    class DeviceMemory;

    //----------------------------------------------------------------------------------------------------
    // UNDER DEVELOPMENT. NOT IMPLEMENTED YET.
    //----------------------------------------------------------------------------------------------------
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Device buffer represents an array of data on the GPU.
    //----------------------------------------------------------------------------------------------------
    class DeviceBuffer final : public GraphicsResource
    {
    public:
        explicit            DeviceBuffer(RenderDevice& device) : GraphicsResource(device) {}
        virtual             ~DeviceBuffer() override;

        /// Operator to cast to Vulkan Type.
        inline              operator VkBuffer() const { return m_handle; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a new buffer resource based on the given description. When this is destroyed,
        ///     so will the buffer resource.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     Init(const BufferDesc& desc);

        // [TODO]: VMA
        //EGraphicsResult     Init(const AllocateBufferDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Debug name for this DeviceBuffer. 
        //----------------------------------------------------------------------------------------------------
        virtual void        SetDebugName(const char* name) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the buffer's properties.
        //----------------------------------------------------------------------------------------------------
        const BufferDesc&   GetDesc() const             { return m_desc; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan buffer handle.
        //----------------------------------------------------------------------------------------------------
        VkBuffer            GetHandle() const           { return m_handle; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroy the buffer resource allocated with VMA.
        //----------------------------------------------------------------------------------------------------
        //void                DestroyVma();

        void                FinishMemoryBinding(DeviceMemory& memory, const uint64 memoryOffset);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get properties about the buffer's memory.
        //----------------------------------------------------------------------------------------------------
        void                GetMemoryDesc(EMemoryLocation memoryLocation, DeviceMemoryDesc& memoryDesc) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return the address of the the CPU memory at a given offset. 
        ///	@param offset : Byte offset from the start of the buffer. Default is 0.
        ///	@param size : Size of the mapped buffer section. Default is to use the remaining size from the offset.
        //----------------------------------------------------------------------------------------------------
        void*               Map(const uint64 offset = 0, const uint64 size = graphics::kUseRemaining);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Flush the current mapped memory range memory.
        //----------------------------------------------------------------------------------------------------
        void                Unmap();

    private:
        VkBuffer            m_handle = nullptr;
        VkDeviceAddress     m_deviceAddress = 0;
        uint8*              m_pMappedMemory = nullptr;
        VkDeviceMemory      m_nonCoherentDeviceMemory = nullptr;
        uint64              m_mappedMemoryOffset = 0;
        uint64              m_mappedMemoryRangeSize = 0;
        uint64              m_mappedMemoryRangeOffset = 0;
        BufferDesc          m_desc{};
        // VMA Allocation*
        bool                m_ownsNativeObjects = true;
    };
}
