// Platform.cpp

#include "Platform.h"
#include <BleachNew.h>
#include "Application.h"

//#if NES_PLATFORM_WINDOWS
//using Window = Window_GLFW;
//using RenderAPI = RenderAPI_Vulkan;
//#else
//// Error for no RenderAPI available on this Platform...
//#endif

namespace nes
{
    // Static instance of the Platform.
    Platform* s_pInstance = nullptr;

    Platform& Platform::Get()
    {
        NES_ASSERTV(s_pInstance != nullptr, "Platform instance is nullptr!");
        return *s_pInstance;
    }

    Platform::Platform(const CommandLineArgs& args)
        : m_commandLineArgs(args)
    {
        if (s_pInstance != nullptr)
        {
            NES_CRITICALV("Platform", "Attempted to create a second platform instance!");
        }
        
        s_pInstance = this;
    }

    Platform::~Platform()
    {
        s_pInstance = nullptr;
    }

    Platform::ExitCode Platform::Initialize()
    {
        BLEACH_INIT_LEAK_DETECTOR();

        Logger::Init(NES_LOG_DIR);
        // [TODO]:
        //Logger::LoadCategories();

        // [TODO]: Load this.
        WindowProperties windowProps;
        windowProps.m_label = "Nessie Engine";
        windowProps.m_extent= { 1600, 900};

        // Create the Window.
        m_pWindow = BLEACH_NEW(Window);
        if (!m_pWindow->Init(*this, windowProps))
        {
            NES_ERRORV("Platform", "Failed to initialize the Window!");
            return ExitCode::FatalError; 
        }

        // Create the Application.
        m_pApplication = BLEACH_NEW(Application);
        if (!m_pApplication->Init())
        {
            NES_ERRORV("Platform", "Failed to initialize the Application!");
            return ExitCode::FatalError;
        }

        return ExitCode::Success;
    }

    Platform::ExitCode Platform::MainLoop()
    {
        if (m_pApplication == nullptr)
        {
            NES_ERRORV("Platform", "Application instance is nullptr!");
            return ExitCode::FatalError;
        }

        m_timer.Start();

        while (!m_pWindow->ShouldClose() && !m_closeRequested)
        {
            const double deltaTime = m_timer.Tick();
            m_pApplication->RunAppFrame(deltaTime);
            m_pWindow->ProcessEvents();
        }

        return ExitCode::Success;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Closes the Platform and the currently running Application immediately. This is called in the
    ///              main function. If you are trying to manually close the Platform, use Close();
    ///		@param exitCode : Result either the Initialize or MainLoop functions.
    //----------------------------------------------------------------------------------------------------
    void Platform::Terminate([[maybe_unused]] const ExitCode exitCode)
    {
        // Close the Application.
        if (m_pApplication)
        {
            m_pApplication->Close();
            BLEACH_DELETE(m_pApplication);
            m_pApplication = nullptr;
        }

        // Close the Window.
        if (m_pWindow)
        {
            m_pWindow->Close();
            BLEACH_DELETE(m_pWindow);
            m_pWindow = nullptr;
        }

        // Close the Logger.
        Logger::Close();

        BLEACH_DUMP_AND_DESTROY_LEAK_DETECTOR();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Request the Platform to close. This won't close immediately.
    //----------------------------------------------------------------------------------------------------
    void Platform::Close()
    {
        m_closeRequested = true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a reference to the Window instance.
    //----------------------------------------------------------------------------------------------------
    Window& Platform::GetWindow() const
    {
        NES_ASSERTV(m_pWindow != nullptr, "Window instance is nullptr!");
        return *m_pWindow;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns a reference to the Application instance.
    //----------------------------------------------------------------------------------------------------
    Application& Platform::GetApplication() const
    {
        NES_ASSERTV(m_pApplication != nullptr, "Application instance is nullptr!");
        return *m_pApplication;
    }

}
