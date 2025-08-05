// ResourceAllocator.cpp
#include "ResourceAllocator.h"
#include "RenderDevice.h"
#include "DeviceBuffer.h"

#include <volk.h>

#include "Texture.h"
#include "Vulkan/VmaUsage.h"

namespace nes
{
    EGraphicsResult ResourceAllocator::Init()
    {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.instance = m_device;
        allocatorInfo.physicalDevice = m_device;
        allocatorInfo.device = m_device;
        allocatorInfo.vulkanApiVersion = m_device.GetDesc().m_apiVersion;
        allocatorInfo.pAllocationCallbacks = m_device.GetVkAllocationCallbacks();

        // Set Flags based on available features.
        const auto& deviceDesc = m_device.GetDesc();
        m_deviceAddressSupported = deviceDesc.m_features.m_deviceAddress;
        
        VmaAllocatorCreateFlags flags = {};
        if (m_deviceAddressSupported)
            flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        if (deviceDesc.m_features.m_memoryBudget)
            flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        if (deviceDesc.m_features.m_memoryPriority)
            flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
        if (deviceDesc.m_features.m_maintenance4)
            flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT;
        if (deviceDesc.m_features.m_maintenance5)
            flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;

        // Import functions from Volk.
        VmaVulkanFunctions vulkanFunctions;
        NES_VK_FAIL_RETURN(m_device, vmaImportVulkanFunctionsFromVolk(&allocatorInfo, &vulkanFunctions));
        allocatorInfo.pVulkanFunctions = &vulkanFunctions;

        // [Consider]:
        //allocatorInfo.pDeviceMemoryCallbacks? Do I need this?
        // -> "Informative callbacks for vkAllocateMemory, vkFreeMemory. Optional." 
        //allocatorInfo.pHeapSizeLimit
        // -> "Either null or a pointer to an array of limits on the maximum number of bytes that can be allocated out of a particular Vulkan memory heap."
        //allocatorInfo.preferredLargeHeapBlockSize?
        // -> "Preferred size of a single VkDeviceMemory block to be allocated from large heaps > 1 GiB. Optional."
        
        allocatorInfo.flags = flags;
        NES_VK_FAIL_RETURN(m_device, vmaCreateAllocator(&allocatorInfo, &m_vmaAllocator));
        
        return EGraphicsResult::Success;
    }

    void ResourceAllocator::Destroy()
    {
        vmaDestroyAllocator(m_vmaAllocator);
        m_vmaAllocator = nullptr;
    }

    EGraphicsResult ResourceAllocator::AllocateBuffer(const AllocateBufferDesc& bufferDesc, DeviceBuffer& outBuffer)
    {
        NES_ASSERT(m_vmaAllocator != nullptr);
        NES_ASSERT(outBuffer.m_handle == nullptr);
        
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
        NES_VK_FAIL_RETURN(m_device, vmaCreateBufferWithAlignment(m_vmaAllocator, &bufferInfo, &allocCreateInfo, alignment, &outBuffer.m_handle, &outBuffer.m_allocation, &allocInfo));
        
        // Mapped Memory, only necessary if host visible.
        if (IsHostVisibleMemory(bufferDesc.m_location))
        {
            outBuffer.m_pMappedMemory = static_cast<uint8*>(allocInfo.pMappedData) - allocInfo.offset;
        }

        // Device Address
        if (m_deviceAddressSupported)
        {
            VkBufferDeviceAddressInfo bufferDeviceAddressInfo = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
            bufferDeviceAddressInfo.buffer = outBuffer.m_handle;
            outBuffer.m_deviceAddress = vkGetBufferDeviceAddress(m_device, &bufferDeviceAddressInfo);
        }

        // Description:
        outBuffer.m_desc = bufferDesc.m_desc;

        // [TODO]: Leak detection?
        
        return EGraphicsResult::Success;
    }

    void ResourceAllocator::FreeBuffer(DeviceBuffer& buffer)
    {
        vmaDestroyBuffer(m_vmaAllocator, buffer.m_handle, buffer.m_allocation);
        buffer.m_handle = nullptr;
        buffer.m_allocation = nullptr;
    }

    EGraphicsResult ResourceAllocator::AllocateTexture(const AllocateTextureDesc& textureDesc, Texture& outTexture)
    {
        // Fill out the ImageCreateInfo object. 
        VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        m_device.FillCreateInfo(textureDesc.m_desc, imageInfo);

        // Allocation Info:
        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT | VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        allocCreateInfo.priority = textureDesc.m_priority * 0.5f + 0.5f;
        allocCreateInfo.usage = IsHostMemory(textureDesc.m_memoryLocation) ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        // Dedicated flag:
        if (textureDesc.m_isDedicated)
            allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        NES_VK_FAIL_RETURN(m_device, vmaCreateImage(m_vmaAllocator, &imageInfo, &allocCreateInfo, &outTexture.m_handle, &outTexture.m_allocation, nullptr));

        outTexture.m_desc = textureDesc.m_desc;
        outTexture.m_desc.Validate();

        return EGraphicsResult::Success;
    }

    void ResourceAllocator::FreeTexture(Texture& texture)
    {
        vmaDestroyImage(m_vmaAllocator, texture.m_handle, texture.m_allocation);
        texture.m_handle = nullptr;
        texture.m_allocation = nullptr;
    }
}



