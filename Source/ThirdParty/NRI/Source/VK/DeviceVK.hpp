// © 2021 NVIDIA Corporation

static constexpr VkBufferUsageFlags GetBufferUsageFlags(BufferUsageBits bufferUsageBits, uint32_t structureStride, bool isDeviceAddressSupported) {
    VkBufferUsageFlags flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; // TODO: ban "the opposite" for UPLOAD/READBACK?

    if (isDeviceAddressSupported)
        flags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    if (bufferUsageBits & BufferUsageBits::VERTEX_BUFFER)
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    if (bufferUsageBits & BufferUsageBits::INDEX_BUFFER)
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    if (bufferUsageBits & BufferUsageBits::CONSTANT_BUFFER)
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    if (bufferUsageBits & BufferUsageBits::ARGUMENT_BUFFER)
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

    if (bufferUsageBits & BufferUsageBits::SCRATCH_BUFFER)
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    if (bufferUsageBits & BufferUsageBits::SHADER_BINDING_TABLE)
        flags |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;

    if (bufferUsageBits & BufferUsageBits::ACCELERATION_STRUCTURE_STORAGE)
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;

    if (bufferUsageBits & BufferUsageBits::ACCELERATION_STRUCTURE_BUILD_INPUT)
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

    if (bufferUsageBits & BufferUsageBits::MICROMAP_STORAGE)
        flags |= VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT;

    if (bufferUsageBits & BufferUsageBits::MICROMAP_BUILD_INPUT)
        flags |= VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT;

    if (bufferUsageBits & BufferUsageBits::SHADER_RESOURCE)
        flags |= structureStride ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

    if (bufferUsageBits & BufferUsageBits::SHADER_RESOURCE_STORAGE)
        flags |= structureStride ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

    return flags;
}

static constexpr VkImageUsageFlags GetImageUsageFlags(TextureUsageBits textureUsageBits) {
    VkImageUsageFlags flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if (textureUsageBits & TextureUsageBits::SHADER_RESOURCE)
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;

    if (textureUsageBits & TextureUsageBits::SHADER_RESOURCE_STORAGE)
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;

    if (textureUsageBits & TextureUsageBits::COLOR_ATTACHMENT)
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (textureUsageBits & TextureUsageBits::DEPTH_STENCIL_ATTACHMENT)
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    if (textureUsageBits & TextureUsageBits::SHADING_RATE_ATTACHMENT)
        flags |= VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;

    return flags;
}

static inline bool IsExtensionSupported(const char* ext, const Vector<VkExtensionProperties>& list) {
    for (auto& e : list) {
        if (!strcmp(ext, e.extensionName))
            return true;
    }

    return false;
}

static inline bool IsExtensionSupported(const char* ext, const Vector<const char*>& list) {
    for (auto& e : list) {
        if (!strcmp(ext, e))
            return true;
    }

    return false;
}

static void* VKAPI_PTR vkAllocateHostMemory(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope) {
    const auto& allocationCallbacks = *(AllocationCallbacks*)pUserData;

    return allocationCallbacks.Allocate(allocationCallbacks.userArg, size, alignment);
}

static void* VKAPI_PTR vkReallocateHostMemory(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope) {
    const auto& allocationCallbacks = *(AllocationCallbacks*)pUserData;

    return allocationCallbacks.Reallocate(allocationCallbacks.userArg, pOriginal, size, alignment);
}

static void VKAPI_PTR vkFreeHostMemory(void* pUserData, void* pMemory) {
    const auto& allocationCallbacks = *(AllocationCallbacks*)pUserData;

    return allocationCallbacks.Free(allocationCallbacks.userArg, pMemory);
}

static VkBool32 VKAPI_PTR MessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData) {
    DeviceVK& device = *(DeviceVK*)userData;

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

    Message severity = Message::INFO;
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        severity = Message::ERROR;
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        severity = Message::WARNING;

    device.ReportMessage(severity, __FILE__, __LINE__, "[%u] %s", callbackData->messageIdNumber, callbackData->pMessage);

    return VK_FALSE;
}

void DeviceVK::FilterInstanceLayers(Vector<const char*>& layers) {
    uint32_t layerNum = 0;
    m_VK.EnumerateInstanceLayerProperties(&layerNum, nullptr);

    Vector<VkLayerProperties> supportedLayers(layerNum, GetStdAllocator());
    m_VK.EnumerateInstanceLayerProperties(&layerNum, supportedLayers.data());

    for (size_t i = 0; i < layers.size(); i++) {
        bool found = false;
        for (uint32_t j = 0; j < layerNum && !found; j++) {
            if (strcmp(supportedLayers[j].layerName, layers[i]) == 0)
                found = true;
        }

        if (!found)
            layers.erase(layers.begin() + i--);
    }
}

void DeviceVK::ProcessInstanceExtensions(Vector<const char*>& desiredInstanceExts) {
    // Query extensions
    uint32_t extensionNum = 0;
    m_VK.EnumerateInstanceExtensionProperties(nullptr, &extensionNum, nullptr);

    Vector<VkExtensionProperties> supportedExts(extensionNum, GetStdAllocator());
    m_VK.EnumerateInstanceExtensionProperties(nullptr, &extensionNum, supportedExts.data());

    REPORT_INFO(this, "Supported instance extensions:");
    for (const VkExtensionProperties& props : supportedExts)
        REPORT_INFO(this, "    %s (v%u)", props.extensionName, props.specVersion);

#ifdef __APPLE__
    // Mandatory
    desiredInstanceExts.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    desiredInstanceExts.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    // Optional
    if (IsExtensionSupported(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, supportedExts))
        desiredInstanceExts.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME, supportedExts)) {
        desiredInstanceExts.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

        if (IsExtensionSupported(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME, supportedExts))
            desiredInstanceExts.push_back(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);

#ifdef VK_USE_PLATFORM_WIN32_KHR
        desiredInstanceExts.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
        desiredInstanceExts.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
        desiredInstanceExts.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
        desiredInstanceExts.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#endif
    }

    if (IsExtensionSupported(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME, supportedExts))
        desiredInstanceExts.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);

    if (IsExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, supportedExts))
        desiredInstanceExts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
}

