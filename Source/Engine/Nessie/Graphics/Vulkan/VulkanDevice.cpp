// VulkanDevice.cpp
#include "VulkanDevice.h"

#include "VulkanGLFW.h"
#include "VulkanLoader.h"
#include "VulkanConversions.h"

#include "Nessie/Application/ApplicationDesc.h"
#include "Nessie/Graphics/Shared/SharedExternal.h"

#ifdef NES_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include "VulkanQueue.h"
#include "GLFW/glfw3native.h"

namespace nes
{
    // #define NES_RETURN_ON_BAD_VKRESULT(deviceBasePtr, vkResult, functionName) \
    //     if (vkResult < 0)\
    //     {               \
    //         nes::EGraphicsError _code = nes::vulkan::ConvertVkResultToErrorCode(vkResult);\
    //         NES_VULKAN_ERROR("{}(): failed! Error = {1}.", functionName, static_cast<int>(vkResult)); \
    //         return _code; \
    //     }
    
    
    static void* VKAPI_PTR vkAllocateHostMemory(void* pUserData, const size_t size, const size_t alignment, VkSystemAllocationScope)
    {
        const auto& allocationCallbacks = *static_cast<nes::AllocationCallbacks*>(pUserData);
        return allocationCallbacks.Allocate(size, alignment);
    }

    static void* VKAPI_PTR vkReallocateHostMemory(void* pUserData, void* pOriginal, const size_t size, const size_t alignment, VkSystemAllocationScope)
    {
        const auto& allocationCallbacks = *static_cast<nes::AllocationCallbacks*>(pUserData);
        return allocationCallbacks.Reallocate(pOriginal, size, alignment);
    }

