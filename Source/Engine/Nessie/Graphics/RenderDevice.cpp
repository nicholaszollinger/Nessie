// RenderDevice.cpp
#include "RenderDevice.h"

#include <numeric>

#include "vulkan/vk_enum_string_helper.h"
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

    //----------------------------------------------------------------------------------------------------
    /// @brief : Check to see if the extension is in the supported extensions array.
    //----------------------------------------------------------------------------------------------------
    static bool IsExtensionSupported(const char* extensionName, const std::vector<ExtensionDesc>& supportedExtensions)
    {
        for (const auto& e : supportedExtensions)
        {
            if (!strcmp(extensionName, e.m_extensionName))
                return true;
        }
        
        return false;
    }

    // [TODO]: Remove? I am opting for NRI's feature setup rather than nvpro.
    [[maybe_unused]]
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

         const std::string formattedMessage = std::format("[{}] | MessageID = {}\n{}", callbackData->pMessageIdName, callbackData->messageIdNumber, callbackData->pMessage);
         device.ReportMessage(logLevel, __FILE__, __LINE__, formattedMessage.c_str(), nes::kVulkanLogTag);

         return VK_FALSE;
     }
    
    RenderDevice::~RenderDevice()
    {
        NES_ASSERT(m_vkInstance == nullptr);
    }

    bool RenderDevice::Init(const ApplicationDesc& appDesc, const RendererDesc& rendererDesc)
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

        // Create the Resource Allocator.
        m_pAllocator = Allocate<ResourceAllocator>(GetAllocationCallbacks(), *this);
        if (m_pAllocator->Init() != EGraphicsResult::Success)
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

        // Destroy the Resource Allocator.
        if (m_pAllocator)
        {
            m_pAllocator->Destroy();
            Free<ResourceAllocator>(allocationCallbacks, m_pAllocator);
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

    EGraphicsResult RenderDevice::WaitUntilIdle()
    {
        // I am not using "vkDeviceWaitIdle" because it requires host access synchronization to all queues, regardless if we are using
        // them or not. Better to do it one by one instead.
        for (auto& queueFamily : m_queueFamilies)
        {
            for (auto* pQueue : queueFamily)
            {
                const EGraphicsResult result = pQueue->WaitUntilIdle();
                if (result != EGraphicsResult::Success)
                    return result;
            }
        }

        return EGraphicsResult::Success;
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

    ResourceAllocator& RenderDevice::GetResourceAllocator() const
    {
        NES_ASSERT(m_pAllocator != nullptr);
        return *m_pAllocator;
    }

    bool RenderDevice::CheckResult(const VkResult result, const char* resultStr, const char* file, int line) const
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
            return false;
        }

        return true;
    }

    bool RenderDevice::CheckResult(const bool result, const char* resultStr, const char* file, int line) const
    {
        if (!result)
        {
            // Format just the incoming format string and arguments
            std::string formattedMessage = fmt::format("'{}' failed!", resultStr);
            
            if (m_debugMessenger.m_callback)
            {
                m_debugMessenger.SendMessage(ELogLevel::Fatal, file, line, formattedMessage.c_str(), nes::kGraphicsLogTag);
            }

            // Format the fatal error message.
            const std::string formattedFinal = fmt::format("{0}({1}): {2}", file, line, formattedMessage);
            nes::internal::HandleFatalError("Fatal Error!", formattedFinal);
            return false;
        }

        return true;
    }

    bool RenderDevice::CheckResult(const EGraphicsResult result, const char* resultStr, const char* file, int line) const
    {
        // [TODO]: It would be nice to print out the EGraphicsResult value.
        return CheckResult(result == EGraphicsResult::Success, resultStr, file, line);
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

    EGraphicsResult RenderDevice::GetSupportedDeviceExtensions(std::vector<VkExtensionProperties>& outSupportedExtensions) const
    {
        uint32 extensionCount = 0;
        NES_VK_FAIL_RETURN(*this, vkEnumerateDeviceExtensionProperties(m_vkPhysicalDevice, nullptr, &extensionCount, nullptr));
        outSupportedExtensions.resize(extensionCount);
        NES_VK_FAIL_RETURN(*this, vkEnumerateDeviceExtensionProperties(m_vkPhysicalDevice, nullptr, &extensionCount, outSupportedExtensions.data()));
        
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
        std::vector<const char*> desiredInstanceExtensions = rendererDesc.m_instanceExtensions;

        // Get the supported Instance Extensions:
        uint32 extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());
        
        // [TODO]: Print supported Extensions if asked:
        
        // Surface Support Extensions:
        // These are added regardless if headless. If I want to change this, I need to check for headless in the device extensions.
        {
            if (IsExtensionSupported(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, supportedExtensions))
                desiredInstanceExtensions.emplace_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

            if (IsExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME, supportedExtensions))
            {
                desiredInstanceExtensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);

                if (IsExtensionSupported(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME, supportedExtensions))
                    desiredInstanceExtensions.emplace_back(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);

                #if NES_PLATFORM_WINDOWS
                desiredInstanceExtensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
                #endif
                
                // [TODO]: Other platforms:
            }

            if (IsExtensionSupported(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME, supportedExtensions))
                desiredInstanceExtensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
        }

        // Debug Utils Support:
        if (IsExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, supportedExtensions))
            desiredInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        VkInstanceCreateInfo createInfo
        {
            .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext                   = nullptr, // [TODO]: rendererDesc.m_pInstanceCreateInfoExt, which makes a chain.
            .pApplicationInfo        = &appInfo,
            .enabledLayerCount       = static_cast<uint32_t>(layers.size()),
            .ppEnabledLayerNames     = layers.data(),
            .enabledExtensionCount   = static_cast<uint32_t>(desiredInstanceExtensions.size()),
            .ppEnabledExtensionNames = desiredInstanceExtensions.data(),
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
                debugMessengerInfo.pUserData = this;
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
            auto& desc = m_desc.m_physicalDeviceDesc;
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
                desc.m_queueCountByType[kGraphicsIndex] = familyProps2[queueFamilyIndices[kGraphicsIndex]].queueFamilyProperties.queueCount;
            
            if (queueFamilyIndices[kComputeIndex] != kInvalidQueueIndex)
                desc.m_queueCountByType[kComputeIndex] = familyProps2[queueFamilyIndices[kComputeIndex]].queueFamilyProperties.queueCount;
            
            if (queueFamilyIndices[kTransferIndex] != kInvalidQueueIndex)
                desc.m_queueCountByType[kTransferIndex] = familyProps2[queueFamilyIndices[kTransferIndex]].queueFamilyProperties.queueCount;
             
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

        // Set our API version; it is supported by the selected physical device.
        m_desc.m_apiVersion = rendererDesc.m_apiVersion;

        // Filter the available extensions now; otherwise the device creation will fail.
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
        m_desc.m_deviceExtensions = std::move(filteredExtensions);
         
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
        if (m_desc.m_apiVersion >= VK_API_VERSION_1_3)
            VULKAN_APPEND_EXT_TO_TAIL(features13);

        VkPhysicalDeviceVulkan13Features features14 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES};
        if (m_desc.m_apiVersion >= VK_API_VERSION_1_4)
            VULKAN_APPEND_EXT_TO_TAIL(features14);

        // Mandatory
        VkPhysicalDeviceSynchronization2Features synchronization2features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES};
        if (IsExtensionSupported(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(synchronization2features);
        }

        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES};
        if (IsExtensionSupported(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(dynamicRenderingFeatures);
        }

        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(extendedDynamicStateFeatures);
        }

        // Optional (for Vulkan < 1.2)
        VkPhysicalDeviceMaintenance4Features maintenance4Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_4_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(maintenance4Features);
        }

        VkPhysicalDeviceImageRobustnessFeatures imageRobustnessFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES};
        if (IsExtensionSupported(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(imageRobustnessFeatures);
        }

        // Optional (KHR)
        VkPhysicalDevicePresentIdFeaturesKHR presentIdFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_PRESENT_ID_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(presentIdFeatures);
        }

        VkPhysicalDevicePresentWaitFeaturesKHR presentWaitFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_PRESENT_WAIT_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(presentWaitFeatures);
        }

        VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(maintenance5Features);
        }

        VkPhysicalDeviceMaintenance6FeaturesKHR maintenance6Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_6_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(maintenance6Features);
        }

        VkPhysicalDeviceMaintenance7FeaturesKHR maintenance7Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_7_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(maintenance7Features);
        }

        // [NOTE] In Vulkan-Headers repo but not in SDK: "Maintenance 8/9"

        VkPhysicalDeviceFragmentShadingRateFeaturesKHR shadingRateFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(shadingRateFeatures);
        }

        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(rayTracingPipelineFeatures);
        }

        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(accelerationStructureFeatures);
        }

        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_RAY_QUERY_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(rayQueryFeatures);
        }

        VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR rayTracingPositionFetchFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(rayTracingPositionFetchFeatures);
        }

        VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR rayTracingMaintenanceFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(rayTracingMaintenanceFeatures);
        }

        VkPhysicalDeviceLineRasterizationFeaturesKHR lineRasterizationFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(lineRasterizationFeatures);
        }

        VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR fragmentShaderBarycentricFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(fragmentShaderBarycentricFeatures);
        }

        VkPhysicalDeviceShaderClockFeaturesKHR shaderClockFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR};
        if (IsExtensionSupported(VK_KHR_SHADER_CLOCK_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(shaderClockFeatures);
        }

        // Optional (EXT)
        VkPhysicalDeviceOpacityMicromapFeaturesEXT micromapFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(micromapFeatures);
        }

        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_MESH_SHADER_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(meshShaderFeatures);
        }

        VkPhysicalDeviceShaderAtomicFloatFeaturesEXT shaderAtomicFloatFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(shaderAtomicFloatFeatures);
        }

        VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT shaderAtomicFloat2Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(shaderAtomicFloat2Features);
        }

        VkPhysicalDeviceMemoryPriorityFeaturesEXT memoryPriorityFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(memoryPriorityFeatures);
        }

        VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT slicedViewFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(slicedViewFeatures);
        }

        VkPhysicalDeviceCustomBorderColorFeaturesEXT borderColorFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(borderColorFeatures);
        }

        VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(robustness2Features);
        }

        VkPhysicalDevicePipelineRobustnessFeaturesEXT pipelineRobustnessFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(pipelineRobustnessFeatures);
        }

        VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT fragmentShaderInterlockFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(fragmentShaderInterlockFeatures);
        }

        VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT swapchainMaintenance1Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(swapchainMaintenance1Features);
        }

        VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT presentModeFifoLatestReadyFeaturesEXT = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_EXT};
        if (IsExtensionSupported(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(presentModeFifoLatestReadyFeaturesEXT);
        }

        if (IsExtensionSupported(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME, m_desc.m_deviceExtensions))
            m_desc.m_features.m_memoryBudget = true;
        
        vkGetPhysicalDeviceFeatures2(m_vkPhysicalDevice, &features);

        // Features
        m_desc.m_features.m_descriptorIndexing = features12.descriptorIndexing;
        m_desc.m_features.m_deviceAddress = features12.bufferDeviceAddress;
        m_desc.m_features.m_swapchainMutableFormat = IsExtensionSupported(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME, m_desc.m_deviceExtensions);
        m_desc.m_features.m_presentId = presentIdFeatures.presentId;
        m_desc.m_features.m_memoryPriority = memoryPriorityFeatures.memoryPriority;
        m_desc.m_features.m_maintenance4 = features13.maintenance4 != 0 || maintenance4Features.maintenance4 != 0;
        m_desc.m_features.m_maintenance5 = maintenance5Features.maintenance5;
        m_desc.m_features.m_maintenance6 = maintenance6Features.maintenance6;
        m_desc.m_features.m_imageSlicedView = slicedViewFeatures.imageSlicedViewOf3D != 0;
        m_desc.m_features.m_customBorderColor = borderColorFeatures.customBorderColors != 0 && borderColorFeatures.customBorderColorWithoutFormat != 0;
        m_desc.m_features.m_robustness = features.features.robustBufferAccess != 0 && (imageRobustnessFeatures.robustImageAccess != 0 || features13.robustImageAccess != 0);
        m_desc.m_features.m_robustness2 = robustness2Features.robustBufferAccess2 != 0 && robustness2Features.robustImageAccess2 != 0;
        m_desc.m_features.m_pipelineRobustness = pipelineRobustnessFeatures.pipelineRobustness;
        m_desc.m_features.m_swapchainMaintenance1 = swapchainMaintenance1Features.swapchainMaintenance1;
        m_desc.m_features.m_fifoLatestReady = presentModeFifoLatestReadyFeaturesEXT.presentModeFifoLatestReady;

        // Check hard requirements:
        {
            bool hasDynamicRendering = features13.dynamicRendering != 0 || (dynamicRenderingFeatures.dynamicRendering != 0 && extendedDynamicStateFeatures.extendedDynamicState != 0);
            bool hasSynchronization2 = features13.synchronization2 != 0 || synchronization2features.synchronization2 != 0;
            NES_GRAPHICS_RETURN_FAIL(*this, hasDynamicRendering && hasSynchronization2, EGraphicsResult::Unsupported, "'DynamicRendering' and 'Synchronization2' are not supported by the device!");
        }

        // List of extensions to enable:
        std::vector<const char*> enabledExtensions;
        for (const auto& extension : m_desc.m_deviceExtensions)
        {
            enabledExtensions.push_back(extension.m_extensionName);
        }

        // Disable Robustness features:
        // Default
        robustness2Features.robustBufferAccess2 = 0;
        robustness2Features.robustImageAccess2 = 0;
        // [TODO]: Robustness = OFF.
        features.features.robustBufferAccess = 0;
        features13.robustImageAccess = 0;

        // Device Create info:
        std::array<VkDeviceQueueCreateInfo, static_cast<size_t>(EQueueType::MaxNum)> queueCreateInfos{};
        VkDeviceCreateInfo deviceCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &features,
            .queueCreateInfoCount = 0,
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
            .ppEnabledExtensionNames = enabledExtensions.data(),
        };

        // Create required QueueCreateInfo:
        // Default Zero queue priorities. I can make this an option in the future.
        std::array<float, 256> zeroPriorities = {};
        
        const auto& queueFamilyIndices = m_desc.m_physicalDeviceDesc.m_queueFamilyIndices;
        const auto& queueFamilyQueueCounts = m_desc.m_physicalDeviceDesc.m_queueCountByType;
        
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

        NES_ASSERT(vkCreateSwapchainKHR != nullptr);

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
                    const EGraphicsResult queueResult = CreateResource<DeviceQueue>(pQueue, queueFamilyType, queueFamilyIndex, handle);
                    if (queueResult == EGraphicsResult::Success)
                        queueFamily.push_back(pQueue);
                }
        
                // Update the number of queues available for this type to match the requested amount.
                m_desc.m_physicalDeviceDesc.m_queueCountByType[queueFamilyTypeIndex] = queueFamilyQueueCount;
            }
            else
            {
                // No Queues are available of this type.
                m_desc.m_physicalDeviceDesc.m_queueCountByType[queueFamilyTypeIndex] = 0;
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

        // Enable default extension support if available.
        EnableDefaultDeviceExtensions(availableExtensionsMap, outFilteredExtensions);

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

    void RenderDevice::EnableDefaultDeviceExtensions(const std::unordered_map<std::string, uint32_t>& availableExtensionsMap, std::vector<ExtensionDesc>& outDesiredExtensions) const
    {
        // Mandatory
        if (m_desc.m_apiVersion.Minor() < 3)
        {
            outDesiredExtensions.emplace_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
            outDesiredExtensions.emplace_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
            outDesiredExtensions.emplace_back(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
            outDesiredExtensions.emplace_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
        }

        // Optional for Vulkan < 1.3
        if (m_desc.m_apiVersion.Minor() < 3 && availableExtensionsMap.contains(VK_KHR_MAINTENANCE_4_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);

        if (m_desc.m_apiVersion.Minor() < 3 && availableExtensionsMap.contains(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME);

        // Optional (KHR)
        if (availableExtensionsMap.contains(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_KHR_PRESENT_ID_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_KHR_PRESENT_ID_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_KHR_PRESENT_WAIT_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_KHR_PRESENT_WAIT_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_KHR_MAINTENANCE_5_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_KHR_MAINTENANCE_6_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_KHR_MAINTENANCE_7_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_KHR_MAINTENANCE_7_EXTENSION_NAME);

        // [TODO]: Maintenance 8

        if (availableExtensionsMap.contains(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);

        // [TODO]: Ray Tracing:
        // if (availableExtensionsMap.contains(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) && !disableRayTracing)
        //     outDesiredExtensions.emplace_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
        //
        // if (availableExtensionsMap.contains(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) && !disableRayTracing)
        //     outDesiredExtensions.emplace_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        //
        // if (availableExtensionsMap.contains(VK_KHR_RAY_QUERY_EXTENSION_NAME) && !disableRayTracing)
        //     outDesiredExtensions.emplace_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
        //
        // if (availableExtensionsMap.contains(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME) && !disableRayTracing)
        //     outDesiredExtensions.emplace_back(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME);
        //
        // if (availableExtensionsMap.contains(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME) && !disableRayTracing)
        //     outDesiredExtensions.emplace_back(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_KHR_SHADER_CLOCK_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_KHR_SHADER_CLOCK_EXTENSION_NAME);

        // Optional (EXT)
        if (availableExtensionsMap.contains(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME);

        // [TODO]: Micromap:
        // if (availableExtensionsMap.contains(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME) && !disableRayTracing)
        //     outDesiredExtensions.emplace_back(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_EXT_MESH_SHADER_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME);

        // Optional
        if (availableExtensionsMap.contains(VK_NV_LOW_LATENCY_2_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_NV_LOW_LATENCY_2_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_NVX_BINARY_IMPORT_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_NVX_BINARY_IMPORT_EXTENSION_NAME);

        if (availableExtensionsMap.contains(VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME);

        // Dependencies
        if (availableExtensionsMap.contains(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME))
            outDesiredExtensions.emplace_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    }

    void RenderDevice::FillOutDeviceDesc()
    {
        // Device properties
        VkPhysicalDeviceProperties2 props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        void** pTail = &props.pNext;

        VkPhysicalDeviceVulkan11Properties props11 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES};
        VULKAN_APPEND_EXT_TO_TAIL(props11);

        VkPhysicalDeviceVulkan12Properties props12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES};
        VULKAN_APPEND_EXT_TO_TAIL(props12);
        
        const uint32 minorVersion = m_desc.m_apiVersion.Minor();
        
        VkPhysicalDeviceVulkan13Properties props13 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES};
        if (minorVersion >= 3)
        {
            VULKAN_APPEND_EXT_TO_TAIL(props13);
        }

        VkPhysicalDeviceVulkan14Properties props14 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES};
        if (minorVersion >= 4)
        {
            VULKAN_APPEND_EXT_TO_TAIL(props14);
        }

        VkPhysicalDeviceMaintenance4PropertiesKHR maintenance4Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_4_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(maintenance4Props);
        }

        VkPhysicalDeviceMaintenance5PropertiesKHR maintenance5Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(maintenance5Props);
        }

        VkPhysicalDeviceMaintenance6PropertiesKHR maintenance6Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_6_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(maintenance6Props);
        }

        VkPhysicalDeviceMaintenance7PropertiesKHR maintenance7Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_7_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(maintenance7Props);
        }

        VkPhysicalDeviceLineRasterizationPropertiesKHR lineRasterizationProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(lineRasterizationProps);
        }

        VkPhysicalDeviceFragmentShadingRatePropertiesKHR shadingRateProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(shadingRateProps);
        }

        VkPhysicalDevicePushDescriptorPropertiesKHR pushDescriptorProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(pushDescriptorProps);
        }

        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(rayTracingProps);
        }

        VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(accelerationStructureProps);
        }

        VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT};
        if (IsExtensionSupported(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(conservativeRasterProps);
            m_desc.m_tieredFeatures.m_conservativeRaster = 1;
        }

        VkPhysicalDeviceSampleLocationsPropertiesEXT sampleLocationsProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT};
        if (IsExtensionSupported(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(sampleLocationsProps);
            m_desc.m_tieredFeatures.m_sampleLocations = 1;
        }

        VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT};
        if (IsExtensionSupported(VK_EXT_MESH_SHADER_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(meshShaderProps);
        }

        VkPhysicalDeviceOpacityMicromapPropertiesEXT micromapProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT};
        if (IsExtensionSupported(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME, m_desc.m_deviceExtensions))
        {
            VULKAN_APPEND_EXT_TO_TAIL(micromapProps);
        }

        vkGetPhysicalDeviceProperties2(m_vkPhysicalDevice, &props);

        // Fill Desc:
        const VkPhysicalDeviceLimits& limits = props.properties.limits;
        
        // Memory:
        constexpr VkMemoryPropertyFlags kNeededFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        for (uint32 i = 0; i < m_memoryProperties.memoryTypeCount; ++i)
        {
            const VkMemoryType memoryType = m_memoryProperties.memoryTypes[i];
            if ((memoryType.propertyFlags & kNeededFlags) == kNeededFlags)
                m_desc.m_memory.m_deviceUploadHeapSize += m_memoryProperties.memoryHeaps[memoryType.heapIndex].size;
        }

        m_desc.m_memory.m_allocationMaxNum = limits.maxMemoryAllocationCount;
        m_desc.m_memory.m_samplerAllocationMaxNum = limits.maxSamplerAllocationCount;
        m_desc.m_memory.m_constantBufferMaxRange = limits.maxUniformBufferRange;
        m_desc.m_memory.m_storageBufferMaxRange = limits.maxStorageBufferRange;
        m_desc.m_memory.m_bufferTextureGranularity = static_cast<uint32_t>(limits.bufferImageGranularity);
        m_desc.m_memory.m_bufferMaxSize = minorVersion >= 3 ? props13.maxBufferSize : maintenance4Props.maxBufferSize;

        // Memory Alignment:
        // VUID-VkCopyBufferToImageInfo2-dstImage-07975: If "dstImage" does not have either a depth/stencil format or a multi-planar format,
        //      "bufferOffset" must be a multiple of the texel block size
        // VUID-VkCopyBufferToImageInfo2-dstImage-07978: If "dstImage" has a depth/stencil format,
        //      "bufferOffset" must be a multiple of 4
        // Least Common Multiple stride across all formats: 1, 2, 4, 8, 16 // TODO: rarely used "12" fucks up the beauty of power-of-2 numbers, such formats must be avoided!
        constexpr uint32_t kLeastCommonMultipleStrideAcrossAllFormats = 16;
        m_desc.m_memoryAlignment.m_uploadBufferTextureRow = static_cast<uint32_t>(limits.optimalBufferCopyRowPitchAlignment);
        m_desc.m_memoryAlignment.m_uploadBufferTextureSlice = std::lcm(static_cast<uint32_t>(limits.optimalBufferCopyOffsetAlignment), kLeastCommonMultipleStrideAcrossAllFormats);
        m_desc.m_memoryAlignment.m_shaderBindingTable = rayTracingProps.shaderGroupBaseAlignment;
        m_desc.m_memoryAlignment.m_bufferShaderResourceOffset = std::lcm(static_cast<uint32_t>(limits.minTexelBufferOffsetAlignment), static_cast<uint32_t>(limits.minStorageBufferOffsetAlignment));
        m_desc.m_memoryAlignment.m_constantBufferOffset = static_cast<uint32_t>(limits.minUniformBufferOffsetAlignment);
        m_desc.m_memoryAlignment.m_scratchBufferOffset = accelerationStructureProps.minAccelerationStructureScratchOffsetAlignment;
        m_desc.m_memoryAlignment.m_accelerationStructureOffset = 256; // see the spec
        m_desc.m_memoryAlignment.m_micromapOffset = 256;              // see the spec
        
    }

    bool RenderDevice::IsHostCoherentMemory(DeviceMemoryTypeIndex memoryTypeIndex) const
    {
        NES_ASSERT(memoryTypeIndex < m_memoryProperties.memoryTypeCount);
        return (m_memoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;
    }

    void RenderDevice::FillCreateInfo(const BufferDesc& bufferDesc, VkBufferCreateInfo& outInfo) const
    {
        outInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        outInfo.size = bufferDesc.m_size;
        outInfo.usage = GetVkBufferUsageFlags(bufferDesc.m_usage, bufferDesc.m_structuredStride, m_desc.m_features.m_deviceAddress);
        outInfo.sharingMode = m_numActiveFamilyIndices <= 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
        outInfo.queueFamilyIndexCount = m_numActiveFamilyIndices;
        outInfo.pQueueFamilyIndices = m_activeQueueIndices.data();
    }

    void RenderDevice::FillCreateInfo(const TextureDesc& textureDesc, VkImageCreateInfo& outInfo) const
    {
        VkImageCreateFlags flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT; // Typeless
        
        const FormatProps& formatProps = GetFormatProps(textureDesc.m_format);
        if (formatProps.m_blockWidth > 1 && (textureDesc.m_usage & ETextureUsageBits::ShaderResourceStorage))
            flags |= VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT; // Format can be used to create a view with an uncompressed format (1 texel covers 1 block)
        if (textureDesc.m_layerCount >= 6 && textureDesc.m_width == textureDesc.m_height)
            flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; // Allow cube maps
        if (textureDesc.m_type == ETextureType::Texture3D)
            flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT; // allow 3D demotion to a set of layers
        if (m_desc.m_tieredFeatures.m_sampleLocations && formatProps.m_isDepth)
            flags |= VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT;

        outInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        outInfo.flags = flags;
        outInfo.imageType = GetVkImageType(textureDesc.m_type);
        outInfo.format = GetVkFormat(textureDesc.m_format);
        outInfo.extent.width = textureDesc.m_width;
        outInfo.extent.height = math::Max(textureDesc.m_height, 1U);
        outInfo.extent.depth = math::Max(textureDesc.m_depth, 1U);
        outInfo.mipLevels = math::Max(textureDesc.m_mipCount, 1U);
        outInfo.arrayLayers = math::Max(textureDesc.m_layerCount, 1U);
        outInfo.samples = static_cast<VkSampleCountFlagBits>(math::Max(textureDesc.m_sampleCount, 1U));
        outInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        outInfo.usage = GetVkImageUsageFlags(textureDesc.m_usage);
        outInfo.sharingMode = (m_numActiveFamilyIndices <= 1 || textureDesc.m_sharingMode == ESharingMode::Exclusive) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
        outInfo.queueFamilyIndexCount = m_numActiveFamilyIndices;
        outInfo.pQueueFamilyIndices = m_activeQueueIndices.data();
        outInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }
}