void DeviceVK::ProcessDeviceExtensions(Vector<const char*>& desiredDeviceExts, bool disableRayTracing) {
    // Query extensions
    uint32_t extensionNum = 0;
    m_VK.EnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionNum, nullptr);

    Vector<VkExtensionProperties> supportedExts(extensionNum, GetStdAllocator());
    m_VK.EnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionNum, supportedExts.data());

    REPORT_INFO(this, "Supported device extensions:");
    for (const VkExtensionProperties& props : supportedExts)
        REPORT_INFO(this, "    %s (v%u)", props.extensionName, props.specVersion);

    // Mandatory
    if (m_MinorVersion < 3) {
        desiredDeviceExts.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
        desiredDeviceExts.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        desiredDeviceExts.push_back(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
        desiredDeviceExts.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
    }

#ifdef __APPLE__
    if (IsExtensionSupported(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

    // Optional for Vulkan < 1.3
    if (m_MinorVersion < 3 && IsExtensionSupported(VK_KHR_MAINTENANCE_4_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);

    if (m_MinorVersion < 3 && IsExtensionSupported(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME);

    // Optional (KHR)
    if (IsExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_PRESENT_ID_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_PRESENT_ID_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_PRESENT_WAIT_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_PRESENT_WAIT_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_MAINTENANCE_6_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_MAINTENANCE_7_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_MAINTENANCE_7_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_MAINTENANCE_8_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_MAINTENANCE_8_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_MAINTENANCE_9_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_MAINTENANCE_9_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, supportedExts) && !disableRayTracing)
        desiredDeviceExts.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, supportedExts) && !disableRayTracing)
        desiredDeviceExts.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_RAY_QUERY_EXTENSION_NAME, supportedExts) && !disableRayTracing)
        desiredDeviceExts.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME, supportedExts) && !disableRayTracing)
        desiredDeviceExts.push_back(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME, supportedExts) && !disableRayTracing)
        desiredDeviceExts.push_back(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME);

    if (IsExtensionSupported(VK_KHR_SHADER_CLOCK_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_SHADER_CLOCK_EXTENSION_NAME);

    // Optional (EXT)
    if (IsExtensionSupported(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME);

    if (IsExtensionSupported(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME);

    if (IsExtensionSupported(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME, supportedExts) && !disableRayTracing)
        desiredDeviceExts.push_back(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME);

    if (IsExtensionSupported(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME);

    if (IsExtensionSupported(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);

    if (IsExtensionSupported(VK_EXT_MESH_SHADER_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);

    if (IsExtensionSupported(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);

    if (IsExtensionSupported(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME);

    if (IsExtensionSupported(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

    if (IsExtensionSupported(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);

    if (IsExtensionSupported(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME);

    if (IsExtensionSupported(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME);

    if (IsExtensionSupported(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);

    if (IsExtensionSupported(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME);

    if (IsExtensionSupported(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME);

    // Optional
    if (IsExtensionSupported(VK_NV_LOW_LATENCY_2_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_NV_LOW_LATENCY_2_EXTENSION_NAME);

    if (IsExtensionSupported(VK_NVX_BINARY_IMPORT_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_NVX_BINARY_IMPORT_EXTENSION_NAME);

    if (IsExtensionSupported(VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_NVX_IMAGE_VIEW_HANDLE_EXTENSION_NAME);

    // Dependencies
    if (IsExtensionSupported(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, supportedExts))
        desiredDeviceExts.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
}

DeviceVK::DeviceVK(const CallbackInterface& callbacks, const AllocationCallbacks& allocationCallbacks)
    : DeviceBase(callbacks, allocationCallbacks)
    , m_QueueFamilies{
          Vector<QueueVK*>(GetStdAllocator()),
          Vector<QueueVK*>(GetStdAllocator()),
          Vector<QueueVK*>(GetStdAllocator()),
      } {
    m_AllocationCallbacks.pUserData = (void*)&GetAllocationCallbacks();
    m_AllocationCallbacks.pfnAllocation = vkAllocateHostMemory;
    m_AllocationCallbacks.pfnReallocation = vkReallocateHostMemory;
    m_AllocationCallbacks.pfnFree = vkFreeHostMemory;

    m_Desc.graphicsAPI = GraphicsAPI::VK;
    m_Desc.nriVersion = NRI_VERSION;
}

DeviceVK::~DeviceVK() {
    DestroyVma();

    for (auto& queueFamily : m_QueueFamilies) {
        for (uint32_t i = 0; i < queueFamily.size(); i++)
            Destroy<QueueVK>(queueFamily[i]);
    }

    if (m_Messenger) {
        typedef PFN_vkDestroyDebugUtilsMessengerEXT Func;
        Func destroyCallback = (Func)m_VK.GetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
        destroyCallback(m_Instance, m_Messenger, m_AllocationCallbackPtr);
    }

    if (m_OwnsNativeObjects) {
        if (m_Device)
            m_VK.DestroyDevice(m_Device, m_AllocationCallbackPtr);

        if (m_Instance)
            m_VK.DestroyInstance(m_Instance, m_AllocationCallbackPtr);
    }

    if (m_Loader)
        UnloadSharedLibrary(*m_Loader);
}

Result DeviceVK::Create(const DeviceCreationDesc& desc, const DeviceCreationVKDesc& descVK) {
    bool isWrapper = descVK.vkDevice != nullptr;
    m_OwnsNativeObjects = !isWrapper;
    m_BindingOffsets = desc.vkBindingOffsets;

    if (!isWrapper && !GetAllocationCallbacks().disable3rdPartyAllocationCallbacks)
        m_AllocationCallbackPtr = &m_AllocationCallbacks;

    // Get adapter description as early as possible for meaningful error reporting
    m_Desc.adapterDesc = *desc.adapterDesc;

    { // Loader
        const char* loaderPath = descVK.libraryPath ? descVK.libraryPath : VULKAN_LOADER_NAME;
        m_Loader = LoadSharedLibrary(loaderPath);
        if (!m_Loader) {
            REPORT_ERROR(this, "Failed to load Vulkan loader: '%s'", loaderPath);
            return Result::UNSUPPORTED;
        }
    }

    // Create instance
    Vector<const char*> desiredInstanceExts(GetStdAllocator());
    {
        Result res = ResolvePreInstanceDispatchTable();
        if (res != Result::SUCCESS)
            return res;

        for (uint32_t i = 0; i < desc.vkExtensions.instanceExtensionNum; i++)
            desiredInstanceExts.push_back(desc.vkExtensions.instanceExtensions[i]);

        m_Instance = (VkInstance)descVK.vkInstance;

        if (!isWrapper) {
            ProcessInstanceExtensions(desiredInstanceExts);

            res = CreateInstance(desc.enableGraphicsAPIValidation, desiredInstanceExts);
            if (res != Result::SUCCESS)
                return res;
        }

        res = ResolveInstanceDispatchTable(desiredInstanceExts);
        if (res != Result::SUCCESS)
            return res;
    }

    { // Physical device
        m_MinorVersion = descVK.minorVersion;
        m_PhysicalDevice = (VkPhysicalDevice)descVK.vkPhysicalDevice;

        if (!isWrapper) {
            uint32_t deviceGroupNum = 0;
            VkResult vkResult = m_VK.EnumeratePhysicalDeviceGroups(m_Instance, &deviceGroupNum, nullptr);
            RETURN_ON_BAD_VKRESULT(this, vkResult, "vkEnumeratePhysicalDeviceGroups");

            Scratch<VkPhysicalDeviceGroupProperties> deviceGroups = AllocateScratch(*this, VkPhysicalDeviceGroupProperties, deviceGroupNum);
            for (uint32_t j = 0; j < deviceGroupNum; j++) {
                deviceGroups[j].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
                deviceGroups[j].pNext = nullptr;
            }

            vkResult = m_VK.EnumeratePhysicalDeviceGroups(m_Instance, &deviceGroupNum, deviceGroups);
            RETURN_ON_BAD_VKRESULT(this, vkResult, "vkEnumeratePhysicalDeviceGroups");

            uint32_t i = 0;
            for (i = 0; i < deviceGroupNum; i++) {
                VkPhysicalDeviceProperties2 props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};

                VkPhysicalDeviceIDProperties idProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES};
                props.pNext = &idProps;

                m_VK.GetPhysicalDeviceProperties2(deviceGroups[i].physicalDevices[0], &props);

                uint32_t majorVersion = VK_API_VERSION_MAJOR(props.properties.apiVersion);
                m_MinorVersion = VK_API_VERSION_MINOR(props.properties.apiVersion);

                bool isSupported = (majorVersion * 10 + m_MinorVersion) >= 12;
                if (desc.adapterDesc) {
                    const uint64_t luid = *(uint64_t*)idProps.deviceLUID;
                    if (luid == desc.adapterDesc->luid) {
                        RETURN_ON_FAILURE(this, isSupported, Result::UNSUPPORTED, "Can't create a device: the specified physical device does not support Vulkan 1.2+!");
                        break;
                    }
                } else if (isSupported)
                    break;
            }

            RETURN_ON_FAILURE(this, i != deviceGroupNum, Result::INVALID_ARGUMENT, "Can't create a device: physical device not found");

            if (deviceGroups[i].physicalDeviceCount > 1 && deviceGroups[i].subsetAllocation == VK_FALSE)
                REPORT_WARNING(this, "The device group does not support memory allocation on a subset of the physical devices");

            m_PhysicalDevice = deviceGroups[i].physicalDevices[0];
        }
    }

    // Queue family indices
    std::array<uint32_t, (size_t)QueueType::MAX_NUM> queueFamilyIndices = {};
    queueFamilyIndices.fill(INVALID_FAMILY_INDEX);
    if (isWrapper) {
        for (uint32_t i = 0; i < descVK.queueFamilyNum; i++) {
            const QueueFamilyVKDesc& queueFamily = descVK.queueFamilies[i];
            queueFamilyIndices[(size_t)queueFamily.queueType] = queueFamily.familyIndex;
        }
    } else {
        uint32_t familyNum = 0;
        m_VK.GetPhysicalDeviceQueueFamilyProperties2(m_PhysicalDevice, &familyNum, nullptr);

        Scratch<VkQueueFamilyProperties2> familyProps2 = AllocateScratch(*this, VkQueueFamilyProperties2, familyNum);
        for (uint32_t i = 0; i < familyNum; i++)
            familyProps2[i] = {VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2};

        m_VK.GetPhysicalDeviceQueueFamilyProperties2(m_PhysicalDevice, &familyNum, familyProps2);

        std::array<uint32_t, (size_t)QueueType::MAX_NUM> scores = {};
        for (uint32_t i = 0; i < familyNum; i++) { // TODO: same code is used in "Creation.cpp"
            const VkQueueFamilyProperties& familyProps = familyProps2[i].queueFamilyProperties;

            bool graphics = familyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT;
            bool compute = familyProps.queueFlags & VK_QUEUE_COMPUTE_BIT;
            bool copy = familyProps.queueFlags & VK_QUEUE_TRANSFER_BIT;
            bool sparse = familyProps.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
            bool videoDecode = familyProps.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR;
            bool videoEncode = familyProps.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR;
            bool protect = familyProps.queueFlags & VK_QUEUE_PROTECTED_BIT;
            bool opticalFlow = familyProps.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV;
            bool taken = false;

            { // Prefer as much features as possible
                size_t index = (size_t)QueueType::GRAPHICS;
                uint32_t score = GRAPHICS_QUEUE_SCORE;

                if (!taken && graphics && score > scores[index]) {
                    queueFamilyIndices[index] = i;
                    scores[index] = score;
                    taken = true;
                }
            }

            { // Prefer compute-only
                size_t index = (size_t)QueueType::COMPUTE;
                uint32_t score = COMPUTE_QUEUE_SCORE;

                if (!taken && compute && score > scores[index]) {
                    queueFamilyIndices[index] = i;
                    scores[index] = score;
                    taken = true;
                }
            }

            { // Prefer copy-only
                size_t index = (size_t)QueueType::COPY;
                uint32_t score = COPY_QUEUE_SCORE;

                if (!taken && copy && score > scores[index]) {
                    queueFamilyIndices[index] = i;
                    scores[index] = score;
                    taken = true;
                }
            }
        }
    }

    { // Memory props
        VkPhysicalDeviceMemoryProperties2 memoryProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
        m_VK.GetPhysicalDeviceMemoryProperties2(m_PhysicalDevice, &memoryProps);

        m_MemoryProps = memoryProps.memoryProperties;
    }

    // Device extensions
    Vector<const char*> desiredDeviceExts(GetStdAllocator());
    if (!isWrapper)
        ProcessDeviceExtensions(desiredDeviceExts, desc.disableVKRayTracing);

    for (uint32_t i = 0; i < desc.vkExtensions.deviceExtensionNum; i++)
        desiredDeviceExts.push_back(desc.vkExtensions.deviceExtensions[i]);

    // Device features
    VkPhysicalDeviceFeatures2 features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    void** tail = &features.pNext;

    VkPhysicalDeviceVulkan11Features features11 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
    APPEND_EXT(features11);

    VkPhysicalDeviceVulkan12Features features12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    APPEND_EXT(features12);

    VkPhysicalDeviceVulkan13Features features13 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    if (m_MinorVersion >= 3) {
        APPEND_EXT(features13);
    }

#ifdef __APPLE__
    VkPhysicalDevicePortabilitySubsetFeaturesKHR portabilitySubsetFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(portabilitySubsetFeatures);
    }
#endif

    // Mandatory
    VkPhysicalDeviceSynchronization2Features synchronization2features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES};
    if (IsExtensionSupported(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(synchronization2features);
    }

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES};
    if (IsExtensionSupported(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(dynamicRenderingFeatures);
    }

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT};
    if (IsExtensionSupported(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(extendedDynamicStateFeatures);
    }

    // Optional (for Vulkan < 1.2)
    VkPhysicalDeviceMaintenance4Features maintenance4Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES};
    if (IsExtensionSupported(VK_KHR_MAINTENANCE_4_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(maintenance4Features);
    }

    VkPhysicalDeviceImageRobustnessFeatures imageRobustnessFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES};
    if (IsExtensionSupported(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(imageRobustnessFeatures);
    }

    // Optional (KHR)
    VkPhysicalDevicePresentIdFeaturesKHR presentIdFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_PRESENT_ID_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(presentIdFeatures);
    }

    VkPhysicalDevicePresentWaitFeaturesKHR presentWaitFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_PRESENT_WAIT_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(presentWaitFeatures);
    }

    VkPhysicalDeviceMaintenance5FeaturesKHR maintenance5Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(maintenance5Features);
    }

    VkPhysicalDeviceMaintenance6FeaturesKHR maintenance6Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_MAINTENANCE_6_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(maintenance6Features);
    }

    VkPhysicalDeviceMaintenance7FeaturesKHR maintenance7Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_MAINTENANCE_7_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(maintenance7Features);
    }

    VkPhysicalDeviceMaintenance8FeaturesKHR maintenance8Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_8_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_MAINTENANCE_8_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(maintenance8Features);
    }

    VkPhysicalDeviceMaintenance9FeaturesKHR maintenance9Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_9_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_MAINTENANCE_9_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(maintenance9Features);
    }

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR shadingRateFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(shadingRateFeatures);
    }

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(rayTracingPipelineFeatures);
    }

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(accelerationStructureFeatures);
    }

    VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_RAY_QUERY_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(rayQueryFeatures);
    }

    VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR rayTracingPositionFetchFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_POSITION_FETCH_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_RAY_TRACING_POSITION_FETCH_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(rayTracingPositionFetchFeatures);
    }

    VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR rayTracingMaintenanceFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MAINTENANCE_1_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_RAY_TRACING_MAINTENANCE_1_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(rayTracingMaintenanceFeatures);
    }

    VkPhysicalDeviceLineRasterizationFeaturesKHR lineRasterizationFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(lineRasterizationFeatures);
    }

    VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR fragmentShaderBarycentricFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_BARYCENTRIC_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(fragmentShaderBarycentricFeatures);
    }

    VkPhysicalDeviceShaderClockFeaturesKHR shaderClockFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR};
    if (IsExtensionSupported(VK_KHR_SHADER_CLOCK_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(shaderClockFeatures);
    }

    // Optional (EXT)
    VkPhysicalDeviceOpacityMicromapFeaturesEXT micromapFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_FEATURES_EXT};
    if (IsExtensionSupported(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(micromapFeatures);
    }

    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT};
    if (IsExtensionSupported(VK_EXT_MESH_SHADER_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(meshShaderFeatures);
    }

    VkPhysicalDeviceShaderAtomicFloatFeaturesEXT shaderAtomicFloatFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT};
    if (IsExtensionSupported(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(shaderAtomicFloatFeatures);
    }

    VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT shaderAtomicFloat2Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_2_FEATURES_EXT};
    if (IsExtensionSupported(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(shaderAtomicFloat2Features);
    }

    VkPhysicalDeviceMemoryPriorityFeaturesEXT memoryPriorityFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT};
    if (IsExtensionSupported(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(memoryPriorityFeatures);
    }

    VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT slicedViewFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_SLICED_VIEW_OF_3D_FEATURES_EXT};
    if (IsExtensionSupported(VK_EXT_IMAGE_SLICED_VIEW_OF_3D_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(slicedViewFeatures);
    }

    VkPhysicalDeviceCustomBorderColorFeaturesEXT borderColorFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUSTOM_BORDER_COLOR_FEATURES_EXT};
    if (IsExtensionSupported(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(borderColorFeatures);
    }

    VkPhysicalDeviceRobustness2FeaturesEXT robustness2Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT};
    if (IsExtensionSupported(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(robustness2Features);
    }

    VkPhysicalDevicePipelineRobustnessFeaturesEXT pipelineRobustnessFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES_EXT};
    if (IsExtensionSupported(VK_EXT_PIPELINE_ROBUSTNESS_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(pipelineRobustnessFeatures);
    }

    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT fragmentShaderInterlockFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADER_INTERLOCK_FEATURES_EXT};
    if (IsExtensionSupported(VK_EXT_FRAGMENT_SHADER_INTERLOCK_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(fragmentShaderInterlockFeatures);
    }

    VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT swapchainMaintenance1Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT};
    if (IsExtensionSupported(VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(swapchainMaintenance1Features);
    }

    VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT presentModeFifoLatestReadyFeaturesEXT = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_MODE_FIFO_LATEST_READY_FEATURES_EXT};
    if (IsExtensionSupported(VK_EXT_PRESENT_MODE_FIFO_LATEST_READY_EXTENSION_NAME, desiredDeviceExts)) {
        APPEND_EXT(presentModeFifoLatestReadyFeaturesEXT);
    }

    if (IsExtensionSupported(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME, desiredDeviceExts))
        m_IsSupported.memoryBudget = true;

    m_VK.GetPhysicalDeviceFeatures2(m_PhysicalDevice, &features);

    m_IsSupported.descriptorIndexing = features12.descriptorIndexing;
    m_IsSupported.deviceAddress = features12.bufferDeviceAddress;
    m_IsSupported.swapChainMutableFormat = IsExtensionSupported(VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME, desiredDeviceExts);
    m_IsSupported.presentId = presentIdFeatures.presentId;
    m_IsSupported.memoryPriority = memoryPriorityFeatures.memoryPriority;
    m_IsSupported.maintenance4 = features13.maintenance4 != 0 || maintenance4Features.maintenance4 != 0;
    m_IsSupported.maintenance5 = maintenance5Features.maintenance5;
    m_IsSupported.maintenance6 = maintenance6Features.maintenance6;
    m_IsSupported.imageSlicedView = slicedViewFeatures.imageSlicedViewOf3D != 0;
    m_IsSupported.customBorderColor = borderColorFeatures.customBorderColors != 0 && borderColorFeatures.customBorderColorWithoutFormat != 0;
    m_IsSupported.robustness = features.features.robustBufferAccess != 0 && (imageRobustnessFeatures.robustImageAccess != 0 || features13.robustImageAccess != 0);
    m_IsSupported.robustness2 = robustness2Features.robustBufferAccess2 != 0 && robustness2Features.robustImageAccess2 != 0;
    m_IsSupported.pipelineRobustness = pipelineRobustnessFeatures.pipelineRobustness;
    m_IsSupported.swapChainMaintenance1 = swapchainMaintenance1Features.swapchainMaintenance1;
    m_IsSupported.fifoLatestReady = presentModeFifoLatestReadyFeaturesEXT.presentModeFifoLatestReady;

    { // Check hard requirements
        bool hasDynamicRendering = features13.dynamicRendering != 0 || (dynamicRenderingFeatures.dynamicRendering != 0 && extendedDynamicStateFeatures.extendedDynamicState != 0);
        bool hasSynchronization2 = features13.synchronization2 != 0 || synchronization2features.synchronization2 != 0;

        RETURN_ON_FAILURE(this, hasDynamicRendering && hasSynchronization2, Result::UNSUPPORTED, "'dynamicRendering' and 'synchronization2' are not supported by the device");
    }

    { // Create device
        if (isWrapper)
            m_Device = (VkDevice)descVK.vkDevice;
        else {
            // Disable undesired features
            if (desc.robustness == Robustness::DEFAULT || desc.robustness == Robustness::VK) {
                robustness2Features.robustBufferAccess2 = 0;
                robustness2Features.robustImageAccess2 = 0;
            } else if (desc.robustness == Robustness::OFF) {
                robustness2Features.robustBufferAccess2 = 0;
                robustness2Features.robustImageAccess2 = 0;
                features.features.robustBufferAccess = 0;
                features13.robustImageAccess = 0;
            }

            // Create device
            std::array<VkDeviceQueueCreateInfo, (size_t)QueueType::MAX_NUM> queueCreateInfos = {};

            VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
            deviceCreateInfo.pNext = &features;
            deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
            deviceCreateInfo.enabledExtensionCount = (uint32_t)desiredDeviceExts.size();
            deviceCreateInfo.ppEnabledExtensionNames = desiredDeviceExts.data();

            std::array<float, 256> zeroPriorities = {};

            for (uint32_t i = 0; i < desc.queueFamilyNum; i++) {
                const QueueFamilyDesc& queueFamily = desc.queueFamilies[i];
                uint32_t queueFamilyIndex = queueFamilyIndices[(size_t)queueFamily.queueType];

                if (queueFamily.queueNum && queueFamilyIndex != INVALID_FAMILY_INDEX) {
                    VkDeviceQueueCreateInfo& queueCreateInfo = queueCreateInfos[deviceCreateInfo.queueCreateInfoCount++];

                    queueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
                    queueCreateInfo.queueCount = queueFamily.queueNum;
                    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
                    queueCreateInfo.pQueuePriorities = queueFamily.queuePriorities ? queueFamily.queuePriorities : zeroPriorities.data();
                }
            }

            VkResult vkResult = m_VK.CreateDevice(m_PhysicalDevice, &deviceCreateInfo, m_AllocationCallbackPtr, &m_Device);
            RETURN_ON_BAD_VKRESULT(this, vkResult, "vkCreateDevice");
        }

        Result res = ResolveDispatchTable(desiredDeviceExts);
        if (res != Result::SUCCESS)
            return res;
    }

    // Create queues
    memset(m_Desc.adapterDesc.queueNum, 0, sizeof(m_Desc.adapterDesc.queueNum)); // patch to reflect available queues
    if (isWrapper) {
        for (uint32_t i = 0; i < descVK.queueFamilyNum; i++) {
            const QueueFamilyVKDesc& queueFamilyDesc = descVK.queueFamilies[i];
            auto& queueFamily = m_QueueFamilies[(size_t)queueFamilyDesc.queueType];
            uint32_t queueFamilyIndex = queueFamilyIndices[(size_t)queueFamilyDesc.queueType];

            if (queueFamilyIndex != INVALID_FAMILY_INDEX) {
                for (uint32_t j = 0; j < queueFamilyDesc.queueNum; j++) {
                    VkDeviceQueueInfo2 queueInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2};
                    queueInfo.queueFamilyIndex = queueFamilyIndex;
                    queueInfo.queueIndex = j;

                    VkQueue handle = VK_NULL_HANDLE;
                    m_VK.GetDeviceQueue2(m_Device, &queueInfo, &handle);

                    QueueVK* queue;
                    Result result = CreateImplementation<QueueVK>(queue, queueFamilyDesc.queueType, queueFamilyDesc.familyIndex, handle);
                    if (result == Result::SUCCESS)
                        queueFamily.push_back(queue);
                }

                m_Desc.adapterDesc.queueNum[(size_t)queueFamilyDesc.queueType] = queueFamilyDesc.queueNum;
            } else
                m_Desc.adapterDesc.queueNum[(size_t)queueFamilyDesc.queueType] = 0;
        }
    } else {
        for (uint32_t i = 0; i < desc.queueFamilyNum; i++) {
            const QueueFamilyDesc& queueFamilyDesc = desc.queueFamilies[i];
            auto& queueFamily = m_QueueFamilies[(size_t)queueFamilyDesc.queueType];
            uint32_t queueFamilyIndex = queueFamilyIndices[(size_t)queueFamilyDesc.queueType];

            if (queueFamilyIndex != INVALID_FAMILY_INDEX) {
                for (uint32_t j = 0; j < queueFamilyDesc.queueNum; j++) {
                    VkDeviceQueueInfo2 queueInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2};
                    queueInfo.queueFamilyIndex = queueFamilyIndices[(size_t)queueFamilyDesc.queueType];
                    queueInfo.queueIndex = j;

                    VkQueue handle = VK_NULL_HANDLE;
                    m_VK.GetDeviceQueue2(m_Device, &queueInfo, &handle);

                    QueueVK* queue;
                    Result result = CreateImplementation<QueueVK>(queue, queueFamilyDesc.queueType, queueInfo.queueFamilyIndex, handle);
                    if (result == Result::SUCCESS)
                        queueFamily.push_back(queue);
                }

                m_Desc.adapterDesc.queueNum[(size_t)queueFamilyDesc.queueType] = queueFamilyDesc.queueNum;
            } else
                m_Desc.adapterDesc.queueNum[(size_t)queueFamilyDesc.queueType] = 0;
        }
    }

    { // Desc
        // Device properties
        VkPhysicalDeviceProperties2 props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        tail = &props.pNext;

        VkPhysicalDeviceVulkan11Properties props11 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES};
        APPEND_EXT(props11);

        VkPhysicalDeviceVulkan12Properties props12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES};
        APPEND_EXT(props12);

        VkPhysicalDeviceVulkan13Properties props13 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES};
        if (m_MinorVersion >= 3) {
            APPEND_EXT(props13);
        }

        VkPhysicalDeviceMaintenance4PropertiesKHR maintenance4Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_4_EXTENSION_NAME, desiredDeviceExts)) {
            APPEND_EXT(maintenance4Props);
        }

        VkPhysicalDeviceMaintenance5PropertiesKHR maintenance5Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, desiredDeviceExts)) {
            APPEND_EXT(maintenance5Props);
        }

        VkPhysicalDeviceMaintenance6PropertiesKHR maintenance6Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_6_EXTENSION_NAME, desiredDeviceExts)) {
            APPEND_EXT(maintenance6Props);
        }

        VkPhysicalDeviceMaintenance7PropertiesKHR maintenance7Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_7_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_7_EXTENSION_NAME, desiredDeviceExts)) {
            APPEND_EXT(maintenance7Props);
        }

        VkPhysicalDeviceMaintenance9PropertiesKHR maintenance9Props = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_9_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_MAINTENANCE_9_EXTENSION_NAME, desiredDeviceExts)) {
            APPEND_EXT(maintenance9Props);
        }

        VkPhysicalDeviceLineRasterizationPropertiesKHR lineRasterizationProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_LINE_RASTERIZATION_EXTENSION_NAME, desiredDeviceExts)) {
            APPEND_EXT(lineRasterizationProps);
        }

        VkPhysicalDeviceFragmentShadingRatePropertiesKHR shadingRateProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, desiredDeviceExts)) {
            APPEND_EXT(shadingRateProps);
        }

        VkPhysicalDevicePushDescriptorPropertiesKHR pushDescriptorProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, desiredDeviceExts)) {
            APPEND_EXT(pushDescriptorProps);
        }

        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, desiredDeviceExts)) {
            APPEND_EXT(rayTracingProps);
        }

        VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR};
        if (IsExtensionSupported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, desiredDeviceExts)) {
            APPEND_EXT(accelerationStructureProps);
        }

        VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT};
        if (IsExtensionSupported(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME, desiredDeviceExts)) {
            APPEND_EXT(conservativeRasterProps);
            m_Desc.tiers.conservativeRaster = 1;
        }

        VkPhysicalDeviceSampleLocationsPropertiesEXT sampleLocationsProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT};
        if (IsExtensionSupported(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME, desiredDeviceExts)) {
            APPEND_EXT(sampleLocationsProps);
            m_Desc.tiers.sampleLocations = 1;
        }

        VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT};
        if (IsExtensionSupported(VK_EXT_MESH_SHADER_EXTENSION_NAME, desiredDeviceExts)) {
            APPEND_EXT(meshShaderProps);
        }

        VkPhysicalDeviceOpacityMicromapPropertiesEXT micromapProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPACITY_MICROMAP_PROPERTIES_EXT};
        if (IsExtensionSupported(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME, desiredDeviceExts)) {
            APPEND_EXT(micromapProps);
        }

        m_VK.GetPhysicalDeviceProperties2(m_PhysicalDevice, &props);

        // Fill desc
        const VkPhysicalDeviceLimits& limits = props.properties.limits;

        m_Desc.viewport.maxNum = limits.maxViewports;
        m_Desc.viewport.boundsMin = (int16_t)limits.viewportBoundsRange[0];
        m_Desc.viewport.boundsMax = (int16_t)limits.viewportBoundsRange[1];

        m_Desc.dimensions.attachmentMaxDim = (Dim_t)std::min(limits.maxFramebufferWidth, limits.maxFramebufferHeight);
        m_Desc.dimensions.attachmentLayerMaxNum = (Dim_t)limits.maxFramebufferLayers;
        m_Desc.dimensions.texture1DMaxDim = (Dim_t)limits.maxImageDimension1D;
        m_Desc.dimensions.texture2DMaxDim = (Dim_t)limits.maxImageDimension2D;
        m_Desc.dimensions.texture3DMaxDim = (Dim_t)limits.maxImageDimension3D;
        m_Desc.dimensions.textureLayerMaxNum = (Dim_t)limits.maxImageArrayLayers;
        m_Desc.dimensions.typedBufferMaxDim = limits.maxTexelBufferElements;

        m_Desc.precision.viewportBits = limits.viewportSubPixelBits;
        m_Desc.precision.subPixelBits = limits.subPixelPrecisionBits;
        m_Desc.precision.subTexelBits = limits.subTexelPrecisionBits;
        m_Desc.precision.mipmapBits = limits.mipmapPrecisionBits;

        const VkMemoryPropertyFlags neededFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        for (uint32_t i = 0; i < m_MemoryProps.memoryTypeCount; i++) {
            const VkMemoryType& memoryType = m_MemoryProps.memoryTypes[i];
            if ((memoryType.propertyFlags & neededFlags) == neededFlags)
                m_Desc.memory.deviceUploadHeapSize += m_MemoryProps.memoryHeaps[memoryType.heapIndex].size;
        }

        m_Desc.memory.allocationMaxNum = limits.maxMemoryAllocationCount;
        m_Desc.memory.samplerAllocationMaxNum = limits.maxSamplerAllocationCount;
        m_Desc.memory.constantBufferMaxRange = limits.maxUniformBufferRange;
        m_Desc.memory.storageBufferMaxRange = limits.maxStorageBufferRange;
        m_Desc.memory.bufferTextureGranularity = (uint32_t)limits.bufferImageGranularity;
        m_Desc.memory.bufferMaxSize = m_MinorVersion >= 3 ? props13.maxBufferSize : maintenance4Props.maxBufferSize;

        // VUID-VkCopyBufferToImageInfo2-dstImage-07975: If "dstImage" does not have either a depth/stencil format or a multi-planar format,
        //      "bufferOffset" must be a multiple of the texel block size
        // VUID-VkCopyBufferToImageInfo2-dstImage-07978: If "dstImage" has a depth/stencil format,
        //      "bufferOffset" must be a multiple of 4
        // Least Common Multiple stride across all formats: 1, 2, 4, 8, 16 // TODO: rarely used "12" fucks up the beauty of power-of-2 numbers, such formats must be avoided!
        constexpr uint32_t leastCommonMultipleStrideAccrossAllFormats = 16;

        m_Desc.memoryAlignment.uploadBufferTextureRow = (uint32_t)limits.optimalBufferCopyRowPitchAlignment;
        m_Desc.memoryAlignment.uploadBufferTextureSlice = std::lcm((uint32_t)limits.optimalBufferCopyOffsetAlignment, leastCommonMultipleStrideAccrossAllFormats);
        m_Desc.memoryAlignment.shaderBindingTable = rayTracingProps.shaderGroupBaseAlignment;
        m_Desc.memoryAlignment.bufferShaderResourceOffset = std::lcm((uint32_t)limits.minTexelBufferOffsetAlignment, (uint32_t)limits.minStorageBufferOffsetAlignment);
        m_Desc.memoryAlignment.constantBufferOffset = (uint32_t)limits.minUniformBufferOffsetAlignment;
        m_Desc.memoryAlignment.scratchBufferOffset = accelerationStructureProps.minAccelerationStructureScratchOffsetAlignment;
        m_Desc.memoryAlignment.accelerationStructureOffset = 256; // see the spec
        m_Desc.memoryAlignment.micromapOffset = 256;              // see the spec

        m_Desc.pipelineLayout.descriptorSetMaxNum = limits.maxBoundDescriptorSets;
        m_Desc.pipelineLayout.rootConstantMaxSize = limits.maxPushConstantsSize;
        m_Desc.pipelineLayout.rootDescriptorMaxNum = pushDescriptorProps.maxPushDescriptors;

        m_Desc.descriptorSet.samplerMaxNum = limits.maxDescriptorSetSamplers;
        m_Desc.descriptorSet.constantBufferMaxNum = limits.maxDescriptorSetUniformBuffers;
        m_Desc.descriptorSet.storageBufferMaxNum = limits.maxDescriptorSetStorageBuffers;
        m_Desc.descriptorSet.textureMaxNum = limits.maxDescriptorSetSampledImages;
        m_Desc.descriptorSet.storageTextureMaxNum = limits.maxDescriptorSetStorageImages;

        m_Desc.descriptorSet.updateAfterSet.samplerMaxNum = props12.maxDescriptorSetUpdateAfterBindSamplers;
        m_Desc.descriptorSet.updateAfterSet.constantBufferMaxNum = props12.maxDescriptorSetUpdateAfterBindUniformBuffers;
        m_Desc.descriptorSet.updateAfterSet.storageBufferMaxNum = props12.maxDescriptorSetUpdateAfterBindStorageBuffers;
        m_Desc.descriptorSet.updateAfterSet.textureMaxNum = props12.maxDescriptorSetUpdateAfterBindSampledImages;
        m_Desc.descriptorSet.updateAfterSet.storageTextureMaxNum = props12.maxDescriptorSetUpdateAfterBindStorageImages;

        m_Desc.shaderStage.descriptorSamplerMaxNum = limits.maxPerStageDescriptorSamplers;
        m_Desc.shaderStage.descriptorConstantBufferMaxNum = limits.maxPerStageDescriptorUniformBuffers;
        m_Desc.shaderStage.descriptorStorageBufferMaxNum = limits.maxPerStageDescriptorStorageBuffers;
        m_Desc.shaderStage.descriptorTextureMaxNum = limits.maxPerStageDescriptorSampledImages;
        m_Desc.shaderStage.descriptorStorageTextureMaxNum = limits.maxPerStageDescriptorStorageImages;
        m_Desc.shaderStage.resourceMaxNum = limits.maxPerStageResources;

        m_Desc.shaderStage.updateAfterSet.descriptorSamplerMaxNum = props12.maxPerStageDescriptorUpdateAfterBindSamplers;
        m_Desc.shaderStage.updateAfterSet.descriptorConstantBufferMaxNum = props12.maxPerStageDescriptorUpdateAfterBindUniformBuffers;
        m_Desc.shaderStage.updateAfterSet.descriptorStorageBufferMaxNum = props12.maxPerStageDescriptorUpdateAfterBindStorageBuffers;
        m_Desc.shaderStage.updateAfterSet.descriptorTextureMaxNum = props12.maxPerStageDescriptorUpdateAfterBindSampledImages;
        m_Desc.shaderStage.updateAfterSet.descriptorStorageTextureMaxNum = props12.maxPerStageDescriptorUpdateAfterBindStorageImages;
        m_Desc.shaderStage.updateAfterSet.resourceMaxNum = props12.maxPerStageUpdateAfterBindResources;

        m_Desc.shaderStage.vertex.attributeMaxNum = limits.maxVertexInputAttributes;
        m_Desc.shaderStage.vertex.streamMaxNum = limits.maxVertexInputBindings;
        m_Desc.shaderStage.vertex.outputComponentMaxNum = limits.maxVertexOutputComponents;

        m_Desc.shaderStage.tesselationControl.generationMaxLevel = (float)limits.maxTessellationGenerationLevel;
        m_Desc.shaderStage.tesselationControl.patchPointMaxNum = limits.maxTessellationPatchSize;
        m_Desc.shaderStage.tesselationControl.perVertexInputComponentMaxNum = limits.maxTessellationControlPerVertexInputComponents;
        m_Desc.shaderStage.tesselationControl.perVertexOutputComponentMaxNum = limits.maxTessellationControlPerVertexOutputComponents;
        m_Desc.shaderStage.tesselationControl.perPatchOutputComponentMaxNum = limits.maxTessellationControlPerPatchOutputComponents;
        m_Desc.shaderStage.tesselationControl.totalOutputComponentMaxNum = limits.maxTessellationControlTotalOutputComponents;

        m_Desc.shaderStage.tesselationEvaluation.inputComponentMaxNum = limits.maxTessellationEvaluationInputComponents;
        m_Desc.shaderStage.tesselationEvaluation.outputComponentMaxNum = limits.maxTessellationEvaluationOutputComponents;

        m_Desc.shaderStage.geometry.invocationMaxNum = limits.maxGeometryShaderInvocations;
        m_Desc.shaderStage.geometry.inputComponentMaxNum = limits.maxGeometryInputComponents;
        m_Desc.shaderStage.geometry.outputComponentMaxNum = limits.maxGeometryOutputComponents;
        m_Desc.shaderStage.geometry.outputVertexMaxNum = limits.maxGeometryOutputVertices;
        m_Desc.shaderStage.geometry.totalOutputComponentMaxNum = limits.maxGeometryTotalOutputComponents;

        m_Desc.shaderStage.fragment.inputComponentMaxNum = limits.maxFragmentInputComponents;
        m_Desc.shaderStage.fragment.attachmentMaxNum = limits.maxFragmentOutputAttachments;
        m_Desc.shaderStage.fragment.dualSourceAttachmentMaxNum = limits.maxFragmentDualSrcAttachments;

        m_Desc.shaderStage.compute.sharedMemoryMaxSize = limits.maxComputeSharedMemorySize;
        m_Desc.shaderStage.compute.workGroupMaxNum[0] = limits.maxComputeWorkGroupCount[0];
        m_Desc.shaderStage.compute.workGroupMaxNum[1] = limits.maxComputeWorkGroupCount[1];
        m_Desc.shaderStage.compute.workGroupMaxNum[2] = limits.maxComputeWorkGroupCount[2];
        m_Desc.shaderStage.compute.workGroupInvocationMaxNum = limits.maxComputeWorkGroupInvocations;
        m_Desc.shaderStage.compute.workGroupMaxDim[0] = limits.maxComputeWorkGroupSize[0];
        m_Desc.shaderStage.compute.workGroupMaxDim[1] = limits.maxComputeWorkGroupSize[1];
        m_Desc.shaderStage.compute.workGroupMaxDim[2] = limits.maxComputeWorkGroupSize[2];

        m_Desc.shaderStage.rayTracing.shaderGroupIdentifierSize = rayTracingProps.shaderGroupHandleSize;
        m_Desc.shaderStage.rayTracing.tableMaxStride = rayTracingProps.maxShaderGroupStride;
        m_Desc.shaderStage.rayTracing.recursionMaxDepth = rayTracingProps.maxRayRecursionDepth;

        m_Desc.shaderStage.meshControl.sharedMemoryMaxSize = meshShaderProps.maxTaskSharedMemorySize;
        m_Desc.shaderStage.meshControl.workGroupInvocationMaxNum = meshShaderProps.maxTaskWorkGroupInvocations;
        m_Desc.shaderStage.meshControl.payloadMaxSize = meshShaderProps.maxTaskPayloadSize;

        m_Desc.shaderStage.meshEvaluation.outputVerticesMaxNum = meshShaderProps.maxMeshOutputVertices;
        m_Desc.shaderStage.meshEvaluation.outputPrimitiveMaxNum = meshShaderProps.maxMeshOutputPrimitives;
        m_Desc.shaderStage.meshEvaluation.outputComponentMaxNum = meshShaderProps.maxMeshOutputComponents;
        m_Desc.shaderStage.meshEvaluation.sharedMemoryMaxSize = meshShaderProps.maxMeshSharedMemorySize;
        m_Desc.shaderStage.meshEvaluation.workGroupInvocationMaxNum = meshShaderProps.maxMeshWorkGroupInvocations;

        m_Desc.other.timestampFrequencyHz = uint64_t(1e9 / double(limits.timestampPeriod) + 0.5);
        m_Desc.other.micromapSubdivisionMaxLevel = micromapProps.maxOpacity2StateSubdivisionLevel;
        m_Desc.other.drawIndirectMaxNum = limits.maxDrawIndirectCount;
        m_Desc.other.samplerLodBiasMax = limits.maxSamplerLodBias;
        m_Desc.other.samplerAnisotropyMax = limits.maxSamplerAnisotropy;
        m_Desc.other.texelOffsetMin = (int8_t)limits.minTexelOffset;
        m_Desc.other.texelOffsetMax = (uint8_t)limits.maxTexelOffset;
        m_Desc.other.texelGatherOffsetMin = (int8_t)limits.minTexelGatherOffset;
        m_Desc.other.texelGatherOffsetMax = (uint8_t)limits.maxTexelGatherOffset;
        m_Desc.other.clipDistanceMaxNum = (uint8_t)limits.maxClipDistances;
        m_Desc.other.cullDistanceMaxNum = (uint8_t)limits.maxCullDistances;
        m_Desc.other.combinedClipAndCullDistanceMaxNum = (uint8_t)limits.maxCombinedClipAndCullDistances;
        m_Desc.other.viewMaxNum = features11.multiview ? (uint8_t)props11.maxMultiviewViewCount : 1;
        m_Desc.other.shadingRateAttachmentTileSize = (uint8_t)shadingRateProps.minFragmentShadingRateAttachmentTexelSize.width;

        if (m_Desc.tiers.conservativeRaster) {
            if (conservativeRasterProps.primitiveOverestimationSize < 1.0f / 2.0f && conservativeRasterProps.degenerateTrianglesRasterized)
                m_Desc.tiers.conservativeRaster = 2;
            if (conservativeRasterProps.primitiveOverestimationSize <= 1.0 / 256.0f && conservativeRasterProps.degenerateTrianglesRasterized)
                m_Desc.tiers.conservativeRaster = 3;
        }

        if (m_Desc.tiers.sampleLocations) {
            constexpr VkSampleCountFlags allSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT | VK_SAMPLE_COUNT_8_BIT | VK_SAMPLE_COUNT_16_BIT;
            if (sampleLocationsProps.sampleLocationSampleCounts == allSampleCounts) // like in D3D12 spec
                m_Desc.tiers.sampleLocations = 2;
        }

        m_Desc.tiers.rayTracing = accelerationStructureFeatures.accelerationStructure != 0;
        if (m_Desc.tiers.rayTracing) {
            if (rayTracingPipelineFeatures.rayTracingPipelineTraceRaysIndirect && rayQueryFeatures.rayQuery)
                m_Desc.tiers.rayTracing++;
            if (micromapFeatures.micromap)
                m_Desc.tiers.rayTracing++;
        }

        m_Desc.tiers.shadingRate = shadingRateFeatures.pipelineFragmentShadingRate != 0;
        if (m_Desc.tiers.shadingRate) {
            if (shadingRateFeatures.primitiveFragmentShadingRate && shadingRateFeatures.attachmentFragmentShadingRate)
                m_Desc.tiers.shadingRate = 2;

            m_Desc.features.additionalShadingRates = shadingRateProps.maxFragmentSize.height > 2 || shadingRateProps.maxFragmentSize.width > 2;
        }

        m_Desc.tiers.bindless = m_IsSupported.descriptorIndexing ? 1 : 0;
        m_Desc.tiers.resourceBinding = 2; // TODO: seems to be the best match
        m_Desc.tiers.memory = 1;          // TODO: seems to be the best match

        m_Desc.features.getMemoryDesc2 = m_IsSupported.maintenance4;
        m_Desc.features.enhancedBarriers = true;
        m_Desc.features.swapChain = IsExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME, desiredDeviceExts);
        m_Desc.features.rayTracing = m_Desc.tiers.rayTracing != 0;
        m_Desc.features.meshShader = meshShaderFeatures.meshShader != 0 && meshShaderFeatures.taskShader != 0;
        m_Desc.features.lowLatency = m_IsSupported.presentId != 0 && IsExtensionSupported(VK_NV_LOW_LATENCY_2_EXTENSION_NAME, desiredDeviceExts);
        m_Desc.features.micromap = micromapFeatures.micromap != 0;

        m_Desc.features.independentFrontAndBackStencilReferenceAndMasks = true;
        m_Desc.features.textureFilterMinMax = features12.samplerFilterMinmax;
        m_Desc.features.logicOp = features.features.logicOp;
        m_Desc.features.depthBoundsTest = features.features.depthBounds;
        m_Desc.features.drawIndirectCount = features12.drawIndirectCount;
        m_Desc.features.lineSmoothing = lineRasterizationFeatures.smoothLines;
        m_Desc.features.copyQueueTimestamp = limits.timestampComputeAndGraphics;
        m_Desc.features.meshShaderPipelineStats = meshShaderFeatures.meshShaderQueries == VK_TRUE;
        m_Desc.features.dynamicDepthBias = true;
        m_Desc.features.viewportOriginBottomLeft = true;
        m_Desc.features.regionResolve = true;
        m_Desc.features.layerBasedMultiview = features11.multiview;
        m_Desc.features.presentFromCompute = true;
        m_Desc.features.waitableSwapChain = presentIdFeatures.presentId != 0 && presentWaitFeatures.presentWait != 0;
        m_Desc.features.pipelineStatistics = features.features.pipelineStatisticsQuery;

        m_Desc.shaderFeatures.nativeI16 = features.features.shaderInt16;
        m_Desc.shaderFeatures.nativeF16 = features12.shaderFloat16;
        m_Desc.shaderFeatures.nativeI64 = features.features.shaderInt64;
        m_Desc.shaderFeatures.nativeF64 = features.features.shaderFloat64;
        m_Desc.shaderFeatures.atomicsF16 = (shaderAtomicFloat2Features.shaderBufferFloat16Atomics || shaderAtomicFloat2Features.shaderSharedFloat16Atomics) ? true : false;
        m_Desc.shaderFeatures.atomicsF32 = (shaderAtomicFloatFeatures.shaderBufferFloat32Atomics || shaderAtomicFloatFeatures.shaderSharedFloat32Atomics) ? true : false;
        m_Desc.shaderFeatures.atomicsI64 = (features12.shaderBufferInt64Atomics || features12.shaderSharedInt64Atomics) ? true : false;
        m_Desc.shaderFeatures.atomicsF64 = (shaderAtomicFloatFeatures.shaderBufferFloat64Atomics || shaderAtomicFloatFeatures.shaderSharedFloat64Atomics) ? true : false;
        m_Desc.shaderFeatures.viewportIndex = features12.shaderOutputViewportIndex;
        m_Desc.shaderFeatures.layerIndex = features12.shaderOutputLayer;
        m_Desc.shaderFeatures.clock = (shaderClockFeatures.shaderDeviceClock || shaderClockFeatures.shaderSubgroupClock) ? true : false;
        m_Desc.shaderFeatures.rasterizedOrderedView = fragmentShaderInterlockFeatures.fragmentShaderPixelInterlock != 0 && fragmentShaderInterlockFeatures.fragmentShaderSampleInterlock != 0;
        m_Desc.shaderFeatures.barycentric = fragmentShaderBarycentricFeatures.fragmentShaderBarycentric;
        m_Desc.shaderFeatures.rayTracingPositionFetch = rayTracingPositionFetchFeatures.rayTracingPositionFetch;
        m_Desc.shaderFeatures.storageReadWithoutFormat = features.features.shaderStorageImageReadWithoutFormat;
        m_Desc.shaderFeatures.storageWriteWithoutFormat = features.features.shaderStorageImageWriteWithoutFormat;

        // Estimate shader model last since it depends on many "m_Desc" fields
        // Based on https://docs.vulkan.org/guide/latest/hlsl.html#_shader_model_coverage // TODO: code below needs to be improved
        m_Desc.shaderModel = 51;
        if (m_Desc.shaderFeatures.nativeI64)
            m_Desc.shaderModel = 60;
        if (m_Desc.other.viewMaxNum > 1 || m_Desc.shaderFeatures.barycentric)
            m_Desc.shaderModel = 61;
        if (m_Desc.shaderFeatures.nativeF16 || m_Desc.shaderFeatures.nativeI16)
            m_Desc.shaderModel = 62;
        if (m_Desc.features.rayTracing)
            m_Desc.shaderModel = 63;
        if (m_Desc.tiers.shadingRate >= 2)
            m_Desc.shaderModel = 64;
        if (m_Desc.features.meshShader || m_Desc.tiers.rayTracing >= 2)
            m_Desc.shaderModel = 65;
        if (m_Desc.shaderFeatures.atomicsI64)
            m_Desc.shaderModel = 66;
        if (features.features.shaderStorageImageMultisample)
            m_Desc.shaderModel = 67;
    }

    ReportDeviceGroupInfo();

    return FillFunctionTable(m_iCore);
}

