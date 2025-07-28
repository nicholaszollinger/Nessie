// RenderDevice.cpp
#include "RenderDevice.h"
#include "vulkan/vk_enum_string_helper.h"
#include "volk.h"
#include "Vulkan/VulkanConversions.h"
#include "DeviceQueue.h"
#undef SendMessage

namespace nes
{
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
    [[maybe_unused]]
    static bool IsExtensionSupported(const char* extensionName, const std::vector<const char*>& supportedExtensions)
    {
        for (const auto& e : supportedExtensions)
        {
            if (!strcmp(extensionName, e))
                return true;
        }
        
        return false;
    }

    static void NextChainPushFront(void* pMainStruct, void* pNewStruct)
    {
        auto* pNewBase  = static_cast<VkBaseOutStructure*>(pNewStruct);
        auto* pMainBase = static_cast<VkBaseOutStructure*>(pMainStruct);

        pNewBase->pNext  = pMainBase->pNext;
        pMainBase->pNext = pNewBase;
    }

    [[maybe_unused]]
     static VkBool32 VKAPI_PTR MessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT, [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* pUserData)
     {
        RenderDevice& device = *(static_cast<RenderDevice*>(pUserData));

         ELogLevel logLevel = ELogLevel::Info;
         switch (messageSeverity)
         {
             case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: logLevel = ELogLevel::Warn; break;
             case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: logLevel = ELogLevel::Error; break;
             default: break;
         }

         const std::string formattedMessage = std::format("[%s] | MessageID = 0x%x\n%s", callbackData->pMessageIdName, callbackData->messageIdNumber, callbackData->pMessage);
         device.ReportMessage(logLevel, __FILE__, __LINE__, formattedMessage.c_str(), nes::kVulkanLogTag);

         return VK_FALSE;
     }
    
    RenderDevice::~RenderDevice()
    {
        NES_ASSERT(m_vkInstance == nullptr);
    }

