// Semaphore.h
#pragma once
#include "Nessie/Core/Thread/Atomics.h"
#include "Nessie/Core/Config.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Implementation pretty much identical to std::counting_semaphore, but with the ability to Acquire
    ///     with a count parameter and to get the current value of the internal counter.
    //----------------------------------------------------------------------------------------------------
    class Semaphore
    {
#ifdef NES_PLATFORM_WINDOWS
        using SemaphoreType = void*;
#else
#error "Unhandled Semaphore type for Platform!";
#endif
        
    public:
        explicit Semaphore(uint32_t initialCount = 0);
        ~Semaphore();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Acquiring the semaphore decrements the internal counter by "count". This will block if the
        ///     internal count becomes >= 0.
        //----------------------------------------------------------------------------------------------------
        void    Acquire(const unsigned count = 1);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Release the semaphore increments the internal counter by "count". Incrementing a counter
        ///     to greater than or equal to zero will unblock threads stuck in Acquire(). 
        //----------------------------------------------------------------------------------------------------
        void    Release(const unsigned count = 1);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the current value of the internal counter.
        //----------------------------------------------------------------------------------------------------
        int     GetValue() const { return m_counter.load(std::memory_order_relaxed); }

    private:
        alignas (NES_CACHE_LINE_SIZE) std::atomic<int> m_counter{};
        SemaphoreType m_semaphore{};
    };
}