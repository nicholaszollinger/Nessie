// Platform.cpp
#include "Platform.h"

#include "Application.h"
#include "HeadlessWindow.h"
#include "Nessie/Debug/Assert.h"
#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Input/InputManager.h"
#include "Nessie/Graphics/Renderer.h"
#include "Nessie/Application/ApplicationDesc.h"
#include "Nessie/Asset/AssetManager.h"
#include "Nessie/Core/ScopeExit.h"
#include "Nessie/Core/Time/ScopedTimer.h"

namespace nes
{
    /// Static instance of the platform.
    static Platform* g_pInstance = nullptr;

    /// ID of the thread that the Application is running on. 
    static std::thread::id s_mainThreadId{};

    const char* CommandLineArgs::operator[](const size_t index) const
    {
        NES_ASSERT(index < m_count);
        return m_args[index];
    }

    const char* CommandLineArgs::operator[](const int index) const
    {
        NES_ASSERT(index < static_cast<int>(m_count));
        return m_args[index];
    }

    Platform::Platform()
    {
        NES_ASSERT(g_pInstance == nullptr);
        g_pInstance = this;
    
        s_mainThreadId = std::this_thread::get_id();
        thread::SetThreadName("Main Thread");
    }

    Platform::~Platform()
    {
        NES_ASSERT(g_pInstance == this);
        g_pInstance = nullptr;
    }

    Application& Platform::GetApplication()
    {
        const auto& instance = GetInstance();
        NES_ASSERT(instance.m_pApp != nullptr);
        return *instance.m_pApp;
    }

    ApplicationWindow& Platform::GetWindow()
    {
        const auto& instance = GetInstance();
        NES_ASSERT(instance.m_pWindow != nullptr);
        return *instance.m_pWindow;
    }

    DeviceManager& Platform::GetDeviceManager()
    {
        const auto& instance = GetInstance();
        NES_ASSERT(instance.m_pDeviceManager != nullptr);
        return *instance.m_pDeviceManager;
    }

    AssetManager& Platform::GetAssetManager()
    {
        const auto& instance = GetInstance();
        NES_ASSERT(instance.m_pAssetManager != nullptr);
        return *instance.m_pAssetManager;
    }

    const AppPerformanceInfo& Platform::GetAppPerformanceInfo()
    {
        NES_ASSERT(g_pInstance != nullptr);
        return g_pInstance->m_performanceInfo;
    }

    std::thread::id Platform::GetMainThreadID()
    {
        return s_mainThreadId;
    }

    bool Platform::IsMainThread()
    {
        return s_mainThreadId == std::this_thread::get_id();
    }

    bool Platform::Init(const CommandLineArgs& args)
    {
        // Initialize the Device Manager.
        m_pDeviceManager = std::make_unique<DeviceManager>();
        if (!m_pDeviceManager->Init())
        {
            NES_ERROR("Failed to initialize the device-manager!");
            return false;
        }

        // Create the Asset Manager
        m_pAssetManager = std::make_unique<AssetManager>();
        if (!m_pAssetManager->Init())
        {
            NES_ERROR("Failed to initialize the AssetManager!");
            return false;
        }

        // Create the Application Description
        // [TODO]: Potentially combine these into one.
        ApplicationDesc appDesc(args);
        WindowDesc windowDesc{};
        RendererDesc rendererDesc{};
        m_pApp = CreateApplication(appDesc, windowDesc, rendererDesc);
        if (!m_pApp)
        {
            NES_ERROR("Failed to create application description!");
            return false;
        }

        // Save the min frame time for the main loop.
        m_minTimeStepMs = appDesc.m_minTimeStepMs;
        
        // Create the Render Device.
        if (!m_pDeviceManager->CreateRenderDevice(appDesc, rendererDesc))
        {
            NES_ERROR("Failed to create render device!");
            return false;
        }

        // Create the Window
        if (appDesc.m_isHeadless)
            m_pWindow = std::make_unique<HeadlessWindow>();
        else
            m_pWindow = std::make_unique<ApplicationWindow>();
        
        // Initialize the window.
        if (!m_pWindow->Internal_Init(*this, windowDesc))
        {
            NES_ERROR("Failed to initialize application window!");
            return false;
        }
        
        // Initialize the Input Manager
        m_pInputManager = std::make_unique<InputManager>();
        if (!m_pInputManager->Init(m_pWindow.get()))
        {
            NES_ERROR("Failed to initialize input manager!");
            return false;
        }

        // Initialize the Renderer.
        m_pRenderer = std::make_unique<Renderer>(m_pDeviceManager->GetRenderDevice());
        ApplicationWindow* pRenderWindow = appDesc.m_isHeadless? nullptr : m_pWindow.get(); 
        if (!m_pRenderer->Init(pRenderWindow, rendererDesc))
        {
            NES_ERROR("Failed to initialize the renderer!");
            return false;
        }
    
        // Initialize the App.
        if (!m_pApp->Internal_AppInit())
        {
            NES_ERROR("Failed to initialize application!");
            return false;
        }

        // [TODO]: Render a single frame?
        
        return true;
    }

