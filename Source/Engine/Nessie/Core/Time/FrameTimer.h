// FrameTimer.h
#pragma once
#include "Timer.h"

namespace nes
{
    //-----------------------------------------------------------------------------------------------------------------------------
    ///	@brief : Wraps the HighPrecisionTimer in a frame time API. Create this timer on the stack (its constructor
    ///     will start the timer), then call NewFrame() at the top of the main loop to get the delta time for that frame.
    //-----------------------------------------------------------------------------------------------------------------------------
    class FrameTimer
    {
    public:
        FrameTimer();

        //-----------------------------------------------------------------------------------------------------------------------------
        ///	@brief : Resets the internal timer to 0.
        //-----------------------------------------------------------------------------------------------------------------------------
        void    ResetTimer();

        //-----------------------------------------------------------------------------------------------------------------------------
        ///	@brief : Begins a new frame and returns the time in milliseconds since the last frame.
        //-----------------------------------------------------------------------------------------------------------------------------
        double  NewFrame();

    private:
        Timer   m_timer;
    };
}