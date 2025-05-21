// JobSystem.cpp
#include "JobSystem.h"

namespace nes
{
    JobSystem::JobHandle::JobHandle(Job* pJob)
        : StrongPtr<JobSystem::Job>(pJob)
    {
        //
    }
    
    void JobSystem::JobHandle::RemoveDependencies(const JobHandle* pHandles, const uint32_t numHandles, const int count)
    {
        NES_ASSERT(pHandles != nullptr);
        NES_ASSERT(numHandles > 0);

        JobSystem* pSystem = pHandles->Get()->GetJobSystem();

        // Allocate a buffer to store the jobs that need to be queued.
        Job** pJobsToQueue = static_cast<Job**>(NES_STACK_ALLOCATE(numHandles * sizeof(Job*)));
        Job** pNextJob = pJobsToQueue;

        // Remove the dependencies on all jobs
        for (const JobHandle* pHandle = pHandles, *pHandleEnd = pHandles + numHandles; pHandle < pHandleEnd; ++pHandle)
        {
            Job* pJob = pHandle->Get();
            NES_ASSERT(pJob->GetJobSystem() == pSystem);
            if (pJob->RemoveDependency(count))
            {
                *(pNextJob) = pJob;
                ++pNextJob;
            }
        }

        // If any jobs need to be scheduled, schedule them as a batch.
        const uint32_t numJobsToQueue = static_cast<uint32_t>(pNextJob - pJobsToQueue);
        if (numJobsToQueue > 0)
            pSystem->QueueJobs(pJobsToQueue, numJobsToQueue);
    }

    JobSystem::Job::Job(const char* pName/*, const Color& color*/, JobSystem* pSystem, const JobFunction& function, const uint32_t numDependencies)
        : m_name(pName)
        , m_pJobSystem(pSystem)
        , m_function(function)
        , m_numDependencies(numDependencies)
        //, m_color(color)
    {
        //
    }
    
    void JobSystem::Job::ReleaseObjectImpl(Job* pThisObject) const
    {
        m_pJobSystem->FreeJob(pThisObject);
    }

    void JobSystem::Job::AddDependency(const int count)
    {
        m_numDependencies.fetch_add(count, std::memory_order_relaxed);
    }

    bool JobSystem::Job::RemoveDependency(const int count)
    {
        const uint32_t oldValue = m_numDependencies.fetch_sub(count, std::memory_order_release);
        const uint32_t newValue = oldValue - count;
        NES_ASSERTV(oldValue > newValue, "Removed more dependencies than were set for Job!");
        
        return newValue == 0;
    }

    void JobSystem::Job::RemoveDependencyAndQueue(const int count)
    {
        if (RemoveDependency(count))
            m_pJobSystem->QueueJob(this);
    }

    bool JobSystem::Job::SetBarrier(Barrier* pBarrier)
    {
        intptr_t barrier = 0;
        if (m_barrier.compare_exchange_strong(barrier, reinterpret_cast<intptr_t>(pBarrier), std::memory_order_relaxed))
            return true;

        NES_ASSERTV(barrier == kBarrierDoneState, "A job can only belong to one barrier!");
        return false;
    }

    uint32_t JobSystem::Job::Execute()
    {
        // Transition to the executing state.
        uint32_t state = 0; // Assume that the dependency count is 0.
        if (!m_numDependencies.compare_exchange_strong(state, kExecutingState, std::memory_order_acquire))
            return state; // We still have dependencies! Return the number of dependencies.

        // Run the Job:
        {
            // [TODO]: Scoped Profile
            m_function();
        }

        // Fetch the Barrier pointer and exchange it for the done state, so we're sure that no barrier gets set after
        // we want to call the callback.
        intptr_t barrier = m_barrier.load(std::memory_order_relaxed);
        while (!m_barrier.compare_exchange_weak(barrier, kBarrierDoneState, std::memory_order_relaxed))
        {
            //
        }
        NES_ASSERT(barrier != kBarrierDoneState);

        // Mark the Job as done.
        state = kExecutingState;
        m_numDependencies.compare_exchange_strong(state, kDoneState, std::memory_order_relaxed);

        if (barrier != 0)
            reinterpret_cast<Barrier*>(barrier)->OnJobFinished(this);

        return kDoneState;
    }
}
