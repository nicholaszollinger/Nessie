// Mutex.h
#pragma once
#include <mutex>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A placeholder mutex class that does no locking.
    //----------------------------------------------------------------------------------------------------
    struct NullMutex
    {
        void lock() const {}
        void unlock() const {}
    };

    template <typename Type>
    concept MutexType = requires(Type type)
    {
        type.lock();
        type.unlock();
    };
}