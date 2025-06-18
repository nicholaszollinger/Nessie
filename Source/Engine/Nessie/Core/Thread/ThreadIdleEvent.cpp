// ThreadIdleEvent.cpp

#include "ThreadIdleEvent.h"

namespace nes
{
    void ThreadIdleEvent::Resume()
    {
        m_mutex.lock();
        m_isIdle = false;
        m_mutex.unlock();
    }
    
    void ThreadIdleEvent::SignalIdle()
    {
        m_mutex.lock();
        m_isIdle = true;
        m_mutex.unlock();

        m_condition.notify_all();
    }
    
    void ThreadIdleEvent::WaitForIdle()
    {
        std::unique_lock lock(m_mutex);

        if (m_isIdle)
            return;

        m_condition.wait(lock, [this]() -> bool { return m_isIdle; });
    }
}
