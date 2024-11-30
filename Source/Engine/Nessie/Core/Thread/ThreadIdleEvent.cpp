// ThreadIdleEvent.cpp

#include "ThreadIdleEvent.h"

namespace nes
{
    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Sets the m_isIdle to false, denoting that the associated Thread is 'active'.
    //-----------------------------------------------------------------------------------------------------------------------------
    void ThreadIdleEvent::Resume()
    {
        m_mutex.lock();
        m_isIdle = false;
        m_mutex.unlock();
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Signal that the associated thread is now idle. This will release any processes that are blocked in the WaitForIdle
    ///             function.
    //-----------------------------------------------------------------------------------------------------------------------------
    void ThreadIdleEvent::Signal()
    {
        m_mutex.lock();
        m_isIdle = true;
        m_mutex.unlock();

        m_condition.notify_all();
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Should be called from external threads. This function will block a process until ThreadIdleEvent.Signal() is called,
    ///         notifying that the associated thread is now idle.
    //-----------------------------------------------------------------------------------------------------------------------------
    void ThreadIdleEvent::WaitForIdle()
    {
        std::unique_lock lock(m_mutex);

        if (m_isIdle)
            return;

        m_condition.wait(lock, [this]() -> bool { return m_isIdle; });
    }



}
