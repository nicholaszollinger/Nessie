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
    {
        if (s_pInstance != nullptr)
        {
            NES_CRITICALV("Application", "Attempted to create a second Application instance!");
        }

        s_pInstance = this;
        s_mainThreadId = std::this_thread::get_id();
        m_properties.m_commandLineArgs = args;
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
        
        const std::string logConfigDir = NES_CONFIG_DIR;
        Logger::LoadCategories(std::string(logConfigDir + "LogConfig.yaml"));

        // Load the Application Settings:
        auto settingsFile = YAML::LoadFile(std::string(logConfigDir + "AppConfig.yaml"));
        if (!settingsFile)
            return ExitCode::FatalError;

        auto application = settingsFile["Application"];
        if (!application)
            return ExitCode::FatalError;

        m_properties.m_appName = application["Name"].as<std::string>();
        m_properties.m_appVersion.Deserialize(application["Version"]);

        // Load Window Properties
        auto window = settingsFile["Window"];
        if (!window)
            return ExitCode::FatalError;
        
        WindowProperties windowProperties;
        windowProperties.m_label = window["Label"].as<std::string>();
        const auto extent = window["Extent"].as<std::array<int, 2>>();
        windowProperties.m_extent.m_width = extent[0];
        windowProperties.m_extent.m_height = extent[1];
        windowProperties.m_windowMode = static_cast<WindowMode>(window["Mode"].as<int>());
        windowProperties.m_isResizable = window["IsResizable"].as<bool>();
        windowProperties.m_vsyncEnabled = window["VsyncEnabled"].as<bool>();
        
        // Create the Window:
        if (!m_window.Init(*this, windowProperties))
        {
            NES_ERRORV("Application", "Failed to initialize the Application! Failed to Initialize the Window!");
            return ExitCode::FatalError;
        }

        // Initialize the InputManager
        if (!m_inputManager.Init(&m_window))
        {
            NES_ERRORV("Application", "Failed to initialize the Application! Failed to Initialize InputManager!");
            return ExitCode::FatalError;
        }

        // Create the Renderer
        if (!m_renderer.Init(&m_window))
        {
            NES_ERRORV("Application", "Failed to initialize the Application! Failed to initialize the Renderer!");
            return ExitCode::FatalError;
        }

        // Scene Manager
        if (!m_sceneManager.Init(settingsFile))
        {
            NES_ERRORV("Application", "Failed to initialize the Application! Failed to initialize the SceneManager!");
            return ExitCode::FatalError;
        }
        
        NES_LOGV("Application", "Initialized App: \"", m_properties.m_appName, "\" Version: ", m_properties.m_appVersion.ToString());
        return ExitCode::Success;
    }

    Application::ExitCode Application::RunMainLoop()
    {
        m_timer.Start();

        while (!m_window.ShouldClose() && !m_closeRequested)
        {
            const double deltaTime = m_timer.Tick();
            m_timeSinceStartup += deltaTime;
            
            m_inputManager.Update(deltaTime);
            
            // [TODO]: Sync with the Render Thread.
            // [TODO]: Sync with the Resource Thread.
            
            m_renderer.BeginFrame();
            {
                // App Update:
                ProcessAppEvents();
                m_sceneManager.Update(deltaTime);
                
                m_renderer.SubmitFrame();
            }
            
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

        m_sceneManager.Close();
        m_renderer.Close();
        m_inputManager.Shutdown();
        m_window.Close();

        NES_LOGV("Application", "Application Closed");
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
        m_sceneManager.OnEvent(e);
    }

    void Application::ProcessAppEvents()
    {
        // [TODO]: Process the Event Queue.
    }

}
