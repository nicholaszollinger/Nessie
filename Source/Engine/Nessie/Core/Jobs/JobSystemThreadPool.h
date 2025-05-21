// JobSystemThreadPool.h
#pragma once
#include "JobSystemWithBarrier.h"
#include "Core/Memory/FixedSizedFreeList.h"

namespace nes
{
    class JobSystemThreadPool final : public JobSystemWithBarrier
    {
    public:
        /// Function signature for both Initialization and Termination functors of the Worker Thread.
        using ThreadInitExitFunction = std::function<void(const int threadIndex)>;

    private:
        static constexpr uint32_t kQueueLength = 1024;
        using ThreadArray = std::vector<std::thread>;
        using AvailableJobs = FixedSizeFreeList<Job>;
        
        AvailableJobs           m_jobs;
        ThreadArray             m_threads;
        std::atomic<Job*>       m_jobQueue[kQueueLength];
        std::atomic<uint32_t>*  m_queueHeads = nullptr;
        std::atomic<uint32_t>   m_queueTail;
        Semaphore               m_semaphore;
        std::atomic_bool        m_quit = false;
        ThreadInitExitFunction  m_threadInitFunction = [](int){ };
        ThreadInitExitFunction  m_threadExitFunction = [](int){ };
        
    public:
        JobSystemThreadPool(const uint32_t maxJobs, const uint32_t maxBarriers, const int numThreads = -1);
        virtual             ~JobSystemThreadPool() override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the initialization function for a Job Thread.
        /// @note : Must be set before calling Init().
        //----------------------------------------------------------------------------------------------------
        void                SetThreadInitFunction(const ThreadInitExitFunction& function) { m_threadInitFunction = function; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the exit function for a Job Thread.
        /// @note : Must be set before calling Init().
        //----------------------------------------------------------------------------------------------------
        void                SetThreadExitFunction(const ThreadInitExitFunction& function) { m_threadExitFunction = function; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the Thread Pool.
        ///	@param maxJobs : Maximum number of Jobs that can be allocated at any time.
        ///	@param maxBarriers : Maximum number of Barriers that can be allocated at any time.
        ///	@param numThreads : Number of threads to start (the number of concurrent jobs is 1 more because the
        ///     main thread will also run jobs while waiting for a barrier to complete. Use -1 to auto-detect
        ///     the amount of CPUs.
        //----------------------------------------------------------------------------------------------------
        void                Init(const uint32_t maxJobs, const uint32_t maxBarriers, const int numThreads = -1);
        
        virtual int         GetMaxConcurrency() override { return static_cast<int>(m_threads.size()) + 1; }
        virtual JobHandle   CreateJob(const char* pName, const JobFunction& jobFunction, const uint32_t numDependencies) override;

    private:
        virtual void        QueueJob(Job* pJob) override;
        virtual void        QueueJobs(Job** pJobs, const uint32_t numHandles) override;
        virtual void        FreeJob(Job* pJob) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Internal helper function to Queue a Job.
        //----------------------------------------------------------------------------------------------------
        void                QueueJobInternal(Job* pJob);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Start the Worker Threads. 
        //----------------------------------------------------------------------------------------------------
        void                StartThreads(int numThreads);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Stop the Worker Threads. 
        //----------------------------------------------------------------------------------------------------
        void                StopThreads();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Head of the Thread that has processed the least amount of jobs. 
        //----------------------------------------------------------------------------------------------------
        inline uint32_t     GetHead() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Entry point for a worker thread. 
        //----------------------------------------------------------------------------------------------------
        void                ThreadMain(int threadIndex);
    };
}
