// ApplicationProperties.h
#pragma once
#include "ApplicationWindow.h"
#include "CommandLineArgs.h"
#include "Nessie/Core/Version.h"
#include "Nessie/Graphics/RendererDesc.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Performance timings for the Application. Updated by the Platform.
    //----------------------------------------------------------------------------------------------------
    struct AppPerformanceInfo
    {
        double  m_timeSinceStartup = 0.f;           /// Time elapsed since the start of the application, in seconds.
        double  m_lastFrameTime = 0.f;              /// The time, in milliseconds, to complete the last frame.
        float   m_fps = 0.f;                        /// The current frames per seconds.
        float   m_mainThreadWorkTime = 0.f;         /// The amount of time the main thread took to complete a single frame.
        float   m_mainThreadWaitTime = 0.f;         /// The amount of time the main thread was waiting for the Render Thread.
        float   m_renderThreadWorkTime = 0.f;       /// The amount of time the render thread took to render a single frame.
        float   m_renderThreadWaitTime = 0.f;       /// The amount of time the render thread was waiting for the main thread.
        float   m_renderThreadGPUWaitTime = 0.f;    /// The amount of time the render thread was waiting on the GPU to finish.
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Settings used to initialize the Application instance. 
    //----------------------------------------------------------------------------------------------------
    struct ApplicationDesc
    {
        ApplicationDesc(const CommandLineArgs& args) : m_commandLineArgs(args) {}
        
        const CommandLineArgs   m_commandLineArgs;
        std::string             m_appName{};                /// Application Name.
        Version                 m_appVersion{};             /// Application Version.
        float                   m_minTimeStepMs = 0.0333f;  /// Minimum time step for an application update, in milliseconds.
        bool                    m_isHeadless = false;       /// If true, the Application will not show a window or receive input.
        uint32                  m_headlessFrameCount = 1;   /// Number of frames to execute in headless mode. Default is a single frame.

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the name of the application. Default is none.
        //----------------------------------------------------------------------------------------------------
        ApplicationDesc&        SetApplicationName(const std::string& appName);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the version of the application. Default is 1.0.0
        //----------------------------------------------------------------------------------------------------
        ApplicationDesc&        SetApplicationVersion(const Version& appVersion);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the minimum time, in milliseconds, for the delta time passed to the Application.
        ///     The default is 0.03 ms.
        //----------------------------------------------------------------------------------------------------
        ApplicationDesc&        SetMinTimeStep(const float minTimeStepMs);

        //----------------------------------------------------------------------------------------------------
        /// @brief : If set to true, the renderer will not be able to present to the screen, but you can still use
        /// the GPU for rendering work. The Application will not receive input either!
        /// @param isHeadless : Whether to run headless or not.
        /// @param numFrames : The number of frames to run in headless mode. Default is 1.
        //----------------------------------------------------------------------------------------------------
        ApplicationDesc&        SetIsHeadless(const bool isHeadless = true, const uint32 numFrames = 1);
    };
}
