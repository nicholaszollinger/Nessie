// DeviceBuffer.cpp
#include "DeviceBuffer.h"
#include "RenderDevice.h"
#include "Renderer.h"

#include "Vulkan/VmaUsage.h"

namespace nes
{
    DeviceBuffer::~DeviceBuffer()
    {
        if (m_ownsNativeObjects)
        {
            if (m_allocation)
            {
                Renderer::SubmitResourceFree([buffer = m_buffer, allocation = m_allocation]() mutable
                {
                    auto& device = Renderer::GetDevice();
                    vmaDestroyBuffer(device, buffer, allocation);
                });
                
                m_buffer = nullptr;
                m_allocation = nullptr;
            }
        }
    }

    EGraphicsResult DeviceBuffer::Init(const AllocateBufferDesc& allocDesc)
    {
        NES_ASSERT(m_buffer == nullptr);
        
        // Fill out the BufferCreateInfo object. 
        vk::BufferCreateInfo bufferInfo{};
        m_device.FillCreateInfo(allocDesc.m_desc, bufferInfo);

        // Allocation CreateInfo:
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT | VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        allocCreateInfo.priority = allocDesc.m_priority * 0.5f + 0.5f;
        allocCreateInfo.usage = IsHostMemory(allocDesc.m_location) ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        if (allocDesc.m_isDedicated)
            allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        if (IsHostVisibleMemory(allocDesc.m_location))
        {
            // VMA_ALLOCATION_CREATE_MAPPED_BIT keeps the allocation mapped.
            allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
            allocCreateInfo.requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            
            if (allocDesc.m_location == EMemoryLocation::HostReadback)
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
        if (allocDesc.m_desc.m_usage & (EBufferUsageBits::ShaderResource | EBufferUsageBits::ShaderResourceStorage))
            alignment = math::Max(alignment, deviceDesc.m_memoryAlignment.m_bufferShaderResourceOffset);
        if (allocDesc.m_desc.m_usage & EBufferUsageBits::UniformBuffer)
            alignment = math::Max(alignment, deviceDesc.m_memoryAlignment.m_constantBufferOffset);
        if (allocDesc.m_desc.m_usage & EBufferUsageBits::ShaderBindingTable)
            alignment = std::max(alignment, deviceDesc.m_memoryAlignment.m_shaderBindingTable);
        if (allocDesc.m_desc.m_usage & EBufferUsageBits::ScratchBuffer)
            alignment = std::max(alignment, deviceDesc.m_memoryAlignment.m_scratchBufferOffset);
        if (allocDesc.m_desc.m_usage & EBufferUsageBits::AccelerationStructureStorage)
            alignment = std::max(alignment, deviceDesc.m_memoryAlignment.m_accelerationStructureOffset);
        if (allocDesc.m_desc.m_usage & EBufferUsageBits::MicromapStorage)
            alignment = std::max(alignment, deviceDesc.m_memoryAlignment.m_micromapOffset);

        // Create the buffer:
        VmaAllocationInfo vmaAllocInfo = {};
        VkBuffer vkBuffer = VK_NULL_HANDLE;
        VkBufferCreateInfo& vkBufferInfo = bufferInfo;
        NES_VK_FAIL_RETURN(m_device, vmaCreateBufferWithAlignment(m_device, &vkBufferInfo, &allocCreateInfo, alignment, &vkBuffer, &m_allocation, &vmaAllocInfo));
        m_buffer = vkBuffer;

        // Mapped Memory, only necessary if host visible.
        if (IsHostVisibleMemory(allocDesc.m_location))
        {
             m_pMappedMemory = static_cast<uint8*>(vmaAllocInfo.pMappedData) - vmaAllocInfo.offset;
        }

        // Device Address
        if (m_device.GetDesc().m_features.m_deviceAddress)
        {
            m_deviceAddress = m_device.GetVkDevice().getBufferAddress(m_buffer);
        }
        
        // Set the Description:
        m_desc = allocDesc.m_desc;
        
        return EGraphicsResult::Success;
    }

    void DeviceBuffer::SetDebugName(const std::string& name)
    {
        m_device.SetDebugNameToTrivialObject(m_buffer, name);
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
