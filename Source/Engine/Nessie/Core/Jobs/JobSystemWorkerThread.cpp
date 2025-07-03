// JobSystemWorkerThread.cpp
#include "Nessie/Core/Jobs/JobSystemWorkerThread.h"

namespace nes
{
    JobSystemWorkerThread::JobSystemWorkerThread(const uint32_t maxJobs, const uint32_t maxBarriers)
    {
        Init(maxJobs, maxBarriers);
    }

    JobSystemWorkerThread::~JobSystemWorkerThread()
    {
        // Terminate the Thread:
        m_workerThread.SendInstruction(JobThreadInstruction::Terminate);
    }

    void JobSystemWorkerThread::Init(const uint32_t maxJobs, const uint32_t maxBarriers)
    {
        JobSystemWithBarrier::Init(maxBarriers);
        m_jobs.Init(maxJobs, maxJobs);

        // Init the queue.
        for (std::atomic<Job*>& job : m_jobQueue)
        {
            job = nullptr;
        }

        // Start and initialize the thread.
        const auto handler = [this](const JobThreadInstruction instruction) -> bool
        {
            return ProcessInstruction(instruction);
        };
        m_workerThread.Start(handler);
        m_workerThread.SendInstruction(JobThreadInstruction::Init);
        m_workerThread.WaitUntilDone();
    }

    JobHandle JobSystemWorkerThread::CreateJob(const char* pName, const JobFunction& jobFunction, const uint32_t numDependencies)
    {
        uint32_t index;
        // Loop until we get a job from the free list.
        while (true)
        {
            index = m_jobs.ConstructObject(pName, this, jobFunction, numDependencies);
            if (index != JobArray::kInvalidObjectIndex)
                break;

            NES_ASSERT(false, "No Jobs available!");
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }

        Job* pJob = &m_jobs.Get(index);
        JobHandle handle(pJob);

        // If there are no dependencies, queue the job now.
        if (numDependencies == 0)
            QueueJob(pJob);

        return handle;
    }

    void JobSystemWorkerThread::QueueJob(Job* pJob)
    {
        QueueJobInternal(pJob);
        m_workerThread.SendInstruction(JobThreadInstruction::JobsAvailable);
    }

    void JobSystemWorkerThread::QueueJobs(Job** pJobs, const uint32_t numHandles)
    {
        for (uint32_t i = 0; i < numHandles; ++i)
        {
            QueueJobInternal(pJobs[i]);
        }
        
        m_workerThread.SendInstruction(JobThreadInstruction::JobsAvailable);
    }

    void JobSystemWorkerThread::QueueJobInternal(Job* pJob)
    {
        // Add a reference to the Job because we are adding it to the queue.
        pJob->AddRef();

        uint32_t head = m_queueHead.load();

        for (;;)
        {
            uint32_t oldValue = m_queueTail;
            if (oldValue - head >= kQueueLength)
            {
                // We calculated the head outside the loop, update both
                // head and tail.
                head = m_queueHead.load();
                oldValue = m_queueTail;

                // Second check to see if there is any space.
                if (oldValue - head >= kQueueLength)
                {
                    // Make sure to wake up the thread to clear any nullptr that have not
                    // been processed.
                    m_workerThread.SendInstruction(JobThreadInstruction::JobsAvailable);

                    // Sleep a little. We need to wait for the thread to make room.
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                    continue;
                }
            }

            Job* pExpectedJob = nullptr;
            const bool success = m_jobQueue[oldValue & (kQueueLength - 1)].compare_exchange_strong(pExpectedJob, pJob);

            // Regardless of who wrote into the Queue slot, update the tail (if the successful thread got scheduled out
            // after writing the pointer we still want to be able to continue).
            m_queueTail.compare_exchange_strong(oldValue, oldValue + 1);

            // If we claimed a slot in the Queue, break.
            if (success)
                break;
        }
    }

    void JobSystemWorkerThread::FreeJob(Job* pJob)
    {
        m_jobs.DestructObject(pJob);
    }

    bool JobSystemWorkerThread::ProcessInstruction(const JobThreadInstruction instruction)
    {
        switch (instruction)
        {
            case JobThreadInstruction::Init:            m_threadInitFunction(); break;
            case JobThreadInstruction::JobsAvailable:   ThreadProcessJobQueue(); break; 
            case JobThreadInstruction::Terminate:       ThreadTerminate(); return false;
        }

        return true;
    }

    void JobSystemWorkerThread::ThreadProcessJobQueue()
    {
        while (m_queueHead != m_queueTail)
        {
            std::atomic<Job*>& job = m_jobQueue[m_queueHead & (kQueueLength - 1)];
            if (job.load() != nullptr)
            {
                Job* pJob = job.exchange(nullptr);
                if (pJob != nullptr)
                {
                    pJob->Execute();
                    pJob->RemoveRef();
                }
            }
            ++m_queueHead;
        }
    }

    void JobSystemWorkerThread::ThreadTerminate()
    {
        // Ensure that there are no lingering Jobs.
        for (uint32_t head = 0; head != m_queueTail; ++head)
        {
            Job* pJob = m_jobQueue[head & (kQueueLength - 1)].exchange(nullptr);
            if (pJob != nullptr)
            {
                pJob->Execute();
                pJob->RemoveRef();
            }
        }

        m_queueHead = 0;
        m_queueTail = 0;
        
        m_threadExitFunction();
    }
}
