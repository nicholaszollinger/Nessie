// JobSystem.h
#pragma once
#include <functional>
#include "Nessie/Core/StaticArray.h"
#include "Nessie/Core/Memory/StrongPtr.h"
#include "Nessie/Debug/Assert.h"

namespace nes
{
    NES_DEFINE_LOG_TAG(LogJobSystem, "Job", Warn);
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Job System facilitates the execution of "Jobs" which are essentially functors. A JobSystem
    ///     is meant to execute Jobs on one or more threads. Jobs can have dependencies so that their order of
    ///     execution is handled correctly.
    ///
    ///     A "JobBarrier" is used to track the completion of a set of jobs. Jobs can be created by other jobs
    ///     and be added to the barrier while it is being waited on.
    ///
    ///     The JobSystem is an abstract class, you will have to implement a version that best suits your situation.
    ///     
    ///     Example Usage:
    ///     <code>     
    ///         nes::JobSystem* pSystem = NES_NEW(JobSystemImplementation());
    /// 
    ///         // Create some Jobs.
    ///         // Job 2 is dependent on Job 1 completing, but Job 3 can be executed as soon as possible.
    ///         nes::JobHandle job2 =  pJobSystem->CreateJob("Job 2", []() { }, 1);
    ///         nes::JobHandle job1 =  pJobSystem->CreateJob("Job 1", [job2]() { job2.RemoveDependency();}, 0);
    ///         nes::JobHandle job3 =  pJobSystem->CreateJob("Job 3", []() { }, 0);
    /// 
    ///         // Create a Barrier to wait on the Jobs to complete.
    ///         nes::JobBarrier* pBarrier = pJobSystem->CreateBarrier();
    ///         pBarrier->AddJob(job1);
    ///         pBarrier->AddJob(job2);
    ///         pBarrier->AddJob(job3);
    ///     
    ///         // Block this thread until all Jobs are complete.
    ///         pJobSystem->WaitForJobs(pBarrier);
    /// 
    ///         // Cleanup
    ///         pJobSystem->DestroyBarrier(pBarrier);
    ///         NES_DELETE(pJobSystem);
    ///     </code>
    //----------------------------------------------------------------------------------------------------
    class JobSystem
    {
    protected:
        class Job;
        
    public:
        using JobFunction = std::function<void()>;

        //----------------------------------------------------------------------------------------------------
        /// @brief : A Job Handle contains a reference to a job. The job will be deleted as soon as there are
        ///     no Job Handles referring to the Jo and when it is not in the Job queue / being processed.
        //----------------------------------------------------------------------------------------------------
        class JobHandle final : private StrongPtr<Job>
        {
        public:
            using StrongPtr<Job>::Get;
        
            JobHandle() = default;
            ~JobHandle() = default;
            JobHandle(const JobHandle&) = default;
            JobHandle(JobHandle&& handle) noexcept : StrongPtr<Job>(std::move(handle)) { }
            explicit JobHandle(Job* pJob) : StrongPtr<Job>(pJob) { }

            JobHandle& operator=(const JobHandle&) = default;
            JobHandle& operator=(JobHandle&& handle) noexcept = default;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Check if this handle points to a Job. 
            //----------------------------------------------------------------------------------------------------
            bool                IsValid() const                                { return Get() != nullptr;}
            
            //----------------------------------------------------------------------------------------------------
            /// @brief : Checks if this handle's Job is done executing.
            //----------------------------------------------------------------------------------------------------
            bool                IsDone() const                                 { return Get() != nullptr && Get()->IsDone(); }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Increment the dependency counter of this handle's Job.
            //----------------------------------------------------------------------------------------------------
            void                AddDependency(const int count = 1) const           { Get()->AddDependency(count); }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Decrement the dependency counter of this handle's Job.
            //----------------------------------------------------------------------------------------------------
            void                RemoveDependency(const int count = 1) const    { Get()->RemoveDependencyAndQueue(count); }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Remove a dependency from a batch of Jobs at once. This can be more efficient than removing
            ///         one by one because it requires less locking.
            //----------------------------------------------------------------------------------------------------
            static inline void  RemoveDependencies(const JobHandle* pHandles, const uint32 numHandles, const int count = 1);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Helper function to remove dependencies using a static array of Job Handles.
            //----------------------------------------------------------------------------------------------------
            template <uint32_t N>
            static inline void  RemovedDependencies(StaticArray<JobHandle, N>& handles, const int count = 1)
            {
                RemoveDependencies(handles.data(), static_cast<uint32>(handles.size()), count);
            }
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : A Job Barrier keeps track of a number of jobs and allows waiting until they are all completed. 
        //----------------------------------------------------------------------------------------------------
        class Barrier
        {
        protected:
            friend class Job;

