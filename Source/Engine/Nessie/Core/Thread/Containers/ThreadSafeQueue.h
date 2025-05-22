// ThreadSafeQueue.h
#pragma once
#include <atomic>
#include <mutex>
#include <queue>
#include "Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///	@brief : Essentially a wrapper around std::queue that provides thread-safe and non-thread-safe interface
    ///     for the queue operations.
    /// @tparam Type : Type that is stored in the queue.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    class ThreadSafeQueue
    {
    public:
        ThreadSafeQueue() = default;

    public:
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Thread-safe push of a value into the queue.
        //----------------------------------------------------------------------------------------------------
        void                EnqueueLocked(const Type& value);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Thread-safe emplace of a value into the queue.
        ///	@tparam Args : Argument Types to construct the value with.
        ///	@param args : Arguments to construct the value with.
        //----------------------------------------------------------------------------------------------------        
        template <typename...Args>
        void                EnqueueLocked(Args&&...args);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Thread-safe dequeue of a value from the queue.
        ///	@param outValue : Value that will be returned from the queue.
        ///	@returns : False if the queue is empty.
        //----------------------------------------------------------------------------------------------------
        bool                DequeueLocked(Type& outValue);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Thread-safe access to the front of the queue.
        //----------------------------------------------------------------------------------------------------
        Type&               FrontLocked();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Thread-safe pop of the top element of the queue. Asserts that the queue is not empty.
        //----------------------------------------------------------------------------------------------------
        void                PopLocked();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Thead-safe transfer of the contents of this queue to the destination queue. At the end
        ///     of the operation, our queue will be empty. The destination queue will be locked during
        ///     the transfer as well.
        ///	@param destination : ThreadSafeQueue to transfer the contents to.
        //----------------------------------------------------------------------------------------------------
        void                TransferLocked(ThreadSafeQueue& destination);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Thead-safe transfer of the contents of this queue to the destination queue. At the end
        ///     of the operation, our queue will be empty. The destination queue will be locked during
        ///     the transfer as well.
        ///	@param destination : ThreadSafeQueue to transfer the contents to.
        //----------------------------------------------------------------------------------------------------
        void                TransferLocked(std::queue<Type>& destination);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Thread-safe swap of the contents of this queue with the destination queue.
        ///	@param other : Queue we are swapping with.
        //----------------------------------------------------------------------------------------------------
        void                SwapLocked(ThreadSafeQueue& other);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Thread-safe swap of the contents of this queue with the destination queue.
        ///	@param other : Queue we are swapping with.
        //----------------------------------------------------------------------------------------------------
        void                SwapLocked(std::queue<Type>& other);

        //----------------------------------------------------------------------------------------------------
        ///		@brief : Thread-safe clear of the queue. O(N) operation.
        //----------------------------------------------------------------------------------------------------
        void                ClearLocked();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Thread-safe check if the queue is empty.
        //----------------------------------------------------------------------------------------------------
        bool                IsEmptyLocked() const;
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Thread-safe check the size of the queue.
        //----------------------------------------------------------------------------------------------------
        size_t              SizeLocked() const;
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Non-thread-safe push of a value into the queue.
        //----------------------------------------------------------------------------------------------------
        void                Enqueue(const Type& value);
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Non-thread-safe emplace of a value from the queue.
        ///	@tparam Args : Argument Types to construct the value with.
        ///	@param args : Arguments to construct the value with.
        //----------------------------------------------------------------------------------------------------
        template <typename...Args>
        void                Enqueue(Args&&...args);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Non-thread-safe dequeue of a value from the queue.
        ///	@param outValue : Value that will be returned from the queue.
        ///	@returns : False if the queue is empty.
        //----------------------------------------------------------------------------------------------------
        bool                Dequeue(Type& outValue);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Non-thread-safe access to the front of the queue. This asserts that the queue is not empty.
        //----------------------------------------------------------------------------------------------------
        Type&               Front();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Non-Thread-safe pop of the top element off the queue. Asserts that the queue is not empty.
        //----------------------------------------------------------------------------------------------------
        void                Pop();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Non-thread-safe transfer of the contents of this queue to the destination queue. At the end
        ///     of the operation, our queue will be empty.
        /// @param destination : ThreadSafeQueue to transfer the contents to.
        //----------------------------------------------------------------------------------------------------
        void                Transfer(ThreadSafeQueue& destination);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Non-thread-safe transfer of the contents of this queue to the destination queue. At the end
        ///     of the operation, our queue will be empty.
        ///	@param destination : ThreadSafeQueue to transfer the contents to.
        //----------------------------------------------------------------------------------------------------
        void                Transfer(std::queue<Type>& destination);
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Non-thread-safe swap of the contents of this queue with the destination queue.
        ///	@param other : Queue we are swapping with.
        //----------------------------------------------------------------------------------------------------
        void                Swap(ThreadSafeQueue& other);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Non-thread-safe swap of the contents of this queue with the destination queue.
        ///	@param other : Queue we are swapping with.
        //----------------------------------------------------------------------------------------------------
        void                Swap(std::queue<Type>& other);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Non-thread-safe clear of the queue. O(N) operation.
        //----------------------------------------------------------------------------------------------------
        void                Clear();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Non-thread-safe check if the queue is empty.
        //----------------------------------------------------------------------------------------------------
        bool                IsEmpty() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Non-thread-safe check the size of the queue.
        //----------------------------------------------------------------------------------------------------
        size_t              Size() const;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Locks the mutex for the queue.
        //----------------------------------------------------------------------------------------------------
        void                Lock();
        
        //----------------------------------------------------------------------------------------------------
        ///	@brief : Unlocks the mutex for the queue.
        //----------------------------------------------------------------------------------------------------
        void                Unlock();

    private:
        std::queue<Type>    m_queue;
        std::mutex          m_mutex;
    };
}