    static void VKAPI_PTR vkFreeHostMemory(void* pUserData, void* pMemory)
    {
        const auto& allocationCallbacks = *static_cast<nes::AllocationCallbacks*>(pUserData);
        return allocationCallbacks.Free(pMemory);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Check to see if the extension is in the supported extensions array.
    //----------------------------------------------------------------------------------------------------
    static bool IsExtensionSupported(const char* extensionName, const std::vector<VkExtensionProperties>& supportedExtensions)
    {
        for (const auto& e : supportedExtensions)
        {
            if (!strcmp(extensionName, e.extensionName))
                return true;
        }
        
        return false;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Check to see if the extension is in the supported extensions array.
    //----------------------------------------------------------------------------------------------------
    static bool IsExtensionSupported(const char* extensionName, const std::vector<const char*>& supportedExtensions)
    {
        for (const auto& e : supportedExtensions)
        {
            if (!strcmp(extensionName, e))
                return true;
        }
        
        return false;
    }

    [[maybe_unused]]
    static VkBool32 VKAPI_PTR MessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT, [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* pUserData)
    {
        // From NRI
        { // TODO: some messages can be muted here
            // Loader info message
            if (callbackData->messageIdNumber == 0)
                return VK_FALSE;
            // Validation Information: [ WARNING-CreateInstance-status-message ] vkCreateInstance(): Khronos Validation Layer Active ...
            if (callbackData->messageIdNumber == 601872502)
                return VK_FALSE;
            // Validation Warning: [ VALIDATION-SETTINGS ] vkCreateInstance(): DebugPrintf logs to the Information message severity, enabling Information level logging otherwise the message will not be seen.
            if (callbackData->messageIdNumber == 2132353751)
                return VK_FALSE;
            // Validation Warning: [ WARNING-DEBUG-PRINTF ] Internal Warning: Setting VkPhysicalDeviceVulkan12Properties::maxUpdateAfterBindDescriptorsInAllPools to 32
            if (callbackData->messageIdNumber == 1985515673)
                return VK_FALSE;
        }
        
        VulkanDevice& device = *(static_cast<VulkanDevice*>(pUserData));

        ELogLevel logLevel = ELogLevel::Info;
        switch (messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: logLevel = ELogLevel::Warn; break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: logLevel = ELogLevel::Error; break;
            default: break;
        }

        const std::string formattedMessage = std::format("[%u] %s", callbackData->messageIdNumber, callbackData->pMessage);
        device.ReportMessage(logLevel, __FILE__, __LINE__, formattedMessage.c_str(), vulkan::kLogTag);

        return VK_FALSE;
    }

    bool VulkanDevice::Init(const ApplicationDesc& appDesc, ApplicationWindow* /*pWindow*/, const RendererDesc& rendererDesc)
    {
        // Initialize the Allocation Callbacks.
        m_vkAllocationCallbacks.pUserData = const_cast<void*>(static_cast<const void*>(&GetAllocationCallbacks()));
        m_vkAllocationCallbacks.pfnAllocation = vkAllocateHostMemory;
        m_vkAllocationCallbacks.pfnReallocation = vkReallocateHostMemory;
        m_vkAllocationCallbacks.pfnFree = vkFreeHostMemory;
        m_vkAllocationCallbacksPtr = &m_vkAllocationCallbacks;
        
        // Get the base instance functions.
        if (InitializeVulkan() != EGraphicsResult::Success)
        {
            return false;
        }
        
        // Create the Instance:
        if (CreateInstance(appDesc, rendererDesc) != EGraphicsResult::Success)
        {
            return false;
        }

        // Select the Physical Device.
        if (SelectPhysicalDevice(rendererDesc) != EGraphicsResult::Success)
        {
            return false;
        }

        // Create the Logical Device:
        if (CreateLogicalDevice(rendererDesc) != EGraphicsResult::Success)
        {
            return false;
        }

        return true;
    }

    void VulkanDevice::Destroy()
    {
        // Destroy the Device Queue objects:
        for (auto& queueFamily : m_queueFamilies)
        {
            for (auto*& pQueue : queueFamily)
            {
                Free<VulkanQueue>(GetAllocationCallbacks(), pQueue);
            }
            queueFamily.clear();
        }
        
        // Destroy the Debug Messenger
        if (m_debugMessenger)
        {
            m_vk.DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, m_vkAllocationCallbacksPtr);
        }

        // Destroy the logical device:
        if (m_vkDevice)
        {
            m_vk.DestroyDevice(m_vkDevice, m_vkAllocationCallbacksPtr);
        }
        
        // Destroy the instance:
        if (m_vkInstance)
        {
            m_vk.DestroyInstance(m_vkInstance, m_vkAllocationCallbacksPtr);    
        }
        
        // Unload the vulkan library.
        VulkanLoader::UnloadLibrary();
    }
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper to get the correct name of a core vulkan device function.
    //----------------------------------------------------------------------------------------------------
#define GET_DEVICE_OPTIONAL_CORE_FUNC(name) \
    /* Core */ \
    m_vk.name = reinterpret_cast<PFN_vk##name>(m_vk.GetDeviceProcAddr(m_vkDevice, NES_STRINGIFY(NES_MERGE_TOKENS(vk, name)))); \
    /* KHR */ \
    if (!m_vk.name) \
        m_vk.name = reinterpret_cast<PFN_vk##name>(m_vk.GetDeviceProcAddr(m_vkDevice, NES_STRINGIFY(NES_MERGE_TOKENS3(vk, name, KHR)))); \
    /* EXT (some extensions were promoted to core from EXT bypassing KHR status) */ \
    if (!m_vk.name) \
        m_vk.name = reinterpret_cast<PFN_vk##name>(m_vk.GetDeviceProcAddr(m_vkDevice, NES_STRINGIFY(NES_MERGE_TOKENS3(vk, name, EXT))));

    //----------------------------------------------------------------------------------------------------
    /// @brief : Attempt to get a core function pointer from the m_vkDevice; Possible name conversions:
    ///     "vk + [name]" or "vk + [name] + KHR" or "vk + [name] + EXT".
    ///	@param name : Name of the member variable in DispatchTable for the instance function.
    //----------------------------------------------------------------------------------------------------
#define GET_DEVICE_CORE_FUNC(name) \
    GET_DEVICE_OPTIONAL_CORE_FUNC(name); \
    if (!m_vk.name) \
    { \
        this->ReportMessage(ELogLevel::Error, __FILE__, __LINE__, std::format("Failed to get device function: `{}`", NES_STRINGIFY(NES_MERGE_TOKENS(vk, name))).c_str(), vulkan::kLogTag); \
        return EGraphicsResult::Unsupported; \
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Attempt to get a function pointer from the m_vkDevice; name conversion: "vk + name".
    ///	@param name : Name of the member variable in DispatchTable for the instance function.
    //----------------------------------------------------------------------------------------------------
#define GET_DEVICE_FUNC(name) \
    m_vk.name = reinterpret_cast<PFN_vk##name>(m_vk.GetDeviceProcAddr(m_vkDevice, NES_STRINGIFY(NES_MERGE_TOKENS(vk, name)))); \
    if (!m_vk.name) \
    { \
        this->ReportMessage(ELogLevel::Error, __FILE__, __LINE__, std::format("Failed to get device function: `{}`", NES_STRINGIFY(NES_MERGE_TOKENS(vk, name))).c_str(), vulkan::kLogTag); \
        return EGraphicsResult::Unsupported; \
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Attempt to get a function pointer from the m_vkInstance.
    ///	@param name : Name of the member variable in DispatchTable for the instance function.
    //----------------------------------------------------------------------------------------------------
#define GET_INSTANCE_FUNC(name) \
    m_vk.name = reinterpret_cast<PFN_vk##name>(m_vk.GetInstanceProcAddr(m_vkInstance, NES_STRINGIFY(NES_MERGE_TOKENS(vk, name)))); \
    if (!m_vk.name) \
    { \
        this->ReportMessage(ELogLevel::Error, __FILE__, __LINE__, std::format("Failed to get instance function: `{}`", NES_STRINGIFY(NES_MERGE_TOKENS(vk, name))).c_str(), vulkan::kLogTag); \
        return EGraphicsResult::Unsupported; \
    }

    EGraphicsResult VulkanDevice::InitializeVulkan()
    {
        // Load/Get the Vulkan library.
        m_vk.GetInstanceProcAddr = VulkanLoader::LoadVulkanLibrary();
        if (m_vk.GetInstanceProcAddr == nullptr)
        {
            NES_VULKAN_ERROR("Failed to load Vulkan Library!");
            return EGraphicsResult::Unsupported;
        }

        GET_INSTANCE_FUNC(CreateInstance)
        GET_INSTANCE_FUNC(EnumerateInstanceExtensionProperties)
        GET_INSTANCE_FUNC(EnumerateInstanceLayerProperties)
        GET_INSTANCE_FUNC(EnumerateInstanceVersion)
        
        return EGraphicsResult::Success;
    }

    EGraphicsResult VulkanDevice::ResolveInstanceDispatchTable(const std::vector<const char*>& desiredInstanceExtensions)
    {
        GET_INSTANCE_FUNC(DestroyInstance)
        GET_INSTANCE_FUNC(GetDeviceProcAddr)
        GET_INSTANCE_FUNC(CreateDebugUtilsMessengerEXT)
        GET_INSTANCE_FUNC(DestroyDebugUtilsMessengerEXT)
        GET_INSTANCE_FUNC(CreateDevice)
        GET_INSTANCE_FUNC(DestroyDevice)
        GET_INSTANCE_FUNC(GetPhysicalDeviceMemoryProperties2)
        GET_INSTANCE_FUNC(GetDeviceGroupPeerMemoryFeatures)
        GET_INSTANCE_FUNC(GetPhysicalDeviceFormatProperties2)
        GET_INSTANCE_FUNC(GetPhysicalDeviceImageFormatProperties2)
        GET_INSTANCE_FUNC(GetDeviceQueue2)
        GET_INSTANCE_FUNC(EnumeratePhysicalDeviceGroups)
        GET_INSTANCE_FUNC(GetPhysicalDeviceProperties2)
        GET_INSTANCE_FUNC(GetPhysicalDeviceFeatures2)
        GET_INSTANCE_FUNC(GetPhysicalDeviceQueueFamilyProperties2)
        GET_INSTANCE_FUNC(EnumerateDeviceExtensionProperties)
        GET_INSTANCE_FUNC(EnumeratePhysicalDevices);

        // Debug Utils Extension:
        if (IsExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, desiredInstanceExtensions))
        {
            GET_INSTANCE_FUNC(SetDebugUtilsObjectNameEXT)
            GET_INSTANCE_FUNC(CmdBeginDebugUtilsLabelEXT)
            GET_INSTANCE_FUNC(CmdEndDebugUtilsLabelEXT)
            GET_INSTANCE_FUNC(CmdInsertDebugUtilsLabelEXT)
            GET_INSTANCE_FUNC(QueueBeginDebugUtilsLabelEXT)
            GET_INSTANCE_FUNC(QueueEndDebugUtilsLabelEXT)
            GET_INSTANCE_FUNC(QueueInsertDebugUtilsLabelEXT)
        }

        // Get Surface Capabilities 2
        if (IsExtensionSupported(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, desiredInstanceExtensions)) {
            GET_INSTANCE_FUNC(GetPhysicalDeviceSurfaceFormats2KHR)
            GET_INSTANCE_FUNC(GetPhysicalDeviceSurfaceCapabilities2KHR)
        }

        // KHR Surface Extension
        if (IsExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME, desiredInstanceExtensions))
        {
            GET_INSTANCE_FUNC(GetPhysicalDeviceSurfaceSupportKHR)
            GET_INSTANCE_FUNC(GetPhysicalDeviceSurfacePresentModesKHR)
            GET_INSTANCE_FUNC(DestroySurfaceKHR)

        #ifdef NES_PLATFORM_WINDOWS
            GET_INSTANCE_FUNC(CreateWin32SurfaceKHR)
            GET_INSTANCE_FUNC(GetMemoryWin32HandlePropertiesKHR)
        #endif
        }

        return EGraphicsResult::Success;
    }

    void VulkanDevice::FilterInstanceLayers(std::vector<const char*>& layers) const
    {
        uint32 layerCount = 0;
        m_vk.EnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> supportedLayers(layerCount);
        m_vk.EnumerateInstanceLayerProperties(&layerCount, supportedLayers.data());

        for (size_t i = 0; i < layers.size(); ++i)
        {
            bool found = false;
            for (uint32 j = 0; j < layerCount && !found; ++j)
            {
                if (strcmp(supportedLayers[j].layerName, layers[i]) == 0)
                    found = true;
            }

            // Layer is not supported, remove.
            if (!found)
            {
                using DifferenceType = std::vector<const char*>::difference_type;
                layers.erase(layers.begin() + static_cast<DifferenceType>(i));
                --i; // Move back to keep the same position in the array after the removal.
            }
        }
    }

#define GRAPHICS_QUEUE_SCORE ((graphics ? 100 : 0) + (compute ? 10 : 0) + (transfer ? 10 : 0) + (sparse ? 5 : 0) + (videoDecode ? 2 : 0) + (videoEncode ? 2 : 0) + (protect ? 1 : 0) + (opticalFlow ? 1 : 0))
#define COMPUTE_QUEUE_SCORE  ((!graphics ? 10 : 0) + (compute ? 100 : 0) + (!transfer ? 10 : 0) + (sparse ? 5 : 0) + (!videoDecode ? 2 : 0) + (!videoEncode ? 2 : 0) + (protect ? 1 : 0) + (!opticalFlow ? 1 : 0))
#define TRANSFER_QUEUE_SCORE ((!graphics ? 10 : 0) + (!compute ? 10 : 0) + (transfer ? 100 * familyProps.queueCount : 0) + (sparse ? 5 : 0) + (!videoDecode ? 2 : 0) + (!videoEncode ? 2 : 0) + (protect ? 1 : 0) + (!opticalFlow ? 1 : 0))

    EGraphicsResult VulkanDevice::SelectPhysicalDevice(const RendererDesc& rendererDesc)
    {
        // Get the Physical Device Handles on the system.
        uint32 numPhysicalDevices = 0;
        VkResult vkResult = m_vk.EnumeratePhysicalDevices(m_vkInstance, &numPhysicalDevices, nullptr);
        NES_RETURN_ON_BAD_VKRESULT(*this, vkResult, "vkEnumeratePhysicalDevices")

        // No Physical Devices present:
        if (numPhysicalDevices == 0)
        {
            NES_GRAPHICS_REPORT_ERROR(*this, "No Physical Devices Found!");
            return EGraphicsResult::Failure;
        }

        std::vector<VkPhysicalDevice> physicalDevices(numPhysicalDevices);
        vkResult = m_vk.EnumeratePhysicalDevices(m_vkInstance, &numPhysicalDevices, physicalDevices.data());
        NES_RETURN_ON_BAD_VKRESULT(*this, vkResult, "vkEnumeratePhysicalDevices")

        const bool requireDeviceType = rendererDesc.m_requiredDeviceType != EPhysicalDeviceType::Unknown;
        
        for (uint32 i = 0; i < numPhysicalDevices; ++i)
        {
            auto& physicalDevice = physicalDevices[i];

            VkPhysicalDeviceProperties2 props{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
            m_vk.GetPhysicalDeviceProperties2(physicalDevice, &props);

            // API version check. Must support 1.2+
            if (props.properties.apiVersion < VK_API_VERSION_1_2 || props.properties.apiVersion < rendererDesc.m_apiVersion)
                continue;

            // Device Type check:
            if (requireDeviceType && vulkan::GetPhysicalDeviceTypeFromVulkanType(props.properties.deviceType) != rendererDesc.m_requiredDeviceType)
                continue;


            std::array<uint32 , static_cast<size_t>(EQueueType::MaxNum)> queueFamilyIndices{};
            queueFamilyIndices.fill(kInvalidQueueIndex);
            
            // Get the queue family info.
            uint32 familyCount = 0;
            m_vk.GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &familyCount, nullptr);

            std::vector<VkQueueFamilyProperties2> familyProps2(familyCount);
            for (uint32 j = 0; j < familyCount; ++j)
            {
                familyProps2[j] = { .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 };
            }
            m_vk.GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &familyCount, familyProps2.data());

            std::array<uint32, static_cast<size_t>(EQueueType::MaxNum)> scores{};
            static constexpr size_t kGraphicsIndex = static_cast<size_t>(EQueueType::Graphics);
            static constexpr size_t kComputeIndex = static_cast<size_t>(EQueueType::Compute);
            static constexpr size_t kTransferIndex = static_cast<size_t>(EQueueType::Transfer);

            // Get the best family indices for each type.
            for (uint32 familyIndex = 0; familyIndex < familyCount; ++familyIndex)
            {
                const VkQueueFamilyProperties& familyProps = familyProps2[familyIndex].queueFamilyProperties;

                bool graphics = familyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT;
                bool compute = familyProps.queueFlags & VK_QUEUE_COMPUTE_BIT;
                bool transfer = familyProps.queueFlags & VK_QUEUE_TRANSFER_BIT;
                bool sparse = familyProps.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
                bool videoDecode = familyProps.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR;
                bool videoEncode = familyProps.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR;
                bool protect = familyProps.queueFlags & VK_QUEUE_PROTECTED_BIT;
                bool opticalFlow = familyProps.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV;
                bool taken = false;

                // Graphics: Prefer as many features as possible:
                {
                    constexpr size_t index = static_cast<size_t>(EQueueType::Graphics);
                    const uint32 score = GRAPHICS_QUEUE_SCORE;

                    if (!taken && graphics && score > scores[index])
                    {
                        queueFamilyIndices[index] = familyIndex;
                        scores[index] = score;
                        taken = true;
                    }
                }

                // Compute: Prefer Dedicated Compute queue.
                {
                    constexpr size_t index = static_cast<size_t>(EQueueType::Compute);
                    const uint32 score = COMPUTE_QUEUE_SCORE;

                    if (!taken && compute && score > scores[index])
                    {
                        queueFamilyIndices[index] = familyIndex;
                        scores[index] = score;
                        taken = true;
                    }
                }

                // Transfer: Prefer Dedicated Transfer Queue.
                {
                    constexpr size_t index = static_cast<size_t>(EQueueType::Transfer);
                    const uint32 score = TRANSFER_QUEUE_SCORE;

                    if (!taken && transfer && score > scores[index])
                    {
                        queueFamilyIndices[index] = familyIndex;
                        scores[index] = score;
                        taken = true;
                    }
                }
            }

            // Check that Queue requirements are met:
            const bool transferComputeDifferent = (queueFamilyIndices[kComputeIndex] != queueFamilyIndices[kTransferIndex]);
            const bool hasSeparateCompute = (queueFamilyIndices[kComputeIndex] != queueFamilyIndices[kGraphicsIndex]);
            const bool hasDedicatedCompute = hasSeparateCompute && transferComputeDifferent;
            const bool hasSeparateTransfer = (queueFamilyIndices[kTransferIndex] != queueFamilyIndices[kGraphicsIndex]);
            const bool hasDedicatedTransfer = hasSeparateTransfer && transferComputeDifferent;
            
            const bool requiresGraphicsQueue = rendererDesc.m_requiredQueueCountsByFamily[kGraphicsIndex] > 0;
            const bool requiresComputeQueue = rendererDesc.m_requiredQueueCountsByFamily[kComputeIndex] > 0;
            const bool requiresTransferQueue = rendererDesc.m_requiredQueueCountsByFamily[kTransferIndex] > 0;

            // Check that enough Graphics Queues are available.
            if (requiresGraphicsQueue && rendererDesc.m_requiredQueueCountsByFamily[kGraphicsIndex] > familyProps2[queueFamilyIndices[kGraphicsIndex]].queueFamilyProperties.queueCount)
                continue;

            // Check Compute Queue requirements.
            if (requiresComputeQueue)
            {
                // Check enough Queues are available.
                if (rendererDesc.m_requiredQueueCountsByFamily[kComputeIndex] > familyProps2[queueFamilyIndices[kComputeIndex]].queueFamilyProperties.queueCount)
                    continue;

                // Check dedicated queue.
                if (rendererDesc.m_requireDedicatedComputeQueue && !hasDedicatedCompute)
                    continue;

                // Check separate queue.
                if (rendererDesc.m_requireSeparateComputeQueue && !hasSeparateCompute)
                    continue;
            }

            // Check Compute Queue requirements.
            if (requiresTransferQueue)
            {
                // Check enough Queues are available.
                if (rendererDesc.m_requiredQueueCountsByFamily[kTransferIndex] > familyProps2[queueFamilyIndices[kTransferIndex]].queueFamilyProperties.queueCount)
                    continue;

                // Check dedicated queue.
                if (rendererDesc.m_requireDedicatedTransferQueue && !hasDedicatedTransfer)
                    continue;

                // Check separate queue.
                if (rendererDesc.m_requireSeparateTransferQueue && !hasSeparateTransfer)
                    continue;
            }
            
            // The device is If fully suitable!
            m_vkPhysicalDevice = physicalDevice;
            
            // Fill out the Physical Device Description:
            auto& desc = m_deviceDesc.m_physicalDeviceDesc;
            memcpy(desc.m_name, props.properties.deviceName, 256);
            desc.m_deviceID = props.properties.deviceID;
            desc.m_vendor = vulkan::GetVendorFromID(props.properties.vendorID);
            desc.m_architecture = vulkan::GetPhysicalDeviceTypeFromVulkanType(props.properties.deviceType);
            desc.m_driverVersion = props.properties.driverVersion;
            desc.m_apiSupport = props.properties.apiVersion;
            desc.m_queueFamilyIndices[kGraphicsIndex] = queueFamilyIndices[kGraphicsIndex];
            desc.m_queueFamilyIndices[kComputeIndex] = queueFamilyIndices[kComputeIndex];
            desc.m_queueFamilyIndices[kTransferIndex] = queueFamilyIndices[kTransferIndex];
            
            if (queueFamilyIndices[kGraphicsIndex] != kInvalidQueueIndex)
                desc.m_numQueuesByType[kGraphicsIndex] = familyProps2[queueFamilyIndices[kGraphicsIndex]].queueFamilyProperties.queueCount;

            if (queueFamilyIndices[kComputeIndex] != kInvalidQueueIndex)
                desc.m_numQueuesByType[kComputeIndex] = familyProps2[queueFamilyIndices[kComputeIndex]].queueFamilyProperties.queueCount;
            
            if (queueFamilyIndices[kTransferIndex] != kInvalidQueueIndex)
                desc.m_numQueuesByType[kTransferIndex] = familyProps2[queueFamilyIndices[kTransferIndex]].queueFamilyProperties.queueCount;
            
            // Memory Properties for the selected Physical Device:
            VkPhysicalDeviceMemoryProperties2 memProps {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
            m_vk.GetPhysicalDeviceMemoryProperties2(m_vkPhysicalDevice, &memProps);
            m_memoryProperties = memProps.memoryProperties;
            
            ReportMessage(ELogLevel::Info, __FILE__, __LINE__, std::format("Selected Device: {}", desc.m_name).c_str());
            break;
        }

        // No Suitable devices found:
        if (!m_vkPhysicalDevice)
        {
            NES_GRAPHICS_REPORT_ERROR(*this, "No Physical Devices found that support the given RendererDesc!");
            return EGraphicsResult::Failure;
        }
        
        return EGraphicsResult::Success;
    }

#define VULKAN_APPEND_EXT_TO_TAIL(extStruct)\
*pTail = &(extStruct); \
pTail = &(extStruct.pNext)

    EGraphicsResult VulkanDevice::CreateInstance(const ApplicationDesc& appDesc, const RendererDesc& rendererDesc)
    {
        // Version Check:
        uint32 instanceVersion = VK_API_VERSION_1_3; // Minimum.
        if (rendererDesc.m_apiVersion > instanceVersion)
        {
            if (m_vk.EnumerateInstanceVersion != nullptr)
            {
                const VkResult result = m_vk.EnumerateInstanceVersion(&instanceVersion);
                // Should always return VK_SUCCESS
                if (result != VK_SUCCESS && rendererDesc.m_apiVersion > 0)
                {
                    NES_VULKAN_ERROR("Requested API Version unavailable!");
                    return EGraphicsResult::Unsupported;
                }
            }
            
            if (m_vk.EnumerateInstanceVersion == nullptr || instanceVersion < rendererDesc.m_apiVersion)
            {
                NES_VULKAN_ERROR("Requested API Version unavailable!");
                return EGraphicsResult::Unsupported;
            }
        }
        
        // [TODO]: Add rendererDesc instanceExtensions.
        std::vector<const char*> outDesiredExtensions;

        // Get the supported Instance Extensions:
        uint32 extensionCount = 0;
        m_vk.EnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
        m_vk.EnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());
        
        // [TODO]: Print supported Extensions if asked:
        
        // Surface Support, only added if not headless.
        if (!appDesc.m_isHeadless)
        {
            if (IsExtensionSupported(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, supportedExtensions))
                outDesiredExtensions.emplace_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

            if (IsExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME, supportedExtensions))
            {
                outDesiredExtensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);

                if (IsExtensionSupported(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME, supportedExtensions))
                    outDesiredExtensions.emplace_back(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);

                #if NES_PLATFORM_WINDOWS
                outDesiredExtensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
                #endif
            }

