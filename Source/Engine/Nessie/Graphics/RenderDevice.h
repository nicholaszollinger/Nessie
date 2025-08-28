// DeviceBase.h
#pragma once
#include "DeviceAsset.h"
#include "RendererDesc.h"
#include "Nessie/Application/ApplicationDesc.h"
#include "Nessie/Core/Thread/Mutex.h"
#include "Vulkan/VulkanConversions.h"

static_assert(VK_HEADER_VERSION >= 304,
              "RenderDevice.h requires at least Vulkan SDK 1.4.304. "
              "Please install a newer Vulkan SDK. A compatible link is available in the README.");

namespace nes
{
    template <DeviceAssetType Type>
    class DeviceAssetPtr;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : A RenderDevice is the intermediary between the program and the hardware device (GPU).
    //----------------------------------------------------------------------------------------------------
    class RenderDevice
    {
    public:
        /* Constructor */           RenderDevice() = default;
        /* No Copy Constructor */   RenderDevice(const RenderDevice&) = delete;
        /* No Move Constructor */   RenderDevice(RenderDevice&&) noexcept = delete;
        /* No Copy Assignment */    RenderDevice& operator=(const RenderDevice&) = delete;
        /* No Move Assignment  */   RenderDevice& operator=(RenderDevice&&) noexcept = delete;
        /* Destructor */            ~RenderDevice();

