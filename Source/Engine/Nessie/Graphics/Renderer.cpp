// Renderer.cpp
#include "Renderer.h"

#include "Nessie/Application/Application.h"
#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Graphics/CommandBuffer.h"
#include "Nessie/Graphics/CommandPool.h"
#include "Nessie/Graphics/DeviceQueue.h"
#include "Nessie/Graphics/Swapchain.h"

namespace nes
{
    static Renderer* g_pRenderer = nullptr;

    Renderer* Renderer::Get()
    {
        NES_ASSERT(g_pRenderer != nullptr);
        return g_pRenderer;
    }

    Renderer::Renderer(RenderDevice& device)
        : m_device(device)
    {
        NES_ASSERT(g_pRenderer == nullptr);
        g_pRenderer = this;
    }

    Renderer::~Renderer()
    {
        NES_ASSERT(g_pRenderer == this);
        g_pRenderer = nullptr;
    }

    RenderDevice& Renderer::GetDevice()
    {
        NES_ASSERT(g_pRenderer != nullptr);
        return g_pRenderer->m_device;
    }

    void Renderer::ExecuteImmediateCommands(const RecordCommandsFunc& func)
    {
        NES_ASSERT(g_pRenderer != nullptr);
        
        CommandBuffer buffer = BeginTempCommands();
        func(g_pRenderer->m_device, buffer);
        SubmitAndWaitTempCommands(buffer);
    }

    CommandBuffer Renderer::BeginTempCommands()
    {
        NES_ASSERT(g_pRenderer != nullptr);
        NES_ASSERT(g_pRenderer->m_transientCommandPool != nullptr);

        CommandBuffer tempBuffer = g_pRenderer->m_transientCommandPool.CreateCommandBuffer();
        tempBuffer.Begin();
        
        return tempBuffer;
    }

    void Renderer::SubmitAndWaitTempCommands(CommandBuffer& cmdBuffer)
    {
        cmdBuffer.End();

        // Submit to the Render Queue.
        auto& vkDevice = g_pRenderer->m_device.GetVkDevice();

        // Create fence for synchronization:
        constexpr vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo();
        vk::Fence fence = (*vkDevice).createFence(fenceInfo, g_pRenderer->m_device.GetVkAllocationCallbacks());

        // Submit
        const vk::CommandBufferSubmitInfo cmdBufferInfo = vk::CommandBufferSubmitInfo()
            .setCommandBuffer(cmdBuffer.GetVkCommandBuffer());

        const vk::SubmitInfo2 submitInfo = vk::SubmitInfo2()
            .setCommandBufferInfos(cmdBufferInfo);
        
        g_pRenderer->m_pRenderQueue->GetVkQueue().submit2(submitInfo, fence);

        // Block until complete:
        NES_VK_FAIL_RETURN_VOID(g_pRenderer->m_device, vkDevice.waitForFences(fence, true, UINT64_MAX));
        
        // Clean up.
        (*vkDevice).destroyFence(fence, g_pRenderer->m_device.GetVkAllocationCallbacks());
    }

    RenderCommandQueue& Renderer::GetRenderResourceReleaseQueue(const uint32 index)
    {
        NES_ASSERT(index < Get()->m_resourceFreeQueues.size());
        return Get()->m_resourceFreeQueues[index];
    }
    
    bool Renderer::Init(ApplicationWindow* pWindow, const RendererDesc& /*rendererDesc*/)
    {
        m_pWindow = pWindow;

        // Get a device queue to present to.
        // [TODO]: In the renderer description, you should be able to specify if you want to present using a compute
        //         queue or not. If that is the case, there is a feature that needs to be enabled.
        //         - Instead of giving the swapchain a DeviceQueue, you could give it a QueueType and index.
        EGraphicsResult result = m_device.GetQueue(EQueueType::Graphics, 0, m_pRenderQueue);
        NES_GRAPHICS_RETURN_FAIL(m_device, result == EGraphicsResult::Success, false, "Failed to get a queue to present to!");

        // Create the transient command pool
        m_transientCommandPool = CommandPool(m_device, *m_pRenderQueue, true);
        
        // Create the Swapchain if we have a window to present to:
        if (m_pWindow != nullptr)
        {
            // Create the swapchain.
            const SwapchainDesc desc
            {
                .m_pWindow = pWindow,
                .m_pDeviceQueue = m_pRenderQueue,
            };
            m_swapchain = Swapchain(m_device, desc);
            
            // Create what is need to submit commands for each frame-in-flight.
            CreateFrameSubmissionResources(m_swapchain.GetMaxFramesInFlight());
        }

        // If no window, still create the frame resources.
        else
        {
            CreateFrameSubmissionResources(kHeadlessFramesInFlight);
        }
        
        return true;
    }

