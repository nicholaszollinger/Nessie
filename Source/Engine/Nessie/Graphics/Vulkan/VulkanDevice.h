// DeviceVk.h
#pragma once
#include "VulkanCore.h"
#include "VulkanDispatchTable.h"
#include "Nessie/Graphics/RendererDesc.h"
#include "Nessie/Graphics/RenderDevice.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Vulkan Render Device. Contains the Vulkan Instance, Logical Device, and Surface.
    //----------------------------------------------------------------------------------------------------
    class VulkanDevice final : public RenderDevice
    {
    public:
        /// Conversion operators for core Vulkan Types.
        inline                  operator VkPhysicalDevice() const    { return m_vkPhysicalDevice; }
        inline                  operator VkInstance() const          { return m_vkInstance; }
        //inline                  operator VkDevice() const            { return m_vkDevice; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialization will load the Vulkan Library and create the instance, logical device, and
        ///     surface for the given window.
        //----------------------------------------------------------------------------------------------------
        virtual bool            Init(const ApplicationDesc& appDesc, ApplicationWindow* pWindow, const RendererDesc& rendererDesc) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroys the Device. 
        //----------------------------------------------------------------------------------------------------
        virtual void            Destroy() override;
    
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

    private:
        static constexpr uint32 kInvalidQueueIndex = std::numeric_limits<uint16>::max();
        
        DeviceDesc              m_deviceDesc{};
        vk::Instance            m_vkInstance{};
        vk::PhysicalDevice      m_vkPhysicalDevice{};
        VkAllocationCallbacks   m_vkAllocationCallbacks{};
        VkAllocationCallbacks*  m_vkAllocationCallbacksPtr = nullptr;
        vulkan::DispatchTable   m_vk{};
        VkDebugUtilsMessengerEXT m_debugMessenger{};
        VkPhysicalDeviceMemoryProperties m_memoryProperties{};
        //vk::Device              m_vkDevice{};
        // vk::PhysicalDeviceMemoryProperties m_memoryProps;
        // uint32                 m_numActiveFamilyIndices;
    };
}