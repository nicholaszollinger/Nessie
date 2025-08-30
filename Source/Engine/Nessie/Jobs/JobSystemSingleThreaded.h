// JobSystemSingleThreaded.h
#pragma once
#include "Nessie/Jobs/JobSystem.h"
#include "Nessie/Core/Memory/FixedSizedFreeList.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Job System implementation that executes Jobs immediately as they are created.
    //----------------------------------------------------------------------------------------------------
    class JobSystemSingleThreaded final : public JobSystem
    {
        //----------------------------------------------------------------------------------------------------
        /// @brief : The Barrier class does nothing, as we don't need to track Jobs in the single threaded
        ///     execution.
        //----------------------------------------------------------------------------------------------------
        class BarrierImpl final : public Barrier
        {
        public:
            virtual void    AddJob(const JobHandle&) override {}
            virtual void    AddJobs(const JobHandle*, const uint32_t) override {} 

        protected:
            virtual void    OnJobFinished(Job*) override {}
        };

        using JobArray      = FixedSizeFreeList<Job>;
        
    public:
                            JobSystemSingleThreaded() = default;
        explicit            JobSystemSingleThreaded(const uint32_t maxJobs) { Init(maxJobs); }

    public:
        void                Init(const uint32_t maxJobs);
        virtual int         GetMaxConcurrency() override { return 1; }
        virtual JobHandle   CreateJob(const char* pName, const JobFunction& jobFunction, const uint32_t numDependencies) override;
        virtual Barrier*    CreateBarrier() override;
        virtual void        DestroyBarrier(Barrier* pBarrier) override;
        virtual void        WaitForJobs(Barrier* pBarrier) override;
    
    private:    
        virtual void        QueueJob(Job* pJob) override;
        virtual void        QueueJobs(Job** pJobs, const uint32_t numHandles) override;
        virtual void        FreeJob(Job* pJob) override;

    private:
        JobArray            m_jobs;
        BarrierImpl         m_dummyBarrier{};
    };
}