        /// Operators for converting to Vulkan Types.
        operator                    VkInstance() const                      { return *m_vkInstance; }
        operator                    VkPhysicalDevice() const                { return *m_vkPhysicalDevice; }
        operator                    VkDevice() const                        { return *m_vkDevice; }
        operator                    VmaAllocator() const                    { return m_vmaAllocator; }
        operator                    const vk::raii::Instance&() const       { return m_vkInstance; } 
        operator                    const vk::raii::Device&() const         { return m_vkDevice; }
        operator                    const vk::raii::PhysicalDevice&() const { return m_vkPhysicalDevice; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the Device. 
        //----------------------------------------------------------------------------------------------------
        bool                        Init(const ApplicationDesc& appDesc, const RendererDesc& rendererDesc);
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroy this device.
        //----------------------------------------------------------------------------------------------------
        void                        Destroy();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Wait until all Device Queues are idle.
        //----------------------------------------------------------------------------------------------------
        void                        WaitUntilIdle();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get info about this device. 
        //----------------------------------------------------------------------------------------------------
        const DeviceDesc&           GetDesc() const                         { return m_desc; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a new graphics class with the given InitArgs. The resulting pointer will be
        ///     stored in pOutObject.
        ///	@tparam Type : Graphics class. Ex: DeviceQueue.
        ///	@tparam InitArgs : Parameter types to send to the Init() function on the Graphics class.
        ///	@param pOutObject : Pointer that will be set to the address of the created Graphics class.
        ///	@param args : Parameters to send to the Init() function on the Graphics class.
        ///	@returns : Result of the Graphics class's Init() function. If not EGraphicsResult::Success, the
        ///     object will be destroyed.
        //----------------------------------------------------------------------------------------------------
        template <DeviceAssetType Type, typename...InitArgs> requires requires(Type type, InitArgs&&... args)
        {
            // Requires an Init function that takes in the give InitArgs, and returns an EGraphicsResult.
            { type.Init(std::forward<InitArgs>(args)...) } -> std::same_as<EGraphicsResult>;
        }
        EGraphicsResult             CreateResource(Type*& pOutObject, InitArgs&&...args);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free a graphics resource type.
        //----------------------------------------------------------------------------------------------------
        template <DeviceAssetType Type>
        void                        FreeResource(Type*& pObject);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a Queue of a particular type.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             GetQueue(const EQueueType type, const uint32 queueIndex, DeviceQueue*& outQueue);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the allocation callbacks set for this device.
        //----------------------------------------------------------------------------------------------------
        const AllocationCallbacks&  GetAllocationCallbacks() const          { return m_allocationCallbacks; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the vulkan allocation callbacks object, to be used with vulkan API calls.   
        //----------------------------------------------------------------------------------------------------
        const vk::AllocationCallbacks& GetVkAllocationCallbacks() const      { return m_vkAllocationCallbacks; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the pointer for the vulkan allocation callbacks object, to be used with vulkan API calls.   
        //----------------------------------------------------------------------------------------------------
        const VkAllocationCallbacks* GetVkAllocationCallbacksPtr() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan Device. 
        //----------------------------------------------------------------------------------------------------
        const vk::raii::Device&     GetVkDevice() const                         { return m_vkDevice; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan Instance. 
        //----------------------------------------------------------------------------------------------------
        const vk::raii::Instance&   GetVkInstance() const                       { return m_vkInstance; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan Physical Device.  
        //----------------------------------------------------------------------------------------------------
        const vk::raii::PhysicalDevice& GetVkPhysicalDevice() const             { return m_vkPhysicalDevice; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan Pipeline Cache.
        //----------------------------------------------------------------------------------------------------
        const vk::raii::PipelineCache& GetVkPipelineCache() const               { return m_vkPipelineCache; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the supported Device Extensions for the physical device.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             GetSupportedDeviceExtensions(std::vector<vk::ExtensionProperties>& outSupportedExtensions) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Test if the memory type at the given index is coherent memory. It does not need 
        ///     Flush/InvalidateMappedMemoryRanges() commands to manage the availability and visibility on
        ///     the host.
        /// @see : https://docs.vulkan.org/spec/latest/chapters/memory.html "VkMemoryType::propertyFlags"
        //----------------------------------------------------------------------------------------------------
        bool                        IsHostCoherentMemory(DeviceMemoryTypeIndex memoryTypeIndex) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Fill out a vulkan buffer CreateInfo object, based on the given buffer description.
        //----------------------------------------------------------------------------------------------------
        void                        FillCreateInfo(const BufferDesc& bufferDesc, vk::BufferCreateInfo& outInfo) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Fill out a vulkan image CreateInfo object, based on the given texture description.
        //----------------------------------------------------------------------------------------------------
        void                        FillCreateInfo(const ImageDesc& textureDesc, vk::ImageCreateInfo& outInfo) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Use NES_VK_CHECK() rather than calling this directly.
        ///     If the result is a failure, this will report the message, assert, and exit. Treats errors as
        ///     fatal errors. 
        //----------------------------------------------------------------------------------------------------
        bool                        CheckResult(const vk::Result result, const char* resultStr, const char* file, int line) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Use NES_GRAPHICS_ASSERT() rather than calling this directly.
        ///     If the result is false, this will report the error and assert.
        //----------------------------------------------------------------------------------------------------
        bool                        CheckResult(const bool result, const char* resultStr, const char* file, int line) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Use NES_GRAPHICS_MUST_PASS() rather than calling this directly.
        ///     If the result is false, this will report the error and assert.
        //----------------------------------------------------------------------------------------------------
        bool                        CheckResult(const EGraphicsResult result, const char* resultStr, const char* file, int line) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Use NES_VK_FAIL_REPORT() rather than calling this directly.
        ///     If the result is a failure, this will report the message, and return the result.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             ReportOnError(const vk::Result result, const char* resultStr, const char* file, int line) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Post a message using the set Debug Messenger callback.
        //----------------------------------------------------------------------------------------------------
        void                        ReportMessage(const ELogLevel level, const char* file, const uint32 line, const char* message, const nes::LogTag& tag = kGraphicsLogTag) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for a Vulkan object. 
        //----------------------------------------------------------------------------------------------------
        template <VulkanObjectType Type>
        void                        SetDebugNameToTrivialObject(Type& object, const std::string& name);
    
    private:
        static constexpr uint32     kInvalidQueueIndex = std::numeric_limits<uint16>::max();
        static constexpr size_t     kGraphicsIndex = static_cast<size_t>(EQueueType::Graphics);
        static constexpr size_t     kComputeIndex = static_cast<size_t>(EQueueType::Compute);
        static constexpr size_t     kTransferIndex = static_cast<size_t>(EQueueType::Transfer);
        
        using QueueArray            = std::vector<DeviceQueue*>;
        using QueueFamilyArray      = std::array<QueueArray, static_cast<size_t>(EQueueType::MaxNum)>;
        using QueueIndicesArray     = std::array<uint32, static_cast<size_t>(EQueueType::MaxNum)>; 
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the Vulkan Instance. Returns EGraphicsResult::Unsupported on failure.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             CreateInstance(const ApplicationDesc& appDesc, const RendererDesc& rendererDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get required extensions for Nessie.
        //----------------------------------------------------------------------------------------------------
        std::vector<const char*>    GetRequiredInstanceExtensions(const RendererDesc& rendererDesc) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates the debug messenger, if necessary.
        //----------------------------------------------------------------------------------------------------
        void                        SetupDebugMessenger();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Select a physical device to use for rendering. Returns EGraphicsResult::Failure on failure.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             SelectPhysicalDevice(const RendererDesc& rendererDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns if the physical device is suitable based on its properties and features.
        //----------------------------------------------------------------------------------------------------
        bool                        QueryQueueFamilySupport(const RendererDesc& rendererDesc, const std::vector<vk::QueueFamilyProperties>& familyProps, QueueIndicesArray& outIndices);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the Logical Device. Returns EGraphicsResult::Failure on failure.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             CreateLogicalDevice(const RendererDesc& rendererDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the VMA allocator.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             CreateAllocator(const RendererDesc& rendererDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Filters out unavailable extensions from the desired extensions based on the supported extensions.
        ///     If any unavailable extension was required, this will return false.
        //----------------------------------------------------------------------------------------------------
        bool                        FilterAvailableExtensions(const std::vector<vk::ExtensionProperties>& supportedExtensions, const std::vector<ExtensionDesc>& desiredExtensions, std::vector<ExtensionDesc>& outFilteredExtensions) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Adds a default of device extensions if they are available to the outDesiredExtensions array.
        //----------------------------------------------------------------------------------------------------
        void                        EnableDefaultDeviceExtensions(const std::unordered_map<std::string, uint32_t>& availableExtensionsMap, std::vector<ExtensionDesc>& outDesiredExtensions) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Fill out the device description info. 
        //----------------------------------------------------------------------------------------------------
        void                        FillOutDeviceDesc();

    private:
        AllocationCallbacks         m_allocationCallbacks{};
        DebugMessenger              m_debugMessenger{};
        vk::raii::Context           m_context{};
        vk::raii::Instance          m_vkInstance = nullptr;
        vk::raii::PhysicalDevice    m_vkPhysicalDevice = nullptr;
        vk::raii::Device            m_vkDevice = nullptr;
        vk::raii::DebugUtilsMessengerEXT m_vkDebugMessenger = nullptr;
        vk::raii::PipelineCache     m_vkPipelineCache = nullptr;
        vk::AllocationCallbacks     m_vkAllocationCallbacks = nullptr;
        QueueFamilyArray            m_queueFamilies{};      /// Contains an array of Queues for each EQueueType. 
        DeviceDesc                  m_desc{};
        vk::PhysicalDeviceMemoryProperties m_memoryProperties{};
        QueueIndicesArray           m_activeQueueIndices{};
        Mutex                       m_activeQueueIndicesMutex{};
        uint32                      m_numActiveFamilyIndices = 0;
        VmaAllocator                m_vmaAllocator = nullptr;
    };

    template <DeviceAssetType Type, typename ... InitArgs> requires requires (Type type, InitArgs&&... args)
    {
        { type.Init(std::forward<InitArgs>(args)...) } -> std::same_as<EGraphicsResult>;
    }
    EGraphicsResult RenderDevice::CreateResource(Type*& pOutObject, InitArgs&&... args)
    {
        // First allocate the object, using passing the RenderDevice into the constructor.
        Type* pImpl = Allocate<Type>(GetAllocationCallbacks(), *this);

        // Then call the create function on the object itself.
        EGraphicsResult result = pImpl->Init(std::forward<InitArgs>(args)...);
        if (result != EGraphicsResult::Success)
        {
            Free<Type>(GetAllocationCallbacks(), pImpl);
            pOutObject = nullptr;
        }
        else
        {
            pOutObject = pImpl;
        }
    
        return result;
    }

    template <DeviceAssetType Type>
    void RenderDevice::FreeResource(Type*& pObject)
    {
        if (pObject != nullptr)
        {
            Free<Type>(GetAllocationCallbacks(), pObject);
            pObject = nullptr;
        }
    }

    template <VulkanObjectType Type>
    void RenderDevice::SetDebugNameToTrivialObject([[maybe_unused]] Type& object, [[maybe_unused]] const std::string& name)
    {
#if NES_DEBUG
        if (m_vkDevice != nullptr)
        {
            constexpr vk::ObjectType kType = GetVkObjectType<Type>();

            uint64 handle;
            // Case for C++ vulkan types:
            if constexpr (requires(){typename Type::NativeType;})
            {
                using NativeType = typename Type::NativeType;
                NativeType type = object;
                handle = reinterpret_cast<uint64>(type);
            }
            // C version vulkan types.
            else
            {
                handle = reinterpret_cast<uint64>(object);
            }
            
            vk::DebugUtilsObjectNameInfoEXT nameInfo = vk::DebugUtilsObjectNameInfoEXT()
                .setObjectHandle(handle)
                .setObjectType(kType)
                .setPObjectName(name.c_str());
            
            m_vkDevice.setDebugUtilsObjectNameEXT(nameInfo);
        }
#endif
    }
}
