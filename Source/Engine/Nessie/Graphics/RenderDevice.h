// DeviceBase.h
#pragma once
#include "GraphicsCommon.h"
#include "RendererDesc.h"
#include "Nessie/Application/ApplicationDesc.h"

namespace nes
{
    class Swapchain;
    struct SwapchainDesc;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for graphics api devices. A RenderDevice is the intermediary between the program and
    ///     the hardware device (GPU).
    //----------------------------------------------------------------------------------------------------
    class RenderDevice
    {
    public:
        RenderDevice() = default;
        RenderDevice(const RenderDevice&) = delete;
        RenderDevice(RenderDevice&&) noexcept = delete;
        RenderDevice& operator=(const RenderDevice&) = delete;
        RenderDevice& operator=(RenderDevice&&) noexcept = delete;
        virtual ~RenderDevice() = default;

        virtual bool                Init(const ApplicationDesc& appDesc, ApplicationWindow* pWindow, const RendererDesc& rendererDesc) = 0;
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroy this device.
        //----------------------------------------------------------------------------------------------------
        virtual void                Destroy() = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Post a message using the set Debug Messenger callback.
        //----------------------------------------------------------------------------------------------------
        void                        ReportMessage(const ELogLevel level, const char* file, const uint32 line, const char* message, const nes::LogTag& tag = kGraphicsLogTag) const;
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the description info of this device. 
        //----------------------------------------------------------------------------------------------------
        //const nri::DeviceDesc&  GetDesc() const;
    
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the allocation callbacks set for this device.
        //----------------------------------------------------------------------------------------------------
        const AllocationCallbacks&  GetAllocationCallbacks() const { return m_allocationCallbacks; }

        // //----------------------------------------------------------------------------------------------------
        // /// @brief : Get the features supported for a given format.
        // //----------------------------------------------------------------------------------------------------
        // virtual EFormatSupportBits          GetFormatSupport(EFormat format) const = 0;
        //
        // //----------------------------------------------------------------------------------------------------
        // /// @brief : Helper function to get the supported depth format for this device.
        // //----------------------------------------------------------------------------------------------------
        // EFormat                             GetSupportedDepthFormat(const uint32 minBits, const bool enableStencil);
        //
        // virtual EGraphicsErrorCodes         CreateSwapchain(const SwapchainDesc& swapchainDesc, Swapchain*& pOutSwapchain) = 0;
        //
        // virtual EGraphicsErrorCodes         GetQueue(EQueueType queueType, uint32 queueIndex, DeviceQueue*& pOutQueue) = 0;
        // virtual EGraphicsErrorCodes         CreateFence(uint64 initialValue, GFence*& pOutFence) = 0;
        // virtual EGraphicsErrorCodes         CreateDescriptorPool(const DescriptorPoolDesc& desc, DescriptorPool*& pOutDescriptorPool) = 0;
        //
        // virtual EGraphicsErrorCodes         CreateBuffer(const GBufferDesc& desc, GBuffer*& pOutBuffer) = 0;
        // virtual EGraphicsErrorCodes         CreateTexture(const GTextureDesc& desc, GTexture*& pOutTexture) = 0;
        // virtual EGraphicsErrorCodes         CreatePipelineLayout(const PipelineLayoutDesc& desc, PipelineLayout*& pOutPipelineLayout) = 0;
        // virtual EGraphicsErrorCodes         CreateGraphicsPipeline(const GraphicsPipelineDesc& desc, Pipeline*& pOutPipeline) = 0;
        // virtual EGraphicsErrorCodes         CreateComputePipeline(const ComputePipelineDesc& desc, Pipeline*& pOutPipeline) = 0;
        //
        // virtual void                        DestroyCommandAllocator(GCommandAllocator& commandAllocator) = 0;
        // virtual void                        DestroyCommandBuffer(GCommandBuffer& commandBuffer) = 0;
        // virtual void                        DestroyDescriptorPool(DescriptorPool& descriptorPool) = 0;
        // virtual void                        DestroyBuffer(GBuffer& buffer) = 0;
        // virtual void                        DestroyTexture(GTexture& texture) = 0;
        // virtual void                        DestroyDescriptor(Descriptor& descriptor) = 0;
        // virtual void                        DestroyPipelineLayout(PipelineLayout& pipelineLayout) = 0;
        // virtual void                        DestroyPipeline(Pipeline& pipeline) = 0;
        // virtual void                        DestroyQueryPool(GQueryPool& queryPool) = 0;
        // virtual void                        DestroyFence(GFence& fence) = 0;
        //
        // virtual EGraphicsErrorCodes         AllocateMemory(const AllocateGMemoryDesc& desc, GMemory*& pOutMemory) = 0;
        // virtual EGraphicsErrorCodes         BindBufferMemory(const GBufferMemoryBindingDesc* pBindingDescs, const uint32 numMemoryBindings) = 0;
        // virtual EGraphicsErrorCodes         BindTextureMemory(const GTextureMemoryBindingDesc* pBindingDescs, const uint32 numMemoryBindings) = 0;
        // virtual void                        FreeMemory(GMemory& memory) = 0;
        // virtual EGraphicsErrorCodes         WaitUntilIdle() = 0;

    protected:

    private:
        AllocationCallbacks     m_allocationCallbacks{};
        DebugMessenger          m_debugMessenger{};
    };
}
