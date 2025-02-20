// HighPrecisionTimer.cpp

#include "Timer.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Start the timer. This can be called again to reset the timer.
    //----------------------------------------------------------------------------------------------------
    void Timer::Start()
    {
        m_startTime = std::chrono::high_resolution_clock::now();
        m_previousTick = m_startTime;
    }
}