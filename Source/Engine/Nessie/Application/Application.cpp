// Application.cpp
#include "Application.h"
#include "BleachNew.h"

namespace nes
{
    static Application* s_pInstance = nullptr;
    static std::thread::id s_mainThreadId{};
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Application instance.
    //----------------------------------------------------------------------------------------------------
    Application& Application::Get()
    {
        NES_ASSERT(s_pInstance);
        return *s_pInstance;
    }

    bool Application::IsMainThread()
    {
        return std::this_thread::get_id() == s_mainThreadId; 
    }

    std::thread::id Application::GetMainThreadID()
    {
        return s_mainThreadId;
    }

    Application::Application(const CommandLineArgs& args)
        : m_commandLineArgs(args)
    {
        if (s_pInstance != nullptr)
        {
            NES_CRITICALV("Application", "Attempted to create a second Application instance!");
        }

        s_pInstance = this;
        s_mainThreadId = std::this_thread::get_id();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Application's Window.
    //----------------------------------------------------------------------------------------------------
    Window& Application::GetWindow()
    {
        return m_window;
    }
    
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Application's Window. (Const version)
    //----------------------------------------------------------------------------------------------------
    const Window& Application::GetWindow() const
    {
        return m_window;
    }
    
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Initialize the Application.
    ///		@returns : 
    //----------------------------------------------------------------------------------------------------
    Application::ExitCode Application::Init()
    {
        BLEACH_INIT_LEAK_DETECTOR();
        Logger::Init(NES_LOG_DIR);
        // [TODO]:
        // Logger::LoadCategories();

        WindowProperties windowProps{};
        windowProps.m_label = "TestApp";
        windowProps.m_extent = {.m_width = 1600, .m_height = 900 };

        // Create the Window:
        if (!m_window.Init(*this, windowProps))
        {
            NES_ERRORV("Application", "Failed to intiailize the Application! Failed to Initialize the Window!");
            return ExitCode::FatalError;
        }

        // Create the Renderer
        if (!m_renderer.Init(&m_window))
        {
            NES_ERRORV("Application", "Failed to intiailize the Application! Failed to initialize the Renderer!");
            return ExitCode::FatalError;
        }
        
        return ExitCode::Success;
    }

    Application::ExitCode Application::RunMainLoop()
    {
        m_timer.Start();

        while (!m_window.ShouldClose() && !m_closeRequested)
        {
            const double deltaTime = m_timer.Tick();
            m_timeSinceStartup += deltaTime;
            
            // [TODO]: Sync with the Render Thread.
            // [TODO]: Sync with the Resource Thread.
            
            m_renderer.BeginFrame();
            
            // App Update:
            ProcessAppEvents();
            Update(deltaTime);
            
            m_renderer.SubmitFrame();
            
            m_window.ProcessEvents();
        }

        return ExitCode::Success;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief :Immediately closes the Application. This should not be called manually-it is intended
    ///             to be called in the main function of the program after RunMainLoop(). Use Application::Quit
    ///             to safely close the Application.
    ///		@param exitCode : Exit code 
    //----------------------------------------------------------------------------------------------------
    void Application::Close([[maybe_unused]] ExitCode exitCode)
    {
        NES_ASSERTV(IsMainThread());
        
        m_renderer.Close();
        m_window.Close();
        Logger::Close();
        BLEACH_DUMP_AND_DESTROY_LEAK_DETECTOR();

        // Set the Instance back to nullptr.
        s_pInstance = nullptr;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Quit the Application. Note: This won't close the App immediately-it will wait until the
    ///              the current frame has finished.
    //----------------------------------------------------------------------------------------------------
    void Application::Quit()
    {
        m_closeRequested = true;
    }

    void Application::PushEvent([[maybe_unused]] Event& e)
    {
        // [TODO]: Push the Event to the Event Queue.
    }

    void Application::ProcessAppEvents()
    {
        // [TODO]: Process the Event Queue.
    }

}
