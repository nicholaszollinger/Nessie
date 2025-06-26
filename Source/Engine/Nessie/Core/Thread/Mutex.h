// Mutex.h
#pragma once
#include "StdMutex.h"
#include "Debug/Assert.h"

namespace nes
{
    using MutexBase = std::mutex;
    using SharedMutexBase = std::shared_mutex;

#if NES_ASSERTS_ENABLED
    //----------------------------------------------------------------------------------------------------
    // [TODO]: Track lock contention in the profiler.
    /// @brief : Simple wrapper around MutexBase which asserts that locks/unlocks take place on the same
    ///     thread.
    //----------------------------------------------------------------------------------------------------
    class Mutex : public MutexBase
    {
    public:
        inline bool try_lock()
        {
            NES_ASSERT(m_lockedThreadID != std::this_thread::get_id());
            if (MutexBase::try_lock())
            {
                NES_IF_ASSERTS_ENABLED(m_lockedThreadID = std::this_thread::get_id());
                return true;
            }
            return false;
        }

        inline void lock()
        {
            if (!try_lock())
            {
                // [TODO]: Profile
                MutexBase::lock();
                NES_IF_ASSERTS_ENABLED(m_lockedThreadID = std::this_thread::get_id());
            }
        }

        inline void unlock()
        {
            NES_ASSERT(m_lockedThreadID == std::this_thread::get_id());
            NES_IF_ASSERTS_ENABLED(m_lockedThreadID = std::thread::id());
            MutexBase::unlock();
        }

#if NES_ASSERTS_ENABLED
        inline bool is_locked() const
        {
            return m_lockedThreadID != std::thread::id();
        }
#endif
    private:
        NES_IF_ASSERTS_ENABLED(std::thread::id m_lockedThreadID);
    };

    //----------------------------------------------------------------------------------------------------
    // [TODO]: Track lock contention in the profiler.
    /// @brief : Simple wrapper around SharedMutexBase which asserts that locks/unlocks take place
    ///     on the same thread.
    //----------------------------------------------------------------------------------------------------
    class SharedMutex : public SharedMutexBase
    {
    public:
        inline bool try_lock()
        {
            NES_ASSERT(m_lockedThreadID != std::this_thread::get_id());
            if (SharedMutexBase::try_lock())
            {
                NES_IF_ASSERTS_ENABLED(m_lockedThreadID = std::this_thread::get_id());
                return true;
            }
            return false;
        }

        inline void lock()
        {
            if (!try_lock())
            {
                // [TODO]: Profile
                SharedMutexBase::lock();
                NES_IF_ASSERTS_ENABLED(m_lockedThreadID = std::this_thread::get_id());
            }
        }

        inline void unlock()
        {
            NES_ASSERT(m_lockedThreadID == std::this_thread::get_id());
            NES_IF_ASSERTS_ENABLED(m_lockedThreadID = std::thread::id());
            SharedMutexBase::unlock();
        }

        inline void lock_shared()
        {
            if (!try_lock_shared())
            {
                // [TODO]: Profile.
                SharedMutexBase::lock_shared();
            }
        }

    #if NES_ASSERTS_ENABLED
        inline bool is_locked() const
        {
            return m_lockedThreadID != std::thread::id();
        }
    #endif
        
    private:
        NES_IF_ASSERTS_ENABLED(std::thread::id m_lockedThreadID);
    };

#else
    using Mutex = MutexBase;
    using SharedMutex = SharedMutexBase;
#endif
    
}