// WorkerThread.h
#pragma once
#include <functional>
#include <queue>
#include "Thread.h"
#include "ThreadIdleEvent.h"
#include "Nessie/Core/Concepts.h"

namespace nes
{
    //-----------------------------------------------------------------------------------------------------------------------------
    ///	@brief : Default implementation of the ThreadInstructionType for the WorkerThread. Contains instructions for initializing,
    ///     running and terminating the WorkerThread.
    //-----------------------------------------------------------------------------------------------------------------------------
    enum class EDefaultThreadInstruction : uint8
    {
        Init,
        Run,
        Terminate,
    };

    //-----------------------------------------------------------------------------------------------------------------------------
    ///	@brief : A WorkerThread is a thread interface that is given a function to handle incoming instructions from other
    ///     threads (probably the main thread). The thread will sleep until there are instructions to process; however, you can
    ///     control when the thread actually wakes up using the SendInstructionWithoutNotify() and NotifyOfInstruction() functions.
    /// @note : An 'instruction' is an enum value. I have created EDefaultThreadInstruction to show what the basic setup could
    ///     be. Some kind of initialization value, and 'work' value, and a termination value.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <EnumType InstructionType = EDefaultThreadInstruction>
    class WorkerThread
    {
    public:
        //-----------------------------------------------------------------------------------------------------------------------------
        ///	@brief : Function for handling Instructions sent to this thread. If the function returns false, the thread is terminated.
        ///     \n An instruction is an enum value. The instruction handler should basically contain a switch statement that calls
        ///     different functions based on the instruction.
        //-----------------------------------------------------------------------------------------------------------------------------
        using ThreadInstructionHandler = std::function<bool(InstructionType)>;

    public:
        WorkerThread() = default;
        WorkerThread(const WorkerThread&) = delete;
        WorkerThread(WorkerThread&& right) noexcept = delete;
        WorkerThread& operator=(const WorkerThread&) = delete;
        WorkerThread& operator=(WorkerThread&& right) noexcept = delete;
        ~WorkerThread();

        //-----------------------------------------------------------------------------------------------------------------------------
        ///	@brief : Begins the thread's execution.
        ///	@param handlerFunc : Function to handle incoming instructions sent to this thread.
        /// @param threadName : Debug name to give the thread.
        /// @note : This will fail if called on a non-terminated thread!
        //-----------------------------------------------------------------------------------------------------------------------------
        void                        Start(ThreadInstructionHandler&& handlerFunc, const char* threadName = "");
        
        //-----------------------------------------------------------------------------------------------------------------------------
        ///	@brief : Tells the thread to shut down and join back up with the main thread.
        //-----------------------------------------------------------------------------------------------------------------------------
        void                        Terminate();

        //-----------------------------------------------------------------------------------------------------------------------------
        /// @brief : Send an instruction to this WorkerThread. The thread will be immediately notified of the new instruction.
        //-----------------------------------------------------------------------------------------------------------------------------
        void                        SendInstruction(const InstructionType instruction);

        //-----------------------------------------------------------------------------------------------------------------------------
        ///	@brief : Add instructions to the thread's internal queue without letting it know that it should wake up. This is useful if you
        ///     want to queue up a bunch of instructions and not have to notify the thread for every add. When you finished queueing up
        ///     instructions, call NotifyOfInstruction() to let the thread know it is time to wake up.
        //-----------------------------------------------------------------------------------------------------------------------------
        void                        SendInstructionWithoutNotify(const InstructionType instruction);

