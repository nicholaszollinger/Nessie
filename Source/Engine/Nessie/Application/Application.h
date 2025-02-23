// Application.h
#pragma once
#include "Window.h"
#include "Core/Generic/Version.h"
#include "Core/Time/Timer.h"
#include "Debug/Assert.h"
#include "Graphics/Renderer.h"
#include "Input/InputManager.h"

namespace nes
{
    class Platform;
    class Window;
    class Event;

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Container for the arguments passed into the executable. The first argument will always be the executable's path.
    //-----------------------------------------------------------------------------------------------------------------------------
    struct CommandLineArgs
    {
        size_t count = 0;
        char** args = nullptr;

        const char* operator[](const size_t index) const
        {
            NES_ASSERT(index < count);
            return args[index];
        }

        const char* operator[](const int index) const
        {
            NES_ASSERT(static_cast<size_t>(index) < count);
            return args[index];
        }
    };

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Settings used to initialize the Application instance. 
    //----------------------------------------------------------------------------------------------------
    struct ApplicationProperties
    {
        CommandLineArgs m_commandLineArgs{};    // Command line arguments sent to the App Executable.
        std::string m_appName{};                // Application Name.
        Version m_appVersion{};                 // Application Version.
    };
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Create an Application instance. This is defined by the Client to create the Application
///              Type that they want. There can only be 1 Application instance at time.
///		@param args : Command Line Arguments passed into the executable.
///		@returns : Pointer to the new Application.
//----------------------------------------------------------------------------------------------------
extern nes::Application* CreateApplication(const nes::CommandLineArgs& args);

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Application runs the game loop and processes events.
    //----------------------------------------------------------------------------------------------------
    class Application
    {
    public:
        enum class ExitCode
        {
            Success = 0,
            FatalError = -1,
        };

    private:
        ApplicationProperties m_properties;
        Window m_window{};
        Renderer m_renderer{};
        InputManager m_inputManager{};
        Timer m_timer{};
        double m_timeSinceStartup = 0.f;
        bool m_closeRequested = false;

    public:
        explicit Application(const CommandLineArgs& args);
        virtual ~Application() = default;
        
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
        Application(Application&&) noexcept = delete;
        Application& operator=(Application&&) noexcept = delete;

    public:
        static Application& Get();
        static bool IsMainThread();
        static std::thread::id GetMainThreadID();

        // Main Loop.
        ExitCode Init();
        ExitCode RunMainLoop();
        void Close(ExitCode exitCode);
        
        void PushEvent(Event& e);
        void Quit();

        [[nodiscard]] Window& GetWindow();
        [[nodiscard]] const Window& GetWindow() const;
        [[nodiscard]] const Renderer& GetRenderer() const { return m_renderer; }

    private:
        //----------------------------------------------------------------------------------------------------
        //		NOTES:
        //      [Consider]: I'd like a better name difference between the Init() function and this. I'd want to
        //                  to change the Main init to something else and call this function Init().
        //		
        ///		@brief : Initialization function that can be overridden by derived classes. This is called at the end of
        ///             the public Init() function during startup.
        //----------------------------------------------------------------------------------------------------
        virtual bool PostInit() { return true; }
        
        //----------------------------------------------------------------------------------------------------
        ///		@brief : Runs a single frame of the Application.
        ///		@param deltaTime : Delta time since the last frame.
        //----------------------------------------------------------------------------------------------------
        virtual void Update(double deltaTime) = 0;
        void ProcessAppEvents();
    };
}