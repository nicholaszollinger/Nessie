// JobSystemWorkerThread.h
#pragma once
#include "Nessie/Jobs/JobSystemWithBarrier.h"
#include "Nessie/Core/Memory/FixedSizedFreeList.h"
#include "Nessie/Core/Thread/WorkerThread.h"
#include "Nessie/Core/Thread/Containers/ThreadSafeQueue.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : JobSystem that uses a single WorkerThread to execute Jobs. 
    //----------------------------------------------------------------------------------------------------
    class JobSystemWorkerThread final : public JobSystemWithBarrier
    {
        enum class JobThreadInstruction : uint8_t
        {
            Init,
            JobsAvailable,
            Terminate,
        };
        
    public:
        /// Function signature for both Initialization and Termination functors of the Worker Thread.
        using ThreadInitExitFunction = std::function<void()>;
        
    private:
        using JobArray = FixedSizeFreeList<Job>;
        static constexpr uint32_t kQueueLength = 512;
    
    public:
                            JobSystemWorkerThread() = default;
        explicit            JobSystemWorkerThread(const uint32_t maxJobs, const uint32_t maxBarriers);
        virtual             ~JobSystemWorkerThread() override;

    public:
        void                SetThreadInitFunction(const ThreadInitExitFunction& threadInitFunction) { m_threadInitFunction = threadInitFunction; }
        void                SetThreadExitFunction(const ThreadInitExitFunction& threadExitFunction) { m_threadExitFunction = threadExitFunction; }
        void                Init(const uint32_t maxJobs, const uint32_t maxBarriers);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : The Maximum concurrency is still 1 - the Jobs are just executed on another thread. 
        //----------------------------------------------------------------------------------------------------
        virtual int         GetMaxConcurrency() override { return 1; }
        virtual JobHandle   CreateJob(const char* pName, const JobFunction& jobFunction, const uint32_t numDependencies) override;

    private:
        virtual void        QueueJob(Job* pJob) override;
        virtual void        QueueJobs(Job** pJobs, const uint32_t numHandles) override;
        void                QueueJobInternal(Job* pJob);
        virtual void        FreeJob(Job* pJob) override;

        /// Thread API
        bool                ProcessInstruction(const JobThreadInstruction instruction);
        void                ThreadProcessJobQueue();
        void                ThreadTerminate();

    private:
        WorkerThread<JobThreadInstruction>  m_workerThread;
        JobArray                            m_jobs;
        std::atomic<Job*>                   m_jobQueue[kQueueLength];
        std::atomic<uint32_t>               m_queueHead;
        std::atomic<uint32_t>               m_queueTail;
        ThreadInitExitFunction              m_threadInitFunction = [](){};
        ThreadInitExitFunction              m_threadExitFunction = [](){};
    };
}
