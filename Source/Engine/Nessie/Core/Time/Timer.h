#pragma once
// HighPrecisionTimer.h
#include <chrono>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Timer class that tracks two time points. One from the last time that Start() was called,
    ///              and another from the last time that Tick() was called.
    //----------------------------------------------------------------------------------------------------
    class Timer
    {
        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = Clock::time_point;

    public:
        using Seconds      = std::ratio<1>;
        using Milliseconds = std::ratio<1, 1000>;
        using Microseconds = std::ratio<1, 1000000>;
        using Nanoseconds  = std::ratio<1, 1000000000>;

    private:
        TimePoint m_startTime{};
        TimePoint m_previousTick{};
        bool m_isRunning = false;

    public:
        Timer() = default;

        void Start();

        template <typename Period = Seconds>
        double Stop();

        template <typename Period = Seconds>
        double Tick();

        template <typename Period = Seconds>
        double GetElapsedTime() const;

        bool IsRunning() const { return m_isRunning; }
    };
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Stops the timer, and returns the elapsed time since the call to Start().
    ///		@tparam Period : Period type to return the time in. Seconds, Milliseconds, Microseconds, Nanoseconds.
    //----------------------------------------------------------------------------------------------------
    template <typename Period>
    double Timer::Stop()
    {
        if (!m_isRunning)
            return 0.0;

        m_isRunning = false;

        const auto duration = std::chrono::duration<double, Period>(Clock::now() - m_startTime);
        m_startTime = Clock::now();

        return duration.count();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Calculates the time since the last time that Tick() was called.
    ///		@tparam Period : Period type to return the time in. Seconds, Milliseconds, Microseconds, Nanoseconds.
    //----------------------------------------------------------------------------------------------------
    template <typename Period>
    double Timer::Tick()
    {
        const auto now = Clock::now();
        const auto duration = std::chrono::duration<double, Period>(now - m_previousTick);
        m_previousTick = now;
        return duration.count();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Calculates the time difference between the current time and when the timer was started.
    ///		@tparam Period : 
    ///		@returns : 
    //----------------------------------------------------------------------------------------------------
    template <typename Period>
    double Timer::GetElapsedTime() const
    {
        if (!m_isRunning)
            return 0.0;

        const auto duration = std::chrono::duration<double, Period>(Clock::now() - m_startTime);
        return duration.count();
    }
}