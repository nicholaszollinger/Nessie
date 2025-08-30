// Timer.h
#pragma once
#include <chrono>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///	@brief : Timer class that tracks two time points. One from the last time that Start() was called,
    ///     and another from the last time that Tick() was called.
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

    public:
        Timer() = default;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Start the timer. This can be called again to reset the timer.
        //----------------------------------------------------------------------------------------------------
        void        Start();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Stops the timer and returns the elapsed time since the call to Start().
        ///	@tparam Period : Period type to return the time in. Seconds, Milliseconds, Microseconds, Nanoseconds.
        //----------------------------------------------------------------------------------------------------
        template <typename Period = Seconds>
        double      Stop();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Calculates the time since the last time that Tick() was called.
        ///	@tparam Period : Period type to return the time in. Seconds, Milliseconds, Microseconds, Nanoseconds.
        //----------------------------------------------------------------------------------------------------
        template <typename Period = Seconds>
        double      Tick();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Calculates the time difference between the current time and when the timer was started.
        ///	@tparam Period : Period type to return the time in. Seconds, Milliseconds, Microseconds, Nanoseconds.
        //----------------------------------------------------------------------------------------------------
        template <typename Period = Seconds>
        double      ElapsedTime() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check whether the timer is currently running.
        //----------------------------------------------------------------------------------------------------
        bool        IsRunning() const { return m_isRunning; }

    private:
        TimePoint   m_startTime{};
        TimePoint   m_previousTick{};
        bool        m_isRunning = false;
    };
}

namespace nes
{
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
    
    template <typename Period>
    double Timer::Tick()
    {
        const auto now = Clock::now();
        const auto duration = std::chrono::duration<double, Period>(now - m_previousTick);
        m_previousTick = now;
        return duration.count();
    }
    
    template <typename Period>
    double Timer::ElapsedTime() const
    {
        if (!m_isRunning)
            return 0.0;

        const auto duration = std::chrono::duration<double, Period>(Clock::now() - m_startTime);
        return duration.count();
    }
}