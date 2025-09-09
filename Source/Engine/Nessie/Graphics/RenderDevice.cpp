// RenderDevice.cpp
#include "RenderDevice.h"

#include <numeric>
#include "Vulkan/VmaUsage.h"
#include "vulkan/vk_enum_string_helper.h"
#include "Vulkan/VulkanConversions.h"
#include "DeviceQueue.h"
#include "GLFW/glfw3.h"
#include "Nessie/Application/Device/DeviceManager.h"
#undef SendMessage

namespace nes
{
    static void* VKAPI_PTR vkAllocateHostMemory(void* pUserData, const size_t size, const size_t alignment, vk::SystemAllocationScope)
    {
        const auto& allocationCallbacks = *static_cast<nes::AllocationCallbacks*>(pUserData);
        return allocationCallbacks.Allocate(size, alignment);
    }

    static void* VKAPI_PTR vkReallocateHostMemory(void* pUserData, void* pOriginal, const size_t size, const size_t alignment, vk::SystemAllocationScope)
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
    [[maybe_unused]]
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
    [[maybe_unused]]
    static bool IsExtensionSupported(const char* extensionName, const std::vector<ExtensionDesc>& supportedExtensions)
    {
        for (const auto& e : supportedExtensions)
        {
            if (!strcmp(extensionName, e.m_extensionName))
                return true;
        }
        
        return false;
    }

