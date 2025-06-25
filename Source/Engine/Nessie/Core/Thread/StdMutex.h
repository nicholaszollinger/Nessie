// StdMutex.h
#pragma once
#include "Core/Config.h"

// [TODO]: 
//NES_SUPPRESS_WARNINGS_STD_BEGIN
#include <mutex>
#include <shared_mutex>
//NES_SUPPRESS_WARNINGS_STD_END

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

    /// Requires the functions: lock() and unlock()
    template <typename Type>
    concept MutexType = requires(Type type)
    {
        type.lock();
        type.unlock();
    };

    /// Requires std-style lock_shared() and unlock_shared()
    template <typename Type>
    concept SharedMutexType = MutexType<Type> && requires(Type type)
    {
        type.lock_shared();
        type.unlock_shared;
    };

    static_assert(MutexType<NullMutex>);
    static_assert(MutexType<std::mutex>);
    static_assert(SharedMutexType<std::shared_mutex>);
}