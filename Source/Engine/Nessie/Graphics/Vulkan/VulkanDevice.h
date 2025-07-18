// DeviceVk.h
#pragma once
#include "VulkanCore.h"
#include "Nessie/Graphics/RendererDesc.h"
#include "Nessie/Graphics/Device.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Under development. Vulkan specific Render Device; contains the instance, physical device and
    ///     logical device.
    //----------------------------------------------------------------------------------------------------
    class VulkanDevice final : public Device
    {
    public:
        // /// Conversion operators for core Vulkan Types.
        // inline                      operator VkPhysicalDevice() const    { return m_vkPhysicalDevice; }
        // inline                      operator VkInstance() const          { return m_vkInstance; }
        // inline                      operator VkDevice() const            { return m_vkDevice; }

        virtual bool                Init(const ApplicationDesc& appDesc, ApplicationWindow* pWindow, const RendererDesc& rendererDesc) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroys the Device. 
        //----------------------------------------------------------------------------------------------------
        virtual void                Destroy() override;

        virtual const DeviceDesc&   GetDesc() const override { return m_deviceDesc; }

        void                        SetDebugNameToTrivialObject(vk::ObjectType type, uint64 handle, const char* name) const;

        // //----------------------------------------------------------------------------------------------------
        // /// @brief : Create a vulkan implementation class.
        // ///	@tparam ImplementationType : Vulkan implementation class. Ex: BufferVk 
        // ///	@tparam ResultType : The type that the result will be cast to. Ex: BufferVk -> Buffer.
        // ///	@tparam Args : Constructor parameters for the type.
        // ///	@param pOutEntity : This pointer will point to the created entity.
        // ///	@param args : Constructor arguments.
        // ///	@returns : If the result != EResult::Success, pOutEntity will be set to nullptr.
        // //----------------------------------------------------------------------------------------------------
        // template <typename ImplementationType, typename ResultType, typename...Args>
        // EGraphicsErrorCodes         CreateImplementation(ResultType*& pOutEntity, const Args&... args);
        //
        // //----------------------------------------------------------------------------------------------------
        // /// @brief : Get this device's properties.
        // //----------------------------------------------------------------------------------------------------
        // virtual const DeviceDesc&   GetDesc() const override;
        //
        // //----------------------------------------------------------------------------------------------------
        // /// @brief : Set the name of this Device.
        // //----------------------------------------------------------------------------------------------------
        // void                        SetDebugName(const char* name) NES_DEBUG_NAME_OVERRIDE;

        // virtual EFormatSupportBits  GetFormatSupport(EFormat format) const override;
        //
        // virtual EResult             GetQueue(EQueueType queueType, uint32 queueIndex, Queue*& pOutQueue) override;
        // virtual EResult             CreateFence(uint64 initialValue, Fence*& pOutFence) override;
        // virtual EResult             CreateDescriptorPool(const DescriptorPoolDesc& desc, DescriptorPool*& pOutDescriptorPool) override;
        // virtual EResult             CreateBuffer(const BufferDesc& desc, Buffer*& pOutBuffer) override;
        // virtual EResult             CreateTexture(const TextureDesc& desc, Texture*& pOutTexture) override;
        // virtual EResult             CreatePipelineLayout(const PipelineLayoutDesc& desc, PipelineLayout*& pOutPipelineLayout) override;
        // virtual EResult             CreateGraphicsPipeline(const GraphicsPipelineDesc& desc, Pipeline*& pOutPipeline) override;
        // virtual EResult             CreateComputePipeline(const ComputePipelineDesc& desc, Pipeline*& pOutPipeline) override;
        // virtual EResult             WaitUntilIdle() override;
        //
        // virtual void                DestroyCommandAllocator(CommandAllocator& commandAllocator) override;
        // virtual void                DestroyCommandBuffer(CommandBuffer& commandBuffer) override;
        // virtual void                DestroyDescriptorPool(DescriptorPool& descriptorPool) override;
        // virtual void                DestroyBuffer(Buffer& buffer) override;
        // virtual void                DestroyTexture(Texture& texture) override;
        // virtual void                DestroyDescriptor(Descriptor& descriptor) override;
        // virtual void                DestroyPipelineLayout(PipelineLayout& pipelineLayout) override;
        // virtual void                DestroyPipeline(Pipeline& pipeline) override;
        // virtual void                DestroyQueryPool(QueryPool& queryPool) override;
        // virtual void*               GetDeviceNativeObject() const override;
        //
        // void                        FillCreateInfo(const BufferDesc& bufferDesc, vk::BufferCreateInfo& outInfo) const;
        // void                        FillCreateInfo(const TextureDesc& textureDesc, vk::ImageCreateInfo& outInfo) const;
        // void                        GetMemoryDesc2(const BufferDesc& bufferDesc, EMemoryLocation location, GHeapDesc& outMemoryDesc) const;
        // void                        GetMemoryDesc2(const TextureDesc& bufferDesc, EMemoryLocation location, GHeapDesc& outMemoryDesc) const;
        // bool                        GetMemoryTypeInfo(EMemoryLocation memoryLocation, uint32 memoryTypeMask, MemoryTypeInfo& outMemoryInfo) const;
        // bool                        GetMemoryTypeByIndex(const uint32 index, MemoryTypeInfo& outTypeInfo) const;
        // void                        SetDebugNameToTrivialObject(vk::ObjectType objectType, const uint64 handle, const char* name);
        //
        // inline bool                 IsHostCoherentMemory(const MemoryTypeIndex memoryTypeIndex) const   { return (m_memoryProps.memoryTypes[memoryTypeIndex].propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent) != vk::MemoryPropertyFlagBits{}; }
        //
        // union
        // {
        //     uint32                  m_isSupportedStorage = 0;
        //     IsSupported             m_isSupported;
        // };

    private:
        void                    FillDeviceDesc();
        //void                        FilterInstanceLayers(std::vector<const char*>& layers);
        //void                        ProcessInstanceExtensions(std::vector<const char*>& desiredInstanceExtensions);
        //void                        ProcessDeviceExtensions(std::vector<const char*>& desiredDeviceExtensions, const bool disableRayTracing);
        //EResult                     CreateInstance(const bool enableGraphicsAPIValidation, const std::vector<const char*>& desiredInstanceExtensions);
        //EResult                     ResolvePreInstanceDispatchTable(const std::vector<const char*>& desiredInstanceExtensions);
        //EResult                     ResolveDispatchTable(const std::vector<const char*>& desiredDeviceExtensions);

    private:
        DeviceDesc              m_deviceDesc{};
        vk::AllocationCallbacks m_vkAllocationCallbacks{};
        vkb::Instance           m_vkInstance{};
        vk::SurfaceKHR          m_vkSurface{};
        vkb::PhysicalDevice     m_vkPhysicalDevice{};
        vkb::Device             m_vkDevice{};
        vkb::DispatchTable      m_vk{};
        // vk::Instance                m_vkInstance;        
        // vk::PhysicalDevice          m_vkPhysicalDevice;
        // vk::Device                  m_vkDevice = nullptr;
        // vk::AllocationCallbacks     m_vkAllocationCallbacks;
        // vk::DebugUtilsMessengerEXT  m_messenger = nullptr;
        // vk::PhysicalDeviceMemoryProperties m_memoryProps;
        // uint32                      m_numActiveFamilyIndices;
        // uint32                      m_minorVersion;
    };
}