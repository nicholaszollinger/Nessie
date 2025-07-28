// DeviceBase.h
#pragma once
#include "GraphicsCommon.h"
#include "RendererDesc.h"
#include "Nessie/Application/ApplicationDesc.h"
#include "Nessie/Core/Thread/Mutex.h"
#include "Vulkan/VulkanCore.h"

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
        RenderDevice() = default;
        RenderDevice(const RenderDevice&) = delete;
        RenderDevice(RenderDevice&&) noexcept = delete;
        RenderDevice& operator=(const RenderDevice&) = delete;
        RenderDevice& operator=(RenderDevice&&) noexcept = delete;
        ~RenderDevice();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the Device. 
        //----------------------------------------------------------------------------------------------------
        bool                        Init(const ApplicationDesc& appDesc, ApplicationWindow* pWindow, const RendererDesc& rendererDesc);
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroy this device.
        //----------------------------------------------------------------------------------------------------
        void                        Destroy();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a new graphics class with the given CreateArgs. The resulting pointer will be
        ///     stored in pOutEntity.
        ///	@tparam GraphicsType : Graphics class. Ex: DeviceQueue.
        ///	@tparam InitArgs : Parameter types to send to the Init() function on the Graphics class.
        ///	@param pOutObject : Pointer that will be set to the address of the created Graphics class.
        ///	@param args : Parameters to send to the Init() function on the Graphics class.
        ///	@returns : Result of the Graphics class's Init() function. If not EGraphicsResult::Success, the
        ///     object will be destroyed.
        //----------------------------------------------------------------------------------------------------
        template <typename GraphicsType, typename...InitArgs> requires requires(RenderDevice& device, GraphicsType type, InitArgs&&... args)
        {
            GraphicsType(device);                                                             // Requires a constructor that takes in a VulkanDevice&
            { type.Init(std::forward<InitArgs>(args)...) } -> std::same_as<EGraphicsResult>;  // Requires an Init function that takes in the give CreateArgs, and returns an EGraphicsResult.
        }
        EGraphicsResult             Create(GraphicsType*& pOutObject, InitArgs&&...args);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a Queue of a particular type.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             GetQueue(const EQueueType type, const uint32 queueIndex, DeviceQueue*& outQueue);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the allocation callbacks set for this device.
        //----------------------------------------------------------------------------------------------------
        const AllocationCallbacks&  GetAllocationCallbacks() const { return m_allocationCallbacks; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the supported Device Extensions for the physical device.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             GetSupportedDeviceExtensions(std::vector<VkExtensionProperties>& outExtensions) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get info about this device. 
        //----------------------------------------------------------------------------------------------------
        const DeviceDesc&           GetDesc() const { return m_deviceDesc; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Use NES_VK_CHECK() rather than calling this directly.
        ///     If the result is a failure, this will report the message, assert, and exit. Treats errors as
        ///     fatal errors. 
        //----------------------------------------------------------------------------------------------------
        void                        CheckResult(const VkResult result, const char* resultStr, const char* file, int line) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Use NES_VK_FAIL_REPORT() rather than calling this directly.
        ///     If the result is a failure, this will report the message, and return the result.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult             ReportOnError(const VkResult result, const char* resultStr, const char* file, int line) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Post a message using the set Debug Messenger callback.
        //----------------------------------------------------------------------------------------------------
        void                        ReportMessage(const ELogLevel level, const char* file, const uint32 line, const char* message, const nes::LogTag& tag = kGraphicsLogTag) const;
        
    
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
        /// @brief : Filters out unavailable extensions from the desired extensions based on the supported extensions.
        ///     If any unavailable extension was required, this will return false.
        //----------------------------------------------------------------------------------------------------
        bool                        FilterAvailableExtensions(const std::vector<VkExtensionProperties>& supportedExtensions, const std::vector<ExtensionDesc>& desiredExtensions, std::vector<ExtensionDesc>& outFilteredExtensions) const;

    private:
        static constexpr uint32     kInvalidQueueIndex = std::numeric_limits<uint16>::max();
        
        using QueueArray            = std::vector<DeviceQueue*>;
        using QueueFamilyArray      = std::array<QueueArray, static_cast<size_t>(EQueueType::MaxNum)>;
        using ActiveQueueIndicesArray = std::array<uint32, static_cast<size_t>(EQueueType::MaxNum)>; 

        AllocationCallbacks         m_allocationCallbacks{};
        DebugMessenger              m_debugMessenger{};
        QueueFamilyArray            m_queueFamilies{};      /// Contains an array of Queues for each EQueueType. 
        DeviceDesc                  m_deviceDesc{};
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
    };

    template <typename GraphicsType, typename ... CreateArgs> requires requires (RenderDevice& device, GraphicsType type, CreateArgs&&...args)
    {
        GraphicsType(device);
        { type.Init(std::forward<CreateArgs>(args)...) } -> std::same_as<EGraphicsResult>;
    }
    EGraphicsResult RenderDevice::Create(GraphicsType*& pOutObject, CreateArgs&&... args)
    {
        // First allocate the object, using passing the RenderDevice into the constructor.
        GraphicsType* pImpl = Allocate<GraphicsType>(GetAllocationCallbacks(), *this);

        // Then call the create function on the object itself.
        EGraphicsResult result = pImpl->Init(std::forward<CreateArgs>(args)...);
        if (result != EGraphicsResult::Success)
        {
            Free<GraphicsType>(GetAllocationCallbacks(), pImpl);
            pOutObject = nullptr;
        }
        else
        {
            pOutObject = pImpl;
        }
    
        return result;
    }
}
