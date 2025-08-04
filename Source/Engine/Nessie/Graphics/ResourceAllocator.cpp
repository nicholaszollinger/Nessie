// ResourceAllocator.cpp
#include "ResourceAllocator.h"
#include "RenderDevice.h"

#include <volk.h>
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
        VmaAllocatorCreateFlags flags = {};
        if (deviceDesc.m_features.m_deviceAddress)
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
}



