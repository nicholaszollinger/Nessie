// FrameTimer.cpp

#include "FrameTimer.h"

namespace nes
{
    FrameTimer::FrameTimer()
    {
        m_timer.Start();
    }
    
    void FrameTimer::ResetTimer()
    {
        m_timer.Start();
    }
    
    double FrameTimer::NewFrame()
    {
        const double deltaTimeMs = m_timer.Tick();
        return deltaTimeMs;
    }
}