// TimelineSemaphore.h
#pragma once
#include "GraphicsCommon.h"
#include "DeviceAsset.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    // [TODO]: Update the C++ API.
    /// @brief : Wrapper for a Timeline Semaphore object with a timeline value.
    ///
    /// It can only be in one of two states:
    /// - Fixed: The timeline value is fixed and cannot change.
    /// - Dynamic: The timeline value is provided at a later time, exactly once.
    ///
    /// The latter use case is intended for use with the nes::DeviceQueue class. Any semaphore state
    /// that is signaled within nes::DeviceQueue::Submit(...) that was created from that nes::DeviceQueue
    /// will have its timeline value updated at that time.
    ///
    /// In both cases, a copy of this class can be made to later check the completion status of the timeline
    /// semaphore.
    //----------------------------------------------------------------------------------------------------
    class SemaphoreState final : public DeviceAsset
    {
    public:
        static constexpr uint64 kDefaultTimeout = 5000 * 1000000ull; // 5 seconds, in microseconds.
        
    public:
        explicit                SemaphoreState(RenderDevice& device) : DeviceAsset(device) {}
        virtual                 ~SemaphoreState() override;

        /// Operator to convert to Vulkan Type.
        operator                VkSemaphore() const { return m_handle; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialization function called by the RenderDevice.
        /// @param initialValue : If set to 0, the Semaphore will be Dynamic and will need to have SetDynamicValue()
        ///     called once at a later time. If non-zero, the Semaphore will be in the Fixed state. 
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         Init(const uint64 initialValue);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this semaphore.
        //----------------------------------------------------------------------------------------------------
        virtual void            SetDebugName(const std::string& name) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : This function can only be called once and is only allowed if the Semaphore was initialized
        ///     with a dynamic state.
        //----------------------------------------------------------------------------------------------------
        void                    SetDynamicValue(const uint64 value);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns if the semaphore has been initialized correctly.
        //----------------------------------------------------------------------------------------------------
        inline bool             IsValid() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check to see if this is a dynamic state. 
        //----------------------------------------------------------------------------------------------------
        inline bool             IsDynamic() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check to see if this is a fixed state. 
        //----------------------------------------------------------------------------------------------------
        inline bool             IsFixed() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks to see if calling Wait() will be valid.
        //----------------------------------------------------------------------------------------------------
        bool                    CanWait() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks to see if calling Wait() will be valid.
        /// @note : This non-const version attempts to convert to a fixed state if possible.
        //----------------------------------------------------------------------------------------------------
        bool                    CanWait();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check whether this semaphore has been signaled or not. Other entities wait on this call. 
        //----------------------------------------------------------------------------------------------------
        bool                    IsSignaled() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check whether this semaphore has been signaled or not. Other entities wait on this call.
        /// @note : This non-const version attempts to convert to a fixed state if possible.
        //----------------------------------------------------------------------------------------------------
        bool                    IsSignaled();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current value of the Semaphore state.
        //----------------------------------------------------------------------------------------------------
        uint64                  GetTimelineValue() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Wait until this Semaphore is signaled.
        ///	@param timeout : How long to wait, in microseconds. Default is 5 seconds. 
        ///	@returns : Can return EGraphicsResult::DeviceLost on wait failure, or EGraphicsResult::InitializationFailed
        ///     if the timeline value was not set properly.
        /// @note : This non-const version attempts to convert to a fixed state if possible.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         Wait(const uint64 timeout = kDefaultTimeout);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Wait until this Semaphore is signaled.
        ///	@param timeout : How long to wait, in microseconds. Default is 5 seconds. 
        ///	@returns : Can return EGraphicsResult::DeviceLost on wait failure, or EGraphicsResult::InitializationFailed
        ///     if the timeline value was not set properly.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult         Wait(const uint64 timeout = kDefaultTimeout) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Vulkan Semaphore handle.
        //----------------------------------------------------------------------------------------------------
        VkSemaphore             GetHandle() const { return m_handle; }
        
        ////----------------------------------------------------------------------------------------------------
        ///// @brief : Create submit info for this Semaphore.
        ////----------------------------------------------------------------------------------------------------
        //VkSemaphoreSubmitInfo   CreateSubmitInfo(const EPipelineStageBits stages) const;
        
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
        VkSemaphore             m_handle = nullptr;

        /// The Dynamic value can be set only once and is a shared state across all copies of this Semaphore.
        /// This doesn't exist for a fixed-value semaphore state.
        /// If non-null and equal to 0, the semaphore state has not been submitted yet.
        DynamicValue            m_dynamicValue = nullptr;

        /// If non-zero, this represents a fixed value or the locally cached value of the dynamic state.
        uint64                  m_fixedValue = 0;
    };
}