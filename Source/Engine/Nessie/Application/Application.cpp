// Application.cpp

#include "Application.h"
#include "Platform.h"
#include "Window.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Application instance.
    //----------------------------------------------------------------------------------------------------
    Application& Application::Get()
    {
        return Platform::Get().GetApplication();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Application's Window.
    //----------------------------------------------------------------------------------------------------
    Window& Application::GetWindow() const
    {
        return Platform::Get().GetWindow();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Initialize the Application.
    ///		@returns : 
    //----------------------------------------------------------------------------------------------------
    bool Application::Init()
    {
        // [TODO]: Set the Application Properties.

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Runs a single frame of the Application.
    ///		@param deltaTime : Delta time since the last frame.
    //----------------------------------------------------------------------------------------------------
    void Application::RunAppFrame(const double deltaTime)
    {
        m_timeSinceStartup += deltaTime;

        // [TODO]: Sync with the Render Thread.
        // [TODO]: Sync with the Resource Thread.

        ProcessEvents();

        // [TODO]: Begin a Render Frame

        // [TODO]: Update the World

        // [TODO]: End the Render Frame
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Quit the Application.
    //----------------------------------------------------------------------------------------------------
    void Application::Quit()
    {
        Platform::Get().Close();
    }

    void Application::Close()
    {
        // [TODO]: Close the WorldSystem;

        // [TODO]: Close the RenderSystem;

        // [TODO]: Close the ResourceSystem;
    }

    void Application::PushEvent([[maybe_unused]] Event& e)
    {
        // [TODO]: Push the Event to the Event Queue.
    }

    void Application::ProcessEvents()
    {
        // [TODO]: Process the Event Queue.
    }

}