// Application.h
#pragma once
#include "Nessie/Application/ApplicationWindow.h"
#include "Nessie/Core/Version.h"
#include "Nessie/Core/Time/Timer.h"
#include "Nessie/Debug/Assert.h"
#include "Nessie/Input/InputManager.h"

namespace nes
{
    NES_DEFINE_LOG_TAG(kApplicationLogTag, "Application", Info);
    
    class Platform;
    class ApplicationWindow;
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
        CommandLineArgs m_commandLineArgs{};        /// Command line arguments sent to the App Executable.
        std::string     m_appName{};                /// Application Name.
        Version         m_appVersion{};             /// Application Version.
        bool            m_isMultithreaded = true;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base Application class.  
    //----------------------------------------------------------------------------------------------------
    class Application
    {
    public:
        Application(ApplicationProperties&& appProps, const WindowProperties& windowProps);
        Application(const Application&) = delete;
        Application(Application&&) noexcept = delete;
        Application& operator=(const Application&) = delete;
        Application& operator=(Application&&) noexcept = delete;
        virtual ~Application() = default;

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the static instance of the Application. 
        //----------------------------------------------------------------------------------------------------
        static Application&         Get();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Push an event to the Application.
        //----------------------------------------------------------------------------------------------------
        virtual void                PushEvent([[maybe_unused]] Event& e) {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Application's main window. 
        //----------------------------------------------------------------------------------------------------
        ApplicationWindow&          GetWindow();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Application's main window. 
        //----------------------------------------------------------------------------------------------------
        const ApplicationWindow&    GetWindow() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Application's Input Manager. 
        //----------------------------------------------------------------------------------------------------
        const InputManager&         GetInputManager() const         { return m_inputManager; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current "frames per second" of the running application.
        //----------------------------------------------------------------------------------------------------
        float                       GetFPS() const                  { return m_fps; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the time elapsed since the start of the Main Loop.
        //----------------------------------------------------------------------------------------------------
        double                      GetTimeSinceStartup() const     { return m_timeSinceStartup; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Quit the Application. The application will finish the current frame before closing. 
        //----------------------------------------------------------------------------------------------------
        void                        Quit()                          { m_shouldQuit = true; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check whether the Application should close. 
        //----------------------------------------------------------------------------------------------------
        bool                        ShouldQuit() const              { return m_shouldQuit; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get information about the application. 
        //----------------------------------------------------------------------------------------------------
        const ApplicationProperties& GetProperties() const          { return m_properties; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the caller is on the Main Thread. 
        //----------------------------------------------------------------------------------------------------
        static bool                 IsMainThread();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the thread ID of the main thread the program is executing on. 
        //----------------------------------------------------------------------------------------------------
        static std::thread::id      GetMainThreadID();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Function called at the start of the entry point.
        /// @note : Do not call directly! This function is to be used in the entry point of the program.
        //----------------------------------------------------------------------------------------------------
        virtual bool                Internal_Init()                 { return true; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Runs the main loop of the application. On exit, the application will close.
        /// @note : Do not call directly! This function is to be used in the entry point of the program.
        //----------------------------------------------------------------------------------------------------
        void                        Internal_RunMainLoop();

    protected:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Called at the beginning of a frame in the main loop, before UpdateFrame() and RenderFrame().
        //----------------------------------------------------------------------------------------------------
        virtual void                OnFrameBegin() {}
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Update the application.
        ///	@param deltaTime : Delta time, in seconds.
        //----------------------------------------------------------------------------------------------------
        virtual void                RunFrame(const double deltaTime) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called at the end of a single application frame.
        //----------------------------------------------------------------------------------------------------
        virtual void                OnFrameEnd() {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called after exiting the main loop. Use to clean up resources, etc.
        //----------------------------------------------------------------------------------------------------
        virtual void                OnAppShutdown() {}

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Shuts down the application; cleans up resources, etc. 
        //----------------------------------------------------------------------------------------------------
        void                        Shutdown();
        
        ApplicationProperties       m_properties{};
        InputManager                m_inputManager{};
        Timer                       m_timer{};
        ApplicationWindow*          m_pWindow = nullptr;
        double                      m_timeSinceStartup = 0.f;
        double                      m_lastFrameTime = 0.f;
        float                       m_fps = 0.f;
        bool                        m_shouldQuit = false;
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Function that needs to be defined to create an application in the entry point. 
    ///	@param args : Command line args sent to the executable.
    ///	@returns : Application instance.
    //----------------------------------------------------------------------------------------------------
    extern nes::Application*     CreateApplication(const nes::CommandLineArgs& args);
}

