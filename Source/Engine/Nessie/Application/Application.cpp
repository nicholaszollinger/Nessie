// Application.cpp
#include "Application.h"
#include "Core/Memory/Memory.h"

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
            NES_FATAL(kApplicationLogTag, "Attempted to create a second Application instance!");
            //NES_FATAL("[Application]: Attempted to create a second Application instance!");
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
    ///	@brief : Initialize the Application.
    ///	@returns : 
    //----------------------------------------------------------------------------------------------------
    Application::EExitCode Application::Init()
    {
        NES_INIT_LEAK_DETECTOR();
        //Logger::Init(NES_LOG_DIR);
        LoggerRegistry::Instance().Internal_Init();
        
        const std::string logConfigDir = NES_CONFIG_DIR;
        //Logger::LoadCategories(std::string(logConfigDir + "LogConfig.yaml"));

        // Load the Application Settings:
        auto settingsFile = YAML::LoadFile(std::string(logConfigDir + "AppConfig.yaml"));
        if (!settingsFile)
            return EExitCode::FatalError;

        auto application = settingsFile["Application"];
        if (!application)
            return EExitCode::FatalError;

        m_properties.m_appName = application["Name"].as<std::string>();
        m_properties.m_appVersion.Deserialize(application["Version"]);

        // Load Window Properties
        auto window = settingsFile["Window"];
        if (!window)
            return EExitCode::FatalError;
        
        WindowProperties windowProperties;
        windowProperties.m_label = window["Label"].as<std::string>();
        const auto extent = window["Extent"].as<std::array<int, 2>>();
        windowProperties.m_extent.m_width = extent[0];
        windowProperties.m_extent.m_height = extent[1];
        windowProperties.m_windowMode = static_cast<EWindowMode>(window["Mode"].as<int>());
        windowProperties.m_isResizable = window["IsResizable"].as<bool>();
        windowProperties.m_vsyncEnabled = window["VsyncEnabled"].as<bool>();
        
        // Create the Window:
        if (!m_window.Init(*this, windowProperties))
        {
            NES_ERROR(kApplicationLogTag, "Failed to initialize the Application! Failed to Initialize the Window!");
            return EExitCode::FatalError;
        }

        // Initialize the InputManager
        if (!m_inputManager.Init(&m_window))
        {
            NES_ERROR(kApplicationLogTag, "Failed to initialize the Application! Failed to Initialize InputManager!");
            return EExitCode::FatalError;
        }
        
        // Create the Renderer
        if (!m_renderer.Init(&m_window, m_properties))
        {
            NES_ERROR(kApplicationLogTag, "Failed to initialize the Application! Failed to initialize the Renderer!");
            return EExitCode::FatalError;
        }
        
        // Scene Manager
        if (!m_sceneManager.Init(settingsFile))
        {
            NES_ERROR(kApplicationLogTag, "Failed to initialize the Application! Failed to initialize the SceneManager!");
            return EExitCode::FatalError;
        }
        
        NES_LOG(kApplicationLogTag, "Initialized App: \"{}\" Version: ", m_properties.m_appName, m_properties.m_appVersion.ToString());
        return EExitCode::Success;
    }

    Application::EExitCode Application::RunMainLoop()
    {
        m_timer.Start();

        while (!m_window.ShouldClose() && !m_closeRequested)
        {
            const double deltaTime = m_timer.Tick<Timer::Seconds>();
            m_timeSinceStartup += deltaTime;
            
            // [TODO]: Sync with the Render Thread.
            // [TODO]: Sync with the Resource Thread.
        
            // Update:
            ProcessAppEvents();
            m_inputManager.Update(deltaTime);
            m_sceneManager.Update(deltaTime);
        
            // Submitting to Renderer:
            if (m_renderer.BeginFrame())
            {
                m_sceneManager.PreRender();
                m_sceneManager.Render();
                
                m_renderer.EndFrame();
            }
            
            m_window.ProcessEvents();
        }

        return EExitCode::Success;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief :Immediately closes the Application. This should not be called manually-it is intended
    ///             to be called in the main function of the program after RunMainLoop(). Use Application::Quit
    ///             to safely close the Application.
    ///		@param exitCode : Exit code 
    //----------------------------------------------------------------------------------------------------
    void Application::Close([[maybe_unused]] EExitCode exitCode)
    {
        NES_ASSERT(IsMainThread());

        m_renderer.WaitUntilIdle();

        m_sceneManager.Shutdown();
        m_renderer.Shutdown();
        m_inputManager.Shutdown();
        m_window.Close();

        NES_LOG(kApplicationLogTag, "Application Closed");
        LoggerRegistry::Instance().Internal_Shutdown();
        NES_DUMP_AND_DESTROY_LEAK_DETECTOR();

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

    void Application::PushEvent(Event& e)
    {
        m_sceneManager.OnEvent(e);
    }

    void Application::ProcessAppEvents()
    {
        // [TODO]: Process the Event Queue.
    }

}
