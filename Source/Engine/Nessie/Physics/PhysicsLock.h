// PhysicsLock.h
#pragma once
#include "Nessie/Core/Thread/Mutex.h"
#include "Nessie/Debug/Assert.h"

namespace nes
{
#if NES_ASSERTS_ENABLED
    //----------------------------------------------------------------------------------------------------
    /// @brief : This is a list of locks used by the physics system. They need to be locked in a particular
    ///     order (from top to bottom) to prevent deadlocks.
    //----------------------------------------------------------------------------------------------------
    enum class EPhysicsLockTypes : uint8
    {
        BroadPhaseQuery     = 1 << 0,
        PerBody             = 1 << 1,
        BodiesArray         = 1 << 2,
        BroadPhaseUpdate    = 1 << 3,
        ConstraintsArray    = 1 << 4,
        ActiveBodiesArray   = 1 << 5,
    };

    class BodyManager;
    using PhysicsLockContext = const BodyManager*;
#endif
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Contains static helpers to lock the different mutexes that are part of the physics system
    ///     while preventing deadlock. This class keeps track, per thread, which locks are taken, and if the
    ///     order of locking is correct.
    //----------------------------------------------------------------------------------------------------
    class PhysicsLock
    {
    public:
    #if NES_ASSERTS_ENABLED
        //----------------------------------------------------------------------------------------------------
        /// @brief : Call before taking the lock. 
        //----------------------------------------------------------------------------------------------------
        static inline void CheckLock(PhysicsLockContext context, EPhysicsLockTypes lockType)
        {
            uint32& mutexes = GetLockedMutexes(context);
            NES_ASSERT(static_cast<uint32>(lockType) > mutexes, "A lock of the same or higher priority was already taken; this can create a deadlock!");
            mutexes = mutexes | static_cast<uint32>(lockType);
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Call after releasing the lock. 
        //----------------------------------------------------------------------------------------------------
        static inline void CheckUnlock(PhysicsLockContext context, EPhysicsLockTypes lockType)
        {
            uint32& mutexes = GetLockedMutexes(context);
            NES_ASSERT((mutexes & static_cast<uint32>(lockType)) != 0, "Mutex was not locked!");
            mutexes = mutexes & ~static_cast<uint32>(lockType);
        }
    #endif

        template <MutexType LockType>
        static inline void Lock(LockType& mutex NES_IF_ASSERTS_ENABLED(, PhysicsLockContext context, EPhysicsLockTypes lockType))
        {
            NES_IF_ASSERTS_ENABLED(CheckLock(context, lockType));
            mutex.lock();
        }

        template <MutexType LockType>
        static inline void Unlock(LockType& mutex NES_IF_ASSERTS_ENABLED(, PhysicsLockContext context, EPhysicsLockTypes lockType))
        {
            NES_IF_ASSERTS_ENABLED(CheckUnlock(context, lockType));
            mutex.unlock();
        }

        template <MutexType LockType>
        static inline void LockShared(LockType& mutex NES_IF_ASSERTS_ENABLED(, PhysicsLockContext context, EPhysicsLockTypes lockType))
        {
            NES_IF_ASSERTS_ENABLED(CheckLock(context, lockType));
            mutex.lock_shared();
        }

        template <MutexType LockType>
        static inline void UnlockShared(LockType& mutex NES_IF_ASSERTS_ENABLED(, PhysicsLockContext context, EPhysicsLockTypes lockType))
        {
            NES_IF_ASSERTS_ENABLED(CheckUnlock(context, lockType));
            mutex.unlock_shared();
        }

    #if NES_ASSERTS_ENABLED
    private:
        struct LockData
        {
            uint32              m_lockedMutexes = 0;
            PhysicsLockContext  m_context = nullptr;
        };

        static uint32&          GetLockedMutexes(PhysicsLockContext context)
        {
            static thread_local LockData s_locks[4];

            // If we find a matching context, we can use it.
            for (LockData& lock : s_locks)
            {
                if (lock.m_context == context)
                    return lock.m_lockedMutexes;
            }

            // Otherwise, we look for an entry not in use.
            for (LockData& lock : s_locks)
            {
                if (lock.m_lockedMutexes == 0)
                {
                    lock.m_context = context;
                    return lock.m_lockedMutexes;
                }
            }

            NES_ASSERT(false, "Too many physics systems locked at the same time!");
            return s_locks[0].m_lockedMutexes;
        }
    #endif
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper class that is similar to std::unique_lock; it will lock the mutex on construction,
    ///     and unlock on destruction.
    //----------------------------------------------------------------------------------------------------
    template <MutexType LockType>
    class UniqueLock
    {
    public:
        explicit UniqueLock(LockType& mutex NES_IF_ASSERTS_ENABLED(, PhysicsLockContext context, EPhysicsLockTypes lockType))
            : m_mutex(mutex)
#if NES_ASSERTS_ENABLED
            , m_context(context)
            , m_lockType(lockType)
#endif
        {
            PhysicsLock::Lock(m_mutex NES_IF_ASSERTS_ENABLED(, m_context, m_lockType));
        }
        
        ~UniqueLock()
        {
            PhysicsLock::Unlock(m_mutex NES_IF_ASSERTS_ENABLED(, m_context, m_lockType));   
        }

    private:
        LockType&           m_mutex;
    #if NES_ASSERTS_ENABLED
        PhysicsLockContext  m_context;
        EPhysicsLockTypes   m_lockType;
    #endif
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper class that is similar to std::shared_lock; it will lock_shared the mutex on construction,
    ///     and unlock_shared on destruction.
    //----------------------------------------------------------------------------------------------------
    template <MutexType LockType>
    class SharedLock
    {
    public:
        explicit SharedLock(LockType& mutex NES_IF_ASSERTS_ENABLED(, PhysicsLockContext context, EPhysicsLockTypes lockType))
            : m_mutex(mutex)
#if NES_ASSERTS_ENABLED
            , m_context(context)
            , m_lockType(lockType)
#endif
        {
            PhysicsLock::LockShared(m_mutex NES_IF_ASSERTS_ENABLED(, m_context, m_lockType));
        }
        
        ~SharedLock()
        {
            PhysicsLock::UnlockShared(m_mutex NES_IF_ASSERTS_ENABLED(, m_context, m_lockType));   
        }
        
    private:
        LockType&           m_mutex;
#if NES_ASSERTS_ENABLED
        PhysicsLockContext  m_context;
        EPhysicsLockTypes   m_lockType;
#endif
    };
}