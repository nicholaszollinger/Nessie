#pragma once
// Platform.h
#include <memory>
#include <string>
#include <vector>
#include "Window.h"
#include "Debug/Assert.h"
#include "Core/Events/Event.h"
#include "Core/Time/Timer.h"

namespace nes
{
    class Window;
    class Application;
    class InputEvent;

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
    ///		@brief : The Platform is basically the backend for the Application. It creates the Window,
    ///             sets up the Logger, and runs the Application.
    //----------------------------------------------------------------------------------------------------
    class Platform
    {
    public:
        enum class ExitCode
        {
            Success = 0,
            FatalError = 1,
        };

    private:
        CommandLineArgs m_commandLineArgs{};
        Window* m_pWindow = nullptr;
        Application* m_pApplication = nullptr;
        Timer m_timer;
        bool m_closeRequested = false;

    public:
        Platform(const CommandLineArgs& args);
        ~Platform();

        // No Copy or Move
        Platform(const Platform&) = delete;
        Platform& operator=(const Platform&) = delete;
        Platform(Platform&&) = delete;
        Platform& operator=(Platform&&) = delete;

        static Platform& Get();

        ExitCode Initialize();
        ExitCode MainLoop();
        void Terminate(const ExitCode exitCode);

        void Close();

        [[nodiscard]] Window& GetWindow() const;
        [[nodiscard]] Application& GetApplication() const;
        [[nodiscard]] const CommandLineArgs& GetCommandLineArgs() const { return m_commandLineArgs; }

    private:
        // Defined by individual Platforms.
        //static std::unique_ptr<Window> CreateWindow(Platform& platform, const WindowProperties& properties);
    };
}