namespace nes
{
    template<typename Type>
    void ThreadSafeQueue<Type>::EnqueueLocked(const Type& value)
    {
        Lock();
        Enqueue(value);
        Unlock();
    }
    
    template<typename Type>
    template<typename...Args>
    void ThreadSafeQueue<Type>::EnqueueLocked(Args&&...args)
    {
        Lock();
        Enqueue(std::forward<Args>(args)...);
        Unlock();
    }
    
    template<typename Type>
    void ThreadSafeQueue<Type>::Enqueue(const Type& value)
    {
        m_queue.push(value);
    }

    template<typename Type>
    template<typename...Args>
    void ThreadSafeQueue<Type>::Enqueue(Args&&...args)
    {
        m_queue.emplace(std::forward<Args>(args)...);
    }
    
    template<typename Type>
    bool ThreadSafeQueue<Type>::DequeueLocked(Type& outValue)
    {
        Lock();
        const bool result = Dequeue(outValue);
        Unlock();
        return result;
    }
    
    template<typename Type>
    bool ThreadSafeQueue<Type>::Dequeue(Type& outValue)
    {
        if (IsEmpty())
            return false;

        outValue = m_queue.front();
        m_queue.pop();
        return true;
    }
    
    template<typename Type>
    void ThreadSafeQueue<Type>::TransferLocked(ThreadSafeQueue& destination)
    {
        Lock();
        destination.Lock();
        Transfer(destination);
        destination.Unlock();
        Unlock();
    }
    
    template<typename Type>
    void ThreadSafeQueue<Type>::TransferLocked(std::queue<Type>& destination)
    {
        Lock();
        Transfer(destination);
        Unlock();
    }
    
    template<typename Type>
    void ThreadSafeQueue<Type>::Transfer(ThreadSafeQueue& destination)
    {
        while (!IsEmpty())
        {
            destination.Enqueue(Front());
            Pop();
        }
    }
    
    template<typename Type>
    void ThreadSafeQueue<Type>::Transfer(std::queue<Type>& destination)
    {
        while (!IsEmpty())
        {
            destination.push(Front());
            Pop();
        }
    }
    
    template<typename Type>
    void ThreadSafeQueue<Type>::SwapLocked(ThreadSafeQueue& other)
    {
        Lock();
        other.Lock();
        Swap(other);
        other.Unlock();
        Unlock();
    }
    
    template<typename Type>
    void ThreadSafeQueue<Type>::SwapLocked(std::queue<Type>& other)
    {
        Lock();
        Swap(other);
        Unlock();
    }

    template<typename Type>
    void ThreadSafeQueue<Type>::Swap(ThreadSafeQueue& other)
    {
        m_queue.swap(other.m_queue);   
    }
    
    template<typename Type>
    void ThreadSafeQueue<Type>::Swap(std::queue<Type>& other)
    {
        m_queue.swap(other);
    }
    
    template<typename Type>
    Type& ThreadSafeQueue<Type>::FrontLocked()
    {
        Lock();
        Type& value = Front();
        Unlock();
        return value;
    }
    
    template<typename Type>
    Type& ThreadSafeQueue<Type>::Front()
    {
        NES_ASSERTV(!IsEmpty(), "Attempting to access the front of an empty queue!");
        return m_queue.front();
    }
    
    template<typename Type>
    void ThreadSafeQueue<Type>::PopLocked()
    {
        Lock();
        Pop();
        Unlock();
    }
    
    template<typename Type>
    void ThreadSafeQueue<Type>::Pop()
    {
        NES_ASSERTV(!IsEmpty(), "Attempting to pop an empty queue!");
        m_queue.pop();
    }
    
    template<typename Type>
    void ThreadSafeQueue<Type>::ClearLocked()
    {
        Lock();
        Clear();
        Unlock();
    }
    
    template<typename Type>
    void ThreadSafeQueue<Type>::Clear()
    {
        while (!IsEmpty())
            m_queue.pop();
    }
    
    template<typename Type>
    bool ThreadSafeQueue<Type>::IsEmptyLocked() const
    {
        Lock();
        const bool result = IsEmpty();
        Unlock();
        return result;
    }

    template<typename Type>
    bool ThreadSafeQueue<Type>::IsEmpty() const
    {
        return m_queue.empty();
    }

    template<typename Type>
    size_t ThreadSafeQueue<Type>::SizeLocked() const
    {
        Lock();
        const size_t result = Size();
        Unlock();
        return result;
    }
    
    template<typename Type>
    size_t ThreadSafeQueue<Type>::Size() const
    {
        return m_queue.size();
    }
    
    template<typename Type>
    void ThreadSafeQueue<Type>::Lock()
    {
        m_mutex.lock();
    }

    template<typename Type>
    void ThreadSafeQueue<Type>::Unlock()
    {
        m_mutex.unlock();
    }
}