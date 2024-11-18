// HighPrecisionTimer.cpp

#include "HighPrecisionTimer.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Start the timer. This can be called again to reset the timer.
    //----------------------------------------------------------------------------------------------------
    void HighPrecisionTimer::Start()
    {
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the number of milliseconds since the timer was started.
    ///		@returns : Time in milliseconds.
    //----------------------------------------------------------------------------------------------------
    double HighPrecisionTimer::GetElapsedTime() const
    {
        const TimePoint endTime = std::chrono::high_resolution_clock::now();
        const TimeDuration duration = std::chrono::duration_cast<TimeDuration>(endTime - m_startTime);
        return duration.count() * 1000.0; // Convert to milliseconds before returning.;
    }
}