    [[maybe_unused]]
     static vk::Bool32 VKAPI_PTR MessageCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT, [[maybe_unused]] const vk::DebugUtilsMessengerCallbackDataEXT* callbackData, void* pUserData)
     {
        RenderDevice& device = *(static_cast<RenderDevice*>(pUserData));

         ELogLevel logLevel = ELogLevel::Info;
         switch (severity)
         {
             case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning: logLevel = ELogLevel::Warn; break;
             case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError: logLevel = ELogLevel::Error; break;
             default: break;
         }

         const std::string formattedMessage = std::format("[{}] | MessageID = {}\n{}", callbackData->pMessageIdName, callbackData->messageIdNumber, callbackData->pMessage);
         device.ReportMessage(logLevel, __FILE__, __LINE__, formattedMessage.c_str(), nes::kVulkanLogTag);

         return vk::False;
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

        // Create the VMA Allocator.
        if (CreateAllocator(rendererDesc) != EGraphicsResult::Success)
        {
            return false;
        }
        
        return true;
    }

    void RenderDevice::Destroy()
    {
        // Destroy the Device Queue objects.
        for (auto& queueFamily : m_queueFamilies)
        {
            queueFamily.clear();
        }

        // Destroy the VMA Allocator.
        if (m_vmaAllocator)
        {
            vmaDestroyAllocator(m_vmaAllocator);
            m_vmaAllocator = nullptr;
        }

        // Destructor for vk::raii types clean up the handles.
        m_vkDevice = nullptr;
        m_vkDebugMessenger = nullptr;
        m_vkInstance = nullptr;
    }

    void RenderDevice::WaitUntilIdle()
    {
        // I am not using "vkDeviceWaitIdle" because it requires host access synchronization to all queues, regardless if we are using
        // them or not. Better to do it one by one instead.
        for (auto& queueFamily : m_queueFamilies)
        {
            for (auto& queue : queueFamily)
            {
                queue.WaitUntilIdle();
            }
        }
    }

    EGraphicsResult RenderDevice::GetQueue(const EQueueType type, const uint32 queueIndex, DeviceQueue*& outQueue)
    {
        auto& queueFamily = m_queueFamilies[static_cast<uint32>(type)];
        if (queueFamily.empty())
            return EGraphicsResult::Unsupported;
        
        if (queueIndex < queueFamily.size())
        {
            DeviceQueue* pQueue = &queueFamily[queueIndex];
            outQueue = pQueue;
        
            return EGraphicsResult::Success;
        }
        
        return EGraphicsResult::Failure;
    }

    bool RenderDevice::CheckResult(const vk::Result result, const char* resultStr, const char* file, int line) const
    {
        if (result != vk::Result::eSuccess)
        {
            std::string vulkanErrorMsg = vk::to_string(result);
            
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

    EGraphicsResult RenderDevice::ReportOnError(const vk::Result result, const char* resultStr, const char* file, int line) const
    {
        if (result != vk::Result::eSuccess)
        {
            std::string vulkanErrorMsg = vk::to_string(result);
            if (m_debugMessenger.m_callback)
            {
                m_debugMessenger.SendMessage(ELogLevel::Fatal, file, line, fmt::format("{} failed! Vulkan Error: {}", resultStr, vulkanErrorMsg).c_str(), nes::kVulkanLogTag);
            }

            return ConvertVkResultToGraphics(static_cast<VkResult>(result));
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

    void RenderDevice::SetDebugNameVkObject(const NativeVkObject& object, const std::string& name) const
    {
        if (object.IsValid() && m_vkDevice != nullptr && m_vkDevice.getDispatcher()->vkSetDebugUtilsObjectNameEXT != nullptr)
        {
            vk::DebugUtilsObjectNameInfoEXT nameInfo = vk::DebugUtilsObjectNameInfoEXT()
                .setObjectHandle(reinterpret_cast<uint64>(object.m_pHandle))
                .setObjectType(object.m_type)
                .setPObjectName(name.c_str());
            
            m_vkDevice.setDebugUtilsObjectNameEXT(nameInfo);
        }
    }

    const VkAllocationCallbacks* RenderDevice::GetVkAllocationCallbacksPtr() const
    {
        const VkAllocationCallbacks& callbacks = m_vkAllocationCallbacks;
        return &callbacks;
    }

    EGraphicsResult RenderDevice::GetSupportedDeviceExtensions(std::vector<vk::ExtensionProperties>& outSupportedExtensions) const
    {
        outSupportedExtensions = m_vkPhysicalDevice.enumerateDeviceExtensionProperties();
        
        return EGraphicsResult::Success;
    }

    EGraphicsResult RenderDevice::CreateInstance(const ApplicationDesc& appDesc, const RendererDesc& rendererDesc)
    {
        // Application Info:
        const vk::ApplicationInfo appInfo = vk::ApplicationInfo()
            .setApplicationVersion(appDesc.m_appVersion)
            .setPApplicationName(appDesc.m_appName.c_str())
            .setPEngineName("Nessie")
            .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
            .setApiVersion(vk::ApiVersion14);

        // Add Validation Layers, if needed:
        std::vector<const char*> requiredLayers;
        if (rendererDesc.m_enableValidationLayer)
        {
            requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
        }

        // [TODO:LATER]: Add any user layers

        // Check if the required layers are supported by the Vulkan implementation.
        auto layerProperties = m_context.enumerateInstanceLayerProperties();
        for (const auto& requiredLayer : requiredLayers)
        {
            if (std::ranges::none_of(layerProperties, [requiredLayer](auto const& layerProperty)
                { return std::strcmp(layerProperty.layerName, requiredLayer) == 0; }))
            {
                NES_GRAPHICS_ERROR(*this, "Failed to create RenderDevice! Failed to create Instance:\n"
                                       "\t- Required layer not supported: {}", requiredLayer);
                
                return EGraphicsResult::InitializationFailed;
            }
        }
        
        // Get the required extensions for Nessie:
        auto extensions = GetRequiredInstanceExtensions(rendererDesc);

        // [TODO]: Add any user extensions:

        // Check if the required Extensions are supported by the Vulkan implementation.
        auto extensionProperties = m_context.enumerateInstanceExtensionProperties();
        for (const auto& extension : extensions)
        {
            if (std::ranges::none_of(extensionProperties, [extension](auto const& extensionProperty)
                { return std::strcmp(extensionProperty.extensionName, extension) == 0; }))
            {
                NES_GRAPHICS_ERROR(*this, "Failed to create RenderDevice! Failed to create Instance:\n"
                                       "\t- Required extension is not supported: {}", extension);
                
                return EGraphicsResult::InitializationFailed;
            }
        }

        // Create the Instance:
        vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo()
            .setEnabledExtensionCount(static_cast<uint32>(extensions.size()))
            .setEnabledLayerCount(static_cast<uint32>(requiredLayers.size()))
            .setPApplicationInfo(&appInfo)
            .setPEnabledExtensionNames(extensions)
            .setPEnabledLayerNames(requiredLayers);

        // Create the Instance:
        m_vkInstance = vk::raii::Instance(m_context, createInfo, m_vkAllocationCallbacks);
        
        // Setup the Debug Messenger for Vulkan.
        if (rendererDesc.m_enableValidationLayer)
            SetupDebugMessenger();
        
        return EGraphicsResult::Success;
    }

    std::vector<const char*> RenderDevice::GetRequiredInstanceExtensions(const RendererDesc& rendererDesc) const
    {
        uint32 glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (rendererDesc.m_enableValidationLayer)
        {
            extensions.emplace_back(vk::EXTDebugUtilsExtensionName);
        }

        // Surface Support Extensions - these are added regardless if headless.
        extensions.emplace_back(vk::KHRGetSurfaceCapabilities2ExtensionName);
        extensions.emplace_back(vk::KHRSurfaceExtensionName);
        extensions.emplace_back(vk::EXTSurfaceMaintenance1ExtensionName);
        extensions.emplace_back(vk::EXTSwapchainColorSpaceExtensionName);
        
        #if NES_PLATFORM_WINDOWS
        extensions.emplace_back(vk::KHRWin32SurfaceExtensionName);
        #endif

        return extensions;
    }

    void RenderDevice::SetupDebugMessenger()
    {
        constexpr vk::DebugUtilsMessageSeverityFlagsEXT kSeverityFlags
        (
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
        );
        
        constexpr vk::DebugUtilsMessageTypeFlagsEXT kMessageTypeFlags
        (
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
        );
        
        const vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT = vk::DebugUtilsMessengerCreateInfoEXT()
            .setMessageSeverity(kSeverityFlags)
            .setMessageType(kMessageTypeFlags)
            .setPfnUserCallback(&MessageCallback)
            .setPUserData(this);
        
        m_vkDebugMessenger = m_vkInstance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
    }
    
    EGraphicsResult RenderDevice::SelectPhysicalDevice(const RendererDesc& rendererDesc)
    {
        NES_ASSERT(m_vkInstance != nullptr);

        // Assemble Required Extension Info:
        // - These are required by Nessie:
        std::vector<const char*> requiredDeviceExtensions =
        {
            vk::KHRSwapchainExtensionName,
            vk::KHRSpirv14ExtensionName,
            vk::KHRSynchronization2ExtensionName,
            vk::KHRCreateRenderpass2ExtensionName
        };
        
        // - Add application requirements:
        for (auto& extension : rendererDesc.m_deviceExtensions)
        {
            requiredDeviceExtensions.emplace_back(extension.m_extensionName);
        }

        // Get the available Physical Devices:
        auto devices = m_vkInstance.enumeratePhysicalDevices();
        if (devices.empty())
        {
            NES_GRAPHICS_ERROR(*this, "No Physical Devices Found!");
            return EGraphicsResult::InitializationFailed;
        }
        
        // Check each device for suitability:
        for (const auto& device : devices)
        {
            const auto deviceProps = device.getProperties();
            const auto deviceExtensions = device.enumerateDeviceExtensionProperties();

            // API version check. Must support 1.3+
            if (deviceProps.apiVersion < VK_API_VERSION_1_3 || deviceProps.apiVersion < rendererDesc.m_apiVersion)
                continue;

            // Device Type check:
            // [TODO]: Allow the Renderer Desc to require this or not?
            if (deviceProps.deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
                continue;

            // Check Required Features for Nessie:
            auto features = device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
            const bool supportsRequiredFeatures = features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering && features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;
            if (!supportsRequiredFeatures)
                continue;

            // Check required Extensions for Nessie and Application:
            bool supportsAllRequiredExtensions = std::ranges::all_of(requiredDeviceExtensions, [&deviceExtensions](const auto& requiredDeviceExtension)
            {
                return std::ranges::any_of(deviceExtensions, [&requiredDeviceExtension](const auto& availableExtension)
                {
                    return strcmp(availableExtension.extensionName, requiredDeviceExtension) == 0;
                });
            });
            if (!supportsAllRequiredExtensions)
                continue;

            // Check Queue Family Support:
            std::array<uint32 , static_cast<size_t>(EQueueType::MaxNum)> queueFamilyIndices{};
            queueFamilyIndices.fill(kInvalidQueueIndex);
            const auto familyProps = device.getQueueFamilyProperties();
            if (!QueryQueueFamilySupport(rendererDesc, familyProps, queueFamilyIndices))
                continue;
            
            // Device is Suitable! Fill out device description, and break out.
            m_vkPhysicalDevice = device;
            
            auto& desc = m_desc.m_physicalDeviceDesc;
            memcpy(desc.m_name, deviceProps.deviceName, 256);
            desc.m_deviceID = deviceProps.deviceID;
            desc.m_vendor = GetVendorFromID(deviceProps.vendorID);
            desc.m_architecture = GetPhysicalDeviceTypeFromVulkanType(deviceProps.deviceType);
            desc.m_driverVersion = deviceProps.driverVersion;
            desc.m_apiSupport = deviceProps.apiVersion;
            desc.m_queueFamilyIndices[kGraphicsIndex] = queueFamilyIndices[kGraphicsIndex];
            desc.m_queueFamilyIndices[kComputeIndex] = queueFamilyIndices[kComputeIndex];
            desc.m_queueFamilyIndices[kTransferIndex] = queueFamilyIndices[kTransferIndex];

            // Set Queue Counts, by type:
            if (queueFamilyIndices[kGraphicsIndex] != kInvalidQueueIndex)
                desc.m_queueCountByType[kGraphicsIndex] = familyProps[queueFamilyIndices[kGraphicsIndex]].queueCount;
        
            if (queueFamilyIndices[kComputeIndex] != kInvalidQueueIndex)
                desc.m_queueCountByType[kComputeIndex] = familyProps[queueFamilyIndices[kComputeIndex]].queueCount;
        
            if (queueFamilyIndices[kTransferIndex] != kInvalidQueueIndex)
                desc.m_queueCountByType[kTransferIndex] = familyProps[queueFamilyIndices[kTransferIndex]].queueCount;

            // Memory Properties for the selected Physical Device:
            m_memoryProperties = m_vkPhysicalDevice.getMemoryProperties2().memoryProperties;
            
            ReportMessage(ELogLevel::Info, __FILE__, __LINE__, std::format("Selected Device: {}", desc.m_name).c_str());
            break;
        }

        // Ensure we have a valid physical device:
        if (m_vkPhysicalDevice == nullptr)
        {
            NES_GRAPHICS_ERROR(*this, "No Physical Suitable Device Found!");
            return EGraphicsResult::Unsupported;
        }

        // Set our API version; it is supported by the selected physical device.
        m_desc.m_apiVersion = rendererDesc.m_apiVersion;

        // Filter the available extensions now; otherwise the device creation will fail.
        std::vector<ExtensionDesc> filteredExtensions;
        std::vector<vk::ExtensionProperties> extensionProps;
        if (EGraphicsResult extResult = GetSupportedDeviceExtensions(extensionProps); extResult != EGraphicsResult::Success)
            return extResult;
        
        const bool allFound = FilterAvailableExtensions(extensionProps, rendererDesc.m_deviceExtensions, filteredExtensions);
        if (!allFound)
        {
            m_vkPhysicalDevice = nullptr;
            return EGraphicsResult::Unsupported;
        }
        m_desc.m_deviceExtensions = std::move(filteredExtensions);
         
        return EGraphicsResult::Success;
    }

    bool RenderDevice::QueryQueueFamilySupport(const RendererDesc& rendererDesc, const std::vector<vk::QueueFamilyProperties>& families, QueueIndicesArray& outIndices)
    {
        // Array of scores for each queue type.
        std::array<uint32, static_cast<size_t>(EQueueType::MaxNum)> scores{};

        // Calculates scores for queue family indices:
        #define GRAPHICS_QUEUE_SCORE ((graphics ? 100 : 0) + (compute ? 10 : 0) + (transfer ? 10 : 0) + (sparse ? 5 : 0) + (videoDecode ? 2 : 0) + (videoEncode ? 2 : 0) + (protect ? 1 : 0) + (opticalFlow ? 1 : 0))
        #define COMPUTE_QUEUE_SCORE  ((!graphics ? 10 : 0) + (compute ? 100 : 0) + (!transfer ? 10 : 0) + (sparse ? 5 : 0) + (!videoDecode ? 2 : 0) + (!videoEncode ? 2 : 0) + (protect ? 1 : 0) + (!opticalFlow ? 1 : 0))
        #define TRANSFER_QUEUE_SCORE ((!graphics ? 10 : 0) + (!compute ? 10 : 0) + (transfer ? 100 * familyProps.queueCount : 0) + (sparse ? 5 : 0) + (!videoDecode ? 2 : 0) + (!videoEncode ? 2 : 0) + (protect ? 1 : 0) + (!opticalFlow ? 1 : 0))

        // Get the best family indices for each type.
        for (uint32 familyIndex = 0; familyIndex < static_cast<uint32>(families.size()); ++familyIndex)
        {
            const VkQueueFamilyProperties& familyProps = families[familyIndex];

            const bool graphics = familyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT;
            const bool compute = familyProps.queueFlags & VK_QUEUE_COMPUTE_BIT;
            const bool transfer = familyProps.queueFlags & VK_QUEUE_TRANSFER_BIT;
            const bool sparse = familyProps.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
            const bool videoDecode = familyProps.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR;
            const bool videoEncode = familyProps.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR;
            const bool protect = familyProps.queueFlags & VK_QUEUE_PROTECTED_BIT;
            const bool opticalFlow = familyProps.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV;
            bool taken = false;

            // Graphics: Prefer as many features as possible:
            {
                constexpr size_t index = static_cast<size_t>(EQueueType::Graphics);
                const uint32 score = GRAPHICS_QUEUE_SCORE;
                if (!taken && graphics && score > scores[index])
                {
                    outIndices[index] = familyIndex;
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
                    outIndices[index] = familyIndex;
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
                    outIndices[index] = familyIndex;
                    scores[index] = score;
                    taken = true;
                }
            }
        }

        #undef GRAPHICS_QUEUE_SCORE
        #undef COMPUTE_QUEUE_SCORE
        #undef TRANSFER_QUEUE_SCORE

        // Check that Queue requirements are met:
        const bool transferComputeDifferent = (outIndices[kComputeIndex] != outIndices[kTransferIndex]);
        const bool hasSeparateCompute = (outIndices[kComputeIndex] != outIndices[kGraphicsIndex]);
        const bool hasDedicatedCompute = hasSeparateCompute && transferComputeDifferent;
        const bool hasSeparateTransfer = (outIndices[kTransferIndex] != outIndices[kGraphicsIndex]);
        const bool hasDedicatedTransfer = hasSeparateTransfer && transferComputeDifferent;
        
        const bool requiresGraphicsQueue = rendererDesc.m_requiredQueueCountsByFamily[kGraphicsIndex] > 0;
        const bool requiresComputeQueue = rendererDesc.m_requiredQueueCountsByFamily[kComputeIndex] > 0;
        const bool requiresTransferQueue = rendererDesc.m_requiredQueueCountsByFamily[kTransferIndex] > 0;

        // Check that enough Graphics Queues are available.
        if (requiresGraphicsQueue && rendererDesc.m_requiredQueueCountsByFamily[kGraphicsIndex] > families[outIndices[kGraphicsIndex]].queueCount)
            return false;

        // Check Compute Queue requirements.
        if (requiresComputeQueue)
        {
            // Check enough Queues are available.
            if (rendererDesc.m_requiredQueueCountsByFamily[kComputeIndex] > families[outIndices[kComputeIndex]].queueCount)
                return false;

            // Check dedicated queue.
            if (rendererDesc.m_requireDedicatedComputeQueue && !hasDedicatedCompute)
                return false;

            // Check separate queue.
            if (rendererDesc.m_requireSeparateComputeQueue && !hasSeparateCompute)
                return false;
        }

        // Check Compute Queue requirements.
        if (requiresTransferQueue)
        {
            // Check enough Queues are available.
            if (rendererDesc.m_requiredQueueCountsByFamily[kTransferIndex] > families[outIndices[kTransferIndex]].queueCount)
                return false;

            // Check dedicated queue.
            if (rendererDesc.m_requireDedicatedTransferQueue && !hasDedicatedTransfer)
                return false;

            // Check separate queue.
            if (rendererDesc.m_requireSeparateTransferQueue && !hasSeparateTransfer)
                return false;
        }

        // All Queue requirements are met! 
        return true;
    }

#define VULKAN_APPEND_EXT_TO_TAIL(extStruct)\
    *pTail = &(extStruct); \
    pTail = &(extStruct.pNext)
    
    EGraphicsResult RenderDevice::CreateLogicalDevice(const RendererDesc& rendererDesc)
    {
        NES_ASSERT(m_vkPhysicalDevice != nullptr);

        // [TODO]: Add more features to the chain. 
        // Create a chain of feature structures
        vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain =
        {
            VkPhysicalDeviceFeatures2{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .features = { .dualSrcBlend = true, .samplerAnisotropy = true, } },                              // vk::PhysicalDeviceFeatures2
            VkPhysicalDeviceVulkan12Features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .timelineSemaphore = true, .bufferDeviceAddress = true,},  // vk::PhysicalDeviceVulkan12Feautres
            VkPhysicalDeviceVulkan13Features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .synchronization2 = true, .dynamicRendering = true,},       // vk::PhysicalDeviceVulkan13Features
            VkPhysicalDeviceExtendedDynamicStateFeaturesEXT{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT, .extendedDynamicState = true}              // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
        };

        // List of extensions to enable:
        std::vector<const char*> enabledExtensions;
        for (const auto& extension : m_desc.m_deviceExtensions)
        {
            enabledExtensions.push_back(extension.m_extensionName);
        }
        
        // Device Create info:
        std::array<vk::DeviceQueueCreateInfo, static_cast<size_t>(EQueueType::MaxNum)> queueCreateInfos{};
        vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo()
            .setPNext(&featureChain.get<vk::PhysicalDeviceFeatures2>())
            .setQueueCreateInfoCount(0)
            .setPQueueCreateInfos(queueCreateInfos.data())
            .setEnabledExtensionCount(static_cast<uint32_t>(enabledExtensions.size()))
            .setPEnabledExtensionNames(enabledExtensions);
        
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
                vk::DeviceQueueCreateInfo& queueCreateInfo = queueCreateInfos[deviceCreateInfo.queueCreateInfoCount++];
                queueCreateInfo.queueCount = queueFamilyQueueCount;
                queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
                queueCreateInfo.pQueuePriorities = zeroPriorities.data();
            }
        }
        m_vkDevice = vk::raii::Device(m_vkPhysicalDevice, deviceCreateInfo);

        // Get the requested Device Queues:
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
                    DeviceQueue queue = DeviceQueue(*this, queueFamilyType, queueFamilyIndex, j);
                    queue.SetDebugName(fmt::format("Queue [{}] : Family = {}", j, queueFamilyIndex));
                    queueFamily.push_back(std::move(queue));
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
        
        // // Device features
        // vk::PhysicalDeviceFeatures2 features{};
        // void** pTail = &features.pNext;
        //
        // vk::PhysicalDeviceVulkan11Features features11{};
        // VULKAN_APPEND_EXT_TO_TAIL(features11);
        //
        // vk::PhysicalDeviceVulkan12Features features12{};
        // VULKAN_APPEND_EXT_TO_TAIL(features12);
        //
        // vk::PhysicalDeviceVulkan13Features features13{};
        // if (m_desc.m_apiVersion >= VK_API_VERSION_1_3)
        //     VULKAN_APPEND_EXT_TO_TAIL(features13);
        //
        // vk::PhysicalDeviceVulkan14Features features14{};
        // if (m_desc.m_apiVersion >= VK_API_VERSION_1_4)
        //     VULKAN_APPEND_EXT_TO_TAIL(features14);
        //
        // // Mandatory
        // vk::PhysicalDeviceSynchronization2Features synchronization2features{};
        // if (IsExtensionSupported(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(synchronization2features);
        // }
        //
        // vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
        // if (IsExtensionSupported(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(dynamicRenderingFeatures);
        // }
        //
        // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{};
        // if (IsExtensionSupported(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(extendedDynamicStateFeatures);
        // }
        //
        // // Optional (for Vulkan < 1.2)
        // vk::PhysicalDeviceMaintenance4Features maintenance4Features{};
        // if (IsExtensionSupported(VK_KHR_MAINTENANCE_4_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(maintenance4Features);
        // }
        //
        // vk::PhysicalDeviceImageRobustnessFeatures imageRobustnessFeatures{};
        // if (IsExtensionSupported(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(imageRobustnessFeatures);
        // }
        //
        // // Optional (KHR)
        // vk::PhysicalDevicePresentIdFeaturesKHR presentIdFeatures{};
        // if (IsExtensionSupported(VK_KHR_PRESENT_ID_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(presentIdFeatures);
        // }
        //
        // vk::PhysicalDevicePresentWaitFeaturesKHR presentWaitFeatures{};
        // if (IsExtensionSupported(VK_KHR_PRESENT_WAIT_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(presentWaitFeatures);
        // }
        //
        // vk::PhysicalDeviceMaintenance5FeaturesKHR maintenance5Features{};
        // if (IsExtensionSupported(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(maintenance5Features);
        // }
        //
        // vk::PhysicalDeviceMaintenance6FeaturesKHR maintenance6Features{};
        // if (IsExtensionSupported(VK_KHR_MAINTENANCE_6_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(maintenance6Features);
        // }
        //
        // vk::PhysicalDeviceMaintenance7FeaturesKHR maintenance7Features{};
        // if (IsExtensionSupported(VK_KHR_MAINTENANCE_7_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(maintenance7Features);
        // }
        //
        // // [NOTE] In Vulkan-Headers repo but not in SDK: "Maintenance 8/9"
        //
        // vk::PhysicalDeviceFragmentShadingRateFeaturesKHR shadingRateFeatures{};
        // if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(shadingRateFeatures);
        // }
        //
        // vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
        // if (IsExtensionSupported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(rayTracingPipelineFeatures);
        // }
        //
        // vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
        // if (IsExtensionSupported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(accelerationStructureFeatures);
        // }
        //
        // vk::PhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{};
        // if (IsExtensionSupported(VK_KHR_RAY_QUERY_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(rayQueryFeatures);
        // }
        //
        // vk::PhysicalDeviceRayTracingPositionFetchFeaturesKHR rayTracingPositionFetchFeatures{};
        // if (IsExtensionSupported(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(rayTracingPositionFetchFeatures);
        // }
        //
        // vk::PhysicalDeviceRayTracingMaintenance1FeaturesKHR rayTracingMaintenanceFeatures{};
        // if (IsExtensionSupported(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(rayTracingMaintenanceFeatures);
        // }
        //
        // vk::PhysicalDeviceLineRasterizationFeaturesKHR lineRasterizationFeatures{};
        // if (IsExtensionSupported(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(lineRasterizationFeatures);
        // }
        //
        // vk::PhysicalDeviceFragmentShaderBarycentricFeaturesKHR fragmentShaderBarycentricFeatures{};
        // if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(fragmentShaderBarycentricFeatures);
        // }
        //
        // vk::PhysicalDeviceShaderClockFeaturesKHR shaderClockFeatures{};
        // if (IsExtensionSupported(VK_KHR_SHADER_CLOCK_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(shaderClockFeatures);
        // }
        //
        // // Optional (EXT)
        // vk::PhysicalDeviceOpacityMicromapFeaturesEXT micromapFeatures{};
        // if (IsExtensionSupported(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(micromapFeatures);
        // }
        //
        // vk::PhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};
        // if (IsExtensionSupported(VK_EXT_MESH_SHADER_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(meshShaderFeatures);
        // }
        //
        // vk::PhysicalDeviceShaderAtomicFloatFeaturesEXT shaderAtomicFloatFeatures{};
        // if (IsExtensionSupported(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(shaderAtomicFloatFeatures);
        // }
        //
        // vk::PhysicalDeviceShaderAtomicFloat2FeaturesEXT shaderAtomicFloat2Features{};
        // if (IsExtensionSupported(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(shaderAtomicFloat2Features);
        // }
        //
        // vk::PhysicalDeviceMemoryPriorityFeaturesEXT memoryPriorityFeatures{};
        // if (IsExtensionSupported(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(memoryPriorityFeatures);
        // }
        //
        // vk::PhysicalDeviceImageSlicedViewOf3DFeaturesEXT slicedViewFeatures{};
        // if (IsExtensionSupported(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(slicedViewFeatures);
        // }
        //
        // vk::PhysicalDeviceCustomBorderColorFeaturesEXT borderColorFeatures{};
        // if (IsExtensionSupported(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(borderColorFeatures);
        // }
        //
        // vk::PhysicalDeviceRobustness2FeaturesEXT robustness2Features{};
        // if (IsExtensionSupported(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(robustness2Features);
        // }
        //
        // vk::PhysicalDevicePipelineRobustnessFeaturesEXT pipelineRobustnessFeatures{};
        // if (IsExtensionSupported(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(pipelineRobustnessFeatures);
        // }
        //
        // vk::PhysicalDeviceFragmentShaderInterlockFeaturesEXT fragmentShaderInterlockFeatures{};
        // if (IsExtensionSupported(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(fragmentShaderInterlockFeatures);
        // }
        //
        // vk::PhysicalDeviceSwapchainMaintenance1FeaturesEXT swapchainMaintenance1Features{};
        // if (IsExtensionSupported(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(swapchainMaintenance1Features);
        // }
        //
        // vk::PhysicalDevicePresentModeFifoLatestReadyFeaturesEXT presentModeFifoLatestReadyFeaturesEXT{};
        // if (IsExtensionSupported(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(presentModeFifoLatestReadyFeaturesEXT);
        // }
        //
        // if (IsExtensionSupported(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME, m_desc.m_deviceExtensions))
        //     m_desc.m_features.m_memoryBudget = true;
        //
        //
        //
        // // Features
        // m_desc.m_features.m_descriptorIndexing = features12.descriptorIndexing;
        // m_desc.m_features.m_deviceAddress = features12.bufferDeviceAddress;
        // m_desc.m_features.m_swapchainMutableFormat = IsExtensionSupported(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME, m_desc.m_deviceExtensions);
        // m_desc.m_features.m_presentId = presentIdFeatures.presentId;
        // m_desc.m_features.m_memoryPriority = memoryPriorityFeatures.memoryPriority;
        // m_desc.m_features.m_maintenance4 = features13.maintenance4 != 0 || maintenance4Features.maintenance4 != 0;
        // m_desc.m_features.m_maintenance5 = maintenance5Features.maintenance5;
        // m_desc.m_features.m_maintenance6 = maintenance6Features.maintenance6;
        // m_desc.m_features.m_imageSlicedView = slicedViewFeatures.imageSlicedViewOf3D != 0;
        // m_desc.m_features.m_customBorderColor = borderColorFeatures.customBorderColors != 0 && borderColorFeatures.customBorderColorWithoutFormat != 0;
        // m_desc.m_features.m_robustness = features.features.robustBufferAccess != 0 && (imageRobustnessFeatures.robustImageAccess != 0 || features13.robustImageAccess != 0);
        // m_desc.m_features.m_robustness2 = robustness2Features.robustBufferAccess2 != 0 && robustness2Features.robustImageAccess2 != 0;
        // m_desc.m_features.m_pipelineRobustness = pipelineRobustnessFeatures.pipelineRobustness;
        // m_desc.m_features.m_swapchainMaintenance1 = swapchainMaintenance1Features.swapchainMaintenance1;
        // m_desc.m_features.m_fifoLatestReady = presentModeFifoLatestReadyFeaturesEXT.presentModeFifoLatestReady;
        //
        // // Check hard requirements:
        // {
        //     bool hasDynamicRendering = features13.dynamicRendering != 0 || (dynamicRenderingFeatures.dynamicRendering != 0 && extendedDynamicStateFeatures.extendedDynamicState != 0);
        //     bool hasSynchronization2 = features13.synchronization2 != 0 || synchronization2features.synchronization2 != 0;
        //     NES_GRAPHICS_RETURN_FAIL(*this, hasDynamicRendering && hasSynchronization2, EGraphicsResult::Unsupported, "'DynamicRendering' and 'Synchronization2' are not supported by the device!");
        // }
        //
        // // List of extensions to enable:
        // std::vector<const char*> enabledExtensions;
        // for (const auto& extension : m_desc.m_deviceExtensions)
        // {
        //     enabledExtensions.push_back(extension.m_extensionName);
        // }
        //
        // // Disable Robustness features:
        // // Default
        // robustness2Features.robustBufferAccess2 = 0;
        // robustness2Features.robustImageAccess2 = 0;
        // // [TODO]: Robustness = OFF.
        // features.features.robustBufferAccess = 0;
        // features13.robustImageAccess = 0;

        FillOutDeviceDesc();

        return EGraphicsResult::Success;
    }

    EGraphicsResult RenderDevice::CreateAllocator(const RendererDesc& /*rendererDesc*/)
    {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.instance = *m_vkInstance;
        allocatorInfo.physicalDevice = *m_vkPhysicalDevice;
        allocatorInfo.device = *m_vkDevice;
        allocatorInfo.vulkanApiVersion = m_desc.m_apiVersion;
        
        VkAllocationCallbacks& callbacks = m_vkAllocationCallbacks;
        allocatorInfo.pAllocationCallbacks = &callbacks;

        VmaAllocatorCreateFlags flags = {};
        //if (m_desc.m_features.m_deviceAddress)
            flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        if (m_desc.m_features.m_memoryBudget)
            flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        if (m_desc.m_features.m_memoryPriority)
            flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
        if (m_desc.m_features.m_maintenance4)
            flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT;
        if (m_desc.m_features.m_maintenance5)
            flags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;
        
        // [Consider]:
        //allocatorInfo.pDeviceMemoryCallbacks? Do I need this?
        // -> "Informative callbacks for vkAllocateMemory, vkFreeMemory. Optional." 
        //allocatorInfo.pHeapSizeLimit
        // -> "Either null or a pointer to an array of limits on the maximum number of bytes that can be allocated out of a particular Vulkan memory heap."
        //allocatorInfo.preferredLargeHeapBlockSize?
        // -> "Preferred size of a single VkDeviceMemory block to be allocated from large heaps > 1 GiB. Optional."

        allocatorInfo.flags = flags;
        const VkResult result = vmaCreateAllocator(&allocatorInfo, &m_vmaAllocator);
        if (result != VK_SUCCESS)
        {
            NES_GRAPHICS_ERROR(*this, "Failed to create VMA Allocator!");
            return EGraphicsResult::Failure;
        }

        return EGraphicsResult::Success;
    }

    bool RenderDevice::FilterAvailableExtensions(const std::vector<vk::ExtensionProperties>& supportedExtensions, const std::vector<ExtensionDesc>& desiredExtensions, std::vector<ExtensionDesc>& outFilteredExtensions) const
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
            outDesiredExtensions.emplace_back(vk::KHRSynchronization2ExtensionName);
            outDesiredExtensions.emplace_back(vk::KHRDynamicRenderingExtensionName);
            outDesiredExtensions.emplace_back(vk::KHRCopyCommands2ExtensionName);
            outDesiredExtensions.emplace_back(vk::EXTExtendedDynamicStateExtensionName);

            // Optional for Vulkan < 1.3
            if (availableExtensionsMap.contains(VK_KHR_MAINTENANCE_4_EXTENSION_NAME))
                outDesiredExtensions.emplace_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);

            if (availableExtensionsMap.contains(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME))
                outDesiredExtensions.emplace_back(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME);
        }

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
        const auto props = m_vkPhysicalDevice.getProperties();

        const auto& limits = props.limits;

        // [TODO]: Viewport
        // [TODO]: Precision
        
        // Dimensions:
        m_desc.m_dimensions.m_maxDimensionAttachment = static_cast<uint16>(math::Min(limits.maxFramebufferWidth, limits.maxFramebufferHeight));
        m_desc.m_dimensions.m_maxAttachmentLayerCount = static_cast<uint16>(limits.maxFramebufferLayers);
        m_desc.m_dimensions.m_maxImageDimension1D = static_cast<uint16>(limits.maxImageDimension1D);
        m_desc.m_dimensions.m_maxImageDimension2D = static_cast<uint16>(limits.maxImageDimension2D);
        m_desc.m_dimensions.m_maxImageDimension3D = static_cast<uint16>(limits.maxImageDimension3D);
        m_desc.m_dimensions.m_maxImageLayerCount = static_cast<uint16>(limits.maxImageArrayLayers);
        m_desc.m_dimensions.m_maxTypedBufferDimension = static_cast<uint16>(limits.maxTexelBufferElements);

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
        m_desc.m_memory.m_bufferImageGranularity = static_cast<uint32_t>(limits.bufferImageGranularity);
        //m_desc.m_memory.m_bufferMaxSize = minorVersion >= 3 ? props13.maxBufferSize : maintenance4Props.maxBufferSize;
        
        // Memory Alignment:
        // VUID-VkCopyBufferToImageInfo2-dstImage-07975: If "dstImage" does not have either a depth/stencil format or a multi-planar format,
        //      "bufferOffset" must be a multiple of the texel block size
        // VUID-VkCopyBufferToImageInfo2-dstImage-07978: If "dstImage" has a depth/stencil format,
        //      "bufferOffset" must be a multiple of 4
        // Least Common Multiple stride across all formats: 1, 2, 4, 8, 16 // TODO: rarely used "12" fucks up the beauty of power-of-2 numbers, such formats must be avoided!
        constexpr uint32_t kLeastCommonMultipleStrideAcrossAllFormats = 16;
        m_desc.m_memoryAlignment.m_uploadBufferImageRow = static_cast<uint32_t>(limits.optimalBufferCopyRowPitchAlignment);
        m_desc.m_memoryAlignment.m_uploadBufferImageSlice = std::lcm(static_cast<uint32_t>(limits.optimalBufferCopyOffsetAlignment), kLeastCommonMultipleStrideAcrossAllFormats);
        //m_desc.m_memoryAlignment.m_shaderBindingTable = rayTracingProps.shaderGroupBaseAlignment;
        m_desc.m_memoryAlignment.m_bufferShaderResourceOffset = std::lcm(static_cast<uint32_t>(limits.minTexelBufferOffsetAlignment), static_cast<uint32_t>(limits.minStorageBufferOffsetAlignment));
        m_desc.m_memoryAlignment.m_constantBufferOffset = static_cast<uint32_t>(limits.minUniformBufferOffsetAlignment);
        //m_desc.m_memoryAlignment.m_scratchBufferOffset = accelerationStructureProps.minAccelerationStructureScratchOffsetAlignment;
        m_desc.m_memoryAlignment.m_accelerationStructureOffset = 256; // see the spec
        m_desc.m_memoryAlignment.m_micromapOffset = 256;              // see the spec

        // Other
        m_desc.m_other.m_maxSamplerAnisotropy = limits.maxSamplerAnisotropy;
        m_desc.m_other.m_maxColorSamples = static_cast<uint8>(GetMaxSampleCount(limits.framebufferColorSampleCounts));
        m_desc.m_other.m_maxDepthSamples = static_cast<uint8>(GetMaxSampleCount(limits.framebufferDepthSampleCounts));
        
        // // Device properties
        // VkPhysicalDeviceProperties2 props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        // void** pTail = &props.pNext;
        //
        // VkPhysicalDeviceVulkan11Properties props11 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES};
        // VULKAN_APPEND_EXT_TO_TAIL(props11);
        //
        // VkPhysicalDeviceVulkan12Properties props12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES};
        // VULKAN_APPEND_EXT_TO_TAIL(props12);
        //
        // const uint32 minorVersion = m_desc.m_apiVersion.Minor();
        //
        // VkPhysicalDeviceVulkan13Properties props13 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES};
        // if (minorVersion >= 3)
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(props13);
        // }
        //
        // VkPhysicalDeviceVulkan14Properties props14 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES};
        // if (minorVersion >= 4)
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(props14);
        // }
        //
        // VkPhysicalDeviceMaintenance4PropertiesKHR maintenance4Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES_KHR};
        // if (IsExtensionSupported(VK_KHR_MAINTENANCE_4_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(maintenance4Props);
        // }
        //
        // VkPhysicalDeviceMaintenance5PropertiesKHR maintenance5Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES_KHR};
        // if (IsExtensionSupported(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(maintenance5Props);
        // }
        //
        // VkPhysicalDeviceMaintenance6PropertiesKHR maintenance6Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES_KHR};
        // if (IsExtensionSupported(VK_KHR_MAINTENANCE_6_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(maintenance6Props);
        // }
        //
        // VkPhysicalDeviceMaintenance7PropertiesKHR maintenance7Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_PROPERTIES_KHR};
        // if (IsExtensionSupported(VK_KHR_MAINTENANCE_7_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(maintenance7Props);
        // }
        //
        // VkPhysicalDeviceLineRasterizationPropertiesKHR lineRasterizationProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES_KHR};
        // if (IsExtensionSupported(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(lineRasterizationProps);
        // }
        //
        // VkPhysicalDeviceFragmentShadingRatePropertiesKHR shadingRateProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR};
        // if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(shadingRateProps);
        // }
        //
        // VkPhysicalDevicePushDescriptorPropertiesKHR pushDescriptorProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR};
        // if (IsExtensionSupported(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(pushDescriptorProps);
        // }
        //
        // VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
        // if (IsExtensionSupported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(rayTracingProps);
        // }
        //
        // VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR};
        // if (IsExtensionSupported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(accelerationStructureProps);
        // }
        //
        // VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT};
        // if (IsExtensionSupported(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(conservativeRasterProps);
        //     m_desc.m_tieredFeatures.m_conservativeRaster = 1;
        // }
        //
        // VkPhysicalDeviceSampleLocationsPropertiesEXT sampleLocationsProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT};
        // if (IsExtensionSupported(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(sampleLocationsProps);
        //     m_desc.m_tieredFeatures.m_sampleLocations = 1;
        // }
        //
        // VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT};
        // if (IsExtensionSupported(VK_EXT_MESH_SHADER_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(meshShaderProps);
        // }
        //
        // VkPhysicalDeviceOpacityMicromapPropertiesEXT micromapProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT};
        // if (IsExtensionSupported(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME, m_desc.m_deviceExtensions))
        // {
        //     VULKAN_APPEND_EXT_TO_TAIL(micromapProps);
        // }
        //
        // vkGetPhysicalDeviceProperties2(m_vkPhysicalDevice, &props);
        //
        // // Fill Desc:
        // const VkPhysicalDeviceLimits& limits = props.properties.limits;
        //
    }

    bool RenderDevice::IsHostCoherentMemory(DeviceMemoryTypeIndex memoryTypeIndex) const
    {
        NES_ASSERT(memoryTypeIndex < m_memoryProperties.memoryTypeCount);
        return (m_memoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent) != vk::MemoryPropertyFlags{};
    }

    bool RenderDevice::FindSuitableMemoryType(const EMemoryLocation location, uint32 memoryTypeMask, DeviceMemoryTypeInfo& outInfo) const
    {
        constexpr vk::MemoryPropertyFlags kNoFlags = {}; 
        vk::MemoryPropertyFlags neededFlags = {};       // Must have
        vk::MemoryPropertyFlags undesiredFlags = {};    // Have higher priority than desired.
        vk::MemoryPropertyFlags desiredFlags = {};      // Nice to have.

        if (location == EMemoryLocation::Device)
        {
            neededFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
            undesiredFlags = vk::MemoryPropertyFlagBits::eHostVisible;
        }
        else if (location == EMemoryLocation::DeviceUpload)
        {
            neededFlags = vk::MemoryPropertyFlagBits::eHostVisible;
            undesiredFlags = vk::MemoryPropertyFlagBits::eHostCached;
            desiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostCoherent;
        }
        else
        {
            neededFlags = vk::MemoryPropertyFlagBits::eHostVisible;
            undesiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
            desiredFlags = (location == EMemoryLocation::HostReadback ? vk::MemoryPropertyFlagBits::eHostCached : kNoFlags) | vk::MemoryPropertyFlagBits::eHostCoherent;
        }

        // Phase 1: needed, undesired and desired
        for (uint32 i = 0; i < m_memoryProperties.memoryTypeCount; ++i)
        {
            const bool isSupported = memoryTypeMask & (1 << i);
            const bool hasNeeded = (m_memoryProperties.memoryTypes[i].propertyFlags & neededFlags) == neededFlags;
            const bool hasUndesired = undesiredFlags == kNoFlags ? false : (m_memoryProperties.memoryTypes[i].propertyFlags & undesiredFlags) == undesiredFlags;
            const bool hasDesired = (m_memoryProperties.memoryTypes[i].propertyFlags & desiredFlags) == desiredFlags;

            if (isSupported && hasNeeded && !hasUndesired && hasDesired)
            {
                outInfo.m_index = static_cast<DeviceMemoryTypeIndex>(i);
                outInfo.m_location = location;
                return true;
            }
        }

        // Phase 2: needed and undesired
        for (uint32 i = 0; i < m_memoryProperties.memoryTypeCount; ++i)
        {
            const bool isSupported = memoryTypeMask & (1 << i);
            const bool hasNeeded = (m_memoryProperties.memoryTypes[i].propertyFlags & neededFlags) == neededFlags;
            const bool hasUndesired = undesiredFlags == kNoFlags ? false : (m_memoryProperties.memoryTypes[i].propertyFlags & undesiredFlags) == undesiredFlags;

            if (isSupported && hasNeeded && !hasUndesired)
            {
                outInfo.m_index = static_cast<DeviceMemoryTypeIndex>(i);
                outInfo.m_location = location;
                return true;
            }
        }

        // Phase 3: needed and desired
        for (uint32 i = 0; i < m_memoryProperties.memoryTypeCount; ++i)
        {
            const bool isSupported = memoryTypeMask & (1 << i);
            const bool hasNeeded = (m_memoryProperties.memoryTypes[i].propertyFlags & neededFlags) == neededFlags;
            const bool hasDesired = (m_memoryProperties.memoryTypes[i].propertyFlags & desiredFlags) == desiredFlags;
            
            if (isSupported && hasNeeded && hasDesired)
            {
                outInfo.m_index = static_cast<DeviceMemoryTypeIndex>(i);
                outInfo.m_location = location;
                return true;
            }
        }

        // Phase 4: only needed.
        for (uint32 i = 0; i < m_memoryProperties.memoryTypeCount; ++i)
        {
            const bool isSupported = memoryTypeMask & (1 << i);
            const bool hasNeeded = (m_memoryProperties.memoryTypes[i].propertyFlags & neededFlags) == neededFlags;
            
            if (isSupported && hasNeeded)
            {
                outInfo.m_index = static_cast<DeviceMemoryTypeIndex>(i);
                outInfo.m_location = location;
                return true;
            }
        }

        NES_ASSERT(false, "Failed to find a suitable memory Type!");
        return false;
    }

    EFormatFeatureBits RenderDevice::GetFormatFeatures(const EFormat format) const
    {
        EFormatFeatureBits features = EFormatFeatureBits::Unsupported;
        const vk::Format vkFormat = GetVkFormat(format);

        vk::FormatProperties3 props3{};
        vk::FormatProperties2 props2 = vk::FormatProperties2();
        props2.pNext = &props3;
        (*m_vkPhysicalDevice).getFormatProperties2(vkFormat, &props2);

        #define UPDATE_TEXTURE_FEATURE(vkFlag, formatSupportValue)      \
            if ((props3.optimalTilingFeatures & (vkFlag)) == (vkFlag))  \
                features |= formatSupportValue
        
        #define UPDATE_BUFFER_FEATURE(vkFlag, formatSupportValue)       \
            if ((props3.bufferFeatures & (vkFlag)) == (vkFlag))         \
            features |= formatSupportValue
        
        // Texture
        UPDATE_TEXTURE_FEATURE(vk::FormatFeatureFlagBits2::eSampledImage, EFormatFeatureBits::Image);
        UPDATE_TEXTURE_FEATURE(vk::FormatFeatureFlagBits2::eStorageImage, EFormatFeatureBits::StorageImage);
        UPDATE_TEXTURE_FEATURE(vk::FormatFeatureFlagBits2::eStorageImageAtomic, EFormatFeatureBits::StorageBufferAtomics);
        UPDATE_TEXTURE_FEATURE(vk::FormatFeatureFlagBits2::eColorAttachment, EFormatFeatureBits::ColorAttachment);
        UPDATE_TEXTURE_FEATURE(vk::FormatFeatureFlagBits2::eDepthStencilAttachment, EFormatFeatureBits::DepthStencilAttachment);
        UPDATE_TEXTURE_FEATURE(vk::FormatFeatureFlagBits2::eColorAttachmentBlend, EFormatFeatureBits::Blend);
        
        // Buffer
        UPDATE_BUFFER_FEATURE(vk::FormatFeatureFlagBits2::eUniformTexelBuffer, EFormatFeatureBits::Buffer);
        UPDATE_BUFFER_FEATURE(vk::FormatFeatureFlagBits2::eStorageTexelBuffer, EFormatFeatureBits::StorageBuffer);
        UPDATE_BUFFER_FEATURE(vk::FormatFeatureFlagBits2::eStorageTexelBufferAtomic, EFormatFeatureBits::StorageBufferAtomics);
        UPDATE_BUFFER_FEATURE(vk::FormatFeatureFlagBits2::eVertexBuffer, EFormatFeatureBits::VertexBuffer);
        
        // Combined:
        if ((props3.optimalTilingFeatures | props3.bufferFeatures) & vk::FormatFeatureFlagBits2::eStorageReadWithoutFormat)
            features |= EFormatFeatureBits::StorageLoadWithoutFormat;
        
        #undef UPDATE_TEXTURE_FEATURE
        #undef UPDATE_BUFFER_FEATURE
        
        // Multisample Features:
        vk::PhysicalDeviceImageFormatInfo2 imageInfo = vk::PhysicalDeviceImageFormatInfo2()
            .setFormat(vkFormat)
            .setType(vk::ImageType::e2D)
            .setTiling(vk::ImageTiling::eOptimal);
        
        imageInfo.usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
        if (features & EFormatFeatureBits::Image)
            imageInfo.usage |= vk::ImageUsageFlagBits::eSampled;
        
        if (features & EFormatFeatureBits::DepthStencilAttachment)
            imageInfo.usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
        
        if (features & EFormatFeatureBits::ColorAttachment)
            imageInfo.usage |= vk::ImageUsageFlagBits::eColorAttachment;
        
        vk::ImageFormatProperties2 imageProps = m_vkPhysicalDevice.getImageFormatProperties2(imageInfo);
        if (imageProps.imageFormatProperties.sampleCounts & vk::SampleCountFlagBits::e2)
            features |= EFormatFeatureBits::Multisample2x;
        if (imageProps.imageFormatProperties.sampleCounts & vk::SampleCountFlagBits::e4)
            features |= EFormatFeatureBits::Multisample4x;
        if (imageProps.imageFormatProperties.sampleCounts & vk::SampleCountFlagBits::e8)
            features |= EFormatFeatureBits::Multisample8x;
        if (imageProps.imageFormatProperties.sampleCounts & vk::SampleCountFlagBits::e16)
            features |= EFormatFeatureBits::Multisample16x;

        return features;
    }
}
