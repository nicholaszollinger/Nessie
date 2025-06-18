// ThreadIdleEvent.h
#pragma once
#include <condition_variable>
#include <mutex>

namespace nes
{
    //-----------------------------------------------------------------------------------------------------------------------------
    ///	@brief : This class defines the interface to manage when a Thread is idle/sleeping/waiting or working. The function
    ///     WaitUntilIdle() will block the process that called it until the associated Thread is put to sleep.
    //-----------------------------------------------------------------------------------------------------------------------------
    class ThreadIdleEvent
    {
    public:
        explicit ThreadIdleEvent(const bool startIdle = true) : m_isIdle(startIdle) {}

        //-----------------------------------------------------------------------------------------------------------------------------
        ///	@brief : Sets the m_isIdle to false, denoting that the associated Thread is 'active'.
        //-----------------------------------------------------------------------------------------------------------------------------
        void                    Resume();

        //-----------------------------------------------------------------------------------------------------------------------------
        ///	@brief : Signal that the associated thread is now idle. This will release any processes that are blocked in the WaitForIdle
        ///     function.
        //-----------------------------------------------------------------------------------------------------------------------------
        void                    SignalIdle();

        //-----------------------------------------------------------------------------------------------------------------------------
        ///	@brief : Should be called from external threads. This function will block a process until SignalIdle() is called,
        ///     notifying that the associated thread is now idle.
        //-----------------------------------------------------------------------------------------------------------------------------
        void                    WaitForIdle();

    private:
        std::mutex              m_mutex;
        std::condition_variable m_condition;
        bool                    m_isIdle;
    };
}