        public:
            Barrier() = default;
            Barrier(const Barrier&) = delete;
            Barrier& operator=(const Barrier&) = delete;

        protected:
            //----------------------------------------------------------------------------------------------------
            /// @brief : Protected Destructor - You should be calling DestroyBarrier() instead of destructing directly. 
            //----------------------------------------------------------------------------------------------------
            virtual             ~Barrier() = default;
        
        public:
            //----------------------------------------------------------------------------------------------------
            /// @brief : Add a job to this Barrier. 
            /// @note : Jobs can keep being added to the barrier while waiting for the Barrier to finish - Jobs can
            ///     create other Jobs - and all will be waited on.
            //----------------------------------------------------------------------------------------------------
            virtual void        AddJob(const JobHandle& handle) = 0;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Add multiple Jobs to this Barrier.
            /// @note : Jobs can keep being added to the barrier while waiting for the Barrier to finish - Jobs can
            ///     create other Jobs and all will be waited on.
            //----------------------------------------------------------------------------------------------------
            virtual void        AddJobs(const JobHandle* pHandles, const uint32_t numHandles) = 0;
        
        protected:
            //----------------------------------------------------------------------------------------------------
            /// @brief : Called by a Job to mark that it is finished. 
            //----------------------------------------------------------------------------------------------------
            virtual void        OnJobFinished([[maybe_unused]] Job* pJob) = 0;
        };

    protected:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Internal Job class, which is ultimately a functor that is executed on a thread. It contains
        ///     data for tracking references and dependencies, as well as its owning JobSystem and optional Barrier.
        ///     - Jobs are queued for execution as soon as their dependency count reaches 0.
        ///     - Jobs are freed as soon as their reference count reaches 0.
        //----------------------------------------------------------------------------------------------------
        class Job final : public RefTarget<Job>
        {
        public:
            /// Value for m_numDependencies when the Job is executing.
            static constexpr uint32     kExecutingState   = 0xe0e0e0e0;

            /// Value for m_numDependencies when the Job is completed.
            static constexpr uint32     kDoneState        = 0xd0d0d0d0;

            /// Value for m_barrier when the barrier has been triggered.
            static constexpr intptr_t   kBarrierDoneState = ~static_cast<intptr_t>(0);

        public:
            inline Job(const char* pName/*, const Color& color*/, JobSystem* pSystem, const JobFunction& function, const uint32 numDependencies);
            
            //----------------------------------------------------------------------------------------------------
            /// @brief : Add a number of dependencies to this Job.
            //----------------------------------------------------------------------------------------------------
            inline void             AddDependency(const int count);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Remove a number of dependencies from this Job. Returns true if the dependency counter is 0.
            //----------------------------------------------------------------------------------------------------
            inline bool             RemoveDependency(const int count);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Remove a number of dependencies from this Job and Queues the Job for execution if there
            ///     are no more dependencies.
            //----------------------------------------------------------------------------------------------------
            inline void             RemoveDependencyAndQueue(const int count);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Set the Barrier associated to this Job.
            //----------------------------------------------------------------------------------------------------
            inline bool             SetBarrier(Barrier* pBarrier);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Executes the Job. Returns either the number of dependencies that this Job still has,
            ///     kExecutingState if the Job is currently running, or kDoneState if it has successfully finished.
            //----------------------------------------------------------------------------------------------------
            inline uint32_t         Execute();