    void Renderer::Shutdown()
    {
        m_device.WaitUntilIdle();
        
        // Free any remaining resources:
        for (auto& releaseQueue : m_resourceFreeQueues)
        {
            releaseQueue.Execute();    
        }
        m_resourceFreeQueues.clear();

        // Cleanup frame data:
        m_frames.clear();
        m_frameTimelineSemaphore = nullptr;

        // Destroy the swapchain and command pool.
        m_swapchain = nullptr;
        m_transientCommandPool = nullptr;
    }

    bool Renderer::BeginFrame()
    {
        // This is a non-headless version, so we must have a swapchain and window.
        NES_ASSERT(m_pWindow);
        NES_ASSERT(m_swapchain != nullptr);
        
        if (m_swapchain.NeedsRebuild())
        {
            const auto& desc = m_pWindow->GetDesc();
            const auto windowSize = desc.m_windowResolution;
            const bool vsyncEnabled = desc.m_vsyncEnabled;
            m_swapchain.OnResize(windowSize, vsyncEnabled);
        }
        
        // Acquire the next image to render to. If out of date (Needs rebuild) or an error occurred, return false.
        // No render commands should be made until the swapchain is rebuilt.
        if (m_swapchain.AcquireNextImage() != EGraphicsResult::Success)
            return false;

        // Free resources from the previous frame.
        ProcessResourceFreeQueue();

        // Prepare frame synchronization.
        PrepareFrameToSignal(m_swapchain.GetMaxFramesInFlight());

        // Reset the command pool and buffer to begin recording commands.
        BeginCommandRecording();

        // Clear the semaphores and command buffers submitted last frame.
        ClearPreviousFrameSubmissionData();

        return true;
    }

    void Renderer::EndFrame()
    {
        // Add Swapchain semaphores to the list of semaphores to wait for and signal:
        // First add the swapchain semaphore to wait for the image to be available
        m_waitSemaphores.push_back
        (
            vk::SemaphoreSubmitInfo()
                .setSemaphore(m_swapchain.GetImageAvailableSemaphore())
                .setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
        );
        // Then add the signal for the swapchain to present once everything else is done.
        m_signalSemaphores.push_back
        (
         vk::SemaphoreSubmitInfo()
                .setSemaphore(m_swapchain.GetRenderFinishedSemaphore())
                .setStageMask(vk::PipelineStageFlagBits2::eBottomOfPipe)
        );

        // Submit the current commands to the Render Queue.
        SubmitFrameCommands();

        // Present the frame:
        m_swapchain.PresentFrame();
        
        // Advance to the next frame:
        AdvanceToNextFrame(m_swapchain.GetMaxFramesInFlight());
    }

    void Renderer::BeginHeadlessFrame()
    {
        PrepareFrameToSignal(kHeadlessFramesInFlight);

        // Reset the command pool and buffer to begin recording commands.
        BeginCommandRecording();

        // Clear the semaphores and command buffers submitted last frame.
        ClearPreviousFrameSubmissionData();
    }

    void Renderer::EndHeadlessFrame()
    {
        // Submit the current commands to the Render Queue.
        SubmitFrameCommands();
        
        AdvanceToNextFrame(kHeadlessFramesInFlight);
    }

    void Renderer::RequestSwapchainRebuild()
    {
        NES_ASSERT(m_swapchain != nullptr);
        m_swapchain.RequestRebuild();
    }

    void Renderer::ProcessResourceFreeQueue()
    {
        auto& freeQueue = m_resourceFreeQueues[m_currentFrameIndex];
        freeQueue.Execute();
    }

    void Renderer::BeginCommandRecording()
    {
        auto& frame = m_frames[m_currentFrameIndex];
        frame.m_commandPool.Reset();
        frame.m_commandBuffer.Begin();
    }

    RenderFrameContext Renderer::GetRenderFrameContext() const
    {
        NES_ASSERT(m_swapchain != nullptr);
        
        RenderFrameContext renderFrameContext;
        renderFrameContext.m_swapchainImage = &m_swapchain.GetImage();
        renderFrameContext.m_swapchainImageDescriptor = &m_swapchain.GetImageDescriptor();
        renderFrameContext.m_swapchainExtent = m_swapchain.GetExtent();

        return renderFrameContext;
    }

