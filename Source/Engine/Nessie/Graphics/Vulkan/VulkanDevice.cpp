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

        return true;
    }

    void VulkanDevice::Destroy()
    {
        // Destroy the Debug Messenger
        if (m_debugMessenger)
        {
            m_vk.DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, m_vkAllocationCallbacksPtr);
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
            for (uint32 j = 0; j < familyCount; ++j)
            {
                const VkQueueFamilyProperties& familyProps = familyProps2[j].queueFamilyProperties;

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
                        queueFamilyIndices[index] = i;
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
                        queueFamilyIndices[index] = i;
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
                        queueFamilyIndices[index] = i;
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
            
            if (rendererDesc.m_requireDedicatedComputeQueue && !hasDedicatedCompute)
                continue;
            if (rendererDesc.m_requireDedicatedTransferQueue && !hasDedicatedTransfer)
                continue;
            if (rendererDesc.m_requireSeparateComputeQueue && !hasSeparateCompute)
                continue;
            if (rendererDesc.m_requireSeparateTransferQueue && !hasSeparateTransfer)
                continue;
            
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

#undef GET_INSTANCE_FUNC

#define VULKAN_APPEND_EXT_TO_TAIL(extStruct)\
*pTail = &(extStruct); \
pTail = &((extStruct).pNext)

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

#undef VULKAN_APPEND_EXT_TO_TAIL
}