    bool RenderDevice::Init(const ApplicationDesc& appDesc, ApplicationWindow* /*pWindow*/, const RendererDesc& rendererDesc)
    {
        m_debugMessenger = rendererDesc.m_debugMessenger;
        m_allocationCallbacks = rendererDesc.m_allocationCallbacks;

        // Initialize the Allocation Callbacks.
        m_vkAllocationCallbacks.pUserData = const_cast<void*>(static_cast<const void*>(&GetAllocationCallbacks()));
        m_vkAllocationCallbacks.pfnAllocation = vkAllocateHostMemory;
        m_vkAllocationCallbacks.pfnReallocation = vkReallocateHostMemory;
        m_vkAllocationCallbacks.pfnFree = vkFreeHostMemory;
        m_vkAllocationCallbacksPtr = &m_vkAllocationCallbacks;

        /// Load Vulkan:
        const EGraphicsResult result = NES_VK_FAIL_REPORT(*this, volkInitialize());
        if (result != EGraphicsResult::Success)
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

    void RenderDevice::Destroy()
    {
        // Destroy the Device Queue objects.
        auto& allocationCallbacks = GetAllocationCallbacks();
        for (auto& queueFamily : m_queueFamilies)
        {
            for (auto* pQueue : queueFamily)
            {
                Free<DeviceQueue>(allocationCallbacks, pQueue);
            }
            queueFamily.clear();
        }
        
        // Destroy the Device.
        if (m_vkDevice != nullptr)
        {
            vkDestroyDevice(m_vkDevice, m_vkAllocationCallbacksPtr);
        }
        
        // Destroy the instance:
        if (m_vkInstance != nullptr)
        {
            if (m_vkDebugMessenger && vkDestroyDebugUtilsMessengerEXT)
            {
                vkDestroyDebugUtilsMessengerEXT(m_vkInstance, m_vkDebugMessenger, m_vkAllocationCallbacksPtr);
                m_vkDebugMessenger = nullptr;
            }
            
            vkDestroyInstance(m_vkInstance, m_vkAllocationCallbacksPtr);
        }

        m_vkDevice = nullptr;
        m_vkInstance = nullptr;
    }

    EGraphicsResult RenderDevice::GetQueue(const EQueueType type, const uint32 queueIndex, DeviceQueue*& outQueue)
    {
        const auto& queueFamily = m_queueFamilies[static_cast<uint32>(type)];
        if (queueFamily.empty())
            return EGraphicsResult::Unsupported;
        
        if (queueIndex < queueFamily.size())
        {
            DeviceQueue* pQueue = queueFamily[queueIndex];
            outQueue = pQueue;
        
            // Update the active family indices:
            {
                std::lock_guard lock(m_activeQueueIndicesMutex);
        
                uint32 i = 0;
                for (; i < m_activeQueueIndices.size(); ++i)
                {
                    if (m_activeQueueIndices[i] == pQueue->GetFamilyIndex())
                        break;
                }
        
                if (i == m_numActiveFamilyIndices)
                    m_activeQueueIndices[m_numActiveFamilyIndices++] = pQueue->GetFamilyIndex();
            }
        
            return EGraphicsResult::Success;
        }
        
        return EGraphicsResult::Failure;
    }

    void RenderDevice::CheckResult(const VkResult result, const char* resultStr, const char* file, int line) const
    {
        if (result < 0)
        {
            const char* vulkanErrorMsg = string_VkResult(result);
            
            // Format just the incoming format string and arguments
            std::string formattedMessage = fmt::format("{} failed! Vulkan Error: {}", resultStr, vulkanErrorMsg);
            
            if (m_debugMessenger.m_callback)
            {
                m_debugMessenger.SendMessage(ELogLevel::Fatal, file, line, formattedMessage.c_str(), nes::kVulkanLogTag);
            }

            // Format the fatal error message.
            const std::string formattedFinal = fmt::format("{0}({1}): {2}", file, line, formattedMessage);
            nes::internal::HandleFatalError("Fatal Error!", formattedFinal);
        }
    }

    EGraphicsResult RenderDevice::ReportOnError(const VkResult result, const char* resultStr, const char* file, int line) const
    {
        if (result < 0)
        {
            const char* vulkanErrorMsg = string_VkResult(result);
            if (m_debugMessenger.m_callback)
            {
                m_debugMessenger.SendMessage(ELogLevel::Fatal, file, line, fmt::format("{} failed! Vulkan Error: {}", resultStr, vulkanErrorMsg).c_str(), nes::kVulkanLogTag);
            }

            return ConvertVkResultToGraphics(result);
        }
        
        return EGraphicsResult::Success;
    }

    void RenderDevice::ReportMessage(const ELogLevel level, const char* file, const uint32 line, const char* message, const LogTag& tag) const
    {
        if (m_debugMessenger.m_callback)
        {
            m_debugMessenger.SendMessage(level, file, line, message, tag);
        }
    }

    EGraphicsResult RenderDevice::GetSupportedDeviceExtensions(std::vector<VkExtensionProperties>& outExtensions) const
    {
        uint32 extensionCount = 0;
        NES_VK_FAIL_RETURN(*this, vkEnumerateDeviceExtensionProperties(m_vkPhysicalDevice, nullptr, &extensionCount, nullptr));
        outExtensions.resize(extensionCount);
        NES_VK_FAIL_RETURN(*this, vkEnumerateDeviceExtensionProperties(m_vkPhysicalDevice, nullptr, &extensionCount, outExtensions.data()));
        return EGraphicsResult::Success;
    }

    EGraphicsResult RenderDevice::CreateInstance(const ApplicationDesc& appDesc, const RendererDesc& rendererDesc)
    {
        VkApplicationInfo appInfo
        {
            .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName   = appDesc.m_appName.c_str(),
            .applicationVersion = appDesc.m_appVersion,
            .pEngineName        = "Nessie",
            .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion         = rendererDesc.m_apiVersion,
        };

        std::vector<const char*> layers;
        if(rendererDesc.m_enableValidationLayer)
        {
            layers.push_back("VK_LAYER_KHRONOS_validation");
        }

        // [TODO]: Add rendererDesc instanceExtensions.
        std::vector<const char*> outDesiredExtensions = rendererDesc.m_instanceExtensions;

        // Get the supported Instance Extensions:
        uint32 extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());
        
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

        VkInstanceCreateInfo createInfo
        {
            .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext                   = nullptr, // [TODO]: rendererDesc.m_pInstanceCreateInfoExt, which makes a chain.
            .pApplicationInfo        = &appInfo,
            .enabledLayerCount       = static_cast<uint32_t>(layers.size()),
            .ppEnabledLayerNames     = layers.data(),
            .enabledExtensionCount   = static_cast<uint32_t>(outDesiredExtensions.size()),
            .ppEnabledExtensionNames = outDesiredExtensions.data(),
        };

        // Create the Instance:
        NES_VK_FAIL_RETURN(*this, vkCreateInstance(&createInfo, m_vkAllocationCallbacksPtr, &m_vkInstance));
        
        // Load Instance functions:
        volkLoadInstance(m_vkInstance);
        
        // Create the Debug Messenger:
        if (rendererDesc.m_enableValidationLayer)
        {
            if (vkCreateDebugUtilsMessengerEXT)
            {
                VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
                debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT    // GPU info, bug
                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;                                    // Invalid usage
                debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT         // Violation of spec
                    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;                                  // Non-optimal use
                
                debugMessengerInfo.pfnUserCallback = MessageCallback;
                NES_VK_FAIL_RETURN(*this, vkCreateDebugUtilsMessengerEXT(m_vkInstance, &debugMessengerInfo, m_vkAllocationCallbacksPtr, &m_vkDebugMessenger));
            }
            else
            {
                NES_GRAPHICS_WARN(*this, "Missing VK_EXT_DEBUG_UTILS extension, cannot use vkCreateDebugUtilsMessengerEXT for validation layers.");    
            }
        }
        
        return EGraphicsResult::Success;
    }

