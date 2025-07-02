// Application.cpp
#include "Application.h"
#include "Core/Memory/Memory.h"

namespace nes
{
    /// Static instance set in the Application's constructor.
    static Application* g_pApplication = nullptr;
    
    /// ID of the thread that the Application is running on. 
    static std::thread::id s_mainThreadId{};

    Application::Application(ApplicationProperties&& appProps, const WindowProperties& windowProps)
        : m_properties(std::move(appProps))
    {
        // Set static variables.
        NES_ASSERT(g_pApplication == nullptr);
        g_pApplication = this;
        s_mainThreadId = std::this_thread::get_id();

        // Create the Window:
        m_pWindow = NES_NEW(ApplicationWindow());
        if (!m_pWindow->Internal_Init(*this, windowProps))
        {
            NES_FATAL("Failed to create window!");
        }

        // Initialize the Input Manager.
        if (!m_inputManager.Init(m_pWindow))
        {
            NES_FATAL("Failed to initialize input manager!");
        }
    }

    Application& Application::Get()
    {
        NES_ASSERT(g_pApplication != nullptr);
        return *g_pApplication;
    }

    ApplicationWindow& Application::GetWindow()
    {
        NES_ASSERT(m_pWindow != nullptr);
        return *m_pWindow;
    }

    const ApplicationWindow& Application::GetWindow() const
    {
        NES_ASSERT(m_pWindow != nullptr);
        return *m_pWindow;
    }

    bool Application::IsMainThread()
    {
        return s_mainThreadId == std::this_thread::get_id();
    }

    std::thread::id Application::GetMainThreadID()
    {
        return s_mainThreadId;
    }

    void Application::Internal_RunMainLoop()
    {
        m_timer.Start();
        
        while (!m_pWindow->ShouldClose() && !m_shouldQuit)
        {
            // Update Frame Time:
            const double deltaTime = m_timer.Tick<Timer::Seconds>();
            m_timeSinceStartup += deltaTime;
            m_lastFrameTime = deltaTime * 1000.f;
            m_fps = 1.f / static_cast<float>(deltaTime);
            
            OnFrameBegin();
            {
                // Update input state.
                m_inputManager.Update(deltaTime);
                
                // Run the Application frame.
                RunFrame(deltaTime);
                
                // Process Window events:
                m_pWindow->Internal_ProcessEvents();
            }
            OnFrameEnd();
        }

        Shutdown();
    }

    void Application::Shutdown()
    {
        OnAppShutdown();

        // Shutdown the input manager.
        m_inputManager.Shutdown();

        // Close the window.
        if (m_pWindow)
        {
            m_pWindow->Close();
            NES_SAFE_DELETE(m_pWindow);
        }

        // Release the instance.
        g_pApplication = nullptr;
    }
}
