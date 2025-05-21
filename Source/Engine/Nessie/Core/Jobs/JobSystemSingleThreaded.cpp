// JobSystemSingleThreaded.cpp
#include "JobSystemSingleThreaded.h"

namespace nes
{
    void JobSystemSingleThreaded::Init(const uint32_t maxJobs)
    {
        m_jobs.Init(maxJobs, maxJobs);
    }

    JobHandle JobSystemSingleThreaded::CreateJob(const char* pName, const JobFunction& jobFunction, const uint32_t numDependencies)
    {
        // Construct the new Job
        const uint32_t index = m_jobs.ConstructObject(pName, this, jobFunction, numDependencies);
        NES_ASSERT(index != JobArray::kInvalidObjectIndex);
        Job* pJob = &m_jobs.Get(index);

        // Create a handle to keep a reference. This job is queued below and will immediately
        // complete.
        JobHandle handle(pJob);

        // If there are no dependencies, queue the Job now.
        if (numDependencies == 0)
            QueueJob(pJob);

        return handle; 
    }

    JobSystem::Barrier* JobSystemSingleThreaded::CreateBarrier()
    {
        return &m_dummyBarrier;
    }

    void JobSystemSingleThreaded::DestroyBarrier([[maybe_unused]] Barrier* pBarrier)
    {
        // Do nothing.
    }

    void JobSystemSingleThreaded::WaitForJobs([[maybe_unused]] Barrier* pBarrier)
    {
        // Do nothing. We execute jobs immediately.
    }

    void JobSystemSingleThreaded::QueueJob(Job* pJob)
    {
        pJob->Execute();
    }

    void JobSystemSingleThreaded::QueueJobs(Job** pJobs, const uint32_t numHandles)
    {
        for (uint32_t i = 0; i < numHandles; ++i)
            QueueJob(pJobs[i]);
    }

    void JobSystemSingleThreaded::FreeJob(Job* pJob)
    {
        m_jobs.DestructObject(pJob);
    }
}