            //----------------------------------------------------------------------------------------------------
            /// @brief : Returns whether this Job can be executed (m_numDependencies == 0). 
            //----------------------------------------------------------------------------------------------------
            inline bool             CanBeExecuted() const  { return m_numDependencies.load(std::memory_order_relaxed) == 0; }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Returns whether this Job has finished execution. 
            //----------------------------------------------------------------------------------------------------
            inline bool             IsDone() const         { return m_numDependencies.load(std::memory_order_relaxed) == kDoneState; }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Get the JobSystem running this Job. 
            //----------------------------------------------------------------------------------------------------
            inline JobSystem*       GetJobSystem() const    { return m_pJobSystem; }
            inline const char*      GetName() const         { return m_name; }

        private:
            //----------------------------------------------------------------------------------------------------
            /// @brief : `Releasing` the Job object calls JobSystem::Free() 
            //----------------------------------------------------------------------------------------------------
            virtual void            ReleaseObjectImpl(Job* pThisObject) const override;
            
            /*Color m_color = Color::White();*/                 // TODO 
            const char*             m_name = "Unnamed";       /// Name of the Job (should be debug only).
            JobSystem*              m_pJobSystem = nullptr;   /// The JobSystem that owns this Job.
            JobFunction             m_function = nullptr;     /// The functor to be executed.
            std::atomic<uint32>     m_numDependencies = 0;    /// The number of Jobs that must be executed before this one.
            std::atomic<intptr_t>   m_barrier = 0;            /// Equal to the numerical value of the pointer to the Barrier (can be null), or kBarrierDoneState to denote that the Barrier is done.
        };

    public:
        JobSystem() = default;
        virtual ~JobSystem() = default;
        JobSystem(const JobSystem&) = delete; 
        JobSystem(JobSystem&&) noexcept = delete;
        JobSystem& operator=(const JobSystem&) = delete; 
        JobSystem& operator=(JobSystem&&) noexcept = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Maximum number of concurrently executing Jobs.
        //----------------------------------------------------------------------------------------------------
        inline virtual int          GetMaxConcurrency() = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a new Job. The Job will be started immediately (when beginning execution with a JobBarrier)
        ///     if the number of dependencies == 0. Otherwise, it will start when RemoveDependency() causes the
        ///     Job's dependency counter to reach 0.
        ///	@param pName : Name of the Job.
        ///	@param jobFunction : Function to execute.
        ///	@param numDependencies : Number of dependencies that this Job is waiting on. Be sure that Jobs that this
        ///     Job depends on removes its dependency!
        ///	@returns : Handle to the newly created Job. You can use this to set up dependencies among other Jobs.
        //----------------------------------------------------------------------------------------------------
        virtual JobHandle           CreateJob(const char* pName/*, const Color& color*/, const JobFunction& jobFunction, const uint32 numDependencies = 0) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a Barrier used to wait until a set of Jobs is completed. This must be followed by
        ///     a call to DestroyBarrier() to properly clean up the barrier when it is no longer in use. 
        //----------------------------------------------------------------------------------------------------
        virtual Barrier*            CreateBarrier() = 0;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroy a Barrier when it is no longer used. The Barrier should be empty at this point.
        //----------------------------------------------------------------------------------------------------
        virtual void                DestroyBarrier(Barrier* pBarrier) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Wait for a set of Jobs to be finished.
        /// @note : Only 1 thread can be waiting on a Barrier at a time!!!
        //----------------------------------------------------------------------------------------------------
        virtual void                WaitForJobs(Barrier* pBarrier) = 0;

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a Job to the Job queue to be executed immediately. 
        //----------------------------------------------------------------------------------------------------
        virtual void                QueueJob(Job* pJob) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a number of Jobs to the Job Queue to be executed immediately.
        //----------------------------------------------------------------------------------------------------
        virtual void                QueueJobs(Job** pJobs, const uint32 numHandles) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Free the Job object. 
        //----------------------------------------------------------------------------------------------------
        virtual void                FreeJob(Job* pJob) = 0;
    };

    using JobHandle = JobSystem::JobHandle;
    using JobBarrier = JobSystem::Barrier;
}

#include "JobSystem.inl"

