// Timer.cpp
#include "Timer.h"

namespace nes
{
    void Timer::Start()
    {
        m_startTime = std::chrono::high_resolution_clock::now();
        m_previousTick = m_startTime;
    }
}