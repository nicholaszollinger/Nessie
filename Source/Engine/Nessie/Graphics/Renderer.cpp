// Renderer.cpp
#include "Renderer.h"

#include "Nessie/Application/Application.h"
#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Asset/AssetManager.h"
#include "Nessie/Graphics/CommandBuffer.h"
#include "Nessie/Graphics/CommandPool.h"
#include "Nessie/Graphics/DeviceQueue.h"
#include "Nessie/Graphics/Swapchain.h"

namespace nes
{
    static Renderer* g_pRenderer = nullptr;

    Renderer* Renderer::Get()
    {
        return g_pRenderer;
    }

    Renderer* Renderer::GetChecked()
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
        return DeviceManager::GetRenderDevice();
    }

    uint32 Renderer::GetMaxFramesInFlight()
    {
        auto* pRenderer = GetChecked();
        return static_cast<uint32>(pRenderer->m_frames.size());
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

        // Get the Command Pool:
        CommandPool* pPool;
        if (AssetManager::IsAssetThread())
            pPool = &g_pRenderer->m_stagingCommandPool;
        else
            pPool = &g_pRenderer->m_transientCommandPool;
        NES_ASSERT(pPool != nullptr);
        
        // Create and begin the Command Buffer.
        CommandBuffer tempBuffer = pPool->CreateCommandBuffer();
        tempBuffer.Begin();
        
        return tempBuffer;
    }

    void Renderer::SubmitAndWaitTempCommands(CommandBuffer& cmdBuffer, const std::vector<vk::Semaphore>& signalSemaphores, const std::vector<ImageBarrierDesc>& acquireBarriers)
    {
        NES_ASSERT(g_pRenderer != nullptr);

        // End the command Buffer
        cmdBuffer.End();

        // Get the Queue to submit to:
        DeviceQueue* pQueue;
        if (AssetManager::IsAssetThread())
        {
            pQueue = g_pRenderer->m_pTransferQueue;
        }
        else
        {
            pQueue = g_pRenderer->m_pRenderQueue;
        }
        NES_ASSERT(pQueue != nullptr);

        // Submit to the Queue
        auto& vkDevice = g_pRenderer->m_device.GetVkDevice();

        // Create fence for synchronization:
        constexpr vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo();
        vk::Fence fence = (*vkDevice).createFence(fenceInfo, g_pRenderer->m_device.GetVkAllocationCallbacks());

        // Build semaphore submit infos
        std::vector<vk::SemaphoreSubmitInfo> signalInfos;
        for (auto semaphore : signalSemaphores)
        {
            signalInfos.push_back(vk::SemaphoreSubmitInfo()
                .setSemaphore(semaphore)
                .setStageMask(vk::PipelineStageFlagBits2::eTransfer));
        }
        
        // Submit
        const vk::CommandBufferSubmitInfo cmdBufferInfo = vk::CommandBufferSubmitInfo()
            .setCommandBuffer(cmdBuffer.GetVkCommandBuffer());

        const vk::SubmitInfo2 submitInfo = vk::SubmitInfo2()
            .setCommandBufferInfos(cmdBufferInfo)
            .setSignalSemaphoreInfos(signalInfos);
        
        pQueue->GetVkQueue().submit2(submitInfo, fence);

        // Block until complete:
        NES_VK_FAIL_RETURN_VOID(g_pRenderer->m_device, vkDevice.waitForFences(fence, true, UINT64_MAX));
        
        // Clean up.
        (*vkDevice).destroyFence(fence, g_pRenderer->m_device.GetVkAllocationCallbacks());

        // NOW that transfer has completed and signaled semaphores,
        // add the acquire barriers for the render thread
        if (AssetManager::IsAssetThread() && !acquireBarriers.empty())
        {
            std::lock_guard lock(g_pRenderer->m_transferMutex);
            for (const auto& barrier : acquireBarriers)
            {
                g_pRenderer->m_acquireImageBarriers.emplace_back(barrier);
            }
        }
    }

    RenderCommandQueue& Renderer::GetRenderResourceReleaseQueue(const uint32 index)
    {
        NES_ASSERT(index < GetChecked()->m_resourceFreeQueues.size());
        return GetChecked()->m_resourceFreeQueues[index];
    }

    EFormat Renderer::GetSwapchainFormat()
    {
        NES_ASSERT(GetChecked()->m_swapchain != nullptr);
        return GetChecked()->m_swapchain.GetImageFormat();
    }

    vk::Extent2D Renderer::GetSwapchainExtent()
    {
        NES_ASSERT(GetChecked()->m_swapchain != nullptr);
        return GetChecked()->m_swapchain.GetExtent();
    }

    DeviceQueue* Renderer::GetRenderQueue()
    {
        return GetChecked()->m_pRenderQueue;
    }

    DeviceQueue* Renderer::GetTransferQueue()
    {
        return GetChecked()->m_pTransferQueue;
    }

    vk::Semaphore Renderer::AcquireTransferSemaphore()
    {
        auto* pRenderer = GetChecked();
        
        std::lock_guard lock(pRenderer->m_transferMutex);

        for (auto& semaphore : pRenderer->m_transferSemaphores)
        {
            if (!semaphore.m_inUse)
            {
                semaphore.m_inUse = true;
                return semaphore.m_semaphore;
            }
        }

        // Create a new one.
        vk::SemaphoreCreateInfo binaryInfo = {};
        pRenderer->m_transferSemaphores.emplace_back(vk::raii::Semaphore(pRenderer->m_device, binaryInfo), true);
        return pRenderer->m_transferSemaphores.back().m_semaphore;
    }

    bool Renderer::Init(ApplicationWindow* pWindow, RendererDesc&& /*rendererDesc*/)
    {
        m_pWindow = pWindow;

        // Get a device queue to present to.
        // [TODO]: In the renderer description, you should be able to specify if you want to present using a compute
        //         queue or not. If that is the case, there is a feature that needs to be enabled.
        //         - Instead of giving the swapchain a DeviceQueue, you could give it a QueueType and index.
        EGraphicsResult result = m_device.GetQueue(EQueueType::Graphics, 0, m_pRenderQueue);
        NES_GRAPHICS_RETURN_FAIL(m_device, result == EGraphicsResult::Success, false, "Failed to get a queue to present to!");

        // Create the transient command pool for the main thread.
        m_transientCommandPool = CommandPool(m_device, *m_pRenderQueue, true);

        // Get a queue for the Asset Thread to use to submit commands to.
        result = m_device.GetQueue(EQueueType::Transfer, 0, m_pTransferQueue);
        if (result != EGraphicsResult::Success)
        {
            result = m_device.GetQueue(EQueueType::Graphics, 1, m_pTransferQueue);
            NES_GRAPHICS_RETURN_FAIL(m_device, result == EGraphicsResult::Success, false, "Failed to get a queue to perform transfer operations on!");
        }
        m_stagingCommandPool = CommandPool(m_device, *m_pTransferQueue, true);
        
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

        m_transferSemaphores.reserve(16);
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

        // Free transfer semaphores
        m_transferSemaphores.clear();

        // Destroy the swapchain and command pool.
        m_swapchain = nullptr;
        m_transientCommandPool = nullptr;
        m_stagingCommandPool = nullptr;
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

        // Record commands for acquiring resources loaded on the Asset Thread.
        RecordAcquireResources();

        return true;
    }

    void Renderer::EndFrame()
    {
        // Add Swapchain semaphores to the list of semaphores to wait for and signal:
        // First add the swapchain semaphore to wait for the image to be available
        m_renderSubmissionDesc.m_waitSemaphores.push_back
        (
            vk::SemaphoreSubmitInfo()
                .setSemaphore(m_swapchain.GetImageAvailableSemaphore())
                .setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
        );
        
        // Then add the signal for the swapchain to present once everything else is done.
        m_renderSubmissionDesc.m_signalSemaphores.push_back
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
        // Prepare to signal the next frame.
        PrepareFrameToSignal(kHeadlessFramesInFlight);

        // Reset the command pool and buffer to begin recording commands.
        BeginCommandRecording();

        // Clear the semaphores and command buffers submitted last frame.
        ClearPreviousFrameSubmissionData();

        // Record commands for acquiring resources loaded on the Asset Thread.
        RecordAcquireResources();
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
        
        // Release the used transfer semaphores.
        auto& frame = m_frames[m_currentFrameIndex];
        std::lock_guard lock(m_transferMutex);
        
        for (auto semaphore : frame.m_transferSemaphoresToRelease)
        {
            for (auto& ts : m_transferSemaphores)
            {
                if (*ts.m_semaphore == semaphore)
                {
                    NES_ASSERT(ts.m_inUse);
                    ts.m_inUse = false;
                    break;
                }
            }
        }
        frame.m_transferSemaphoresToRelease.clear();
    }

    void Renderer::BeginCommandRecording()
    {
        auto& frame = m_frames[m_currentFrameIndex];
        frame.m_commandPool.Reset();
        frame.m_commandBuffer.Begin();
    }

    RenderFrameContext Renderer::GetRenderFrameContext()
    {
        NES_ASSERT(m_swapchain != nullptr);
        
        RenderFrameContext renderFrameContext;
        renderFrameContext.m_swapchainImage = m_swapchain.GetImage();
        renderFrameContext.m_swapchainImageDescriptor = &m_swapchain.GetImageDescriptor();
        renderFrameContext.m_swapchainExtent = m_swapchain.GetExtent();
        renderFrameContext.m_frameIndex = m_currentFrameIndex;

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
            frame.m_frameNumber = i; // Track the index for synchronization.
            frame.m_commandPool = CommandPool(m_device, *m_pRenderQueue);
            frame.m_commandBuffer = frame.m_commandPool.CreateCommandBuffer();
            frame.m_commandBuffer.SetDebugName(fmt::format("Frame Command Buffer ({})", i));
        }
        
        // Create the Resource Free Queues
        m_resourceFreeQueues.resize(numFramesInFlight);
    }

    void Renderer::ClearPreviousFrameSubmissionData()
    {
        m_renderSubmissionDesc.m_waitSemaphores.clear();
        m_renderSubmissionDesc.m_signalSemaphores.clear();
        m_renderSubmissionDesc.m_commandBuffers.clear();
    }

    void Renderer::RecordAcquireResources()
    {
        // Add all pending barriers and 
        std::lock_guard lock(m_transferMutex);

        if (m_acquireImageBarriers.empty())
            return;
        
        BarrierGroupDesc barriers{};
        std::vector<vk::Semaphore> semaphoresToRelease{}; 
        for (auto& acquireImageBarrier : m_acquireImageBarriers)
        {
            if (acquireImageBarrier.m_transferSemaphore != nullptr)
            {
                barriers.m_imageBarriers.emplace_back(acquireImageBarrier);
                
                // Add the wait semaphore to the specific barrier:
                vk::SemaphoreSubmitInfo waitInfo = vk::SemaphoreSubmitInfo()
                    .setSemaphore(acquireImageBarrier.m_transferSemaphore)
                    .setStageMask(vk::PipelineStageFlagBits2::eTopOfPipe);
                
                m_renderSubmissionDesc.m_waitSemaphores.emplace_back(waitInfo);

                // Track for cleanup.
                semaphoresToRelease.emplace_back(acquireImageBarrier.m_transferSemaphore);
            }
        }
        
        auto& commandBuffer = m_frames[m_currentFrameIndex].m_commandBuffer;
        commandBuffer.SetBarriers(barriers);

        m_frames[m_currentFrameIndex].m_transferSemaphoresToRelease = std::move(semaphoresToRelease);

        // Clear the barriers.
        m_acquireImageBarriers.clear();
    }

    void Renderer::SubmitFrameCommands()
    {
        // Get the frame data for the current frame in the buffer.
        FrameData& frame = m_frames[m_currentFrameIndex];
        
        // End Recording commands for the frame.
        frame.m_commandBuffer.End();
        
        // Add timeline semaphore to signal when GPU completes this frame.
        m_renderSubmissionDesc.m_signalSemaphores.push_back(vk::SemaphoreSubmitInfo()
            .setSemaphore(m_frameTimelineSemaphore)
            .setValue(frame.m_frameNumber)
            .setStageMask(vk::PipelineStageFlagBits2::eBottomOfPipe));
        
        // Adding the command buffer of the frame to the list of command buffers to submit
        // Note: extra command buffers could have been added to the list from other parts of the render frame.
        m_renderSubmissionDesc.m_commandBuffers.push_back(vk::CommandBufferSubmitInfo()
            .setCommandBuffer(frame.m_commandBuffer.GetVkCommandBuffer()));

        const vk::SubmitInfo2 submitInfo = vk::SubmitInfo2()
            .setWaitSemaphoreInfos(m_renderSubmissionDesc.m_waitSemaphores)
            .setCommandBufferInfos(m_renderSubmissionDesc.m_commandBuffers)
            .setSignalSemaphoreInfos(m_renderSubmissionDesc.m_signalSemaphores);

        m_pRenderQueue->GetVkQueue().submit2(submitInfo, nullptr);
    }

    void Renderer::AdvanceToNextFrame(const uint32 numFramesInFlight)
    {
        m_currentFrameIndex = (m_currentFrameIndex + 1) % numFramesInFlight;
    }
}
