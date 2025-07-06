// JobSystemWithBarrier.h
#pragma once
#include "Nessie/Core/Jobs/JobSystem.h"
#include "Nessie/Core/Thread/Semaphore.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper class that implements the Barrier interface for the Job System.  
    //----------------------------------------------------------------------------------------------------
    class JobSystemWithBarrier : public JobSystem
    {
        class BarrierImpl final : public Barrier
        {
            static constexpr uint32_t kMaxJobs = 2048;
            
        public:
            BarrierImpl();
            virtual             ~BarrierImpl() override;

        public:
            /// Whether this Barrier is currently being used or not.
            std::atomic_bool    m_isInUse { false };

        public:
            virtual void        AddJob(const JobHandle& handle) override;
            virtual void        AddJobs(const JobHandle* pHandles, const uint32_t numHandles) override;
            inline bool         IsEmpty() const { return m_readIndex == m_writeIndex; }
            void                WaitForJobs();

        private:
            virtual void        OnJobFinished(Job* pJob) override;
            
            std::atomic<Job*>   m_jobs[kMaxJobs];
            alignas (NES_CACHE_LINE_SIZE) std::atomic<uint32> m_readIndex{0};
            alignas (NES_CACHE_LINE_SIZE) std::atomic<uint32> m_writeIndex{0};
            Semaphore           m_semaphore{0};
            
            /// At the start of the WaitForJobs(), this will be equal to (the number of calls to AddJob() & AddJobs()) +
            /// (the number of Jobs that have been scheduled).
            std::atomic<uint32> m_numLeftToAcquire{0};
        };
    
    public:
        JobSystemWithBarrier() = default;
        explicit            JobSystemWithBarrier(const uint32_t maxBarriers) { Init(maxBarriers); }
        virtual             ~JobSystemWithBarrier() override;

        void                Init(const uint32_t maxBarriers);

        virtual Barrier*    CreateBarrier() override;
        virtual void        DestroyBarrier(Barrier* pBarrier) override;
        virtual void        WaitForJobs(Barrier* pBarrier) override;

    private:
        uint32              m_maxBarriers = 0;
        BarrierImpl*        m_barriers = nullptr;
    };
}