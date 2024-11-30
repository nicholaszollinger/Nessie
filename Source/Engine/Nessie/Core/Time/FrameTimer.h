#pragma once
// FrameTimer.h
#include "Timer.h"

namespace nes
{
    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Wraps the HighPrecisionTimer in a frame time API. You simply create this timer on the stack (its constructor
    ///         will start the timer) then call NewFrame() at the top of the main loop to get the delta time for that frame.
    //-----------------------------------------------------------------------------------------------------------------------------
    class FrameTimer
    {
        Timer m_timer;

    public:
        FrameTimer();

        void ResetTimer();
        double NewFrame();
    };
}