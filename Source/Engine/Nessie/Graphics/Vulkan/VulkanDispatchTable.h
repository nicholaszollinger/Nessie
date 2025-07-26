// VulkanDispatchTable.h
#pragma once
#include "VulkanCore.h"

NES_VULKAN_NAMESPACE_BEGIN

#define VK_FUNC(name) PFN_vk##name name = nullptr

//----------------------------------------------------------------------------------------------------
/// @brief : Function table for the vulkan API.
//----------------------------------------------------------------------------------------------------
struct DispatchTable
{
    // Instance, Pre-Creation:
    VK_FUNC(GetInstanceProcAddr);
    VK_FUNC(CreateInstance);
    VK_FUNC(EnumerateInstanceVersion);
    VK_FUNC(EnumerateInstanceExtensionProperties);
    VK_FUNC(EnumerateInstanceLayerProperties);

    // Instance, Post-Creation:
    VK_FUNC(DestroyInstance);
    VK_FUNC(CreateDebugUtilsMessengerEXT);
    VK_FUNC(DestroyDebugUtilsMessengerEXT);
    VK_FUNC(GetDeviceProcAddr);
    VK_FUNC(CreateDevice);
    VK_FUNC(DestroyDevice);
    VK_FUNC(GetDeviceQueue2);
    VK_FUNC(GetDeviceGroupPeerMemoryFeatures);
    VK_FUNC(EnumerateDeviceExtensionProperties);
    VK_FUNC(GetPhysicalDeviceMemoryProperties2);
    VK_FUNC(GetPhysicalDeviceFormatProperties2);
    VK_FUNC(GetPhysicalDeviceImageFormatProperties2);
    VK_FUNC(GetPhysicalDeviceProperties2);
    VK_FUNC(GetPhysicalDeviceFeatures2);
    VK_FUNC(GetPhysicalDeviceQueueFamilyProperties2);
    VK_FUNC(EnumeratePhysicalDeviceGroups);
    VK_FUNC(EnumeratePhysicalDevices);

    // VK_KHR_get_surface_capabilities2
    VK_FUNC(GetPhysicalDeviceSurfaceFormats2KHR);
    VK_FUNC(GetPhysicalDeviceSurfaceCapabilities2KHR);

    // VK_KHR_surface
    VK_FUNC(GetPhysicalDeviceSurfaceSupportKHR);
    VK_FUNC(GetPhysicalDeviceSurfacePresentModesKHR);
    VK_FUNC(DestroySurfaceKHR);

#ifdef NES_PLATFORM_WINDOWS
    VK_FUNC(CreateWin32SurfaceKHR);
    VK_FUNC(GetMemoryWin32HandlePropertiesKHR);
#endif

    // VK_EXT_debug_utils
    VK_FUNC(SetDebugUtilsObjectNameEXT);
    VK_FUNC(CmdBeginDebugUtilsLabelEXT);
    VK_FUNC(CmdEndDebugUtilsLabelEXT);
    VK_FUNC(CmdInsertDebugUtilsLabelEXT);
    VK_FUNC(QueueBeginDebugUtilsLabelEXT);
    VK_FUNC(QueueEndDebugUtilsLabelEXT);
    VK_FUNC(QueueInsertDebugUtilsLabelEXT);
    
};

#undef VK_FUNC

NES_VULKAN_NAMESPACE_END