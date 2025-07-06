// JobSystemWithBarrier.cpp
#include <thread>
#include "Nessie/Core/Jobs/JobSystemWithBarrier.h"

namespace nes
{
    JobSystemWithBarrier::BarrierImpl::BarrierImpl()
    {
        for (std::atomic<Job*>& job : m_jobs)
            job = nullptr;
    }

    JobSystemWithBarrier::BarrierImpl::~BarrierImpl()
    {
        NES_ASSERT(IsEmpty());
    }

    void JobSystemWithBarrier::BarrierImpl::AddJob(const JobHandle& handle)
    {
        bool shouldSignalSemaphore = false;

        // Set the Barrier for the job. This returns true if the barrier is successfully set, otherwise the
        // Job is already done and we don't need to add it to the list.
        Job* pJob = handle.Get();
        if (pJob->SetBarrier(this))
        {
            // If the Job can be executed: we want to release the semaphore an extra time to allow the
            // waiting thread to start executing it.
            ++m_numLeftToAcquire;
            if (pJob->CanBeExecuted())
            {
                shouldSignalSemaphore = true;
                ++m_numLeftToAcquire;
            }
            
            // Add the Job to our Job list.
            pJob->AddRef();
            const uint32_t writeIndex = m_writeIndex.fetch_add(1, std::memory_order_relaxed);
            while (writeIndex - m_readIndex >= kMaxJobs)
            {
                NES_ASSERT(false, "Barrier full! stalling!");
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
            m_jobs[writeIndex & (kMaxJobs - 1)] = pJob;
        }

        // Notify the waiting thread(s) that a new executable job is available.
        if (shouldSignalSemaphore)
            m_semaphore.Release();
    }

    void JobSystemWithBarrier::BarrierImpl::AddJobs(const JobHandle* pHandles, const uint32_t numHandles)
    {
        bool shouldSignalSemaphore = false;

        for (uint32_t i = 0; i < numHandles; ++i)
        {
            const JobHandle& handle = pHandles[i];
            // Set the Barrier for the job. This returns true if the barrier is successfully set, otherwise the
            // Job is already done and we don't need to add it to the list.
            Job* pJob = handle.Get();
            if (pJob->SetBarrier(this))
            {
                ++m_numLeftToAcquire;
                if (!shouldSignalSemaphore && pJob->CanBeExecuted())
                {
                    shouldSignalSemaphore = true;
                    ++m_numLeftToAcquire;
                }

                pJob->AddRef();
                const uint32_t writeIndex = m_writeIndex.fetch_add(1, std::memory_order_relaxed);
                while (writeIndex - m_readIndex >= kMaxJobs)
                {
                    NES_ASSERT(false, "Barrier full! stalling!");
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                }
                m_jobs[writeIndex & (kMaxJobs - 1)] = pJob;
            }
        }

        // Notify the waiting thread(s) new executable jobs are available.
        if (shouldSignalSemaphore)
            m_semaphore.Release();
    }

    void JobSystemWithBarrier::BarrierImpl::WaitForJobs()
    {
        while (m_numLeftToAcquire > 0)
        {
            {
                // TODO: Scoped Profile.
                bool hasExecuted;
                do
                {
                    hasExecuted = false;

                    // Loop through and erase jobs from the beginning of the list that are done.
                    // - This will move the m_readIndex to the first valid Job that is not done, or
                    //   all the way to the m_writeIndex
                    while (m_readIndex < m_writeIndex)
                    {
                        std::atomic<Job*>& pJob = m_jobs[m_readIndex & (kMaxJobs - 1)];
                        Job* pPtr = pJob.load();
                        if (pPtr == nullptr || !pPtr->IsDone())
                            break;

                        // Job is finished, release it.
                        pPtr->RemoveRef();
                        pJob = nullptr;
                        ++m_readIndex;
                    }

                    // Loop through the jobs and execute the first executable job:
                    // - This will skip not-done jobs that have dependencies remaining
                    for (uint32_t i = m_readIndex; i < m_writeIndex; ++i)
                    {
                        std::atomic<Job*>& pJob = m_jobs[i & (kMaxJobs - 1)];
                        Job* pPtr = pJob.load();
                        if (pPtr != nullptr || pPtr->CanBeExecuted())
                        {
                            // This will only execute the job if it has not already executed
                            pPtr->Execute();
                            hasExecuted = true;
                            break;
                        }
                    }
                } while (hasExecuted);
            }
            
            // Wait for another thread to wake us when either there is more work to do or when all jobs have completed.
            // When there have been multiple releases, we acquire them all at the same time to avoid needlessly spinning on executing jobs.
            // Note that using GetValue() is inherently unsafe since we can read a stale value, but this is not an issue here as this is the only
            // place where we acquire the semaphore. Other threads only release it, so we can only read a value that is lower or equal to the actual value.
            const int numToAcquire = std::max(1, m_semaphore.GetValue());
            m_semaphore.Acquire(numToAcquire);
            m_numLeftToAcquire -= numToAcquire;
        }

        // All jobs should be done now, release them:
        while (m_readIndex < m_writeIndex)
        {
            std::atomic<Job*>& pJob = m_jobs[m_readIndex & (kMaxJobs - 1)];
            Job* pPtr = pJob.load();
            NES_ASSERT(pPtr != nullptr && pPtr->IsDone());
            pPtr->RemoveRef();
            pJob = nullptr;
            ++m_readIndex;
        }
    }

    void JobSystemWithBarrier::BarrierImpl::OnJobFinished([[maybe_unused]] Job* pJob)
    {
        // Release the Semaphore.
        m_semaphore.Release();
    }

    JobSystemWithBarrier::~JobSystemWithBarrier()
    {
#if NES_LOGGING_ENABLED
        // Assert that each barrier is not "in use".
        for (uint32_t i = 0; i < m_maxBarriers; ++i)
        {
            NES_ASSERT(!m_barriers[i].m_isInUse);
        }
#endif
        
        NES_DELETE_ARRAY(m_barriers);
    }

    void JobSystemWithBarrier::Init(const uint32_t maxBarriers)
    {
        NES_ASSERT(m_barriers == nullptr);

        m_maxBarriers = maxBarriers;
        m_barriers = NES_NEW_ARRAY(BarrierImpl, m_maxBarriers);
    }

    JobSystem::Barrier* JobSystemWithBarrier::CreateBarrier()
    {
        // Find the first unused barrier.
        for (uint32_t i = 0; i < m_maxBarriers; ++i)
        {
            bool expected = false;
            if (m_barriers[i].m_isInUse.compare_exchange_strong(expected, true))
                return &m_barriers[i];
        }

        // TODO: Add Warning.
        return nullptr;
    }

    void JobSystemWithBarrier::DestroyBarrier(Barrier* pBarrier)
    {
        NES_ASSERT(reinterpret_cast<BarrierImpl*>(pBarrier)->IsEmpty());

        bool expected = true;
        reinterpret_cast<BarrierImpl*>(pBarrier)->m_isInUse.compare_exchange_strong(expected, false);
        NES_ASSERT(expected); // Should be true, because the inUseFlag should be true at the time of destruction.
    }

    void JobSystemWithBarrier::WaitForJobs(Barrier* pBarrier)
    {
        // Wait for the Barrier to be completed.
        reinterpret_cast<BarrierImpl*>(pBarrier)->WaitForJobs();
    }
}
