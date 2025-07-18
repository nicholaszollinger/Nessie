// Application.cpp
#include "Application.h"
#include "Platform.h"

namespace nes
{
    Application::Application(const ApplicationDesc& appDesc)
        : m_desc(appDesc)
    {
        //
    }

    Application& Application::Get()
    {
        return Platform::GetApplication();
    }

    ApplicationWindow& Application::GetWindow()
    {
        return Platform::GetWindow();
    }

    const ApplicationWindow& Application::GetWindow() const
    {
        return Platform::GetWindow();
    }

    const AppPerformanceInfo& Application::GetPerformanceInfo() const
    {
        return Platform::GetAppPerformanceInfo();
    }
}
