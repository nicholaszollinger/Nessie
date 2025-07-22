// Renderer.cpp
#include "Renderer.h"
#include "Nessie/Application/Application.h"
#include "Nessie/Graphics/Swapchain.h"

namespace nes
{
    static Renderer* g_pRenderer = nullptr;
    static Timer     g_renderTimer{};

    Renderer* Renderer::Get()
    {
        NES_ASSERT(g_pRenderer != nullptr);
        return g_pRenderer;
    }

    Renderer::Renderer()
    {
        NES_ASSERT(g_pRenderer == nullptr);
        g_pRenderer = this;
    }

    Renderer::~Renderer()
    {
        NES_ASSERT(g_pRenderer == this);
        g_pRenderer = nullptr;
    }

    bool Renderer::IsRenderThread()
    {
        return Get()->m_renderThread.GetThreadId() == std::this_thread::get_id();
    }

    RenderCommandQueue& Renderer::GetRenderResourceReleaseQueue(const uint32 index)
    {
        NES_ASSERT(false);
        //NES_ASSERT(index < GetConfig().m_framesInFlight);
        return Get()->m_resourceFreeQueues[index];
    }

    uint32 Renderer::RT_GetCurrentFrameIndex()
    {
        // Swapchain owns the render thread frame index.
        NES_ASSERT(false);
        return 0;
        //return g_pRenderer->m_pSwapchain->GetCurrentFrameIndex();
    }

    bool Renderer::Init(const RendererDesc& config)
    {
        m_desc = config;

        m_resourceFreeQueues = NES_NEW_ARRAY(RenderCommandQueue, 3);
        m_commandQueues[0] = NES_NEW(RenderCommandQueue());
        m_commandQueues[1] = NES_NEW(RenderCommandQueue());
        
        // Start the Render Thread.
        m_renderThread.Start([this](const ERenderThreadInstruction instruction)
        {
            return RT_ProcessInstruction(instruction);
        }, "Render Thread");
        
        // [TODO]: Create default resources.

        // if (!m_pAPI->Init())
        // {
        //     NES_ERROR(kRendererLogTag, "Failed to initialize Renderer! Failed to initialize Renderer API!");
        //     return false;
        // }

        return true;
    }

    void Renderer::Shutdown()
    {
        RenderSingleFrame();
        m_renderThread.Terminate();

        // [TODO]: Free default resources:

        // [TODO]: 
        // Free any remaining resources:
        // for (uint32 i = 0; i < m_desc.m_framesInFlight; ++i)
        // {
        //     auto& queue = GetRenderResourceReleaseQueue(i);
        //     queue.Execute();
        // }
        
        // Delete the command queues.
        NES_DELETE(m_commandQueues[0]);
        NES_DELETE(m_commandQueues[1]);
        NES_DELETE_ARRAY(m_resourceFreeQueues);
    }

    void Renderer::BeginFrame()
    {
        // [TODO]: Swapchain 
    }

    void Renderer::EndFrame()
    {
        // [TODO]: 
    }

    void Renderer::UpdateSwapchain(const uint32 /*width*/, const uint32 /*height*/, const bool /*vsyncEnabled*/)
    {
        //NES_ASSERT(m_pSwapchain != nullptr);
        //m_pSwapchain->OnResize(width, height, vsyncEnabled);
    }

    void Renderer::SwapCommandQueues()
    {
        m_commandQueueWriteIndex ^= 1;
    }

    RenderCommandQueue& Renderer::GetRenderCommandSubmissionQueue() const
    {
        return *m_commandQueues[m_commandQueueWriteIndex];
    }

    RenderCommandQueue& Renderer::GetRenderCommandExecuteQueue() const 
    {
        return *m_commandQueues[m_commandQueueWriteIndex ^ 1];
    }

    bool Renderer::RT_ProcessInstruction(const ERenderThreadInstruction instruction)
    {
        switch (instruction)
        {
            case ERenderThreadInstruction::Init: return true;
            case ERenderThreadInstruction::RenderPreviousFrame: return RT_RenderPreviousFrame();
            case ERenderThreadInstruction::Terminate: break;
        }
        
        return false;
    }

    bool Renderer::RT_RenderPreviousFrame()
    {
        // Save how long we waited.
        NES_IF_NOT_RELEASE(m_renderThreadWaitTime = static_cast<float>(g_renderTimer.Stop<Timer::Milliseconds>()));

        // Execute the commands for the previous frame.
        {
            NES_IF_NOT_RELEASE(g_renderTimer.Start());
            GetRenderCommandExecuteQueue().Execute();
            NES_IF_NOT_RELEASE(m_renderThreadWorkTime = static_cast<float>(g_renderTimer.Stop<Timer::Milliseconds>()));
        }

        // Start the wait timer.
        NES_IF_NOT_RELEASE(g_renderTimer.Start());
        return true;
    }
    
    void Renderer::RenderSingleFrame()
    {
        SwapCommandQueues();
        SignalRenderThread();
        WaitUntilRenderComplete();
    }

    void Renderer::SignalRenderThread()
    {
        if (m_desc.m_threadPolicy == EThreadPolicy::Multithreaded)
        {
            // Signal the render thread to begin rendering the previous frame.
            m_renderThread.SendInstruction(ERenderThreadInstruction::RenderPreviousFrame);
        }

        else
        {
            // Otherwise, render immediately.
            RT_RenderPreviousFrame();
        }
    }
    
    void Renderer::WaitUntilRenderComplete()
    {
        if (m_desc.m_threadPolicy == EThreadPolicy::Multithreaded)
        {
            m_renderThread.WaitUntilDone();
        }

        // Do nothing in single-threaded mode.
    }
}
