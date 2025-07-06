// JobSystemThreadPool.cpp
#include "Nessie/Core/Jobs/JobSystemThreadPool.h"

namespace nes
{
    JobSystemThreadPool::JobSystemThreadPool(const uint32_t maxJobs, const uint32_t maxBarriers, const int numThreads)
    {
        Init(maxJobs, maxBarriers, numThreads);
    }

    JobSystemThreadPool::~JobSystemThreadPool()
    {
        StopThreads();
    }

    void JobSystemThreadPool::Init(const uint32_t maxJobs, const uint32_t maxBarriers, const int numThreads)
    {
        JobSystemWithBarrier::Init(maxBarriers);

        // Init the Jobs free list.
        m_jobs.Init(maxJobs, maxJobs);

        // Init the Queue
        for (auto& job : m_jobQueue)
        {
            job = nullptr;
        }

        // Start up the worker threads.
        StartThreads(numThreads);
    }

    JobHandle JobSystemThreadPool::CreateJob(const char* pName, const JobFunction& jobFunction, const uint32_t numDependencies)
    {
        uint32_t index;
        // Loop until we have an available Job.
        for (;;)
        {
            index = m_jobs.ConstructObject(pName, this, jobFunction, numDependencies);
            if (index != AvailableJobs::kInvalidObjectIndex)
                break;

            NES_ASSERT(false, "No jobs available!");
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        Job* pJob = &m_jobs.Get(index);

        // Construct the handle to keep a reference.
        JobHandle handle(pJob);

        // If there are no dependencies, queue the job now.
        if (numDependencies == 0)
            QueueJob(pJob);

        return handle;
    }

    void JobSystemThreadPool::QueueJob(Job* pJob)
    {
        if (m_threads.empty())
            return;
        
        // Queue the Job then wake up the thread.
        QueueJobInternal(pJob);
        m_semaphore.Release();
    }

    void JobSystemThreadPool::QueueJobs(Job** pJobs, const uint32_t numHandles)
    {
        if (m_threads.empty())
            return;

        NES_ASSERT(pJobs != nullptr && numHandles > 0);

        for (uint32_t i = 0; i < numHandles; ++i)
        {
            QueueJobInternal(pJobs[i]);
        }

        // Wake up threads.
        m_semaphore.Release(std::min(numHandles, static_cast<uint32_t>(m_threads.size())));
    }

    void JobSystemThreadPool::FreeJob(Job* pJob)
    {
        m_jobs.DestructObject(pJob);
    }

    void JobSystemThreadPool::QueueJobInternal(Job* pJob)
    {
        // Add a reference to the Job because we're adding it to the queue.
        pJob->AddRef();

        // Need to read head first because otherwise the tail can already have passed the head.
        // We read the head outside the loop since it involves iterating over all threads, and we only need
        // to update it if there's not enough space.
        uint32_t head = GetHead();

        for (;;)
        {
            // Check if there is space in the queue.
            uint32_t oldValue = m_queueTail;
            if (oldValue - head >= kQueueLength)
            {
                // We calculated the head outside the loop, update the head and tail to prevent
                // it from passing the head.
                head = GetHead();
                oldValue = m_queueTail;
                if (oldValue - head >= kQueueLength)
                {
                    // Wake up all threads in order to ensure that they can clear any nullptr jobs they
                    // may not have processed yet.
                    m_semaphore.Release(static_cast<uint32_t>(m_threads.size()));

                    // Sleep a little to wait for the threads to update their head pointer.
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                    continue;
                }
            }

            // Try to claim a job slot:
            Job* pExpected = nullptr;
            const bool success = m_jobQueue[oldValue & (kQueueLength - 1)].compare_exchange_strong(pExpected, pJob);

            // Regardless of who got there first to claim the slot, update the tail. If the successful thread was
            // beaten to the punch, we still want to be able to continue.
            m_queueTail.compare_exchange_strong(oldValue, oldValue + 1);

            // If we successfully claimed the slot in the queue, we're done.
            if (success)
                break;
        }
    }

    void JobSystemThreadPool::StartThreads(int numThreads)
    {
        // Assuming Thread support.
        // If less than zero, assume that we want all available - 1 (subtract 1 for main thread).
        if (numThreads < 0)
            numThreads = static_cast<int>(std::thread::hardware_concurrency()) - 1;

        // If no threads requested, return
        if (numThreads == 0)
            return;

        // Don't quit the threads.
        m_quit = false;

        // Allocate the head for each thread.
        m_queueHeads = static_cast<std::atomic<uint32_t>*>(NES_ALLOC(sizeof(std::atomic<uint32_t>) * numThreads));
        for (int i = 0; i < numThreads; ++i)
        {
            m_queueHeads[i] = 0;
        }

        // Start the threads:
        NES_ASSERT(m_threads.empty());
        m_threads.reserve(numThreads);
        for (int i = 0; i < numThreads; ++i)
        {
            m_threads.emplace_back([this, i]() { ThreadMain(i); } );
        }
    }

    void JobSystemThreadPool::StopThreads()
    {
        if (m_threads.empty())
            return;

        // Signal threads that we want to quit.
        m_quit = true;
        m_semaphore.Release(static_cast<unsigned>(m_threads.size()));

        // Wait for all threads to finish
        for (auto& thread : m_threads)
        {
            if (thread.joinable())
                thread.join();
        }
        m_threads.clear();

        // Ensure that there are no lingering Jobs
        for (uint32_t head = 0; head != m_queueTail; ++head)
        {
            Job* pJob = m_jobQueue[head & (kQueueLength - 1)].exchange(nullptr);
            if (pJob != nullptr)
            {
                pJob->Execute();
                pJob->RemoveRef();
            }
        }

        // Destroy heads and reset.
        NES_FREE(m_queueHeads);
        m_queueHeads = nullptr;
        m_queueTail = 0; 
    }

    uint32_t JobSystemThreadPool::GetHead() const
    {
        // Find the minimal value across all threads.
        uint32_t head = m_queueTail;
        for (size_t i = 0; i < m_threads.size(); ++i)
        {
            head = std::min(head, m_queueHeads[i].load());
        }
        return head;
    }

    void JobSystemThreadPool::ThreadMain(const int threadIndex)
    {
        // [TODO]: Name the thread:

        // Call initialization function:
        m_threadInitFunction(threadIndex);

        // Get the head index associated with this thread.
        std::atomic<uint32_t>& head = m_queueHeads[threadIndex]; 
        
        while (!m_quit)
        {
            // Wait for jobs
            m_semaphore.Acquire();

            {
                // [TODO]: Scoped Profile for Job Execution.
                while (head != m_queueTail)
                {
                    std::atomic<Job*>& job = m_jobQueue[head & (kQueueLength - 1)];
                    if (job.load() != nullptr)
                    {
                        // Attempt to claim this Job for this Thread.
                        Job* pJob = job.exchange(nullptr);
                        if (pJob != nullptr)
                        {
                            pJob->Execute();
                            pJob->RemoveRef();
                        }
                    }
                    ++head;
                }
            }
        }

        // Call the exit function:
        m_threadExitFunction(threadIndex);
    }
}
