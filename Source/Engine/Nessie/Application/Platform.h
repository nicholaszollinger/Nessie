// Platform.h
#pragma once
#include <memory>
#include <string>
#include <thread>
#include "Nessie/Core/Config.h"
#include "CommandLineArgs.h"
#include "Nessie/Core/Time/Timer.h"
#include "Nessie/Application/ApplicationDesc.h"

namespace nes
{
    class InputManager;
    class Renderer;
    class DeviceManager;
    class Application;
    class ApplicationWindow;
    class Event;

    //----------------------------------------------------------------------------------------------------
    /// @brief : The platform provides base functionality for running Applications. It handles the main loop,
    ///     creates the Window and Application, manages the Renderer, DeviceManager, and Input.
    //----------------------------------------------------------------------------------------------------
    class Platform
    {
    public:
        Platform();
        ~Platform();
        Platform(const Platform&) = delete;
        Platform(Platform&&) noexcept = delete;
        Platform& operator=(const Platform&) = delete;
        Platform& operator=(Platform&&) noexcept = delete;

    public:
        static ApplicationWindow&           GetWindow();
        static Application&                 GetApplication();
        static DeviceManager&               GetDeviceManager();
        static const AppPerformanceInfo&    GetAppPerformanceInfo();
        static std::thread::id              GetMainThreadID();
        static bool                         IsMainThread();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the platform.
        //----------------------------------------------------------------------------------------------------
        bool                                Init(const CommandLineArgs& args);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Main loop of the program. Exits when the Application wants to quit, or the window has
        ///     been closed.
        //----------------------------------------------------------------------------------------------------
        void                                RunMainLoop();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Clean up all resources. 
        //----------------------------------------------------------------------------------------------------
        void                                Shutdown();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Handle incoming input events from the ApplicationWindow. 
        //----------------------------------------------------------------------------------------------------
        void                                OnInputEvent(Event& event) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Handle any changes to the Window's framebuffer. This will include changes to the vsync setting.
        //----------------------------------------------------------------------------------------------------
        void                                OnWindowResize(const uint32 width, const uint32 height) const;

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Synchronize the Main and Render threads. 
        //----------------------------------------------------------------------------------------------------
        void                                SyncFrame();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Main loop for headless applications. It will run a number of iterations equal to
        ///     m_headlessFrameCount in the ApplicationDesc.
        //----------------------------------------------------------------------------------------------------
        void                                RunHeadlessLoop();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Update time values after finishing a frame.
        //----------------------------------------------------------------------------------------------------
        void                                UpdateFrameTime();

    private:
        std::unique_ptr<DeviceManager>      m_pDeviceManager;
        std::unique_ptr<ApplicationWindow>  m_pWindow;
        std::unique_ptr<Application>        m_pApp;
        std::unique_ptr<InputManager>       m_pInputManager;
        std::unique_ptr<Renderer>           m_pRenderer;
        Timer                               m_timer{};
        AppPerformanceInfo                  m_performanceInfo{};
        float                               m_timeStep;
        float                               m_minTimeStepMs = 0.0333f;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Function that needs to be defined to create an Application in the entry point. 
    /// @param outAppDesc : Set properties for the application. This contains the command line
    ///     args passed into the executable.
    /// @param outWindowDesc : Set properties for the Application window, like an initial size.
    /// @param outRendererDesc : Set requirements for the Rendering operations you want to use.
    ///	@returns : Returning nullptr will abort the program
    //----------------------------------------------------------------------------------------------------
    extern std::unique_ptr<Application> CreateApplication(ApplicationDesc& outAppDesc, WindowDesc& outWindowDesc, RendererDesc& outRendererDesc);
}