void DeviceVK::FillCreateInfo(const BufferDesc& bufferDesc, VkBufferCreateInfo& info) const {
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO; // should be already set
    info.size = bufferDesc.size;
    info.usage = GetBufferUsageFlags(bufferDesc.usage, bufferDesc.structureStride, m_IsSupported.deviceAddress);
    info.sharingMode = m_NumActiveFamilyIndices <= 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    info.queueFamilyIndexCount = m_NumActiveFamilyIndices;
    info.pQueueFamilyIndices = m_ActiveQueueFamilyIndices.data();
}

void DeviceVK::FillCreateInfo(const TextureDesc& textureDesc, VkImageCreateInfo& info) const {
    VkImageCreateFlags flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT; // typeless
    const FormatProps& formatProps = GetFormatProps(textureDesc.format);
    if (formatProps.blockWidth > 1 && (textureDesc.usage & TextureUsageBits::SHADER_RESOURCE_STORAGE))
        flags |= VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT; // format can be used to create a view with an uncompressed format (1 texel covers 1 block)
    if (textureDesc.layerNum >= 6 && textureDesc.width == textureDesc.height)
        flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; // allow cube maps
    if (textureDesc.type == TextureType::TEXTURE_3D)
        flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT; // allow 3D demotion to a set of layers // TODO: hook up "VK_EXT_image_2d_view_of_3d"?
    if (m_Desc.tiers.sampleLocations && formatProps.isDepth)
        flags |= VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT;

    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO; // should be already set
    info.flags = flags;
    info.imageType = ::GetImageType(textureDesc.type);
    info.format = ::GetVkFormat(textureDesc.format, true);
    info.extent.width = textureDesc.width;
    info.extent.height = std::max(textureDesc.height, (Dim_t)1);
    info.extent.depth = std::max(textureDesc.depth, (Dim_t)1);
    info.mipLevels = std::max(textureDesc.mipNum, (Dim_t)1);
    info.arrayLayers = std::max(textureDesc.layerNum, (Dim_t)1);
    info.samples = (VkSampleCountFlagBits)std::max(textureDesc.sampleNum, (Sample_t)1);
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = GetImageUsageFlags(textureDesc.usage);
    info.sharingMode = (m_NumActiveFamilyIndices <= 1 || textureDesc.sharingMode == SharingMode::EXCLUSIVE) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    info.queueFamilyIndexCount = m_NumActiveFamilyIndices;
    info.pQueueFamilyIndices = m_ActiveQueueFamilyIndices.data();
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void DeviceVK::GetMemoryDesc2(const BufferDesc& bufferDesc, MemoryLocation memoryLocation, MemoryDesc& memoryDesc) const {
    VkBufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    FillCreateInfo(bufferDesc, createInfo);

    VkMemoryDedicatedRequirements dedicatedRequirements = {VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS};

    VkMemoryRequirements2 requirements = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2};
    requirements.pNext = &dedicatedRequirements;

    VkDeviceBufferMemoryRequirements bufferMemoryRequirements = {VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS};
    bufferMemoryRequirements.pCreateInfo = &createInfo;

    const auto& vk = GetDispatchTable();
    vk.GetDeviceBufferMemoryRequirements(m_Device, &bufferMemoryRequirements, &requirements);

    MemoryTypeInfo memoryTypeInfo = {};
    memoryTypeInfo.mustBeDedicated = dedicatedRequirements.prefersDedicatedAllocation;

    memoryDesc = {};
    if (GetMemoryTypeInfo(memoryLocation, requirements.memoryRequirements.memoryTypeBits, memoryTypeInfo)) {
        memoryDesc.size = requirements.memoryRequirements.size;
        memoryDesc.alignment = (uint32_t)requirements.memoryRequirements.alignment;
        memoryDesc.type = Pack(memoryTypeInfo);
        memoryDesc.mustBeDedicated = memoryTypeInfo.mustBeDedicated;
    }
}

