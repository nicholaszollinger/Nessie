// DeviceBuffer.cpp
#include "DeviceBuffer.h"
#include "RenderDevice.h"

#include "Vulkan/VmaUsage.h"

namespace nes
{
    DeviceBuffer::~DeviceBuffer()
    {
        if (m_ownsNativeObjects)
        {
            if (m_allocation)
            {
                vmaDestroyBuffer(m_device, m_handle, m_allocation);
                m_allocation = nullptr;
                m_handle = nullptr;
            }
        }
    }

    EGraphicsResult DeviceBuffer::Init(const AllocateBufferDesc& bufferDesc)
    {
        NES_ASSERT(m_handle == nullptr);
        
        // Fill out the BufferCreateInfo object. 
        VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        m_device.FillCreateInfo(bufferDesc.m_desc, bufferInfo);

        // Allocation CreateInfo:
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT | VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        allocCreateInfo.priority = bufferDesc.m_priority * 0.5f + 0.5f;
        allocCreateInfo.usage = IsHostMemory(bufferDesc.m_location) ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        if (bufferDesc.m_isDedicated)
            allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        if (IsHostVisibleMemory(bufferDesc.m_location))
        {
            // VMA_ALLOCATION_CREATE_MAPPED_BIT keeps the allocation mapped.
            allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
            allocCreateInfo.requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            
            if (bufferDesc.m_location == EMemoryLocation::HostReadback)
            {
                allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
                allocCreateInfo.preferredFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            }
            else
            {
                allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
                allocCreateInfo.preferredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            }
        }
        
        // Calculate Memory Alignment based on usage: 
        const DeviceDesc& deviceDesc = m_device.GetDesc();
        uint32 alignment = 1;
        if (bufferDesc.m_desc.m_usage & (EBufferUsageBits::ShaderResource | EBufferUsageBits::ShaderResourceStorage))
            alignment = math::Max(alignment, deviceDesc.m_memoryAlignment.m_bufferShaderResourceOffset);
        if (bufferDesc.m_desc.m_usage & EBufferUsageBits::ConstantBuffer)
            alignment = math::Max(alignment, deviceDesc.m_memoryAlignment.m_constantBufferOffset);
        if (bufferDesc.m_desc.m_usage & EBufferUsageBits::ShaderBindingTable)
            alignment = std::max(alignment, deviceDesc.m_memoryAlignment.m_shaderBindingTable);
        if (bufferDesc.m_desc.m_usage & EBufferUsageBits::ScratchBuffer)
            alignment = std::max(alignment, deviceDesc.m_memoryAlignment.m_scratchBufferOffset);
        if (bufferDesc.m_desc.m_usage & EBufferUsageBits::AccelerationStructureStorage)
            alignment = std::max(alignment, deviceDesc.m_memoryAlignment.m_accelerationStructureOffset);
        if (bufferDesc.m_desc.m_usage & EBufferUsageBits::MicromapStorage)
            alignment = std::max(alignment, deviceDesc.m_memoryAlignment.m_micromapOffset);

        VmaAllocationInfo allocInfo = {};
        NES_VK_FAIL_RETURN(m_device, vmaCreateBufferWithAlignment(m_device, &bufferInfo, &allocCreateInfo, alignment, &m_handle, &m_allocation, &allocInfo));
        
        // Mapped Memory, only necessary if host visible.
        if (IsHostVisibleMemory(bufferDesc.m_location))
        {
            m_pMappedMemory = static_cast<uint8*>(allocInfo.pMappedData) - allocInfo.offset;
        }
        
        // Device Address
        if (m_device.GetDesc().m_features.m_deviceAddress)
        {
            VkBufferDeviceAddressInfo bufferDeviceAddressInfo = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
            bufferDeviceAddressInfo.buffer = m_handle;
            m_deviceAddress = vkGetBufferDeviceAddress(m_device, &bufferDeviceAddressInfo);
        }
        
        // Description:
        m_desc = bufferDesc.m_desc;

        return EGraphicsResult::Success;
    }

    void DeviceBuffer::SetDebugName(const char* name)
    {
        m_device.SetDebugNameToTrivialObject(m_handle, name);
    }

    EGraphicsResult DeviceBuffer::CopyToBuffer(const void* data, const size_t offset, const size_t size)
    {
        if (!m_allocation)
            return EGraphicsResult::Failure;

        size_t actualSize = size;
        if (size == graphics::kUseRemaining)
            actualSize = m_desc.m_size - offset;

        NES_ASSERT(offset + actualSize <= m_desc.m_size);
        NES_VK_FAIL_RETURN(m_device, vmaCopyMemoryToAllocation(m_device, data, m_allocation, offset, actualSize));

        return EGraphicsResult::Success;
    }

    EGraphicsResult DeviceBuffer::CopyFromBuffer(void* pOutData, const size_t srcOffset, const size_t size)
    {
        if (!m_allocation)
            return EGraphicsResult::Failure;

        size_t actualSize = size;
        if (size == graphics::kUseRemaining)
            actualSize = m_desc.m_size - srcOffset;

        NES_ASSERT(srcOffset + actualSize <= m_desc.m_size);
        NES_VK_FAIL_RETURN(m_device, vmaCopyAllocationToMemory(m_device, m_allocation, srcOffset, pOutData, actualSize));

        return EGraphicsResult::Success;
    }
}
