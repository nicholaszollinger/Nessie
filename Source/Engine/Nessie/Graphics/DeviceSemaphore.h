// TimelineSemaphore.h
#pragma once
#include "GraphicsCommon.h"
#include "DeviceObject.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Device Semaphore is used can be used to synchronize operations across device queues, as
    ///     well as facilitate Host (CPU) to Device (GPU) synchronization.
    //----------------------------------------------------------------------------------------------------
    class DeviceSemaphore
    {
    public:
        static constexpr uint64 kDefaultTimeout = 5000 * 1000000ull; // 5 seconds, in microseconds.
        
    public:
        DeviceSemaphore(std::nullptr_t) {}
        DeviceSemaphore(const DeviceSemaphore&) = delete;
        DeviceSemaphore(DeviceSemaphore&& other) noexcept;
        DeviceSemaphore& operator=(std::nullptr_t);
        DeviceSemaphore& operator=(const DeviceSemaphore&) = delete;
        DeviceSemaphore& operator=(DeviceSemaphore&& other) noexcept;
        ~DeviceSemaphore();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates a timeline semaphore.
        /// @param initialValue : Initial value to give the semaphore. If the value is equal to 0, then the
        ///     semaphore cannot be waiting
        //----------------------------------------------------------------------------------------------------
        DeviceSemaphore(RenderDevice& device, const uint64 initialValue = 0);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Wait until this Semaphore is signaled with the given value.
        ///	@param value : Must be non-zero. This will block until the semaphore's value is signaled with this value.
        ///	@param timeout : How long to wait, in microseconds. Default is 5 seconds. 
        ///	@returns : Can return EGraphicsResult::DeviceLost on wait failure, or EGraphicsResult::Failure
        ///     if the value is set.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         Wait(const uint64 value, const uint64 timeout = kDefaultTimeout) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sets value timeline value of the semaphore and signals it. Any parties waiting on this
        ///     particular value will be released.
        //----------------------------------------------------------------------------------------------------
        void                    Signal(const uint64 value) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current value of the semaphore.
        //----------------------------------------------------------------------------------------------------
        uint64                  GetValue() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this semaphore.
        //----------------------------------------------------------------------------------------------------
        void                    SetDebugName(const std::string& name);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan Semaphore object.
        //----------------------------------------------------------------------------------------------------
        vk::Semaphore           GetVkSemaphore() const { return *m_semaphore; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the native vulkan object handle, and the type.
        //----------------------------------------------------------------------------------------------------
        NativeVkObject          GetNativeVkObject() const;
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Free the Semaphore object. 
        //----------------------------------------------------------------------------------------------------
        void                    FreeSemaphore();

    private:
        RenderDevice*           m_pDevice = nullptr;
        vk::raii::Semaphore     m_semaphore = nullptr;
    };

    static_assert(DeviceObjectType<DeviceSemaphore>);
    
    //----------------------------------------------------------------------------------------------------
    // [TODO]: Device Queue Submit with a SemaphoreState param. Make the Device Queue a 'timeline' interface.
    //
    // Desc:
    // "The latter use case is intended for use with the nes::DeviceQueue class. Any semaphore state
    // that is signaled within nes::DeviceQueue::Submit(...) that was created from that nes::DeviceQueue
    // will have its timeline value updated at that time."
    //
    // "In both cases, a copy of this class can be made to later check the completion status of the timeline
    // semaphore."
    //
    /// @brief : Contains a Device Semaphore and a timeline value. 
    ///
    /// The time value can only be in one of two states:
    /// - Fixed: The timeline value is fixed and cannot change. Set on construction.
    /// - Dynamic: The timeline value is provided at a later time, exactly once. Not giving an initial value
    ///     on construction will set the dynamic state. SetDynamicValue() must be called once at a later time.
    //----------------------------------------------------------------------------------------------------
    class SemaphoreValue
    {
    public:
        SemaphoreValue() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sets the Semaphore and its initial value. If the initial value is 0, then this state will
        ///     be Dynamic and will need to have SetDynamicValue() called once at a later time.
        ///     If non-zero, the Semaphore will be in the Fixed state and can be waited on.
        //----------------------------------------------------------------------------------------------------
        SemaphoreValue(DeviceSemaphore* pSemaphore, const uint64 initialValue = 0);

        //----------------------------------------------------------------------------------------------------
        /// @brief : This function can only be called once and is only allowed if the Semaphore was initialized
        ///     with an initial value of 0.
        //----------------------------------------------------------------------------------------------------
        void                    SetDynamicValue(const uint64 value);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns if the semaphore has been initialized correctly.
        //----------------------------------------------------------------------------------------------------
        inline bool             IsValid() const     { return m_pSemaphore != nullptr && (m_fixedValue != 0 || m_dynamicValue); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check to see if this is a dynamic state. If dynamic, the time value has either not been
        ///     set yet, or it has not been cached yet.
        //----------------------------------------------------------------------------------------------------
        inline bool             IsDynamic() const   { return m_pSemaphore != nullptr && (m_dynamicValue); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check to see if this is a fixed state. If fixed, it means that the timeline value has been
        ///     set.
        //----------------------------------------------------------------------------------------------------
        inline bool             IsFixed() const     { return m_pSemaphore != nullptr && (m_fixedValue != 0); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks to see if calling Wait() will be valid.
        //----------------------------------------------------------------------------------------------------
        inline bool             CanWait() const     { return m_pSemaphore != nullptr && (m_fixedValue != 0 || (m_dynamicValue && m_dynamicValue->load() != 0)); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks to see if calling Wait() will be valid.
        /// @note : This non-const version attempts to convert to a fixed state if possible.
        //----------------------------------------------------------------------------------------------------
        bool                    CanWait();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check whether the semaphore has been signaled or not. The timeline value must have been set
        ///  for this to become signaled.
        //----------------------------------------------------------------------------------------------------
        bool                    IsSignaled() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check whether the semaphore has been signaled or not.
        /// @note : This non-const version attempts to convert to a fixed state if possible.
        //----------------------------------------------------------------------------------------------------
        bool                    IsSignaled();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current value of the Semaphore state. This is the value that will be used to
        ///     wait on the semaphore.
        //----------------------------------------------------------------------------------------------------
        uint64                  GetTimelineValue() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Wait until the Semaphore is signaled with the state's timeline value.
        ///	@param timeout : How long to wait, in microseconds. Default is 5 seconds. 
        ///	@returns : Can return EGraphicsResult::DeviceLost on wait failure, or EGraphicsResult::InitializationFailed
        ///     if the timeline value was not set properly.
        /// @note : This non-const version attempts to convert to a fixed state if possible.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         Wait(const uint64 timeout = DeviceSemaphore::kDefaultTimeout);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Wait until the Semaphore is signaled with the state's timeline value.
        ///	@param timeout : How long to wait, in microseconds. Default is 5 seconds. 
        ///	@returns : Can return EGraphicsResult::DeviceLost on wait failure, or EGraphicsResult::InitializationFailed
        ///     if the timeline value was not set properly.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         Wait(const uint64 timeout = DeviceSemaphore::kDefaultTimeout) const;
        
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Attempts to convert a dynamic state to a fixed value. This can speed up future waits. 
        //----------------------------------------------------------------------------------------------------
        void                    TryFixate();
        
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : The Dynamic value is a shared ptr because it must be updated across all copies of the
        ///     Semaphore state, and it does not exist for fixed-value semaphores.
        //----------------------------------------------------------------------------------------------------
        using DynamicValue = std::shared_ptr<std::atomic<uint64>>;

        /// Handle to the Semaphore object.
        DeviceSemaphore*        m_pSemaphore = nullptr;

        /// The Dynamic value can be set only once and is a shared state across all copies of this Semaphore.
        /// This doesn't exist for a fixed-value semaphore state.
        /// If non-null and equal to 0, the semaphore state has not been submitted yet.
        DynamicValue            m_dynamicValue = nullptr;

        /// If non-zero, this represents a fixed value or the locally cached value of the dynamic state.
        uint64                  m_fixedValue = 0;
    };

    
}