    void Platform::RunMainLoop()
    {
        // Headless loop: iterates through m_headlessFrameCount number of frames, then exits.
        if (m_pApp->GetDesc().m_isHeadless)
        {
            RunHeadlessLoop();
            return;
        }

        // Main loop, with a window.
        m_timer.Start();
    
        while (!m_pWindow->ShouldClose() && !m_pApp->ShouldQuit())
        {
            // Wait for the Render frame to finish.
            SyncFrame();
            
            // Process window events.
            m_pWindow->Internal_ProcessEvents();

            // Skip loop if minimized.
            if (m_pWindow->IsMinimized())
            {
                // Keep time steps reasonable.
                UpdateFrameTime();
                continue;
            }

            // Main thread Update:
            {
                NES_SCOPED_TIMER_MEMBER(m_performanceInfo.m_mainThreadWorkTime, Timer::Milliseconds);

                // Update input state.
                m_pInputManager->Update(m_timeStep);

                // Update the Application frame.
                m_pApp->Internal_AppUpdate(m_timeStep);
            
                // Begin a Render Frame:
                // If false, then there was an error, or the swapchain needs to be rebuilt (out of date).
                // Skip this render frame.
                if (m_pRenderer->BeginFrame())
                {
                    // Render the frame.
                    m_pApp->Internal_AppRender(m_pRenderer->GetCurrentCommandBuffer(), m_pRenderer->GetRenderFrameContext());

                    // Stop recording render commands.
                    m_pRenderer->EndFrame();
                }
            }

            // Update time step values.
            UpdateFrameTime();
        }
    }

    void Platform::Shutdown()
    {
        if (m_pRenderer)
        {
            m_pRenderer->WaitUntilAllFramesCompleted();
        }
        
        // Shutdown the app.
        if (m_pApp != nullptr)
        {
            m_pApp->Internal_AppShutdown();
            m_pApp.reset();
        }
        
        // Shutdown the renderer.
        if (m_pRenderer != nullptr)
        {
            m_pRenderer->Shutdown();
            m_pRenderer.reset();
        }

        // Shutdown the input manager.
        if (m_pInputManager != nullptr)
        {
            m_pInputManager->Shutdown();
            m_pInputManager.reset();
        }

        // Close the Window.
        if (m_pWindow != nullptr)
        {
            m_pWindow->Internal_Shutdown();
            m_pWindow.reset();
        }

        // Shutdown the AssetManager
        // - Must be done before the Render Device is destroyed to ensure that
        //   any graphics remaining graphics resources can be destroyed properly.
        if (m_pAssetManager != nullptr)
        {
            m_pAssetManager->Shutdown();
            m_pAssetManager.reset();
        }
        
        // Shutdown the Device Manager.
        if (m_pDeviceManager != nullptr)
        {
            m_pDeviceManager->Shutdown();
            m_pDeviceManager.reset();
        }
        
        NES_LOG("Platform Shutdown Success.");
    }

    void Platform::OnInputEvent(Event& event) const
    {
        // The Window calls this function, so the input manager should be valid.
        NES_ASSERT(m_pInputManager != nullptr);
        
        // Update the input manager.
        m_pInputManager->OnInputEvent(event);

        // [TODO]: When I get InputActions working, I shouldn't need to
        // send these directly to the application. I should be able to just it all
        // handled by the input manager.
        // Send the event to the application.
        if (m_pApp)
            m_pApp->PushEvent(event);
    }

    void Platform::OnWindowResize(const uint32 width, const uint32 height) const
    {
        // Rebuild the swap chain if necessary.
        if (!m_pApp->GetDesc().m_isHeadless)
        {
            // Update the Renderer:
            m_pRenderer->RequestSwapchainRebuild();

            m_pApp->Internal_OnResize(width, height);

            // Push the resize event to the Application:
            WindowResizeEvent event(width, height);
            m_pApp->PushEvent(event);
        }
    }

    void Platform::SyncFrame()
    {
        // Sync with the Render thread.
        {
            NES_SCOPED_TIMER_MEMBER(m_performanceInfo.m_mainThreadWaitTime, Timer::Milliseconds);
            m_pRenderer->WaitForFrameCompletion();
        }
        
        // Update render thread timings while synced.
        m_performanceInfo.m_renderThreadWaitTime = m_pRenderer->GetRenderThreadWaitTime();
        m_performanceInfo.m_renderThreadWorkTime = m_pRenderer->GetRenderThreadWorkTime();

        // Sync with the asset thread.
        if (m_pAssetManager != nullptr)
        {
            m_pAssetManager->SyncFrame();
        }
    }

    void Platform::RunHeadlessLoop()
    {
        const auto& appDesc = m_pApp->GetDesc();
        const uint32 numFrames = appDesc.m_headlessFrameCount;

        m_timer.Start();
        
        for (uint32 frameID = 0; frameID < numFrames && !m_pApp->ShouldQuit(); ++frameID)
        {
            // Wait for the Render frame to finish.
            SyncFrame();
            
            // Begin Render frame:
            m_pRenderer->BeginHeadlessFrame();
            
            // App Frame:
            m_pApp->Internal_AppUpdate(m_timeStep);

            // End Render Frame:
            m_pRenderer->EndHeadlessFrame();

            // Update time step values.
            UpdateFrameTime();
        }
    }

    void Platform::UpdateFrameTime()
    {
        const double deltaTimeMs = m_timer.Tick<Timer::Milliseconds>();
        m_timeStep = math::Min(static_cast<float>(deltaTimeMs), m_minTimeStepMs) / 1000.f;
        m_performanceInfo.m_timeSinceStartup += deltaTimeMs / 1000.f;
        m_performanceInfo.m_lastFrameTime = deltaTimeMs;
        m_performanceInfo.m_fps = 1.f / static_cast<float>(deltaTimeMs) / 1000.f;
    }

    Platform& Platform::GetInstance()
    {
        NES_ASSERT(g_pInstance != nullptr);
        return *g_pInstance;
    }
}