void DeviceVK::GetMemoryDesc2(const TextureDesc& textureDesc, MemoryLocation memoryLocation, MemoryDesc& memoryDesc) const {
    VkImageCreateInfo createInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    FillCreateInfo(textureDesc, createInfo);

    VkMemoryDedicatedRequirements dedicatedRequirements = {VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS};

    VkMemoryRequirements2 requirements = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2};
    requirements.pNext = &dedicatedRequirements;

    VkDeviceImageMemoryRequirements imageMemoryRequirements = {VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS};
    imageMemoryRequirements.pCreateInfo = &createInfo;

    const auto& vk = GetDispatchTable();
    vk.GetDeviceImageMemoryRequirements(m_Device, &imageMemoryRequirements, &requirements);

    MemoryTypeInfo memoryTypeInfo = {};
    memoryTypeInfo.mustBeDedicated = dedicatedRequirements.prefersDedicatedAllocation;

    memoryDesc = {};
    if (GetMemoryTypeInfo(memoryLocation, requirements.memoryRequirements.memoryTypeBits, memoryTypeInfo)) {
        memoryDesc.size = requirements.memoryRequirements.size;
        memoryDesc.alignment = (uint32_t)requirements.memoryRequirements.alignment;
        memoryDesc.type = Pack(memoryTypeInfo);
        memoryDesc.mustBeDedicated = memoryTypeInfo.mustBeDedicated;
    }
}

