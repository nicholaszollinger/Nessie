// Semaphore.cpp
#include "Semaphore.h"
#include "Debug/Assert.h"

#ifdef NES_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#pragma warning (push)
#pragma warning (disable : 5039)
#include <Windows.h>
#pragma warning (pop)

#else
#endif

namespace nes
{
    Semaphore::Semaphore(uint32_t initialCount)
    {
#ifdef NES_PLATFORM_WINDOWS
        m_semaphore = CreateSemaphore(nullptr, static_cast<long>(initialCount), INT_MAX, nullptr);
        if (m_semaphore == nullptr)
        {
            std::abort();
        }
#endif
    }

    Semaphore::~Semaphore()
    {
#ifdef NES_PLATFORM_WINDOWS
        CloseHandle(m_semaphore);
#endif
    }

    void Semaphore::Acquire(const unsigned count)
    {
        NES_ASSERT(count > 0);
        
#ifdef NES_PLATFORM_WINDOWS
        const int oldValue = m_counter.fetch_sub(static_cast<int>(count), std::memory_order_acquire);
        const int newValue = oldValue - static_cast<int>(count);
        if (newValue < 0)
        {
            const int numToAcquire = std::min(oldValue, 0) - newValue;
            for (int i = 0; i < numToAcquire; ++i)
            {
                WaitForSingleObject(m_semaphore, INFINITE);
            }
        }
#endif
    }

    void Semaphore::Release(const unsigned count)
    {
        NES_ASSERT(count > 0);
        
#ifdef NES_PLATFORM_WINDOWS
        const int oldValue = m_counter.fetch_add(static_cast<int>(count), std::memory_order_release);
        if (oldValue < 0)
        {
            const int newValue = oldValue + static_cast<int>(count);
            const int numToRelease = std::min(newValue, 0) - oldValue;
            ::ReleaseSemaphore(m_semaphore, numToRelease, nullptr);
        }
#endif
        
    }    
}