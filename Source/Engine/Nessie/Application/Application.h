// Application.h
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
    NES_DEFINE_LOG_TAG(kApplicationLogTag, "Application", Info);
    
    class ApplicationWindow;
    class Event;
    class Renderer;
    class Swapchain;
    class RenderFrameContext;
    class CommandBuffer;
    class DeviceManager;
    class AssetManager;
    class InputManager;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base Application class.
    //----------------------------------------------------------------------------------------------------
    class Application
    {
    public:
        explicit Application(ApplicationDesc&& desc, WindowDesc&& windowDesc, RendererDesc&& rendererDesc);
        Application(const Application&) = delete;
        Application(Application&&) noexcept = delete;
        Application& operator=(const Application&) = delete;
        Application& operator=(Application&&) noexcept = delete;
        virtual ~Application();

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the static instance of the Application. 
        //----------------------------------------------------------------------------------------------------
        static Application&                 Get();
        static std::thread::id              GetMainThreadID();
        static bool                         IsMainThread();
        static DeviceManager&               GetDeviceManager();
        static AssetManager&                GetAssetManager();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Quit the Application. The application will finish the current frame before closing. 
        //----------------------------------------------------------------------------------------------------
        void                                Quit()                          { m_shouldQuit = true; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Push an event to the Application.
        //----------------------------------------------------------------------------------------------------
        virtual void                        PushEvent([[maybe_unused]] Event& e) {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Application's main window. 
        //----------------------------------------------------------------------------------------------------
        ApplicationWindow&                  GetWindow();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Application's main window. 
        //----------------------------------------------------------------------------------------------------
        const ApplicationWindow&            GetWindow() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get information about the application. 
        //----------------------------------------------------------------------------------------------------
        const ApplicationDesc&              GetDesc() const                 { return m_desc; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the performance info for the application. 
        //----------------------------------------------------------------------------------------------------
        const AppPerformanceInfo&           GetPerformanceInfo() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current "frames per second" of the running application.
        //----------------------------------------------------------------------------------------------------
        float                               GetFPS() const                  { return GetPerformanceInfo().m_fps; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the time elapsed since the start of the application.
        //----------------------------------------------------------------------------------------------------
        double                              GetTimeSinceStartup() const     { return GetPerformanceInfo().m_timeSinceStartup; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called in the entry point of the program. Initialize the Application.
        //----------------------------------------------------------------------------------------------------
        bool                                Internal_Init();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Called in the entry point of the program. Main loop of the program.
        /// Exits when the Application wants to quit, or the window has been closed.
        //----------------------------------------------------------------------------------------------------
        void                                Internal_RunMainLoop();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called by the ApplicationWindow. Handle incoming input events from the window. 
        //----------------------------------------------------------------------------------------------------
        void                                Internal_OnInputEvent(Event& event);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called by the ApplicationWindow. Handle any changes to the Window's framebuffer.
        /// This will include changes to the vsync setting.
        //----------------------------------------------------------------------------------------------------
        void                                Internal_OnWindowResize(const uint32 width, const uint32 height);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called by the entry point of the program. Closes the application and all managers.
        /// You should use Application::Quit() to close the application. Not this.
        //----------------------------------------------------------------------------------------------------
        void                                Internal_Shutdown();
        
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Function called at the start of the Application. Returning false results in the program shutting down.
        //----------------------------------------------------------------------------------------------------
        virtual bool                        Init() = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called after exiting the main loop. Use to clean up resources, etc.
        //----------------------------------------------------------------------------------------------------
        virtual void                        PreShutdown() = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Run a single frame of the application. Delta time is in seconds.
        //----------------------------------------------------------------------------------------------------
        virtual void                        Update(const float deltaTime) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called any time the Application window is resized. Width and height are in pixels.
        //----------------------------------------------------------------------------------------------------
        virtual void                        OnResize(const uint32 width, const uint32 height) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Render the current frame.
        ///	@param commandBuffer : The command buffer associated with this frame.
        ///	@param context : The current frame context, including the swapchain image that will be rendered to.
        //----------------------------------------------------------------------------------------------------
        virtual void                        Render(CommandBuffer& commandBuffer, const RenderFrameContext& context) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Start up the Application. Creates the window and all manager classes.
        //----------------------------------------------------------------------------------------------------
        void                                OnStartup(WindowDesc&& windowDesc, RendererDesc&& rendererDesc);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Update time values after finishing a frame.
        //----------------------------------------------------------------------------------------------------
        void                                UpdateFrameTime();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Synchronize application threads.
        //----------------------------------------------------------------------------------------------------
        void                                SyncFrame();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Main loop for headless applications. It will run a number of iterations equal to
        ///     m_headlessFrameCount in the ApplicationDesc.
        //----------------------------------------------------------------------------------------------------
        void                                RunHeadlessLoop();

    private:
        ApplicationDesc                     m_desc;
        std::unique_ptr<DeviceManager>      m_pDeviceManager;
        std::unique_ptr<ApplicationWindow>  m_pWindow;
        std::unique_ptr<InputManager>       m_pInputManager;
        std::unique_ptr<Renderer>           m_pRenderer;
        std::unique_ptr<AssetManager>       m_pAssetManager;
        Timer                               m_timer{};
        AppPerformanceInfo                  m_performanceInfo{};
        float                               m_timeStep;
        float                               m_minTimeStepMs = 0.0333f;
        bool                                m_shouldQuit = false;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Function that needs to be defined by the executable to configure the Application's settings.
    /// @param args : Arguments passed to the executable.
    ///	@returns : Returning nullptr will abort the program.
    //----------------------------------------------------------------------------------------------------
    extern std::unique_ptr<nes::Application> CreateApplication(const CommandLineArgs& args);
}