    void Renderer::WaitForFrameCompletion()
    {
        vk::SemaphoreWaitInfo waitInfo = vk::SemaphoreWaitInfo()
            .setSemaphores(*m_frameTimelineSemaphore)
            .setValues(m_frames[m_currentFrameIndex].m_frameNumber);

        // Wait until GPU has finished processing the frame that was using these resources previously (m_numFramesInFlight frames ago)
        NES_VK_FAIL_REPORT(m_device, m_device.GetVkDevice().waitSemaphores(waitInfo, std::numeric_limits<uint64_t>::max()));
    }

    void Renderer::WaitUntilAllFramesCompleted() const
    {
        m_device.WaitUntilIdle();
    }

    void Renderer::PrepareFrameToSignal(const uint32 numFramesInFlight)
    {
        m_frames[m_currentFrameIndex].m_frameNumber += numFramesInFlight;
    }

    void Renderer::CreateFrameSubmissionResources(const uint32 numFramesInFlight)
    {
        NES_ASSERT(numFramesInFlight >= 2);

        m_frames.resize(numFramesInFlight);

        // Initialize timeline semaphore with (numFrames - 1) to allow concurrent frame submission.
        const uint64 initialValue = (static_cast<uint64>(numFramesInFlight) - 1);

        // Create the timeline semaphore for GPU-CPU synchronization
        // This ensures that resources aren't overwritten while still in use by the GPU.
        vk::SemaphoreTypeCreateInfo timelineCreateInfo = vk::SemaphoreTypeCreateInfo()
            .setSemaphoreType(vk::SemaphoreType::eTimeline)
            .setInitialValue(initialValue);

        vk::SemaphoreCreateInfo semaphoreCreateInfo = vk::SemaphoreCreateInfo()
            .setPNext(&timelineCreateInfo);
        
        m_frameTimelineSemaphore = m_device.GetVkDevice().createSemaphore(semaphoreCreateInfo, m_device.GetVkAllocationCallbacks());
        m_device.SetDebugNameVkObject(NativeVkObject(*m_frameTimelineSemaphore, vk::ObjectType::eSemaphore), "Frame Timeline Semaphore");
        
        // Create a command pool and command buffer for each frame.
        // Each frame gets its own command pool to allow parallel command recording while previous frames may still be executing on the GPU.
        for (uint32 i = 0; i < numFramesInFlight; ++i)
        {
            auto& frame = m_frames[i];
            // Track the index for synchronization.
            frame.m_frameNumber = i;
            frame.m_commandPool = CommandPool(m_device, *m_pRenderQueue);
            frame.m_commandBuffer = frame.m_commandPool.CreateCommandBuffer();
        }
        
        // Create the Resource Free Queues
        m_resourceFreeQueues.resize(numFramesInFlight);
    }

    void Renderer::ClearPreviousFrameSubmissionData()
    {
        m_waitSemaphores.clear();
        m_signalSemaphores.clear();
        m_commandBuffers.clear();
    }

    void Renderer::SubmitFrameCommands()
    {
        // Get the frame data for the current frame in the buffer.
        FrameData& frame = m_frames[m_currentFrameIndex];
        
        // End Recording commands for the frame.
        frame.m_commandBuffer.End();
        
        // Add timeline semaphore to signal when GPU completes this frame
        // The color attachment output stage is used since that's when the frame is fully rendered.
        m_signalSemaphores.push_back(vk::SemaphoreSubmitInfo()
            .setSemaphore(m_frameTimelineSemaphore)
            .setValue(frame.m_frameNumber)
            .setStageMask(vk::PipelineStageFlagBits2::eBottomOfPipe)); // Wait until everything is completed.
        
        // Adding the command buffer of the frame to the list of command buffers to submit
        // Note: extra command buffers could have been added to the list from other parts of the render frame.
        m_commandBuffers.push_back(vk::CommandBufferSubmitInfo()
            .setCommandBuffer(frame.m_commandBuffer.GetVkCommandBuffer()));

        const vk::SubmitInfo2 submitInfo = vk::SubmitInfo2()
            .setWaitSemaphoreInfoCount(static_cast<uint32>(m_waitSemaphores.size()))
            .setPWaitSemaphoreInfos(m_waitSemaphores.data())
            .setCommandBufferInfoCount(static_cast<uint32>(m_commandBuffers.size()))
            .setPCommandBufferInfos(m_commandBuffers.data())
            .setSignalSemaphoreInfoCount(static_cast<uint32>(m_signalSemaphores.size()))
            .setPSignalSemaphoreInfos(m_signalSemaphores.data());

        m_pRenderQueue->GetVkQueue().submit2(submitInfo, nullptr);
    }

    void Renderer::AdvanceToNextFrame(const uint32 numFramesInFlight)
    {
        m_currentFrameIndex = (m_currentFrameIndex + 1) % numFramesInFlight;
    }
}
