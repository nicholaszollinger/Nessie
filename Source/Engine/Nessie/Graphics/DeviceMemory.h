// DeviceMemory.h
#pragma once
#include "GraphicsCommon.h"
#include "GraphicsResource.h"

namespace nes
{
    struct DeviceMemoryCreateInfo
    {
        VkDeviceMemory  m_deviceMemory = nullptr;
        void*           m_pMappedMemory = nullptr;
        uint64          m_size = 0;
        uint32          m_memoryTypeIndex = 0;
    };

    //----------------------------------------------------------------------------------------------------
    // UNDER DEVELOPMENT. NOT IMPLEMENTED YET.
    //----------------------------------------------------------------------------------------------------
    
    class DeviceMemory final : public GraphicsResource
    {
    public:
        explicit            DeviceMemory(RenderDevice& device) : GraphicsResource(device) {}
        virtual             ~DeviceMemory() override;

        /// Operator to cast to Vulkan Type.
        inline              operator VkDeviceMemory() const { return m_handle; }

        EGraphicsResult     Init(const DeviceMemoryCreateInfo& desc);
        // [TODO]: VMA
        //EGraphicsResult     Init(const AllocateMemoryDesc& desc);
        EGraphicsResult     InitDedicated(const DeviceBuffer& buffer);
        EGraphicsResult     InitDedicated(const Texture& texture);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this memory object. 
        //----------------------------------------------------------------------------------------------------
        virtual void        SetDebugName(const char* name) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan device memory handle.
        //----------------------------------------------------------------------------------------------------
        VkDeviceMemory      GetHandle() const { return m_handle; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the memory type. 
        //----------------------------------------------------------------------------------------------------
        DeviceMemoryType    GetMemoryType() const { return m_type; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the mapped device memory.
        //----------------------------------------------------------------------------------------------------
        uint8*              GetMappedMemory() const { return m_pMappedMemory; }

    private:
        VkDeviceMemory      m_handle = nullptr;
        uint8*              m_pMappedMemory = nullptr;
        DeviceMemoryType    m_type{};
        float               m_priority = 0.f;
        bool                m_ownsNativeObjects = true;
    };
}
