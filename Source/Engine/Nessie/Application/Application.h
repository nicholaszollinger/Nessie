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
    /// @brief : Container for the arguments passed into the executable. The first argument will always be the executable's path.
    //-----------------------------------------------------------------------------------------------------------------------------
    struct CommandLineArgs
    {
        size_t m_count  = 0;
        char** m_args   = nullptr;

        const char* operator[](const size_t index) const
        {
            NES_ASSERT(index < m_count);
            return m_args[index];
        }

        const char* operator[](const int index) const
        {
            NES_ASSERT(std::cmp_less(index, m_count));
            return m_args[index];
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Settings used to initialize the Application instance. 
    //----------------------------------------------------------------------------------------------------
    struct ApplicationProperties
    {
        CommandLineArgs m_commandLineArgs{};    /// Command line arguments sent to the App Executable.
        std::string     m_appName{};            /// Application Name.
        Version         m_appVersion{};         /// Application Version.
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

    public:
        explicit Application(const CommandLineArgs& args);
        
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
        Application(Application&&) noexcept = delete;
        Application& operator=(Application&&) noexcept = delete;

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the static instance of the Application. 
        //----------------------------------------------------------------------------------------------------
        static Application&     Get();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the caller is on the Main Thread. 
        //----------------------------------------------------------------------------------------------------
        static bool             IsMainThread();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the thread ID of the main thread the program is executing on. 
        //----------------------------------------------------------------------------------------------------
        static std::thread::id  GetMainThreadID();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Manually push an event to the Application, to be handled immediately.
        //----------------------------------------------------------------------------------------------------
        void                    PushEvent(Event& e);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Quit the Application.
        /// @note : Note: This won't close the App immediately-it will wait until the current frame has finished.
        //----------------------------------------------------------------------------------------------------
        void                    Quit();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Application's main window. 
        //----------------------------------------------------------------------------------------------------
        Window&                 GetWindow();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Application's main window. 
        //----------------------------------------------------------------------------------------------------
        const Window&           GetWindow() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get access to the Application's Renderer, used to draw to the main window. 
        //----------------------------------------------------------------------------------------------------
        const Renderer&         GetRenderer() const       { return m_renderer; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Application's Input Manager. 
        //----------------------------------------------------------------------------------------------------
        const InputManager&     GetInputManager() const   { return m_inputManager; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Application's Scene Manager. 
        //----------------------------------------------------------------------------------------------------
        SceneManager&           GetSceneManager()         { return m_sceneManager; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Function called at the start of the entry point.
        /// @note : Do not call directly! This function is to be used in the entry point of the program.
        //----------------------------------------------------------------------------------------------------
        EExitCode               Internal_Init();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Runs the main loop of the application. On exit, the application will close.
        /// @note : Do not call directly! This function is to be used in the entry point of the program.
        //----------------------------------------------------------------------------------------------------
        EExitCode               Internal_RunMainLoop();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Closes the Application. 
        ///	@param exitCode : Code returned by either the Initialization the result of running the main loop.
        /// @note : Do not call directly! This function is to be used in the entry point of the program. Use
        ///     Application::Quit() to close the Application from code.
        //----------------------------------------------------------------------------------------------------
        void                    Internal_Close(EExitCode exitCode);
        
    private:
        //----------------------------------------------------------------------------------------------------
        // [TODO]: 
        /// @brief : Processes all queued events.
        //----------------------------------------------------------------------------------------------------
        void                    ProcessAppEvents();
    
    private:
        ApplicationProperties   m_properties;
        Window                  m_window{};
        Renderer                m_renderer{};
        InputManager            m_inputManager{};
        SceneManager            m_sceneManager{};
        Timer                   m_timer{};
        double                  m_timeSinceStartup = 0.f;
        bool                    m_closeRequested = false;
    };
}