        //-----------------------------------------------------------------------------------------------------------------------------
        ///	@brief : Lets the thread know that there are instructions to be processed. If the thread was idle, it will wake up and begin work
        ///     again. This function is to be used in tandem with AddWorkWithoutNotify(), so that you can queue up a bunch of
        ///     work before resuming execution.
        //-----------------------------------------------------------------------------------------------------------------------------
        void                        NotifyOfInstruction();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : This function will block the current process until the thread has finished running through its instructions.
        ///     Good for syncing between threads.
        //----------------------------------------------------------------------------------------------------
        void                        WaitUntilDone();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if this thread has been terminated. 
        //----------------------------------------------------------------------------------------------------
        bool                        IsTerminated() const        { return m_isTerminated; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get this thread's ID. If the thread is terminated, it will return an invalid id.
        //----------------------------------------------------------------------------------------------------
        std::thread::id             GetThreadId() const         { return m_isTerminated? std::thread::id() : m_thread.get_id(); }

    private:
        //-----------------------------------------------------------------------------------------------------------------------------
        ///	@brief : This is the main loop of the thread's execution, processing any instructions it is given.
        //-----------------------------------------------------------------------------------------------------------------------------
        void                        ProcessInstructions();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Signal that this thread is now idle. 
        //----------------------------------------------------------------------------------------------------
        void                        SignalIdle();

    private:
        std::thread                 m_thread;
        std::string                 m_threadName;           /// Debug name for the thread.
        ThreadIdleEvent             m_idleEvent;            /// Interface for handling when this thread is put to sleep or not.
        std::mutex                  m_instructionMutex;     /// Mutex that will guard access to the instruction queue.
        std::condition_variable     m_wakeCondition;        /// Condition variable for waking up the thread.
        std::atomic_bool            m_isTerminated;         /// Whether the thread has been terminated or not.
        std::queue<InstructionType> m_instructionQueue;     /// Queue of instructions to be processed on the thread.
        ThreadInstructionHandler    m_instructionHandler;   /// Function given to the thread to process incoming instructions.
    };
}

namespace nes
{
    template <EnumType InstructionType>
    WorkerThread<InstructionType>::~WorkerThread()
    {
        Terminate();
    }
    
    template <EnumType InstructionType>
    void WorkerThread<InstructionType>::Start(ThreadInstructionHandler&& handlerFunc, const char* threadName)
    {
        if (m_thread.joinable())
        {
            NES_WARN(nes::kLogTagThread, "Tried to initialize a WorkerThread that is already running!");
            return;
        }

        m_isTerminated = false;
        m_instructionHandler = std::forward<ThreadInstructionHandler>(handlerFunc);
        m_threadName = threadName;

        m_thread = std::thread([this]() -> void
        {
            if (!m_threadName.empty())
                thread::SetThreadName(m_threadName.c_str());
            
            return ProcessInstructions();
        });
    }
    
    template <EnumType InstructionType>
    void WorkerThread<InstructionType>::Terminate()
    {
        if (m_thread.joinable())
        {
            // Exit our Process Work function.
            m_isTerminated = true;

            // Signal to any waiting external threads that this thread is done.
            m_idleEvent.SignalIdle();

            // Notify the worker thread.
            m_wakeCondition.notify_all();

            // Join with the main thread.
            m_thread.join();
        }
    }
    
    template <EnumType InstructionType>
    void WorkerThread<InstructionType>::SendInstruction(const InstructionType instruction)
    {
        SendInstructionWithoutNotify(instruction);
        NotifyOfInstruction();
    }
    
    template <EnumType InstructionType>
    void WorkerThread<InstructionType>::SendInstructionWithoutNotify(const InstructionType instruction)
    {
        std::lock_guard lock(m_instructionMutex);
        m_instructionQueue.push(instruction);
    }
    
    template <EnumType InstructionType>
    void WorkerThread<InstructionType>::NotifyOfInstruction()
    {
        std::lock_guard lock(m_instructionMutex);
        // If we were notified, but don't actually have any work, then return.
        if (m_instructionQueue.empty())
            return;

        // Notify the two condition variables that we are now working.
        m_idleEvent.Resume();
        m_wakeCondition.notify_all();
    }
    
    template <EnumType InstructionType>
    void WorkerThread<InstructionType>::WaitUntilDone()
    {
        m_idleEvent.WaitForIdle();
    }
    
    template <EnumType InstructionType>
    void WorkerThread<InstructionType>::ProcessInstructions()
    {
        while (!m_isTerminated)
        {
            // Signal that this thread is sleeping/idle
            SignalIdle();

            std::unique_lock lock(m_instructionMutex);
            // If we are done or our instruction queue is not empty, then we need to wake up.
            m_wakeCondition.wait(lock, [this]() -> bool { return m_isTerminated || !m_instructionQueue.empty(); });

            // If we are done, let's exit.
            if (m_isTerminated)
                continue;

            m_idleEvent.Resume();

            // While we have instructions available,
            while (!m_instructionQueue.empty())
            {
                InstructionType instruction = m_instructionQueue.front();
                m_instructionQueue.pop();
                lock.unlock();

                // Send the instruction to the instruction handler.
                // If the function returns false, then we stop processing, the thread's work has resulted in a termination.
                if (!m_instructionHandler(instruction))
                {
                    m_isTerminated = true;
                    break;
                }

                lock.lock();
            }
        }
    }

    template <EnumType InstructionType>
    void WorkerThread<InstructionType>::SignalIdle()
    {
        m_idleEvent.SignalIdle();
    }
}
