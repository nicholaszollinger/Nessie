// DeviceVk.h
#pragma once
#include "VulkanCore.h"
#include "VulkanDispatchTable.h"
#include "VulkanConversions.h"
#include "Nessie/Graphics/RendererDesc.h"
#include "Nessie/Graphics/RenderDevice.h"

namespace nes
{
    class VulkanQueue;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Vulkan Render Device. Contains the Vulkan Instance, Logical Device, and Surface.
    //----------------------------------------------------------------------------------------------------
    class VulkanDevice final : public RenderDevice
    {
    public:
        /// Conversion operators for core Vulkan Types.
        inline                  operator VkPhysicalDevice() const    { return m_vkPhysicalDevice; }
        inline                  operator VkInstance() const          { return m_vkInstance; }
        inline                  operator VkDevice() const            { return m_vkDevice; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialization will load the Vulkan Library and create the instance, logical device, and
        ///     surface for the given window.
        //----------------------------------------------------------------------------------------------------
        virtual bool            Init(const ApplicationDesc& appDesc, ApplicationWindow* pWindow, const RendererDesc& rendererDesc) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroys the Device. 
        //----------------------------------------------------------------------------------------------------
        virtual void            Destroy() override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create an implementation class with the given CreateArgs. The pointer will be cast to and
        ///     stored in the Interface type 'pEntity' parameter.
        ///	@tparam ImplementationType : Vulkan implementation class. Ex: VulkanQueue.
        ///	@tparam InterfaceType : Type that will store the pointer of the created Implementation class. For example,
        ///     if we are creating a VulkanQueue, the InterfaceType can be a VulkanQueue, or a DeviceQueue.
        ///	@tparam CreateArgs : Parameter types to send to the Create() function on the Implementation class.
        ///	@param pEntity : Pointer that will be set to the address of the created Implementation.
        ///	@param args : Parameters to send to the Create() function on the Implementation class.
        ///	@returns : Result of the Implementation class's Create() function.
        //----------------------------------------------------------------------------------------------------
        template <typename ImplementationType, typename InterfaceType, typename...CreateArgs> requires requires(VulkanDevice& device, ImplementationType type, CreateArgs&&... args)
        {
            ImplementationType(device);                                                             // Requires a constructor that takes in a VulkanDevice&
            { type.Create(std::forward<CreateArgs>(args)...) } -> std::same_as<EGraphicsResult>;    // Requires a Create function that takes in the give CreateArgs, and returns an EGraphicsResult.
        }
        EGraphicsResult         CreateImplementation(InterfaceType*& pEntity, CreateArgs&&...args);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the core vulkan function table. Functions may be nullptr depending on set features!
        //----------------------------------------------------------------------------------------------------
        const VulkanDispatchTable& GetDispatchTable() const { return m_vk; }
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Loads the Vulkan Library, and initializes the pre-instance dispatch functions, to
        ///     create the instance. Returns EGraphicsResult::Unsupported on failure.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         InitializeVulkan();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the Vulkan Instance. Returns EGraphicsResult::Unsupported on failure.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         CreateInstance(const ApplicationDesc& appDesc, const RendererDesc& rendererDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the remaining Instance functions in the dispatch table, using the desiredExtensions array.
        /// If any are not available, this will return EGraphicsResult::Unsupported on failure.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         ResolveInstanceDispatchTable(const std::vector<const char*>& desiredInstanceExtensions);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Removes any unsupported layers from the layers array. 
        //----------------------------------------------------------------------------------------------------
        void                    FilterInstanceLayers(std::vector<const char*>& layers) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Select a physical device to use for rendering. Returns EGraphicsResult::Failure on failure.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         SelectPhysicalDevice(const RendererDesc& rendererDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the Logical Device. Returns EGraphicsResult::Failure on failure.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         CreateLogicalDevice(const RendererDesc& rendererDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add the default device extensions from the supported physical device extensions.
        //----------------------------------------------------------------------------------------------------
        void                    ProcessDeviceExtensions(std::vector<const char*>& desiredExtensions, const bool rayTracingEnabled = false) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Resolve the Logical Device's dispatch table functions.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         ResolveDeviceDispatchTable(const std::vector<const char*> desiredDeviceExtensions);

    private:
        static constexpr uint32 kInvalidQueueIndex = std::numeric_limits<uint16>::max();

        using QueueArray = std::vector<VulkanQueue*>;
        using QueueFamilyArray = std::array<QueueArray, static_cast<size_t>(EQueueType::MaxNum)>;
        using ActiveQueueIndicesArray = std::array<uint32, static_cast<size_t>(EQueueType::MaxNum)>; 

        QueueFamilyArray        m_queueFamilies{};      /// Contains an array of VulkanQueues for each EQueueType. 
        //ActiveQueueIndicesArray m_activeQueueIndices{};
        DeviceDesc              m_deviceDesc{};
        vk::Instance            m_vkInstance{};
        vk::PhysicalDevice      m_vkPhysicalDevice{};
        VkAllocationCallbacks   m_vkAllocationCallbacks{};
        VkAllocationCallbacks*  m_vkAllocationCallbacksPtr = nullptr;
        VulkanDispatchTable     m_vk{};
        VkDebugUtilsMessengerEXT m_debugMessenger{};
        VkPhysicalDeviceMemoryProperties m_memoryProperties{};
        vk::Device              m_vkDevice{};
        // vk::PhysicalDeviceMemoryProperties m_memoryProps;
        // uint32                 m_numActiveFamilyIndices;
    };

    template <typename ImplementationType, typename InterfaceType, typename ... CreateArgs> requires requires (VulkanDevice& device, ImplementationType type, CreateArgs&&...args)
    {
        ImplementationType(device);
        { type.Create(std::forward<CreateArgs>(args)...) } -> std::same_as<EGraphicsResult>;
    }
    EGraphicsResult VulkanDevice::CreateImplementation(InterfaceType*& pEntity, CreateArgs&&... args)
    {
        ImplementationType* pImpl = Allocate<ImplementationType>(GetAllocationCallbacks(), *this);
        EGraphicsResult result = pImpl->Create(std::forward<CreateArgs>(args)...);
        if (result != EGraphicsResult::Success)
        {
            Free<ImplementationType>(GetAllocationCallbacks(), pImpl);
            pEntity = nullptr;
        }
        else
        {
            // This isn't a checked_cast, because I am allowing a conversion to a stand-in type like DeviceQueue, which
            // is only forward declared.
            pEntity = reinterpret_cast<InterfaceType*>(pImpl);
        }

        return result;
    }
}
