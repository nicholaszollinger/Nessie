#pragma once
// Application.h
#include "Core/Time/Timer.h"

namespace nes
{
    class Platform;
    class Window;
    class Event;

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Application runs the game loop and processes events.
    //----------------------------------------------------------------------------------------------------
    class Application
    {
        friend Platform;

        double m_timeSinceStartup = 0.f;
        //bool m_shouldClose = false;

    private:
        Application() = default;
        ~Application() = default;

    public:
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
        Application(Application&&) noexcept = delete;
        Application& operator=(Application&&) noexcept = delete;

    public:
        static Application& Get();

        void PushEvent(Event& e);
        void Quit();

        [[nodiscard]] Window& GetWindow() const;

    private:
        bool Init();
        void RunAppFrame(const double deltaTime);
        void ProcessEvents();
        void Close();
    };
}