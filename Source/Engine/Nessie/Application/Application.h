// Application.h
#pragma once
#include "Nessie/Application/Platform.h"

namespace nes
{
    NES_DEFINE_LOG_TAG(kApplicationLogTag, "Application", Info);
    
    class Platform;
    class ApplicationWindow;
    class Event;
    class Renderer;
    class Swapchain;
    class RenderFrameContext;
    class CommandBuffer;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base Application class.  
    //----------------------------------------------------------------------------------------------------
    class Application
    {
    public:
        Application(const ApplicationDesc& appDesc);
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
        const ApplicationDesc&      GetDesc() const                 { return m_desc; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the performance info for the application. 
        //----------------------------------------------------------------------------------------------------
        const AppPerformanceInfo&   GetPerformanceInfo() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current "frames per second" of the running application.
        //----------------------------------------------------------------------------------------------------
        float                       GetFPS() const                  { return GetPerformanceInfo().m_fps; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the time elapsed since the start of the application.
        //----------------------------------------------------------------------------------------------------
        double                      GetTimeSinceStartup() const     { return GetPerformanceInfo().m_timeSinceStartup; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Function called at the start of the Application. Returning false results in the program failing.
        /// @note : Do not call directly! This is called by the Platform.
        //----------------------------------------------------------------------------------------------------
        virtual bool                Internal_AppInit()              { return true; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Run a single update frame of the application.
        /// @note : Do not call directly! This is called by the Platform.
        //----------------------------------------------------------------------------------------------------
        virtual void                Internal_AppUpdate(const float timeStep) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Render the current frame.
        ///	@param commandBuffer : The command buffer associated with this frame.
        ///	@param context : The current frame context, including the swapchain image that will be rendered to.
        /// @note : Do not call directly! This is called by the Platform.
        //----------------------------------------------------------------------------------------------------
        virtual void                Internal_AppRender(CommandBuffer& commandBuffer, const RenderFrameContext& context) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Called after exiting the main loop. Use to clean up resources, etc.
        /// @note : Do not call directly! This is called by the Platform.
        //----------------------------------------------------------------------------------------------------
        virtual void                Internal_AppShutdown() {}

    private:
        ApplicationDesc             m_desc;
        bool                        m_shouldQuit = false;
    };
}