    #define GRAPHICS_QUEUE_SCORE ((graphics ? 100 : 0) + (compute ? 10 : 0) + (transfer ? 10 : 0) + (sparse ? 5 : 0) + (videoDecode ? 2 : 0) + (videoEncode ? 2 : 0) + (protect ? 1 : 0) + (opticalFlow ? 1 : 0))
    #define COMPUTE_QUEUE_SCORE  ((!graphics ? 10 : 0) + (compute ? 100 : 0) + (!transfer ? 10 : 0) + (sparse ? 5 : 0) + (!videoDecode ? 2 : 0) + (!videoEncode ? 2 : 0) + (protect ? 1 : 0) + (!opticalFlow ? 1 : 0))
    #define TRANSFER_QUEUE_SCORE ((!graphics ? 10 : 0) + (!compute ? 10 : 0) + (transfer ? 100 * familyProps.queueCount : 0) + (sparse ? 5 : 0) + (!videoDecode ? 2 : 0) + (!videoEncode ? 2 : 0) + (protect ? 1 : 0) + (!opticalFlow ? 1 : 0))

    EGraphicsResult RenderDevice::SelectPhysicalDevice(const RendererDesc& rendererDesc)
    {
        NES_ASSERT(m_vkInstance != nullptr);

        // Get the Physical Device Handles on the system.
        uint32 numPhysicalDevices = 0;
        VkResult vkResult = vkEnumeratePhysicalDevices(m_vkInstance, &numPhysicalDevices, nullptr);
        NES_VK_FAIL_RETURN(*this, vkResult);
        
        // No Physical Devices present:
        if (numPhysicalDevices == 0)
        {
            NES_GRAPHICS_ERROR(*this, "No Physical Devices Found!");
            return EGraphicsResult::Failure;
        }
        
        std::vector<VkPhysicalDevice> physicalDevices(numPhysicalDevices);
        vkResult = vkEnumeratePhysicalDevices(m_vkInstance, &numPhysicalDevices, physicalDevices.data());
        NES_VK_FAIL_RETURN(*this, vkResult);

        for (uint32 i = 0; i < numPhysicalDevices; ++i)
        {
            auto& physicalDevice = physicalDevices[i];
            VkPhysicalDeviceProperties2 props{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
            vkGetPhysicalDeviceProperties2(physicalDevice, &props);

            // API version check. Must support 1.2+
            if (props.properties.apiVersion < VK_API_VERSION_1_2 || props.properties.apiVersion < rendererDesc.m_apiVersion)
                continue;

            // Device Type check:
            if (props.properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                continue;


            std::array<uint32 , static_cast<size_t>(EQueueType::MaxNum)> queueFamilyIndices{};
            queueFamilyIndices.fill(kInvalidQueueIndex);
             
            // Get the queue family info.
            uint32 familyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &familyCount, nullptr);

            std::vector<VkQueueFamilyProperties2> familyProps2(familyCount);
            for (uint32 j = 0; j < familyCount; ++j)
            {
                familyProps2[j] = { .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 };
            }
            vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &familyCount, familyProps2.data());

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
            desc.m_vendor = GetVendorFromID(props.properties.vendorID);
            desc.m_architecture = GetPhysicalDeviceTypeFromVulkanType(props.properties.deviceType);
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
            vkGetPhysicalDeviceMemoryProperties2(m_vkPhysicalDevice, &memProps);
            m_memoryProperties = memProps.memoryProperties;
             
            ReportMessage(ELogLevel::Info, __FILE__, __LINE__, std::format("Selected Device: {}", desc.m_name).c_str());
            break;
        }

        // No Suitable devices found:
        if (!m_vkPhysicalDevice)
        {
            NES_GRAPHICS_ERROR(*this, "No Physical Devices found that support the given RendererDesc!");
            return EGraphicsResult::Failure;
        }

        // Filter the available extensions otherwise the device creation will fail
        std::vector<ExtensionDesc> filteredExtensions;
        std::vector<VkExtensionProperties> extensionProps;
        if (EGraphicsResult extResult = GetSupportedDeviceExtensions(extensionProps); extResult != EGraphicsResult::Success)
            return extResult;

        const bool allFound = FilterAvailableExtensions(extensionProps, rendererDesc.m_deviceExtensions, filteredExtensions);
        if (!allFound)
        {
            m_vkPhysicalDevice = {};
            return EGraphicsResult::Unsupported;
        }
        m_deviceDesc.m_deviceExtensions = std::move(filteredExtensions);
         
        return EGraphicsResult::Success;
    }

#undef GRAPHICS_QUEUE_SCORE
#undef COMPUTE_QUEUE_SCORE
#undef TRANSFER_QUEUE_SCORE

#define VULKAN_APPEND_EXT_TO_TAIL(extStruct)\
    *pTail = &(extStruct); \
    pTail = &(extStruct.pNext)
    
