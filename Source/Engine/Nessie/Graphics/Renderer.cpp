// Renderer.cpp
#include "Renderer.h"

#include "ShaderLibrary.h"
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
        EGraphicsResult result = m_device.GetQueue(EQueueType::Graphics, 0, m_pRenderSubmissionQueue);
        NES_GRAPHICS_RETURN_FAIL(m_device, result == EGraphicsResult::Success, false, "Failed to get a queue to present to!");
        
        // Create the Swapchain if we have a window to present to:
        if (m_pWindow != nullptr)
        {
            // Create a command pool for present commands.
            result = m_device.CreateResource<CommandPool>(m_pPresentCommandPool, *m_pRenderSubmissionQueue);
            NES_GRAPHICS_RETURN_FAIL(m_device, result == EGraphicsResult::Success, false, "Failed to create a command pool for the swapchain!");

            // Create the swapchain.
            SwapchainDesc desc
            {
                .m_pWindow = pWindow,
                .m_pDeviceQueue = m_pRenderSubmissionQueue,
                .m_pCommandPool = m_pPresentCommandPool,
            };
            
            result = m_device.CreateResource<Swapchain>(m_pSwapchain, desc);
            NES_GRAPHICS_RETURN_FAIL(m_device, result == EGraphicsResult::Success, false, "Failed to create the swapchain!");
            
            // Create what is need to submit commands for each frame-in-flight.
            CreateFrameSubmissionResources(m_pSwapchain->GetMaxFramesInFlight());
        }

        // If no window, still create the frame resources.
        else
        {
            CreateFrameSubmissionResources(kHeadlessFramesInFlight);
        }

        // Create the Shader Library
        // [TODO]: Make the settings available for the RendererDesc.
        m_pShaderLibrary = std::make_unique<ShaderLibrary>(m_device);
        ShaderLibraryDesc shaderLibraryDesc{};
        shaderLibraryDesc.m_searchDirs.emplace_back(NES_SHADER_DIR);
        shaderLibraryDesc.m_compileOutDir = NES_SHADER_DIR;
        if (!m_pShaderLibrary->Init(shaderLibraryDesc))
        {
            NES_GRAPHICS_ERROR(m_device, "Failed to initialize shader library!");
            return false;
        }
        
        // [TODO]: Create default resources.

        return true;
    }

    void Renderer::Shutdown()
    {
        m_device.WaitUntilIdle();
        
        // [TODO]: Free default resources:

        if (m_pShaderLibrary)
        {
            m_pShaderLibrary->Shutdown();
            m_pShaderLibrary.reset();
        }

        // Free any remaining resources:
        for (auto& releaseQueue : m_resourceFreeQueues)
        {
            releaseQueue.Execute();    
        }
        m_resourceFreeQueues.clear();

        // Cleanup frame data:
        for (auto& frame : m_frames)
        {
            m_device.FreeResource(frame.m_pCommandBuffer);
            m_device.FreeResource(frame.m_pCommandPool);
        }
        vkDestroySemaphore(m_device, m_frameTimelineSemaphore, m_device.GetVulkanAllocationCallbacks());

        // Destroy the swapchain:
        if (m_pSwapchain != nullptr)
        {
            m_device.FreeResource(m_pSwapchain);
        }

        // Free the command pool.
        if (m_pPresentCommandPool != nullptr)
        {
            m_device.FreeResource(m_pPresentCommandPool);
        }
    }

    bool Renderer::BeginFrame()
    {
        // This is a non-headless version, so we must have a swapchain and window.
        NES_ASSERT(m_pWindow);
        NES_ASSERT(m_pSwapchain != nullptr);
        
        if (m_pSwapchain->NeedsRebuild())
        {
            const auto& desc = m_pWindow->GetDesc();
            const auto windowSize = desc.m_windowResolution;
            const bool vsyncEnabled = desc.m_vsyncEnabled;
            m_pSwapchain->OnResize(windowSize, vsyncEnabled);
        }

        // Acquire the next image to render to. If out of date (Needs rebuild) or an error occurred, return false.
        // No render commands should be made until the swapchain is rebuilt.
        if (m_pSwapchain->AcquireNextImage() != EGraphicsResult::Success)
            return false;

        // Free resources from the previous frame.
        ProcessResourceFreeQueue();

        // Prepare frame synchronization.
        PrepareFrameToSignal(m_pSwapchain->GetMaxFramesInFlight());

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
        ({
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .semaphore = m_pSwapchain->GetImageAvailableSemaphore(),
            .stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        });
        // Then add the signal for the swapchain to present once everything else is done.
        m_signalSemaphores.push_back
        ({
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .pNext = nullptr,
            .semaphore = m_pSwapchain->GetRenderFinishedSemaphore(),
            .stageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        });

        // Submit the current commands to the Render Queue.
        SubmitFrameCommands();

        // Present the frame:
        m_pSwapchain->PresentFrame();
        
        // Advance to the next frame:
        AdvanceToNextFrame(m_pSwapchain->GetMaxFramesInFlight());
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
        NES_ASSERT(m_pSwapchain != nullptr);
        m_pSwapchain->RequestRebuild();
    }

    void Renderer::ProcessResourceFreeQueue()
    {
        auto& freeQueue = m_resourceFreeQueues[m_currentFrameIndex];
        freeQueue.Execute();
    }

    void Renderer::BeginCommandRecording()
    {
        const auto& frame = m_frames[m_currentFrameIndex];
        frame.m_pCommandPool->Reset();
        frame.m_pCommandBuffer->Begin();
    }

    void Renderer::WaitForFrameCompletion()
    {
        // Wait until GPU has finished processing the frame that was using these resources previously (m_numFramesInFlight frames ago)
        const VkSemaphoreWaitInfo waitInfo
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .semaphoreCount = 1,
            .pSemaphores = &m_frameTimelineSemaphore,
            .pValues = &m_frames[m_currentFrameIndex].m_frameNumber,
        };
        vkWaitSemaphores(m_device, &waitInfo, std::numeric_limits<uint64_t>::max());
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
        VkSemaphoreTypeCreateInfo timelineCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext = nullptr,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue = initialValue,
        };
        const VkSemaphoreCreateInfo semaphoreCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &timelineCreateInfo,
        };
        NES_VK_MUST_PASS(m_device, vkCreateSemaphore(m_device, &semaphoreCreateInfo, m_device.GetVulkanAllocationCallbacks(), &m_frameTimelineSemaphore));
        m_device.SetDebugNameToTrivialObject(m_frameTimelineSemaphore, "Frame Timeline Semaphore");
        
        // Create a command pool and command buffer for each frame.
        // Each frame gets its own command pool to allow parallel command recording while previous frames may still be executing on the GPU.
        for (uint32 i = 0; i < numFramesInFlight; ++i)
        {
            // Track the index for synchronization.
            m_frames[i].m_frameNumber = i;
            NES_GRAPHICS_MUST_PASS(m_device, m_device.CreateResource(m_frames[i].m_pCommandPool, *m_pRenderSubmissionQueue));
            NES_GRAPHICS_MUST_PASS(m_device, m_frames[i].m_pCommandPool->CreateCommandBuffer(m_frames[i].m_pCommandBuffer));
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
        frame.m_pCommandBuffer->End();
        
        // Add timeline semaphore to signal when GPU completes this frame
        // The color attachment output stage is used since that's when the frame is fully rendered.
        m_signalSemaphores.push_back
        ({
            .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = m_frameTimelineSemaphore,
            .value     = frame.m_frameNumber,
            .stageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,  // Wait until everything is completed.
        });
        
        // Adding the command buffer of the frame to the list of command buffers to submit
        // Note: extra command buffers could have been added to the list from other parts of the application (elements)
        m_commandBuffers.push_back({ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .commandBuffer = *frame.m_pCommandBuffer} );

        const VkSubmitInfo2 submitInfo
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext = nullptr,
            .waitSemaphoreInfoCount = static_cast<uint32>(m_waitSemaphores.size()),
            .pWaitSemaphoreInfos = m_waitSemaphores.data(),
            .commandBufferInfoCount = static_cast<uint32>(m_commandBuffers.size()),
            .pCommandBufferInfos = m_commandBuffers.data(),
            .signalSemaphoreInfoCount = static_cast<uint32>(m_signalSemaphores.size()),
            .pSignalSemaphoreInfos = m_signalSemaphores.data(),
        };

        // [TODO]: Should be m_pRenderSubmissionQueue->Submit().
        // Submit the command buffer to the GPU and signal when it's done.
        NES_VK_MUST_PASS(m_device, vkQueueSubmit2(*m_pRenderSubmissionQueue, 1, &submitInfo, nullptr));
    }

    void Renderer::AdvanceToNextFrame(const uint32 numFramesInFlight)
    {
        m_currentFrameIndex = (m_currentFrameIndex + 1) % numFramesInFlight;
    }
}
