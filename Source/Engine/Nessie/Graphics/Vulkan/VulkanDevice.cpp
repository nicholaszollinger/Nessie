//// VulkanDevice.cpp
//#include "VulkanDevice.h"
//
//#include "VulkanGLFW.h"
//#include "Nessie/Application/ApplicationDesc.h"
//#include "Nessie/Graphics/Shared/SharedExternal.h"
//
//static void* VKAPI_PTR vkAllocateHostMemory(void* pUserData, const size_t size, const size_t alignment, vk::SystemAllocationScope)
//{
//    const auto& allocationCallbacks = *static_cast<nes::AllocationCallbacks*>(pUserData);
//    return allocationCallbacks.Allocate(size, alignment);
//}
//
//static void* VKAPI_PTR vkReallocateHostMemory(void* pUserData, void* pOriginal, const size_t size, const size_t alignment, vk::SystemAllocationScope)
//{
//    const auto& allocationCallbacks = *static_cast<nes::AllocationCallbacks*>(pUserData);
//    return allocationCallbacks.Reallocate(pOriginal, size, alignment);
//}
//
//static void VKAPI_PTR vkFreeHostMemory(void* pUserData, void* pMemory)
//{
//    const auto& allocationCallbacks = *static_cast<nes::AllocationCallbacks*>(pUserData);
//    return allocationCallbacks.Free(pMemory);
//}
//
//[[maybe_unused]]
//static VkBool32 VKAPI_PTR MessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT, [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void*)
//{
//    // [TODO]: Use the DebugMessenger for the call.
//    // If the message is important enough to show.
//    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) 
//    {
//        NES_VULKAN_ERROR("[{}]: {}", callbackData->messageIdNumber, callbackData->pMessage);
//    }
//    
//    return VK_FALSE;
//}
//
//namespace nes
//{
//    bool VulkanDevice::Init(const ApplicationDesc& appDesc, ApplicationWindow* pWindow, const RendererDesc& rendererDesc)
//    {
//        if (!RenderDevice::Init(appDesc, pWindow, rendererDesc))
//            return false;
//    
//        auto extensions = nes::vulkan::glfw::GetRequiredExtensions();
//        // Include the debug extensions.
//        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
//
//        // Create the Vulkan Allocation callbacks by wrapping the passed in value. 
//        m_vkAllocationCallbacks
//            .setPUserData(const_cast<AllocationCallbacks*>(&GetAllocationCallbacks()))
//            .setPfnAllocation(vkAllocateHostMemory)
//            .setPfnReallocation(vkReallocateHostMemory)
//            .setPfnFree(vkFreeHostMemory);
//
//        VkAllocationCallbacks& allocCallbacks = m_vkAllocationCallbacks;
//    
//        auto instBuilder = vkb::InstanceBuilder()
//            .set_engine_name("Nessie")
//            .set_app_name(appDesc.m_appName.c_str())
//            .set_app_version(appDesc.m_appVersion)
//            .set_allocation_callbacks(&allocCallbacks)
//            .enable_extensions(extensions)
//            .require_api_version(rendererDesc.m_apiVersion);
//
//        // Set debug messenger and validation layers if not in release.
//#ifndef NES_RELEASE
//        instBuilder.request_validation_layers(rendererDesc.m_enableValidationLayer)
//            .set_debug_callback(MessageCallback)
//            .set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
//                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
//                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
//                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
//            .set_debug_messenger_type(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
//                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
//                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
//            .set_debug_callback_user_data_pointer(this);
//#endif
//
//
//        // Build the instance:
//        auto instResult = instBuilder.build();
//        if (!instResult)
//        {
//            NES_VULKAN_ERROR("Failed to initialize Vulkan! Failed to build vkb::Instance! {}", instResult.error().message());
//            return false;
//        }
//        m_vkInstance = instResult.value();
//
//        /// Select physical Device:
//        auto selector = vkb::PhysicalDeviceSelector(m_vkInstance);
//        if (!appDesc.m_isHeadless)
//        {
//            // If not headless, create the surface.
//            m_vkSurface = nes::vulkan::glfw::CreateSurface(m_vkInstance.instance, checked_cast<GLFWwindow*>(pWindow->GetNativeWindowHandle()));
//            if (!m_vkSurface)
//            {
//                NES_VULKAN_ERROR("Failed to create Vulkan surface!");
//                return false;
//            }
//        
//            selector.set_surface(m_vkSurface)
//                .require_present(true);   
//        }
//        else
//        {
//            // Otherwise, present is not needed.
//            selector.require_present(false);
//        }
//    
//        selector.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete);
//        // [TODO]: require_compute_queue
//        // [TODO]: require_transfer_queue
//
//        // Select a physical device:
//        auto selectResult = selector.select(vkb::DeviceSelectionMode::only_fully_suitable);
//        if (!selectResult)
//        {
//            NES_VULKAN_ERROR("Failed to select physical device with Renderer requirements!");
//        
//            auto enumerator = vkb::PhysicalDeviceSelector(m_vkInstance).defer_surface_initialization();
//            auto devices = enumerator.select_devices();
//            if (!devices)
//            {
//                NES_VULKAN_ERROR("No physical devices available!");
//                return false;
//            }
//        
//            NES_VULKAN_INFO("Possible Devices: ");
//            for ([[maybe_unused]] const auto& device : devices.value())
//            {
//                NES_VULKAN_INFO("- {}", device.name);
//            }
//        }
//        m_vkPhysicalDevice = selectResult.value();
//
//        // Create the logical device.
//        // [TODO]: .custom_queue_setup().
//        auto deviceResult = vkb::DeviceBuilder(m_vkPhysicalDevice)
//            .set_allocation_callbacks(&allocCallbacks)
//            .build();
//
//        if (!deviceResult)
//        {
//            NES_VULKAN_ERROR("Failed to initialize Vulkan!");
//            return false;
//        }
//        m_vkDevice = deviceResult.value();
//        m_vk = m_vkDevice.make_table();
//
//    
//        FillDeviceDesc();
//    
//        return true;
//    }
//
//    void VulkanDevice::Destroy()
//    {
//        if (const vk::Device device = m_vkDevice.device)
//        {
//            device.waitIdle();
//
//
//            // [TODO]: 
//        }
//        vkb::destroy_device(m_vkDevice);
//        m_vkPhysicalDevice = vkb::PhysicalDevice();
//
//        if (const vk::Instance instance = m_vkInstance.instance)
//        {
//            if (m_vkSurface)
//            {
//                instance.destroySurfaceKHR(m_vkSurface);
//                m_vkSurface = vk::SurfaceKHR();
//            }
//        }
//        vkb::destroy_instance(m_vkInstance);
//    }
//
//    void VulkanDevice::SetDebugNameToTrivialObject(const vk::ObjectType type, const uint64 handle, const char* name) const
//    {
//        auto nameInfo = vk::DebugUtilsObjectNameInfoEXT()
//            .setObjectType(type)
//            .setObjectHandle(handle)
//            .setPObjectName(name);
//
//        VkDebugUtilsObjectNameInfoEXT& vkNameInfo = nameInfo;
//        NES_VULKAN_C_MUST_PASS(m_vk.setDebugUtilsObjectNameEXT(&vkNameInfo));
//    }
//
//    void VulkanDevice::FillDeviceDesc()
//    {
//        m_deviceDesc.m_graphicsAPI = EGraphicsAPI::Vulkan;
//
//        auto& adapterDesc = m_deviceDesc.m_adapterDesc;
//        auto& deviceProps = m_vkPhysicalDevice.properties;
//    
//        // Name
//        memcpy(adapterDesc.m_name, m_vkPhysicalDevice.properties.deviceName, 256);
//
//        // Vendor
//        adapterDesc.m_vendor = graphics::GetVendorFromID(deviceProps.vendorID);
//
//        // Architecture Type:
//        if (deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
//            adapterDesc.m_architecture = EArchitecture::Discrete;
//        else if (deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
//            adapterDesc.m_architecture = EArchitecture::Integrated;
//        else
//            adapterDesc.m_architecture = EArchitecture::Unknown;
//
//        // [TODO]: More...
//    }
//}