    EGraphicsResult RenderDevice::CreateLogicalDevice(const RendererDesc& rendererDesc)
    {
        NES_ASSERT(m_vkPhysicalDevice != nullptr);
        
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

        for (const auto& extension : m_deviceDesc.m_deviceExtensions)
        {
            if (extension.m_pFeature)
                NextChainPushFront(&features, extension.m_pFeature);
        }

        std::array<VkDeviceQueueCreateInfo, static_cast<size_t>(EQueueType::MaxNum)> queueCreateInfos{};

        // List of extensions to enable:
        std::vector<const char*> enabledExtensions;
        for (const auto& extension : m_deviceDesc.m_deviceExtensions)
        {
            enabledExtensions.push_back(extension.m_extensionName);
        }
        
        VkDeviceCreateInfo deviceCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &features,
            .queueCreateInfoCount = 0,
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
            .ppEnabledExtensionNames = enabledExtensions.data(),
        };

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

        const VkResult result = vkCreateDevice(m_vkPhysicalDevice, &deviceCreateInfo, m_vkAllocationCallbacksPtr, &m_vkDevice);
        if (result != VK_SUCCESS)
        {
            NES_GRAPHICS_ERROR(*this, "Failed to create logical device! Vulkan Error: {}", string_VkResult(result));
            return EGraphicsResult::Failure;
        }
        volkLoadDevice(m_vkDevice);

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
                    vkGetDeviceQueue2(m_vkDevice, &queueInfo, &handle);

                    // Create the Queue:
                    DeviceQueue* pQueue;
                    const EGraphicsResult queueResult = Create<DeviceQueue>(pQueue, queueFamilyType, queueFamilyIndex, handle);
                    if (queueResult == EGraphicsResult::Success)
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

        return EGraphicsResult::Success;
    }

    bool RenderDevice::FilterAvailableExtensions(const std::vector<VkExtensionProperties>& supportedExtensions, const std::vector<ExtensionDesc>& desiredExtensions, std::vector<ExtensionDesc>& outFilteredExtensions) const
    {
        bool allFound = true;

        // Create a map for quick lookup of available extensions and their versions
        std::unordered_map<std::string, uint32_t> availableExtensionsMap;
        for (const auto& ext : supportedExtensions)
        {
            availableExtensionsMap[ext.extensionName] = ext.specVersion;
        }

        // Iterate through all desired extensions
        for (const auto& desiredExtension : desiredExtensions)
        {
            auto it = availableExtensionsMap.find(desiredExtension.m_extensionName);

            const bool      found        = it != availableExtensionsMap.end();
            const uint32_t  specVersion  = found ? it->second : 0;
            const bool      validVersion = desiredExtension.m_requireExactVersion ? desiredExtension.m_version == specVersion :
                                                                        desiredExtension.m_version <= specVersion;
            if (found && validVersion)
            {
                outFilteredExtensions.push_back(desiredExtension);
            }

            // Either missing or invalid version:
            else
            {
                std::string versionInfo;
                if (desiredExtension.m_version != 0 || desiredExtension.m_requireExactVersion)
                {
                    versionInfo = " (v." + std::to_string(specVersion) + " " + (specVersion ? "== " : ">= ")
                                  + std::to_string(desiredExtension.m_version) + ")";
                }
                
                if (desiredExtension.m_isRequired)
                {
                    allFound = false;
                    NES_GRAPHICS_ERROR(*this, "Extension not available: {} {}", desiredExtension.m_extensionName, versionInfo.c_str());
                }
                else
                {
                    NES_GRAPHICS_WARN(*this, "Extension not available: {} {}", desiredExtension.m_extensionName, versionInfo.c_str());
                }
            }
        }

        return allFound;
    }
}
