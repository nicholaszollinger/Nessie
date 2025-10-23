// Application.cpp
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
    static Application* g_pInstance = nullptr;

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

    Application::Application(ApplicationDesc&& desc, WindowDesc&& windowDesc, RendererDesc&& rendererDesc)
        : m_desc(std::move(desc))
    {
        // Set instance variables
        NES_ASSERT(g_pInstance == nullptr);
        g_pInstance = this;
    
        s_mainThreadId = std::this_thread::get_id();
        thread::SetThreadName("Main Thread");

        // Start the application:
        OnStartup(std::move(windowDesc), std::move(rendererDesc));
    }

    Application::~Application()
    {
        NES_ASSERT(g_pInstance == this);
        g_pInstance = nullptr;
    }

    Application& Application::Get()
    {
        NES_ASSERT(g_pInstance != nullptr);
        return *g_pInstance;
    }

    std::thread::id Application::GetMainThreadID()
    {
        return s_mainThreadId;
    }

    bool Application::IsMainThread()
    {
        return s_mainThreadId == std::this_thread::get_id();
    }

    DeviceManager& Application::GetDeviceManager()
    {
        const auto& instance = Get();
        NES_ASSERT(instance.m_pDeviceManager != nullptr);
        return *instance.m_pDeviceManager;
    }

    AssetManager& Application::GetAssetManager()
    {
        const auto& instance = Get();
        NES_ASSERT(instance.m_pAssetManager != nullptr);
        return *instance.m_pAssetManager;
    }
    
    bool Application::Internal_Init()
    {
        // If this is true, there was an error during startup:
        if (m_shouldQuit)
            return false;
        
        return Init();
    }

    void Application::Internal_RunMainLoop()
    {
        // Set the initial frame time.
        m_timer.Start();
        UpdateFrameTime();
        
        // Headless loop: iterates through m_headlessFrameCount number of frames, then exits.
        if (m_desc.m_isHeadless)
        {
            RunHeadlessLoop();
            return;
        }

        while (!m_shouldQuit && !m_pWindow->ShouldClose())
        {
            // Thread Sync.
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
                Update(m_timeStep);
            
                // Begin a Render Frame:
                // If false, then there was an error, or the swapchain needs to be rebuilt (out of date).
                // Skip this render frame.
                if (m_pRenderer->BeginFrame())
                {
                    // Render the frame.
                    Render(m_pRenderer->GetCurrentCommandBuffer(), m_pRenderer->GetRenderFrameContext());

                    // Stop recording render commands.
                    m_pRenderer->EndFrame();
                }
            }

            // Update time step values.
            UpdateFrameTime();
        }
    }

    void Application::Internal_OnInputEvent(Event& event)
    {
        // The Window calls this function, so the input manager should be valid.
        NES_ASSERT(m_pInputManager != nullptr);
        
        // Update the input manager.
        m_pInputManager->OnInputEvent(event);

        // [TODO]: When I get InputActions working, I shouldn't need to
        // send these directly to the application. I should be able to just it all
        // handled by the input manager.
        // Send the event to the application.
        PushEvent(event);
    }

    void Application::Internal_OnWindowResize(const uint32 width, const uint32 height)
    {
        // Rebuild the swap chain if necessary.
        if (!m_desc.m_isHeadless)
        {
            // Update the Renderer:
            m_pRenderer->RequestSwapchainRebuild();

            // [TODO]: This needs to be passed to the world.
            //Internal_OnResize(width, height);

            // Push the resize event to the Application:
            WindowResizeEvent event(width, height);
            PushEvent(event);
        }
    }

    ApplicationWindow& Application::GetWindow()
    {
        const auto& instance = Get();
        NES_ASSERT(instance.m_pWindow != nullptr);
        return *instance.m_pWindow;
    }

    const ApplicationWindow& Application::GetWindow() const
    {
        const auto& instance = Get();
        NES_ASSERT(instance.m_pWindow != nullptr);
        return *instance.m_pWindow;
    }

    const AppPerformanceInfo& Application::GetPerformanceInfo() const
    {
        return m_performanceInfo;
    }

    void Application::OnStartup(WindowDesc&& windowDesc, RendererDesc&& rendererDesc)
    {
        // Initialize the Device Manager.
        m_pDeviceManager = std::make_unique<DeviceManager>();
        if (!m_pDeviceManager->Init())
        {
            NES_ERROR("Failed to initialize the DeviceManager!");
            m_shouldQuit = true;
            return;
        }

        // Create the Asset Manager
        m_pAssetManager = std::make_unique<AssetManager>();
        if (!m_pAssetManager->Init())
        {
            NES_ERROR("Failed to initialize the AssetManager!");
            m_shouldQuit = true;
            return;
        }
        
        // Create the Render Device.
        if (!m_pDeviceManager->CreateRenderDevice(m_desc, rendererDesc))
        {
            NES_ERROR("Failed to create render device!");
            m_shouldQuit = true;
            return;
        }

        // Create the Window
        if (m_desc.m_isHeadless)
            m_pWindow = std::make_unique<HeadlessWindow>();
        else
            m_pWindow = std::make_unique<ApplicationWindow>();

        // Initialize the window.
        if (!m_pWindow->Internal_Init(*this, std::move(windowDesc)))
        {
            NES_ERROR("Failed to initialize application window!");
            m_shouldQuit = true;
            return;
        }

        // Initialize the Input Manager
        m_pInputManager = std::make_unique<InputManager>();
        if (!m_pInputManager->Init(m_pWindow.get()))
        {
            NES_ERROR("Failed to initialize input manager!");
            m_shouldQuit = true;
            return;
        }

        // Initialize the Renderer.
        m_pRenderer = std::make_unique<Renderer>(DeviceManager::GetRenderDevice());
        ApplicationWindow* pRenderWindow = m_desc.m_isHeadless? nullptr : m_pWindow.get(); 
        if (!m_pRenderer->Init(pRenderWindow, std::move(rendererDesc)))
        {
            NES_ERROR("Failed to initialize the renderer!");
            m_shouldQuit = true;
        }
    }

    void Application::Internal_Shutdown()
    {
        // Allow the Application to respond.
        PreShutdown();
        
        if (m_pRenderer)
        {
            m_pRenderer->WaitUntilAllFramesCompleted();
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

        NES_LOG("Closed {} successfully.", m_desc.m_appName);
    }

    void Application::UpdateFrameTime()
    {
        const double deltaTimeMs = m_timer.Tick<Timer::Milliseconds>();
        m_timeStep = math::Max(static_cast<float>(deltaTimeMs), m_minTimeStepMs) / 1000.f;
        m_performanceInfo.m_timeSinceStartup += deltaTimeMs / 1000.f;
        m_performanceInfo.m_lastFrameTime = deltaTimeMs;
        m_performanceInfo.m_fps = 1.f / static_cast<float>(deltaTimeMs) / 1000.f;
    }

    void Application::SyncFrame()
    {
        // Sync the Render Frame.
        {
            NES_SCOPED_TIMER_MEMBER(m_performanceInfo.m_mainThreadWaitTime, Timer::Milliseconds);
            m_pRenderer->WaitForFrameCompletion();
        }
        
        // Sync with the asset thread.
        if (m_pAssetManager != nullptr)
        {
            m_pAssetManager->SyncFrame();
        }
    }

    void Application::RunHeadlessLoop()
    {
        const uint32 numFrames = m_desc.m_headlessFrameCount;
        
        for (uint32 frameID = 0; frameID < numFrames && !m_shouldQuit; ++frameID)
        {
            // Synchronize the frame.
            SyncFrame();
            
            // Begin Render frame:
            m_pRenderer->BeginHeadlessFrame();
            
            // App Frame:
            Update(m_timeStep);

            // End Render Frame:
            m_pRenderer->EndHeadlessFrame();

            // Update time step values.
            UpdateFrameTime();
        }
    }
}
