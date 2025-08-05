// DeviceBase.h
#pragma once
#include "GraphicsResource.h"
#include "RendererDesc.h"
#include "Nessie/Application/ApplicationDesc.h"
#include "Nessie/Core/Thread/Mutex.h"
#include "Vulkan/VulkanConversions.h"

static_assert(VK_HEADER_VERSION >= 304,
              "RenderDevice.h requires at least Vulkan SDK 1.4.304. "
              "Please install a newer Vulkan SDK. A compatible link is available in the README.");

namespace nes
{
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
        operator                    VkInstance() const          { return m_vkInstance; }
        operator                    VkPhysicalDevice() const    { return m_vkPhysicalDevice; }
        operator                    VkDevice() const            { return m_vkDevice; }
        operator                    VmaAllocator() const        { return m_vmaAllocator; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the Device. 
        //----------------------------------------------------------------------------------------------------
        bool                        Init(const ApplicationDesc& appDesc, const RendererDesc& rendererDesc);
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroy this device.
        //----------------------------------------------------------------------------------------------------
        void                        Destroy();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Wait until all Device Queues are finished.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             WaitUntilIdle();

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
        template <GraphicsResourceType Type, typename...InitArgs> requires requires(Type type, InitArgs&&... args)
        {
            // Requires an Init function that takes in the give InitArgs, and returns an EGraphicsResult.
            { type.Init(std::forward<InitArgs>(args)...) } -> std::same_as<EGraphicsResult>;
        }
        EGraphicsResult             CreateResource(Type*& pOutObject, InitArgs&&...args);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free a graphics resource type.
        //----------------------------------------------------------------------------------------------------
        template <GraphicsResourceType Type>
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
        /// @brief : Get the pointer for the vulkan allocation callbacks object, to be used with vulkan API calls.   
        //----------------------------------------------------------------------------------------------------
        VkAllocationCallbacks*      GetVkAllocationCallbacks() const        { return m_vkAllocationCallbacksPtr; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the supported Device Extensions for the physical device.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             GetSupportedDeviceExtensions(std::vector<VkExtensionProperties>& outSupportedExtensions) const;

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
        void                        FillCreateInfo(const BufferDesc& bufferDesc, VkBufferCreateInfo& outInfo) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Fill out a vulkan image CreateInfo object, based on the given texture description.
        //----------------------------------------------------------------------------------------------------
        void                        FillCreateInfo(const TextureDesc& textureDesc, VkImageCreateInfo& outInfo) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Use NES_VK_CHECK() rather than calling this directly.
        ///     If the result is a failure, this will report the message, assert, and exit. Treats errors as
        ///     fatal errors. 
        //----------------------------------------------------------------------------------------------------
        bool                        CheckResult(const VkResult result, const char* resultStr, const char* file, int line) const;

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
        EGraphicsResult             ReportOnError(const VkResult result, const char* resultStr, const char* file, int line) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Post a message using the set Debug Messenger callback.
        //----------------------------------------------------------------------------------------------------
        void                        ReportMessage(const ELogLevel level, const char* file, const uint32 line, const char* message, const nes::LogTag& tag = kGraphicsLogTag) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for a Vulkan object. 
        //----------------------------------------------------------------------------------------------------
        template <VulkanObjectType Type>
        void                        SetDebugNameToTrivialObject(Type object, const std::string& name);
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the Vulkan Instance. Returns EGraphicsResult::Unsupported on failure.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             CreateInstance(const ApplicationDesc& appDesc, const RendererDesc& rendererDesc);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Select a physical device to use for rendering. Returns EGraphicsResult::Failure on failure.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             SelectPhysicalDevice(const RendererDesc& rendererDesc);
        
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
        bool                        FilterAvailableExtensions(const std::vector<VkExtensionProperties>& supportedExtensions, const std::vector<ExtensionDesc>& desiredExtensions, std::vector<ExtensionDesc>& outFilteredExtensions) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Adds a default of device extensions if they are available to the outDesiredExtensions array.
        //----------------------------------------------------------------------------------------------------
        void                        EnableDefaultDeviceExtensions(const std::unordered_map<std::string, uint32_t>& availableExtensionsMap, std::vector<ExtensionDesc>& outDesiredExtensions) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Fill out the device description info. 
        //----------------------------------------------------------------------------------------------------
        void                        FillOutDeviceDesc();

    private:
        static constexpr uint32     kInvalidQueueIndex = std::numeric_limits<uint16>::max();
        
        using QueueArray            = std::vector<DeviceQueue*>;
        using QueueFamilyArray      = std::array<QueueArray, static_cast<size_t>(EQueueType::MaxNum)>;
        using ActiveQueueIndicesArray = std::array<uint32, static_cast<size_t>(EQueueType::MaxNum)>; 

        AllocationCallbacks         m_allocationCallbacks{};
        DebugMessenger              m_debugMessenger{};
        QueueFamilyArray            m_queueFamilies{};      /// Contains an array of Queues for each EQueueType. 
        DeviceDesc                  m_desc{};
        VkInstance                  m_vkInstance{};
        VkPhysicalDevice            m_vkPhysicalDevice{};
        VkDevice                    m_vkDevice{};
        VkAllocationCallbacks       m_vkAllocationCallbacks{};
        VkAllocationCallbacks*      m_vkAllocationCallbacksPtr = nullptr;
        VkDebugUtilsMessengerEXT    m_vkDebugMessenger{};
        VkPhysicalDeviceMemoryProperties m_memoryProperties{};
        ActiveQueueIndicesArray     m_activeQueueIndices{};
        Mutex                       m_activeQueueIndicesMutex{};
        uint32                      m_numActiveFamilyIndices = 0;
        VmaAllocator                m_vmaAllocator;
    };

    template <GraphicsResourceType Type, typename ... InitArgs> requires requires (Type type, InitArgs&&... args)
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

    template <GraphicsResourceType Type>
    void RenderDevice::FreeResource(Type*& pObject)
    {
        if (pObject != nullptr)
        {
            Free<Type>(GetAllocationCallbacks(), pObject);
            pObject = nullptr;
        }
    }

    template <VulkanObjectType Type>
    void RenderDevice::SetDebugNameToTrivialObject(Type object, const std::string& name)
    {
        if (vkSetDebugUtilsObjectNameEXT != nullptr && m_vkDevice != nullptr)
        {
            constexpr VkObjectType kType = GetVkObjectType<Type>();
            
            VkDebugUtilsObjectNameInfoEXT nameInfo
            {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = kType,
                .objectHandle = reinterpret_cast<uint64>(object),
                .pObjectName = name.c_str(),
            };

            vkSetDebugUtilsObjectNameEXT(m_vkDevice, &nameInfo);
        }
    }
}
