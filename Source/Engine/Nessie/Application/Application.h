// Application.h
#pragma once
#include "Window.h"
#include "Core/Generic/Version.h"
#include "Core/Time/Timer.h"
#include "Debug/Assert.h"
#include "Graphics/Renderer.h"
#include "Input/InputManager.h"
#include "Scene/SceneManager.h"

namespace nes
{
    NES_DEFINE_LOG_TAG(kApplicationLogTag, "Application", Info);
    
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
        enum class EExitCode
        {
            Success = 0,
            FatalError = -1,
        };

    private:
        ApplicationProperties m_properties;
        Window m_window{};
        Renderer m_renderer{};
        InputManager m_inputManager{};
        SceneManager m_sceneManager{};
        Timer m_timer{};
        double m_timeSinceStartup = 0.f;
        bool m_closeRequested = false;

    public:
        explicit Application(const CommandLineArgs& args);
        
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
        Application(Application&&) noexcept = delete;
        Application& operator=(Application&&) noexcept = delete;

    public:
        static Application& Get();
        static bool IsMainThread();
        static std::thread::id GetMainThreadID();

        // Main Loop.
        EExitCode Init();
        EExitCode RunMainLoop();
        void Close(EExitCode exitCode);
        
        void PushEvent(Event& e);
        void Quit();

        [[nodiscard]] Window&             GetWindow();
        [[nodiscard]] const Window&       GetWindow() const;
        [[nodiscard]] const Renderer&     GetRenderer() const       { return m_renderer; }
        [[nodiscard]] const InputManager& GetInputManager() const   { return m_inputManager; }
        [[nodiscard]] SceneManager&       GetSceneManager()         { return m_sceneManager; }

    private:
        void ProcessAppEvents();
    };
}