void DeviceVK::GetMemoryDesc2(const AccelerationStructureDesc& accelerationStructureDesc, MemoryLocation memoryLocation, MemoryDesc& memoryDesc) {
    VkAccelerationStructureBuildSizesInfoKHR sizesInfo = {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
    GetAccelerationStructureBuildSizesInfo(accelerationStructureDesc, sizesInfo);

    BufferDesc bufferDesc = {};
    bufferDesc.size = sizesInfo.accelerationStructureSize;
    bufferDesc.usage = BufferUsageBits::ACCELERATION_STRUCTURE_STORAGE;

    GetMemoryDesc2(bufferDesc, memoryLocation, memoryDesc);
}

void DeviceVK::GetMemoryDesc2(const MicromapDesc& micromapDesc, MemoryLocation memoryLocation, MemoryDesc& memoryDesc) {
    VkMicromapBuildSizesInfoEXT sizesInfo = {VK_STRUCTURE_TYPE_MICROMAP_BUILD_SIZES_INFO_EXT};
    GetMicromapBuildSizesInfo(micromapDesc, sizesInfo);

    BufferDesc bufferDesc = {};
    bufferDesc.size = sizesInfo.micromapSize;
    bufferDesc.usage = BufferUsageBits::ACCELERATION_STRUCTURE_STORAGE;

    GetMemoryDesc2(bufferDesc, memoryLocation, memoryDesc);
}

bool DeviceVK::GetMemoryTypeInfo(MemoryLocation memoryLocation, uint32_t memoryTypeMask, MemoryTypeInfo& memoryTypeInfo) const {
    VkMemoryPropertyFlags neededFlags = 0;    // must have
    VkMemoryPropertyFlags undesiredFlags = 0; // have higher priority than desired
    VkMemoryPropertyFlags desiredFlags = 0;   // nice to have

    if (memoryLocation == MemoryLocation::DEVICE) {
        neededFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        undesiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    } else if (memoryLocation == MemoryLocation::DEVICE_UPLOAD) {
        neededFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        undesiredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        desiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    } else {
        neededFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        undesiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        desiredFlags = (memoryLocation == MemoryLocation::HOST_READBACK ? VK_MEMORY_PROPERTY_HOST_CACHED_BIT : 0) | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    // Phase 1: needed, undesired and desired
    for (uint32_t i = 0; i < m_MemoryProps.memoryTypeCount; i++) {
        bool isSupported = memoryTypeMask & (1 << i);
        bool hasNeededFlags = (m_MemoryProps.memoryTypes[i].propertyFlags & neededFlags) == neededFlags;
        bool hasUndesiredFlags = undesiredFlags == 0 ? false : (m_MemoryProps.memoryTypes[i].propertyFlags & undesiredFlags) == undesiredFlags;
        bool hasDesiredFlags = (m_MemoryProps.memoryTypes[i].propertyFlags & desiredFlags) == desiredFlags;

        if (isSupported && hasNeededFlags && !hasUndesiredFlags && hasDesiredFlags) {
            memoryTypeInfo.index = (MemoryTypeIndex)i;
            memoryTypeInfo.location = memoryLocation;

            return true;
        }
    }

    // Phase 2: needed and undesired
    for (uint32_t i = 0; i < m_MemoryProps.memoryTypeCount; i++) {
        bool isSupported = memoryTypeMask & (1 << i);
        bool hasNeededFlags = (m_MemoryProps.memoryTypes[i].propertyFlags & neededFlags) == neededFlags;
        bool hasUndesiredFlags = undesiredFlags == 0 ? false : (m_MemoryProps.memoryTypes[i].propertyFlags & undesiredFlags) == undesiredFlags;

        if (isSupported && hasNeededFlags && !hasUndesiredFlags) {
            memoryTypeInfo.index = (MemoryTypeIndex)i;
            memoryTypeInfo.location = memoryLocation;

            return true;
        }
    }

    // Phase 3: needed and desired
    for (uint32_t i = 0; i < m_MemoryProps.memoryTypeCount; i++) {
        bool isSupported = memoryTypeMask & (1 << i);
        bool hasNeededFlags = (m_MemoryProps.memoryTypes[i].propertyFlags & neededFlags) == neededFlags;
        bool hasDesiredFlags = (m_MemoryProps.memoryTypes[i].propertyFlags & desiredFlags) == desiredFlags;

        if (isSupported && hasNeededFlags && hasDesiredFlags) {
            memoryTypeInfo.index = (MemoryTypeIndex)i;
            memoryTypeInfo.location = memoryLocation;

            return true;
        }
    }

    // Phase 4: only needed
    for (uint32_t i = 0; i < m_MemoryProps.memoryTypeCount; i++) {
        bool isSupported = memoryTypeMask & (1 << i);
        bool hasNeededFlags = (m_MemoryProps.memoryTypes[i].propertyFlags & neededFlags) == neededFlags;

        if (isSupported && hasNeededFlags) {
            memoryTypeInfo.index = (MemoryTypeIndex)i;
            memoryTypeInfo.location = memoryLocation;

            return true;
        }
    }

    CHECK(false, "Can't find suitable memory type");

    return false;
}

bool DeviceVK::GetMemoryTypeByIndex(uint32_t index, MemoryTypeInfo& memoryTypeInfo) const {
    if (index >= m_MemoryProps.memoryTypeCount)
        return false;

    const VkMemoryType& memoryType = m_MemoryProps.memoryTypes[index];
    bool isHostVisible = memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    bool isDevice = memoryType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    memoryTypeInfo.index = (MemoryTypeIndex)index;
    if (isDevice)
        memoryTypeInfo.location = isHostVisible ? MemoryLocation::DEVICE_UPLOAD : MemoryLocation::DEVICE;
    else
        memoryTypeInfo.location = MemoryLocation::HOST_UPLOAD;

    return true;
}

void DeviceVK::GetAccelerationStructureBuildSizesInfo(const AccelerationStructureDesc& accelerationStructureDesc, VkAccelerationStructureBuildSizesInfoKHR& sizesInfo) {
    // Allocate scratch
    uint32_t geometryNum = 0;
    uint32_t micromapNum = 0;

    if (accelerationStructureDesc.type == AccelerationStructureType::BOTTOM_LEVEL) {
        geometryNum = accelerationStructureDesc.geometryOrInstanceNum;

        for (uint32_t i = 0; i < geometryNum; i++) {
            const BottomLevelGeometryDesc& geometryDesc = accelerationStructureDesc.geometries[i];

            if (geometryDesc.type == BottomLevelGeometryType::TRIANGLES && geometryDesc.triangles.micromap)
                micromapNum++;
        }
    } else
        geometryNum = 1;

    Scratch<uint32_t> primitiveNums = AllocateScratch(*this, uint32_t, geometryNum);
    Scratch<VkAccelerationStructureGeometryKHR> geometries = AllocateScratch(*this, VkAccelerationStructureGeometryKHR, geometryNum);
    Scratch<VkAccelerationStructureTrianglesOpacityMicromapEXT> trianglesMicromaps = AllocateScratch(*this, VkAccelerationStructureTrianglesOpacityMicromapEXT, micromapNum);

    // Convert geometries
    if (accelerationStructureDesc.type == AccelerationStructureType::BOTTOM_LEVEL) {
        micromapNum = ConvertBotomLevelGeometries(nullptr, geometries, trianglesMicromaps, accelerationStructureDesc.geometries, geometryNum);

        for (uint32_t i = 0; i < geometryNum; i++) {
            const BottomLevelGeometryDesc& in = accelerationStructureDesc.geometries[i];

            if (in.type == BottomLevelGeometryType::TRIANGLES) {
                uint32_t triangleNum = (in.triangles.indexNum ? in.triangles.indexNum : in.triangles.vertexNum) / 3;
                primitiveNums[i] = triangleNum;
            } else if (in.type == BottomLevelGeometryType::AABBS)
                primitiveNums[i] = in.aabbs.num;
        }
    } else {
        geometries[0] = {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
        geometries[0].geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        geometries[0].geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;

        primitiveNums[0] = accelerationStructureDesc.geometryOrInstanceNum;
    }

    // Get sizes
    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
    buildInfo.type = GetAccelerationStructureType(accelerationStructureDesc.type);
    buildInfo.flags = GetBuildAccelerationStructureFlags(accelerationStructureDesc.flags);
    buildInfo.geometryCount = geometryNum;
    buildInfo.pGeometries = geometries;

    const auto& vk = GetDispatchTable();
    vk.GetAccelerationStructureBuildSizesKHR(m_Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, primitiveNums, &sizesInfo);
}

void DeviceVK::GetMicromapBuildSizesInfo(const MicromapDesc& micromapDesc, VkMicromapBuildSizesInfoEXT& sizesInfo) {
    static_assert((uint32_t)MicromapFormat::OPACITY_2_STATE == VK_OPACITY_MICROMAP_FORMAT_2_STATE_EXT, "Format doesn't match");
    static_assert((uint32_t)MicromapFormat::OPACITY_4_STATE == VK_OPACITY_MICROMAP_FORMAT_4_STATE_EXT, "Format doesn't match");

    Scratch<VkMicromapUsageEXT> usages = AllocateScratch(*this, VkMicromapUsageEXT, micromapDesc.usageNum);
    for (uint32_t i = 0; i < micromapDesc.usageNum; i++) {
        const MicromapUsageDesc& in = micromapDesc.usages[i];

        VkMicromapUsageEXT& out = usages[i];
        out = {};
        out.count = in.triangleNum;
        out.subdivisionLevel = in.subdivisionLevel;
        out.format = (VkOpacityMicromapFormatEXT)in.format;
    }

    VkMicromapBuildInfoEXT buildInfo = {VK_STRUCTURE_TYPE_MICROMAP_BUILD_INFO_EXT};
    buildInfo.type = VK_MICROMAP_TYPE_OPACITY_MICROMAP_EXT;
    buildInfo.flags = GetBuildMicromapFlags(micromapDesc.flags);
    buildInfo.usageCountsCount = micromapDesc.usageNum;
    buildInfo.pUsageCounts = usages;

    const auto& vk = GetDispatchTable();
    vk.GetMicromapBuildSizesEXT(m_Device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &sizesInfo);
}

Result DeviceVK::CreateInstance(bool enableGraphicsAPIValidation, const Vector<const char*>& desiredInstanceExts) {
    Vector<const char*> layers(GetStdAllocator());
    if (enableGraphicsAPIValidation)
        layers.push_back("VK_LAYER_KHRONOS_validation");

    FilterInstanceLayers(layers);

    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.apiVersion = VK_API_VERSION_1_3;

    const VkValidationFeatureEnableEXT enabledValidationFeatures[] = {
        VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT, // TODO: add VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT?
    };

    VkInstanceCreateInfo instanceCreateInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
#ifdef __APPLE__
    instanceCreateInfo.flags = (VkInstanceCreateFlags)VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
#endif
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = (uint32_t)layers.size();
    instanceCreateInfo.ppEnabledLayerNames = layers.data();
    instanceCreateInfo.enabledExtensionCount = (uint32_t)desiredInstanceExts.size();
    instanceCreateInfo.ppEnabledExtensionNames = desiredInstanceExts.data();

    const void** tail = &instanceCreateInfo.pNext;

    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    messengerCreateInfo.pUserData = this;
    messengerCreateInfo.pfnUserCallback = MessageCallback;
    messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    APPEND_EXT(messengerCreateInfo);

    VkValidationFeaturesEXT validationFeatures = {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT};
    validationFeatures.enabledValidationFeatureCount = GetCountOf(enabledValidationFeatures);
    validationFeatures.pEnabledValidationFeatures = enabledValidationFeatures;

    if (enableGraphicsAPIValidation) {
        APPEND_EXT(validationFeatures);
    }

    VkResult vkResult = m_VK.CreateInstance(&instanceCreateInfo, m_AllocationCallbackPtr, &m_Instance);
    RETURN_ON_BAD_VKRESULT(this, vkResult, "vkCreateInstance");

    if (enableGraphicsAPIValidation) {
        PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)m_VK.GetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
        vkResult = vkCreateDebugUtilsMessengerEXT(m_Instance, &messengerCreateInfo, m_AllocationCallbackPtr, &m_Messenger);

        RETURN_ON_BAD_VKRESULT(this, vkResult, "vkCreateDebugUtilsMessengerEXT");
    }

    return Result::SUCCESS;
}

void DeviceVK::SetDebugNameToTrivialObject(VkObjectType objectType, uint64_t handle, const char* name) {
    if (!m_VK.SetDebugUtilsObjectNameEXT)
        return;

    VkDebugUtilsObjectNameInfoEXT objectNameInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, nullptr, objectType, (uint64_t)handle, name};

    VkResult vkResult = m_VK.SetDebugUtilsObjectNameEXT(m_Device, &objectNameInfo);
    RETURN_VOID_ON_BAD_VKRESULT(this, vkResult, "vkSetDebugUtilsObjectNameEXT");
}

void DeviceVK::ReportDeviceGroupInfo() {
    String text(GetStdAllocator());

    REPORT_INFO(this, "Memory heaps:");
    for (uint32_t i = 0; i < m_MemoryProps.memoryHeapCount; i++) {
        text.clear();

        if (m_MemoryProps.memoryHeaps[i].flags == 0)
            text += "*SYSMEM* ";
        if (m_MemoryProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            text += "DEVICE_LOCAL_BIT ";
        if (m_MemoryProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT)
            text += "MULTI_INSTANCE_BIT ";

        double size = double(m_MemoryProps.memoryHeaps[i].size) / (1024.0 * 1024.0);
        REPORT_INFO(this, "  Heap #%u: %.f Mb - %s", i, size, text.c_str());
    }

    REPORT_INFO(this, "Memory types:");
    for (uint32_t i = 0; i < m_MemoryProps.memoryTypeCount; i++) {
        text.clear();

        REPORT_INFO(this, "  Memory type #%u", i);
        REPORT_INFO(this, "    Heap #%u", m_MemoryProps.memoryTypes[i].heapIndex);

        VkMemoryPropertyFlags flags = m_MemoryProps.memoryTypes[i].propertyFlags;
        if (flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            text += "DEVICE_LOCAL_BIT ";
        if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            text += "HOST_VISIBLE_BIT ";
        if (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            text += "HOST_COHERENT_BIT ";
        if (flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
            text += "HOST_CACHED_BIT ";
        if (flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
            text += "LAZILY_ALLOCATED_BIT ";
        if (flags & VK_MEMORY_PROPERTY_PROTECTED_BIT)
            text += "PROTECTED_BIT ";
        if (flags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD)
            text += "DEVICE_COHERENT_BIT_AMD ";
        if (flags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD)
            text += "DEVICE_UNCACHED_BIT_AMD ";
        if (flags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV)
            text += "RDMA_CAPABLE_BIT_NV ";

        if (!text.empty())
            REPORT_INFO(this, "    %s", text.c_str());
    }
}

#define MERGE_TOKENS2(a, b)    a##b
#define MERGE_TOKENS3(a, b, c) a##b##c

#define GET_DEVICE_OPTIONAL_CORE_FUNC(name) \
    /* Core */ \
    m_VK.name = (PFN_vk##name)m_VK.GetDeviceProcAddr(m_Device, NRI_STRINGIFY(MERGE_TOKENS2(vk, name))); \
    /* KHR */ \
    if (!m_VK.name) \
        m_VK.name = (PFN_vk##name)m_VK.GetDeviceProcAddr(m_Device, NRI_STRINGIFY(MERGE_TOKENS3(vk, name, KHR))); \
    /* EXT (some extensions were promoted to core from EXT bypassing KHR status) */ \
    if (!m_VK.name) \
    m_VK.name = (PFN_vk##name)m_VK.GetDeviceProcAddr(m_Device, NRI_STRINGIFY(MERGE_TOKENS3(vk, name, EXT)))

#define GET_DEVICE_CORE_FUNC(name) \
    GET_DEVICE_OPTIONAL_CORE_FUNC(name); \
    if (!m_VK.name) { \
        REPORT_ERROR(this, "Failed to get device function: '%s'", NRI_STRINGIFY(MERGE_TOKENS2(vk, name))); \
        return Result::UNSUPPORTED; \
    }

#define GET_DEVICE_FUNC(name) \
    m_VK.name = (PFN_vk##name)m_VK.GetDeviceProcAddr(m_Device, NRI_STRINGIFY(MERGE_TOKENS2(vk, name))); \
    if (!m_VK.name) { \
        REPORT_ERROR(this, "Failed to get device function: '%s'", NRI_STRINGIFY(MERGE_TOKENS2(vk, name))); \
        return Result::UNSUPPORTED; \
    }

#define GET_INSTANCE_FUNC(name) \
    m_VK.name = (PFN_vk##name)m_VK.GetInstanceProcAddr(m_Instance, NRI_STRINGIFY(MERGE_TOKENS2(vk, name))); \
    if (!m_VK.name) { \
        REPORT_ERROR(this, "Failed to get instance function: '%s'", NRI_STRINGIFY(MERGE_TOKENS2(vk, name))); \
        return Result::UNSUPPORTED; \
    }

Result DeviceVK::ResolvePreInstanceDispatchTable() {
    m_VK = {};

    m_VK.GetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetSharedLibraryFunction(*m_Loader, "vkGetInstanceProcAddr");
    if (!m_VK.GetInstanceProcAddr) {
        REPORT_ERROR(this, "Failed to get 'vkGetInstanceProcAddr'");
        return Result::UNSUPPORTED;
    }

    GET_INSTANCE_FUNC(CreateInstance);
    GET_INSTANCE_FUNC(EnumerateInstanceExtensionProperties);
    GET_INSTANCE_FUNC(EnumerateInstanceLayerProperties);

    return Result::SUCCESS;
}

Result DeviceVK::ResolveInstanceDispatchTable(const Vector<const char*>& desiredInstanceExts) {
    GET_INSTANCE_FUNC(GetDeviceProcAddr);
    GET_INSTANCE_FUNC(DestroyInstance);
    GET_INSTANCE_FUNC(DestroyDevice);
    GET_INSTANCE_FUNC(GetPhysicalDeviceMemoryProperties2);
    GET_INSTANCE_FUNC(GetDeviceGroupPeerMemoryFeatures);
    GET_INSTANCE_FUNC(GetPhysicalDeviceFormatProperties2);
    GET_INSTANCE_FUNC(GetPhysicalDeviceImageFormatProperties2);
    GET_INSTANCE_FUNC(CreateDevice);
    GET_INSTANCE_FUNC(GetDeviceQueue2);
    GET_INSTANCE_FUNC(EnumeratePhysicalDeviceGroups);
    GET_INSTANCE_FUNC(GetPhysicalDeviceProperties2);
    GET_INSTANCE_FUNC(GetPhysicalDeviceFeatures2);
    GET_INSTANCE_FUNC(GetPhysicalDeviceQueueFamilyProperties2);
    GET_INSTANCE_FUNC(EnumerateDeviceExtensionProperties);

    if (IsExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, desiredInstanceExts)) {
        GET_INSTANCE_FUNC(SetDebugUtilsObjectNameEXT);
        GET_INSTANCE_FUNC(CmdBeginDebugUtilsLabelEXT);
        GET_INSTANCE_FUNC(CmdEndDebugUtilsLabelEXT);
        GET_INSTANCE_FUNC(CmdInsertDebugUtilsLabelEXT);
        GET_INSTANCE_FUNC(QueueBeginDebugUtilsLabelEXT);
        GET_INSTANCE_FUNC(QueueEndDebugUtilsLabelEXT);
        GET_INSTANCE_FUNC(QueueInsertDebugUtilsLabelEXT);
    }

    if (IsExtensionSupported(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME, desiredInstanceExts)) {
        GET_INSTANCE_FUNC(GetPhysicalDeviceSurfaceFormats2KHR);
        GET_INSTANCE_FUNC(GetPhysicalDeviceSurfaceCapabilities2KHR);
    }

    if (IsExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME, desiredInstanceExts)) {
        GET_INSTANCE_FUNC(GetPhysicalDeviceSurfaceSupportKHR);
        GET_INSTANCE_FUNC(GetPhysicalDeviceSurfacePresentModesKHR);
        GET_INSTANCE_FUNC(DestroySurfaceKHR);

#ifdef VK_USE_PLATFORM_WIN32_KHR
        GET_INSTANCE_FUNC(CreateWin32SurfaceKHR);
        GET_INSTANCE_FUNC(GetMemoryWin32HandlePropertiesKHR);
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
        GET_INSTANCE_FUNC(CreateXlibSurfaceKHR);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
        GET_INSTANCE_FUNC(CreateWaylandSurfaceKHR);
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
        GET_INSTANCE_FUNC(CreateMetalSurfaceEXT);
#endif
    }

    return Result::SUCCESS;
}

Result DeviceVK::ResolveDispatchTable(const Vector<const char*>& desiredDeviceExts) {
    GET_DEVICE_CORE_FUNC(CreateBuffer);
    GET_DEVICE_CORE_FUNC(CreateImage);
    GET_DEVICE_CORE_FUNC(CreateBufferView);
    GET_DEVICE_CORE_FUNC(CreateImageView);
    GET_DEVICE_CORE_FUNC(CreateSampler);
    GET_DEVICE_CORE_FUNC(CreateQueryPool);
    GET_DEVICE_CORE_FUNC(CreateCommandPool);
    GET_DEVICE_CORE_FUNC(CreateSemaphore);
    GET_DEVICE_CORE_FUNC(CreateDescriptorPool);
    GET_DEVICE_CORE_FUNC(CreatePipelineLayout);
    GET_DEVICE_CORE_FUNC(CreateDescriptorSetLayout);
    GET_DEVICE_CORE_FUNC(CreateShaderModule);
    GET_DEVICE_CORE_FUNC(CreateGraphicsPipelines);
    GET_DEVICE_CORE_FUNC(CreateComputePipelines);
    GET_DEVICE_CORE_FUNC(AllocateMemory);

    GET_DEVICE_CORE_FUNC(DestroyBuffer);
    GET_DEVICE_CORE_FUNC(DestroyImage);
    GET_DEVICE_CORE_FUNC(DestroyBufferView);
    GET_DEVICE_CORE_FUNC(DestroyImageView);
    GET_DEVICE_CORE_FUNC(DestroySampler);
    GET_DEVICE_CORE_FUNC(DestroyFramebuffer);
    GET_DEVICE_CORE_FUNC(DestroyQueryPool);
    GET_DEVICE_CORE_FUNC(DestroyCommandPool);
    GET_DEVICE_CORE_FUNC(DestroySemaphore);
    GET_DEVICE_CORE_FUNC(DestroyDescriptorPool);
    GET_DEVICE_CORE_FUNC(DestroyPipelineLayout);
    GET_DEVICE_CORE_FUNC(DestroyDescriptorSetLayout);
    GET_DEVICE_CORE_FUNC(DestroyShaderModule);
    GET_DEVICE_CORE_FUNC(DestroyPipeline);
    GET_DEVICE_CORE_FUNC(FreeMemory);
    GET_DEVICE_CORE_FUNC(FreeCommandBuffers);

    GET_DEVICE_CORE_FUNC(MapMemory);
    GET_DEVICE_CORE_FUNC(FlushMappedMemoryRanges);
    GET_DEVICE_CORE_FUNC(QueueWaitIdle);
    GET_DEVICE_CORE_FUNC(QueueSubmit2);
    GET_DEVICE_CORE_FUNC(GetSemaphoreCounterValue);
    GET_DEVICE_CORE_FUNC(WaitSemaphores);
    GET_DEVICE_CORE_FUNC(ResetCommandPool);
    GET_DEVICE_CORE_FUNC(ResetDescriptorPool);
    GET_DEVICE_CORE_FUNC(AllocateCommandBuffers);
    GET_DEVICE_CORE_FUNC(AllocateDescriptorSets);
    GET_DEVICE_CORE_FUNC(UpdateDescriptorSets);
    GET_DEVICE_CORE_FUNC(BindBufferMemory2);
    GET_DEVICE_CORE_FUNC(BindImageMemory2);
    GET_DEVICE_CORE_FUNC(GetBufferMemoryRequirements2);
    GET_DEVICE_CORE_FUNC(GetImageMemoryRequirements2);
    GET_DEVICE_CORE_FUNC(ResetQueryPool);
    GET_DEVICE_CORE_FUNC(GetBufferDeviceAddress);

    GET_DEVICE_CORE_FUNC(BeginCommandBuffer);
    GET_DEVICE_CORE_FUNC(CmdSetViewportWithCount);
    GET_DEVICE_CORE_FUNC(CmdSetScissorWithCount);
    GET_DEVICE_CORE_FUNC(CmdSetDepthBounds);
    GET_DEVICE_CORE_FUNC(CmdSetStencilReference);
    GET_DEVICE_CORE_FUNC(CmdSetBlendConstants);
    GET_DEVICE_CORE_FUNC(CmdSetDepthBias);
    GET_DEVICE_CORE_FUNC(CmdClearAttachments);
    GET_DEVICE_CORE_FUNC(CmdClearColorImage);
    GET_DEVICE_CORE_FUNC(CmdBindVertexBuffers2);
    GET_DEVICE_CORE_FUNC(CmdBindIndexBuffer);
    GET_DEVICE_CORE_FUNC(CmdBindPipeline);
    GET_DEVICE_CORE_FUNC(CmdBindDescriptorSets);
    GET_DEVICE_CORE_FUNC(CmdPushConstants);
    GET_DEVICE_CORE_FUNC(CmdDispatch);
    GET_DEVICE_CORE_FUNC(CmdDispatchIndirect);
    GET_DEVICE_CORE_FUNC(CmdDraw);
    GET_DEVICE_CORE_FUNC(CmdDrawIndexed);
    GET_DEVICE_CORE_FUNC(CmdDrawIndirect);
    GET_DEVICE_CORE_FUNC(CmdDrawIndirectCount);
    GET_DEVICE_CORE_FUNC(CmdDrawIndexedIndirect);
    GET_DEVICE_CORE_FUNC(CmdDrawIndexedIndirectCount);
    GET_DEVICE_CORE_FUNC(CmdCopyBuffer2);
    GET_DEVICE_CORE_FUNC(CmdCopyImage2);
    GET_DEVICE_CORE_FUNC(CmdResolveImage2);
    GET_DEVICE_CORE_FUNC(CmdCopyBufferToImage2);
    GET_DEVICE_CORE_FUNC(CmdCopyImageToBuffer2);
    GET_DEVICE_CORE_FUNC(CmdPipelineBarrier2);
    GET_DEVICE_CORE_FUNC(CmdBeginQuery);
    GET_DEVICE_CORE_FUNC(CmdEndQuery);
    GET_DEVICE_CORE_FUNC(CmdWriteTimestamp2);
    GET_DEVICE_CORE_FUNC(CmdCopyQueryPoolResults);
    GET_DEVICE_CORE_FUNC(CmdResetQueryPool);
    GET_DEVICE_CORE_FUNC(CmdFillBuffer);
    GET_DEVICE_CORE_FUNC(CmdBeginRendering);
    GET_DEVICE_CORE_FUNC(CmdEndRendering);
    GET_DEVICE_CORE_FUNC(EndCommandBuffer);

    // IMPORTANT: { } are mandatory here!

    if (m_MinorVersion >= 3 || IsExtensionSupported(VK_KHR_MAINTENANCE_4_EXTENSION_NAME, desiredDeviceExts)) {
        GET_DEVICE_CORE_FUNC(GetDeviceBufferMemoryRequirements);
        GET_DEVICE_CORE_FUNC(GetDeviceImageMemoryRequirements);
    }

    if (IsExtensionSupported(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, desiredDeviceExts)) {
        GET_DEVICE_FUNC(CmdBindIndexBuffer2KHR);
    }

    if (IsExtensionSupported(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME, desiredDeviceExts)) {
        GET_DEVICE_FUNC(CmdPushDescriptorSetKHR);
    }

    if (IsExtensionSupported(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME, desiredDeviceExts)) {
        GET_DEVICE_FUNC(CmdSetFragmentShadingRateKHR);
    }

    if (IsExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME, desiredDeviceExts)) {
        GET_DEVICE_FUNC(AcquireNextImage2KHR);
        GET_DEVICE_FUNC(QueuePresentKHR);
        GET_DEVICE_FUNC(CreateSwapchainKHR);
        GET_DEVICE_FUNC(DestroySwapchainKHR);
        GET_DEVICE_FUNC(GetSwapchainImagesKHR);
    }

    if (IsExtensionSupported(VK_KHR_PRESENT_WAIT_EXTENSION_NAME, desiredDeviceExts)) {
        GET_DEVICE_FUNC(WaitForPresentKHR);
    }

    if (IsExtensionSupported(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, desiredDeviceExts)) {
        GET_DEVICE_FUNC(CreateAccelerationStructureKHR);
        GET_DEVICE_FUNC(DestroyAccelerationStructureKHR);
        GET_DEVICE_FUNC(GetAccelerationStructureDeviceAddressKHR);
        GET_DEVICE_FUNC(GetAccelerationStructureBuildSizesKHR);
        GET_DEVICE_FUNC(CmdBuildAccelerationStructuresKHR);
        GET_DEVICE_FUNC(CmdCopyAccelerationStructureKHR);
        GET_DEVICE_FUNC(CmdWriteAccelerationStructuresPropertiesKHR);
    }

    if (IsExtensionSupported(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, desiredDeviceExts)) {
        GET_DEVICE_FUNC(CreateRayTracingPipelinesKHR);
        GET_DEVICE_FUNC(GetRayTracingShaderGroupHandlesKHR);
        GET_DEVICE_FUNC(CmdTraceRaysKHR);
        GET_DEVICE_FUNC(CmdTraceRaysIndirect2KHR);
    }

    if (IsExtensionSupported(VK_EXT_OPACITY_MICROMAP_EXTENSION_NAME, desiredDeviceExts)) {
        GET_DEVICE_FUNC(CreateMicromapEXT);
        GET_DEVICE_FUNC(DestroyMicromapEXT);
        GET_DEVICE_FUNC(GetMicromapBuildSizesEXT);
        GET_DEVICE_FUNC(CmdBuildMicromapsEXT);
        GET_DEVICE_FUNC(CmdCopyMicromapEXT);
        GET_DEVICE_FUNC(CmdWriteMicromapsPropertiesEXT);
    }

    if (IsExtensionSupported(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME, desiredDeviceExts)) {
        GET_DEVICE_FUNC(CmdSetSampleLocationsEXT);
    }

    if (IsExtensionSupported(VK_EXT_MESH_SHADER_EXTENSION_NAME, desiredDeviceExts)) {
        GET_DEVICE_FUNC(CmdDrawMeshTasksEXT);
        GET_DEVICE_FUNC(CmdDrawMeshTasksIndirectEXT);
        GET_DEVICE_FUNC(CmdDrawMeshTasksIndirectCountEXT);
    }

    if (IsExtensionSupported(VK_NV_LOW_LATENCY_2_EXTENSION_NAME, desiredDeviceExts)) {
        GET_DEVICE_FUNC(GetLatencyTimingsNV);
        GET_DEVICE_FUNC(LatencySleepNV);
        GET_DEVICE_FUNC(SetLatencyMarkerNV);
        GET_DEVICE_FUNC(SetLatencySleepModeNV);
    }

    return Result::SUCCESS;
}

#undef MERGE_TOKENS2
#undef MERGE_TOKENS3
#undef GET_DEVICE_OPTIONAL_CORE_FUNC
#undef GET_DEVICE_CORE_FUNC
#undef GET_DEVICE_FUNC
#undef GET_INSTANCE_FUNC

void DeviceVK::Destruct() {
    Destroy(GetAllocationCallbacks(), this);
}

NRI_INLINE void DeviceVK::SetDebugName(const char* name) {
    SetDebugNameToTrivialObject(VK_OBJECT_TYPE_DEVICE, (uint64_t)m_Device, name);
}

NRI_INLINE Result DeviceVK::GetQueue(QueueType queueType, uint32_t queueIndex, Queue*& queue) {
    const auto& queueFamily = m_QueueFamilies[(uint32_t)queueType];
    if (queueFamily.empty())
        return Result::UNSUPPORTED;

    if (queueIndex < queueFamily.size()) {
        QueueVK* queueVK = m_QueueFamilies[(uint32_t)queueType].at(queueIndex);
        queue = (Queue*)queueVK;

        { // Update active family indices
            ExclusiveScope lock(m_Lock);

            uint32_t i = 0;
            for (; i < m_NumActiveFamilyIndices; i++) {
                if (m_ActiveQueueFamilyIndices[i] == queueVK->GetFamilyIndex())
                    break;
            }

            if (i == m_NumActiveFamilyIndices)
                m_ActiveQueueFamilyIndices[m_NumActiveFamilyIndices++] = queueVK->GetFamilyIndex();
        }

        return Result::SUCCESS;
    }

    return Result::FAILURE;
}

NRI_INLINE Result DeviceVK::WaitIdle() {
    // Don't use "vkDeviceWaitIdle" because it requires host access synchronization to all queues, better do it one by one instead
    for (auto& queueFamily : m_QueueFamilies) {
        for (auto queue : queueFamily) {
            Result result = queue->WaitIdle();
            if (result != Result::SUCCESS)
                return result;
        }
    }

    return Result::SUCCESS;
}

NRI_INLINE Result DeviceVK::BindBufferMemory(const BufferMemoryBindingDesc* memoryBindingDescs, uint32_t memoryBindingDescNum) {
    if (!memoryBindingDescNum)
        return Result::SUCCESS;

    Scratch<VkBindBufferMemoryInfo> infos = AllocateScratch(*this, VkBindBufferMemoryInfo, memoryBindingDescNum);

    for (uint32_t i = 0; i < memoryBindingDescNum; i++) {
        const BufferMemoryBindingDesc& memoryBindingDesc = memoryBindingDescs[i];

        BufferVK& bufferImpl = *(BufferVK*)memoryBindingDesc.buffer;
        MemoryVK& memoryImpl = *(MemoryVK*)memoryBindingDesc.memory;

        MemoryTypeInfo memoryTypeInfo = Unpack(memoryImpl.GetType());
        if (memoryTypeInfo.mustBeDedicated)
            memoryImpl.CreateDedicated(bufferImpl);

        VkBindBufferMemoryInfo& info = infos[i];
        info = {VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO};
        info.buffer = bufferImpl.GetHandle();
        info.memory = memoryImpl.GetHandle();
        info.memoryOffset = memoryBindingDesc.offset;
    }

    VkResult vkResult = m_VK.BindBufferMemory2(m_Device, memoryBindingDescNum, infos);
    RETURN_ON_BAD_VKRESULT(this, vkResult, "vkBindBufferMemory2");

    for (uint32_t i = 0; i < memoryBindingDescNum; i++) {
        const BufferMemoryBindingDesc& memoryBindingDesc = memoryBindingDescs[i];

        BufferVK& bufferImpl = *(BufferVK*)memoryBindingDesc.buffer;
        MemoryVK& memoryImpl = *(MemoryVK*)memoryBindingDesc.memory;

        bufferImpl.FinishMemoryBinding(memoryImpl, memoryBindingDesc.offset);
    }

    return Result::SUCCESS;
}

NRI_INLINE Result DeviceVK::BindTextureMemory(const TextureMemoryBindingDesc* memoryBindingDescs, uint32_t memoryBindingDescNum) {
    if (!memoryBindingDescNum)
        return Result::SUCCESS;

    Scratch<VkBindImageMemoryInfo> infos = AllocateScratch(*this, VkBindImageMemoryInfo, memoryBindingDescNum);

    for (uint32_t i = 0; i < memoryBindingDescNum; i++) {
        const TextureMemoryBindingDesc& memoryBindingDesc = memoryBindingDescs[i];

        MemoryVK& memoryImpl = *(MemoryVK*)memoryBindingDesc.memory;
        TextureVK& textureImpl = *(TextureVK*)memoryBindingDesc.texture;

        MemoryTypeInfo memoryTypeInfo = Unpack(memoryImpl.GetType());
        if (memoryTypeInfo.mustBeDedicated)
            memoryImpl.CreateDedicated(textureImpl);

        VkBindImageMemoryInfo& info = infos[i];
        info = {VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO};
        info.image = textureImpl.GetHandle();
        info.memory = memoryImpl.GetHandle();
        info.memoryOffset = memoryBindingDesc.offset;
    }

    VkResult vkResult = m_VK.BindImageMemory2(m_Device, memoryBindingDescNum, infos);
    RETURN_ON_BAD_VKRESULT(this, vkResult, "vkBindImageMemory2");

    return Result::SUCCESS;
}

NRI_INLINE Result DeviceVK::BindAccelerationStructureMemory(const AccelerationStructureMemoryBindingDesc* memoryBindingDescs, uint32_t memoryBindingDescNum) {
    if (!memoryBindingDescNum)
        return Result::SUCCESS;

    Scratch<BufferMemoryBindingDesc> bufferMemoryBindingDescs = AllocateScratch(*this, BufferMemoryBindingDesc, memoryBindingDescNum);

    for (uint32_t i = 0; i < memoryBindingDescNum; i++) {
        const AccelerationStructureMemoryBindingDesc& memoryBindingDesc = memoryBindingDescs[i];
        AccelerationStructureVK& accelerationStructure = *(AccelerationStructureVK*)memoryBindingDesc.accelerationStructure;

        BufferMemoryBindingDesc& bufferMemoryBinding = bufferMemoryBindingDescs[i];
        bufferMemoryBinding = {};
        bufferMemoryBinding.buffer = (Buffer*)accelerationStructure.GetBuffer();
        bufferMemoryBinding.memory = memoryBindingDesc.memory;
        bufferMemoryBinding.offset = memoryBindingDesc.offset;
    }

    Result result = BindBufferMemory(bufferMemoryBindingDescs, memoryBindingDescNum);

    for (uint32_t i = 0; i < memoryBindingDescNum && result == Result::SUCCESS; i++) {
        AccelerationStructureVK& accelerationStructure = *(AccelerationStructureVK*)memoryBindingDescs[i].accelerationStructure;
        result = accelerationStructure.FinishCreation();
    }

    return result;
}

NRI_INLINE Result DeviceVK::BindMicromapMemory(const MicromapMemoryBindingDesc* memoryBindingDescs, uint32_t memoryBindingDescNum) {
    if (!memoryBindingDescNum)
        return Result::SUCCESS;

    Scratch<BufferMemoryBindingDesc> bufferMemoryBindingDescs = AllocateScratch(*this, BufferMemoryBindingDesc, memoryBindingDescNum);

    for (uint32_t i = 0; i < memoryBindingDescNum; i++) {
        const MicromapMemoryBindingDesc& memoryBindingDesc = memoryBindingDescs[i];
        MicromapVK& micromap = *(MicromapVK*)memoryBindingDesc.micromap;

        BufferMemoryBindingDesc& bufferMemoryBinding = bufferMemoryBindingDescs[i];
        bufferMemoryBinding = {};
        bufferMemoryBinding.buffer = (Buffer*)micromap.GetBuffer();
        bufferMemoryBinding.memory = memoryBindingDesc.memory;
        bufferMemoryBinding.offset = memoryBindingDesc.offset;
    }

    Result result = BindBufferMemory(bufferMemoryBindingDescs, memoryBindingDescNum);

    for (uint32_t i = 0; i < memoryBindingDescNum && result == Result::SUCCESS; i++) {
        MicromapVK& micromap = *(MicromapVK*)memoryBindingDescs[i].micromap;
        result = micromap.FinishCreation();
    }

    return result;
}

#define UPDATE_TEXTURE_SUPPORT_BITS(required, bit) \
    if ((props3.optimalTilingFeatures & (required)) == (required)) \
        supportBits |= bit;

#define UPDATE_BUFFER_SUPPORT_BITS(required, bit) \
    if ((props3.bufferFeatures & (required)) == (required)) \
        supportBits |= bit;

NRI_INLINE FormatSupportBits DeviceVK::GetFormatSupport(Format format) const {
    FormatSupportBits supportBits = FormatSupportBits::UNSUPPORTED;
    VkFormat vkFormat = GetVkFormat(format);

    VkFormatProperties3 props3 = {VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3};
    VkFormatProperties2 props2 = {VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2, &props3};
    m_VK.GetPhysicalDeviceFormatProperties2(m_PhysicalDevice, vkFormat, &props2);

    UPDATE_TEXTURE_SUPPORT_BITS(VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT, FormatSupportBits::TEXTURE);
    UPDATE_TEXTURE_SUPPORT_BITS(VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT, FormatSupportBits::STORAGE_TEXTURE);
    UPDATE_TEXTURE_SUPPORT_BITS(VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT, FormatSupportBits::COLOR_ATTACHMENT);
    UPDATE_TEXTURE_SUPPORT_BITS(VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT, FormatSupportBits::DEPTH_STENCIL_ATTACHMENT);
    UPDATE_TEXTURE_SUPPORT_BITS(VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT, FormatSupportBits::BLEND);
    UPDATE_TEXTURE_SUPPORT_BITS(VK_FORMAT_FEATURE_2_STORAGE_IMAGE_ATOMIC_BIT, FormatSupportBits::STORAGE_TEXTURE_ATOMICS);

    UPDATE_BUFFER_SUPPORT_BITS(VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT, FormatSupportBits::BUFFER);
    UPDATE_BUFFER_SUPPORT_BITS(VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT, FormatSupportBits::STORAGE_BUFFER);
    UPDATE_BUFFER_SUPPORT_BITS(VK_FORMAT_FEATURE_2_VERTEX_BUFFER_BIT, FormatSupportBits::VERTEX_BUFFER);
    UPDATE_BUFFER_SUPPORT_BITS(VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_ATOMIC_BIT, FormatSupportBits::STORAGE_BUFFER_ATOMICS);

    if ((props3.optimalTilingFeatures | props3.bufferFeatures) & VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT)
        supportBits |= FormatSupportBits::STORAGE_LOAD_WITHOUT_FORMAT;

    VkPhysicalDeviceImageFormatInfo2 imageInfo = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2};
    imageInfo.format = vkFormat;
    imageInfo.type = VK_IMAGE_TYPE_2D;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.flags = 0; // TODO: kinda needed, but unknown here

    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (supportBits & FormatSupportBits::TEXTURE)
        imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (supportBits & FormatSupportBits::DEPTH_STENCIL_ATTACHMENT)
        imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (supportBits & FormatSupportBits::COLOR_ATTACHMENT)
        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageFormatProperties2 imageProps = {VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2};
    m_VK.GetPhysicalDeviceImageFormatProperties2(m_PhysicalDevice, &imageInfo, &imageProps);

    if (imageProps.imageFormatProperties.sampleCounts & VK_SAMPLE_COUNT_2_BIT)
        supportBits |= FormatSupportBits::MULTISAMPLE_2X;
    if (imageProps.imageFormatProperties.sampleCounts & VK_SAMPLE_COUNT_4_BIT)
        supportBits |= FormatSupportBits::MULTISAMPLE_4X;
    if (imageProps.imageFormatProperties.sampleCounts & VK_SAMPLE_COUNT_8_BIT)
        supportBits |= FormatSupportBits::MULTISAMPLE_8X;

    return supportBits;
}

#undef UPDATE_TEXTURE_SUPPORT_BITS
#undef UPDATE_BUFFER_SUPPORT_BITS

NRI_INLINE Result DeviceVK::QueryVideoMemoryInfo(MemoryLocation memoryLocation, VideoMemoryInfo& videoMemoryInfo) const {
    videoMemoryInfo = {};

    if (!m_IsSupported.memoryBudget)
        return Result::UNSUPPORTED;

    VkPhysicalDeviceMemoryBudgetPropertiesEXT budgetProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT};

    VkPhysicalDeviceMemoryProperties2 memoryProps = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
    memoryProps.pNext = &budgetProps;

    const auto& vk = GetDispatchTable();
    vk.GetPhysicalDeviceMemoryProperties2(m_PhysicalDevice, &memoryProps);

    bool isLocal = memoryLocation == MemoryLocation::DEVICE || memoryLocation == MemoryLocation::DEVICE_UPLOAD;

    for (uint32_t i = 0; i < GetCountOf(budgetProps.heapBudget); i++) {
        VkDeviceSize size = budgetProps.heapBudget[i];
        bool state = m_MemoryProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;

        if (size && state == isLocal)
            videoMemoryInfo.budgetSize += size;
    }

    for (uint32_t i = 0; i < GetCountOf(budgetProps.heapUsage); i++) {
        VkDeviceSize size = budgetProps.heapUsage[i];
        bool state = m_MemoryProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;

        if (size && state == isLocal)
            videoMemoryInfo.usageSize += size;
    }

    return Result::SUCCESS;
}