            if (IsExtensionSupported(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME, supportedExtensions))
                outDesiredExtensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
        }

        // Debug Utils Support:
        if (IsExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, supportedExtensions))
            outDesiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        // Instance Layer Support:
        std::vector<const char*> layers{};
        if (rendererDesc.m_enableValidationLayer)
            layers.emplace_back("VK_LAYER_KHRONOS_validation");
        
        FilterInstanceLayers(layers);
        
        // Application Info:
        VkApplicationInfo appInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .pNext = nullptr};
        appInfo.pEngineName = "Nessie";
        appInfo.apiVersion = rendererDesc.m_apiVersion;
        appInfo.pApplicationName = appDesc.m_appName.c_str();
        appInfo.applicationVersion = appDesc.m_appVersion;
        
        // Instance Create Info.
        VkInstanceCreateInfo instanceCreateInfo{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pNext = nullptr};
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(outDesiredExtensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = outDesiredExtensions.data();
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        instanceCreateInfo.ppEnabledLayerNames = layers.data();
        const void** pTail = &instanceCreateInfo.pNext;
        
        // Debug Messenger
        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, .pNext = nullptr};
        if (rendererDesc.m_useDebugMessenger && rendererDesc.m_debugMessenger.m_callback != nullptr)
        {
            debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            
            debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
             
            debugMessengerCreateInfo.pUserData = this;
            debugMessengerCreateInfo.pfnUserCallback = MessageCallback;
            VULKAN_APPEND_EXT_TO_TAIL(debugMessengerCreateInfo);
        }
        
        // Validation Features:
        constexpr VkValidationFeatureEnableEXT kEnabledValidateFeatures[] =
        {
            VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
        };
        
        VkValidationFeaturesEXT validationFeatures{.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT, .pNext = nullptr};
        if (rendererDesc.m_enableValidationLayer)
        {
            validationFeatures.enabledValidationFeatureCount = 1;
            validationFeatures.pEnabledValidationFeatures = kEnabledValidateFeatures;
            VULKAN_APPEND_EXT_TO_TAIL(validationFeatures);
        }
        
        VkInstance instance;
        VkResult result = m_vk.CreateInstance(&instanceCreateInfo, m_vkAllocationCallbacksPtr, &instance);
        NES_RETURN_ON_BAD_VKRESULT(*this, result, "vkCreateInstance")
        m_vkInstance = instance;

        // Resolve Instance Dispatch Table.
        if (ResolveInstanceDispatchTable(outDesiredExtensions) != EGraphicsResult::Success)
        {
            return EGraphicsResult::Unsupported;
        }

        // Create the Debug Messenger.
        if (rendererDesc.m_enableValidationLayer)
        {
            result = m_vk.CreateDebugUtilsMessengerEXT(m_vkInstance, &debugMessengerCreateInfo, m_vkAllocationCallbacksPtr, &m_debugMessenger);
            NES_RETURN_ON_BAD_VKRESULT(*this, result, "vkCreateDebugUtilsMessengerEXT")
        }
            
        return EGraphicsResult::Success;
    }

    EGraphicsResult VulkanDevice::CreateLogicalDevice(const RendererDesc& rendererDesc)
    {
        // Set our API version.
        m_deviceDesc.m_apiVersion = rendererDesc.m_apiVersion;
        
        std::vector<const char*> deviceExtensions{};
        ProcessDeviceExtensions(deviceExtensions, false); // TODO: Check Ray Tracing Support.

        // [TODO]: Add rendererDesc device Extensions to the array.

        // Device features
        VkPhysicalDeviceFeatures2 features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        void** pTail = &features.pNext;

        VkPhysicalDeviceVulkan11Features features11 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
        VULKAN_APPEND_EXT_TO_TAIL(features11);

        VkPhysicalDeviceVulkan12Features features12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
        VULKAN_APPEND_EXT_TO_TAIL(features12);

        VkPhysicalDeviceVulkan13Features features13 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
        if (m_deviceDesc.m_apiVersion >= VK_API_VERSION_1_3)
            VULKAN_APPEND_EXT_TO_TAIL(features13);
        
        // Mandatory
        VkPhysicalDeviceSynchronization2Features synchronization2features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES};
        if (IsExtensionSupported(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(synchronization2features);
        
        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES};
        if (IsExtensionSupported(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(dynamicRenderingFeatures);
        
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(extendedDynamicStateFeatures);
        
        // Optional (for Vulkan < 1.2)
        VkPhysicalDeviceMaintenance4Features maintenance4Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_4_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(maintenance4Features);
        
        VkPhysicalDeviceImageRobustnessFeatures imageRobustnessFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES};
        if (IsExtensionSupported(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(imageRobustnessFeatures);

        // Optional (KHR)
        VkPhysicalDevicePresentIdFeaturesKHR presentIdFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_PRESENT_ID_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(presentIdFeatures);
        
        VkPhysicalDevicePresentWaitFeaturesKHR presentWaitFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_PRESENT_WAIT_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(presentWaitFeatures);
        
        VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(maintenance5Features);
        
        VkPhysicalDeviceMaintenance6FeaturesKHR maintenance6Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_6_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(maintenance6Features);
        
        VkPhysicalDeviceMaintenance7FeaturesKHR maintenance7Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_7_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(maintenance7Features);
        
        VkPhysicalDeviceMaintenance8FeaturesKHR maintenance8Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_8_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(maintenance8Features);
        
        VkPhysicalDeviceMaintenance9FeaturesKHR maintenance9Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_9_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_9_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(maintenance9Features);
        
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR shadingRateFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(shadingRateFeatures);
        
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(rayTracingPipelineFeatures);
        
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(accelerationStructureFeatures);
        
        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_RAY_QUERY_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(rayQueryFeatures);
        
        VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR rayTracingPositionFetchFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(rayTracingPositionFetchFeatures);
        
        VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR rayTracingMaintenanceFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(rayTracingMaintenanceFeatures);
        
        VkPhysicalDeviceLineRasterizationFeaturesKHR lineRasterizationFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(lineRasterizationFeatures);
        
        VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR fragmentShaderBarycentricFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(fragmentShaderBarycentricFeatures);
        
        VkPhysicalDeviceShaderClockFeaturesKHR shaderClockFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_SHADER_CLOCK_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(shaderClockFeatures);
        
        // Optional (EXT)
        VkPhysicalDeviceOpacityMicromapFeaturesEXT micromapFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(micromapFeatures);
        
        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_MESH_SHADER_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(meshShaderFeatures);
        
        VkPhysicalDeviceShaderAtomicFloatFeaturesEXT shaderAtomicFloatFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(shaderAtomicFloatFeatures);
        
        VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT shaderAtomicFloat2Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(shaderAtomicFloat2Features);
        
        VkPhysicalDeviceMemoryPriorityFeaturesEXT memoryPriorityFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(memoryPriorityFeatures);
        
        VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT slicedViewFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(slicedViewFeatures);
        
        VkPhysicalDeviceCustomBorderColorFeaturesEXT borderColorFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(borderColorFeatures);
        
        VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(robustness2Features);
        
        VkPhysicalDevicePipelineRobustnessFeaturesEXT pipelineRobustnessFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(pipelineRobustnessFeatures);
        
        VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT fragmentShaderInterlockFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(fragmentShaderInterlockFeatures);
        
        VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT swapchainMaintenance1Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(swapchainMaintenance1Features);
        
        VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT presentModeFifoLatestReadyFeaturesEXT = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME, deviceExtensions))
            VULKAN_APPEND_EXT_TO_TAIL(presentModeFifoLatestReadyFeaturesEXT);
        
        // Fill out the physical device features.
        m_vk.GetPhysicalDeviceFeatures2(m_vkPhysicalDevice, &features);
        
        // [TODO]: IsSupported Struct... Move to Device Desc?

         // Check hard Requirements
         {
             const bool hasDynamicRendering = features13.dynamicRendering != 0 || (dynamicRenderingFeatures.dynamicRendering != 0 && extendedDynamicStateFeatures.extendedDynamicState != 0);
             const bool hasSynchronization2 = features13.synchronization2 != 0 || synchronization2features.synchronization2 != 0;
        
             if (!hasDynamicRendering || !hasSynchronization2)
             {
                 NES_GRAPHICS_REPORT_ERROR(*this, "'Dynamic Rendering' and 'Synchronization 2' are not supported by this device!");
                 return EGraphicsResult::Unsupported;
             }
         }

        // Create the Device:
        // Disable Undesired Features.
        // - In NRI, it has to deal with robustness; when I know what that does, I will update this.
        // - Default was to disable these two:
        robustness2Features.robustBufferAccess2 = 0;
        robustness2Features.robustImageAccess2 = 0;

        std::array<VkDeviceQueueCreateInfo, static_cast<size_t>(EQueueType::MaxNum)> queueCreateInfos{};

        VkDeviceCreateInfo deviceCreateInfo = {.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        deviceCreateInfo.pNext = &features;
        deviceCreateInfo.queueCreateInfoCount = 0;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());

        // Default Zero queue priorities. I can make this an option in the future.
        std::array<float, 256> zeroPriorities = {};

        const auto& queueFamilyIndices = m_deviceDesc.m_physicalDeviceDesc.m_queueFamilyIndices;
        const auto& queueFamilyQueueCounts = m_deviceDesc.m_physicalDeviceDesc.m_numQueuesByType;
        
        for (uint32 i = 0; i < static_cast<size_t>(EQueueType::MaxNum); ++i)
        {
            const uint32 queueFamilyIndex = queueFamilyIndices[i];
            const uint32 queueFamilyQueueCount = queueFamilyQueueCounts[i];

            if (queueFamilyQueueCount && queueFamilyIndex != kInvalidQueueIndex)
            {
                // Get the next queue create info and increment the count.
                VkDeviceQueueCreateInfo& queueCreateInfo = queueCreateInfos[deviceCreateInfo.queueCreateInfoCount++];

                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueCount = queueFamilyQueueCount;
                queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
                queueCreateInfo.pQueuePriorities = zeroPriorities.data();
            }
        }
        
        VkDevice device = nullptr;
        VkResult vkResult = m_vk.CreateDevice(m_vkPhysicalDevice, &deviceCreateInfo, m_vkAllocationCallbacksPtr, &device);
        NES_RETURN_ON_BAD_VKRESULT(*this, vkResult, "vkCreateDevice")
        m_vkDevice = device;

        // Resolve the Dispatch Table
        {
            const EGraphicsResult result = ResolveDeviceDispatchTable(deviceExtensions);
            if (result != EGraphicsResult::Success)
                return result;
        }
        
        // Create the Device Queues:
        // [TODO]: This should be the requested number of queue families.
        for (uint32 i = 0; i < static_cast<uint32>(EQueueType::MaxNum); ++i)
        {
            const uint32 queueFamilyTypeIndex = i;
            const EQueueType queueFamilyType = static_cast<EQueueType>(i);
            const uint32 queueFamilyQueueCount = rendererDesc.m_requiredQueueCountsByFamily[queueFamilyTypeIndex];
            
            auto& queueFamily = m_queueFamilies[queueFamilyTypeIndex]; 
            const uint32 queueFamilyIndex = queueFamilyIndices[queueFamilyTypeIndex];

            if (queueFamilyIndex != kInvalidQueueIndex)
            {
                for (uint32 j = 0; j < queueFamilyQueueCount; ++j)
                {
                    VkDeviceQueueInfo2 queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2 };
                    queueInfo.queueFamilyIndex = queueFamilyIndex;
                    queueInfo.queueIndex = j;

                    VkQueue handle = nullptr;
                    m_vk.GetDeviceQueue2(m_vkDevice, &queueInfo, &handle);
                    
                    VulkanQueue* pQueue;
                    const EGraphicsResult result = CreateImplementation<VulkanQueue>(pQueue, queueFamilyType, queueFamilyIndex, handle);
                    if (result == EGraphicsResult::Success)
                        queueFamily.push_back(pQueue);
                }

                // Update the number of queues available for this type to match the requested amount.
                m_deviceDesc.m_physicalDeviceDesc.m_numQueuesByType[queueFamilyTypeIndex] = queueFamilyQueueCount;
            }
            else
            {
                // No Queues are available of this type.
                m_deviceDesc.m_physicalDeviceDesc.m_numQueuesByType[queueFamilyTypeIndex] = 0;
            }
        }

        // [TODO]:
        // Fill out the device description:

        // [TODO]:
        // Fill out the core interface.
        
        return EGraphicsResult::Success;
    }

    void VulkanDevice::ProcessDeviceExtensions(std::vector<const char*>& desiredExtensions, const bool rayTracingEnabled) const
    {
        // Get the supported extensions:
        uint32 extensionCount = 0;
        m_vk.EnumerateDeviceExtensionProperties(m_vkPhysicalDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionCount);
        m_vk.EnumerateDeviceExtensionProperties(m_vkPhysicalDevice, nullptr, &extensionCount, extensions.data());

        const bool lessThanVersion1_3 = m_deviceDesc.m_apiVersion < VK_VERSION_1_3;
        
        // Mandatory
        if (lessThanVersion1_3) {
            desiredExtensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
            desiredExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
            desiredExtensions.push_back(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
            desiredExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
        }
        
        // Optional for Vulkan < 1.3
        if (lessThanVersion1_3 && IsExtensionSupported(VK_KHR_MAINTENANCE_4_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);

        if (lessThanVersion1_3 && IsExtensionSupported(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME);
        // Optional (KHR)
        if (IsExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        if (IsExtensionSupported(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME);

        if (IsExtensionSupported(VK_KHR_PRESENT_ID_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_PRESENT_ID_EXTENSION_NAME);

        if (IsExtensionSupported(VK_KHR_PRESENT_WAIT_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_PRESENT_WAIT_EXTENSION_NAME);

        if (IsExtensionSupported(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);

        if (IsExtensionSupported(VK_KHR_MAINTENANCE_6_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);

        if (IsExtensionSupported(VK_KHR_MAINTENANCE_7_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_MAINTENANCE_7_EXTENSION_NAME);

        if (IsExtensionSupported(VK_KHR_MAINTENANCE_8_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);
        
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_9_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);

        if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

        if (IsExtensionSupported(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

        if (IsExtensionSupported(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);

        if (rayTracingEnabled && IsExtensionSupported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

        if (rayTracingEnabled && IsExtensionSupported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

        if (rayTracingEnabled && IsExtensionSupported(VK_KHR_RAY_QUERY_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);

        if (rayTracingEnabled && IsExtensionSupported(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME);

        if (rayTracingEnabled && IsExtensionSupported(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);

        if (IsExtensionSupported(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME);

        if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME);

        if (IsExtensionSupported(VK_KHR_SHADER_CLOCK_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_SHADER_CLOCK_EXTENSION_NAME);

        // Optional (EXT)
        if (IsExtensionSupported(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);

        if (IsExtensionSupported(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME);

        if (rayTracingEnabled && IsExtensionSupported(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);

        if (IsExtensionSupported(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);

        if (IsExtensionSupported(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);

        if (IsExtensionSupported(VK_EXT_MESH_SHADER_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);

        if (IsExtensionSupported(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);

        if (IsExtensionSupported(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME);

        if (IsExtensionSupported(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

        if (IsExtensionSupported(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);

        if (IsExtensionSupported(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME);

        if (IsExtensionSupported(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);

        if (IsExtensionSupported(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);

        if (IsExtensionSupported(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);

        if (IsExtensionSupported(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME);

        // Optional
        if (IsExtensionSupported(VK_NV_LOW_LATENCY_2_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_NV_LOW_LATENCY_2_EXTENSION_NAME);

        if (IsExtensionSupported(VK_NVX_BINARY_IMPORT_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_NVX_BINARY_IMPORT_EXTENSION_NAME);

        if (IsExtensionSupported(VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME);

        // Dependencies
        if (IsExtensionSupported(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, extensions))
            desiredExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    }

    EGraphicsResult VulkanDevice::ResolveDeviceDispatchTable(const std::vector<const char*> desiredDeviceExtensions)
    {
        GET_DEVICE_CORE_FUNC(CreateBuffer)
        GET_DEVICE_CORE_FUNC(CreateImage)
        GET_DEVICE_CORE_FUNC(CreateBufferView)
        GET_DEVICE_CORE_FUNC(CreateImageView)
        GET_DEVICE_CORE_FUNC(CreateSampler)
        GET_DEVICE_CORE_FUNC(CreateQueryPool)
        GET_DEVICE_CORE_FUNC(CreateCommandPool)
        GET_DEVICE_CORE_FUNC(CreateSemaphore)
        GET_DEVICE_CORE_FUNC(CreateDescriptorPool)
        GET_DEVICE_CORE_FUNC(CreatePipelineLayout)
        GET_DEVICE_CORE_FUNC(CreateDescriptorSetLayout)
        GET_DEVICE_CORE_FUNC(CreateShaderModule)
        GET_DEVICE_CORE_FUNC(CreateGraphicsPipelines)
        GET_DEVICE_CORE_FUNC(CreateComputePipelines)
        GET_DEVICE_CORE_FUNC(AllocateMemory)

        GET_DEVICE_CORE_FUNC(DestroyBuffer)
        GET_DEVICE_CORE_FUNC(DestroyImage)
        GET_DEVICE_CORE_FUNC(DestroyBufferView)
        GET_DEVICE_CORE_FUNC(DestroyImageView)
        GET_DEVICE_CORE_FUNC(DestroySampler)
        GET_DEVICE_CORE_FUNC(DestroyFramebuffer)
        GET_DEVICE_CORE_FUNC(DestroyQueryPool)
        GET_DEVICE_CORE_FUNC(DestroyCommandPool)
        GET_DEVICE_CORE_FUNC(DestroySemaphore)
        GET_DEVICE_CORE_FUNC(DestroyDescriptorPool)
        GET_DEVICE_CORE_FUNC(DestroyPipelineLayout)
        GET_DEVICE_CORE_FUNC(DestroyDescriptorSetLayout)
        GET_DEVICE_CORE_FUNC(DestroyShaderModule)
        GET_DEVICE_CORE_FUNC(DestroyPipeline)
        GET_DEVICE_CORE_FUNC(FreeMemory)
        GET_DEVICE_CORE_FUNC(FreeCommandBuffers)

        GET_DEVICE_CORE_FUNC(MapMemory)
        GET_DEVICE_CORE_FUNC(FlushMappedMemoryRanges)
        GET_DEVICE_CORE_FUNC(QueueWaitIdle)
        GET_DEVICE_CORE_FUNC(QueueSubmit2)
        GET_DEVICE_CORE_FUNC(GetSemaphoreCounterValue)
        GET_DEVICE_CORE_FUNC(WaitSemaphores)
        GET_DEVICE_CORE_FUNC(ResetCommandPool)
        GET_DEVICE_CORE_FUNC(ResetDescriptorPool)
        GET_DEVICE_CORE_FUNC(AllocateCommandBuffers)
        GET_DEVICE_CORE_FUNC(AllocateDescriptorSets)
        GET_DEVICE_CORE_FUNC(UpdateDescriptorSets)
        GET_DEVICE_CORE_FUNC(BindBufferMemory2)
        GET_DEVICE_CORE_FUNC(BindImageMemory2)
        GET_DEVICE_CORE_FUNC(GetBufferMemoryRequirements2)
        GET_DEVICE_CORE_FUNC(GetImageMemoryRequirements2)
        GET_DEVICE_CORE_FUNC(ResetQueryPool)
        GET_DEVICE_CORE_FUNC(GetBufferDeviceAddress)

        GET_DEVICE_CORE_FUNC(BeginCommandBuffer)
        GET_DEVICE_CORE_FUNC(CmdSetViewportWithCount)
        GET_DEVICE_CORE_FUNC(CmdSetScissorWithCount)
        GET_DEVICE_CORE_FUNC(CmdSetDepthBounds)
        GET_DEVICE_CORE_FUNC(CmdSetStencilReference)
        GET_DEVICE_CORE_FUNC(CmdSetBlendConstants)
        GET_DEVICE_CORE_FUNC(CmdSetDepthBias)
        GET_DEVICE_CORE_FUNC(CmdClearAttachments)
        GET_DEVICE_CORE_FUNC(CmdClearColorImage)
        GET_DEVICE_CORE_FUNC(CmdBindVertexBuffers2)
        GET_DEVICE_CORE_FUNC(CmdBindIndexBuffer)
        GET_DEVICE_CORE_FUNC(CmdBindPipeline)
        GET_DEVICE_CORE_FUNC(CmdBindDescriptorSets)
        GET_DEVICE_CORE_FUNC(CmdPushConstants)
        GET_DEVICE_CORE_FUNC(CmdDispatch)
        GET_DEVICE_CORE_FUNC(CmdDispatchIndirect)
        GET_DEVICE_CORE_FUNC(CmdDraw)
        GET_DEVICE_CORE_FUNC(CmdDrawIndexed)
        GET_DEVICE_CORE_FUNC(CmdDrawIndirect)
        GET_DEVICE_CORE_FUNC(CmdDrawIndirectCount)
        GET_DEVICE_CORE_FUNC(CmdDrawIndexedIndirect)
        GET_DEVICE_CORE_FUNC(CmdDrawIndexedIndirectCount)
        GET_DEVICE_CORE_FUNC(CmdCopyBuffer2)
        GET_DEVICE_CORE_FUNC(CmdCopyImage2)
        GET_DEVICE_CORE_FUNC(CmdResolveImage2)
        GET_DEVICE_CORE_FUNC(CmdCopyBufferToImage2)
        GET_DEVICE_CORE_FUNC(CmdCopyImageToBuffer2)
        GET_DEVICE_CORE_FUNC(CmdPipelineBarrier2)
        GET_DEVICE_CORE_FUNC(CmdBeginQuery)
        GET_DEVICE_CORE_FUNC(CmdEndQuery)
        GET_DEVICE_CORE_FUNC(CmdWriteTimestamp2)
        GET_DEVICE_CORE_FUNC(CmdCopyQueryPoolResults)
        GET_DEVICE_CORE_FUNC(CmdResetQueryPool)
        GET_DEVICE_CORE_FUNC(CmdFillBuffer)
        GET_DEVICE_CORE_FUNC(CmdBeginRendering)
        GET_DEVICE_CORE_FUNC(CmdEndRendering)
        GET_DEVICE_CORE_FUNC(EndCommandBuffer)

        if (m_deviceDesc.m_apiVersion >= VK_API_VERSION_1_3 || IsExtensionSupported(VK_KHR_MAINTENANCE_4_EXTENSION_NAME, desiredDeviceExtensions))
        {
            GET_DEVICE_CORE_FUNC(GetDeviceBufferMemoryRequirements)
            GET_DEVICE_CORE_FUNC(GetDeviceImageMemoryRequirements)
        }

        if (IsExtensionSupported(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, desiredDeviceExtensions))
        {
            GET_DEVICE_FUNC(CmdBindIndexBuffer2KHR)
        }

        if (IsExtensionSupported(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, desiredDeviceExtensions))
        {
            GET_DEVICE_FUNC(CmdPushDescriptorSetKHR)
        }

        if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, desiredDeviceExtensions))
        {
            GET_DEVICE_FUNC(CmdSetFragmentShadingRateKHR)
        }

        if (IsExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME, desiredDeviceExtensions))
        {
            GET_DEVICE_FUNC(AcquireNextImage2KHR)
            GET_DEVICE_FUNC(QueuePresentKHR)
            GET_DEVICE_FUNC(CreateSwapchainKHR)
            GET_DEVICE_FUNC(DestroySwapchainKHR)
            GET_DEVICE_FUNC(GetSwapchainImagesKHR)
        }

        if (IsExtensionSupported(VK_KHR_PRESENT_WAIT_EXTENSION_NAME, desiredDeviceExtensions))
        {
            GET_DEVICE_FUNC(WaitForPresentKHR)
        }

        if (IsExtensionSupported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, desiredDeviceExtensions))
        {
            GET_DEVICE_FUNC(CreateAccelerationStructureKHR)
            GET_DEVICE_FUNC(DestroyAccelerationStructureKHR)
            GET_DEVICE_FUNC(GetAccelerationStructureDeviceAddressKHR)
            GET_DEVICE_FUNC(GetAccelerationStructureBuildSizesKHR)
            GET_DEVICE_FUNC(CmdBuildAccelerationStructuresKHR)
            GET_DEVICE_FUNC(CmdCopyAccelerationStructureKHR)
            GET_DEVICE_FUNC(CmdWriteAccelerationStructuresPropertiesKHR)
        }

        if (IsExtensionSupported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, desiredDeviceExtensions))
        {
            GET_DEVICE_FUNC(CreateRayTracingPipelinesKHR)
            GET_DEVICE_FUNC(GetRayTracingShaderGroupHandlesKHR)
            GET_DEVICE_FUNC(CmdTraceRaysKHR)
            GET_DEVICE_FUNC(CmdTraceRaysIndirect2KHR)
        }

        if (IsExtensionSupported(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME, desiredDeviceExtensions))
        {
            GET_DEVICE_FUNC(CreateMicromapEXT)
            GET_DEVICE_FUNC(DestroyMicromapEXT)
            GET_DEVICE_FUNC(GetMicromapBuildSizesEXT)
            GET_DEVICE_FUNC(CmdBuildMicromapsEXT)
            GET_DEVICE_FUNC(CmdCopyMicromapEXT)
            GET_DEVICE_FUNC(CmdWriteMicromapsPropertiesEXT)
        }

        if (IsExtensionSupported(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME, desiredDeviceExtensions))
        {
            GET_DEVICE_FUNC(CmdSetSampleLocationsEXT)
        }

        if (IsExtensionSupported(VK_EXT_MESH_SHADER_EXTENSION_NAME, desiredDeviceExtensions))
        {
            GET_DEVICE_FUNC(CmdDrawMeshTasksEXT)
            GET_DEVICE_FUNC(CmdDrawMeshTasksIndirectEXT)
            GET_DEVICE_FUNC(CmdDrawMeshTasksIndirectCountEXT)
        }

        if (IsExtensionSupported(VK_NV_LOW_LATENCY_2_EXTENSION_NAME, desiredDeviceExtensions))
        {
            GET_DEVICE_FUNC(GetLatencyTimingsNV)
            GET_DEVICE_FUNC(LatencySleepNV)
            GET_DEVICE_FUNC(SetLatencyMarkerNV)
            GET_DEVICE_FUNC(SetLatencySleepModeNV)
        }
        
        return EGraphicsResult::Success;
    }

#undef GET_INSTANCE_FUNC
#undef GET_DEVICE_FUNC
#undef GET_DEVICE_CORE_FUNC
#undef GET_DEVICE_OPTIONAL_CORE_FUNC
#undef VULKAN_APPEND_EXT_TO_TAIL
#undef GRAPHICS_QUEUE_SCORE
#undef COMPUTE_QUEUE_SCORE
#undef TRANSFER_QUEUE_SCORE
    
}
