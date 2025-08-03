// SemaphoreState.cpp
#include "SemaphoreState.h"
#include "RenderDevice.h"

namespace nes
{
    SemaphoreState::~SemaphoreState()
    {
        if (m_handle != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(m_device, m_handle, m_device.GetVkAllocationCallbacks());
        }
    }

    EGraphicsResult SemaphoreState::Init(const uint64 initialValue)
    {
        NES_GRAPHICS_ASSERT(m_device, m_handle == nullptr);

        // Create the semaphore:
        VkSemaphoreTypeCreateInfo timelineSemaphoreCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext = nullptr,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue = initialValue,
        };
        
        VkSemaphoreCreateInfo semaphoreCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &timelineSemaphoreCreateInfo,
            .flags = 0,
        };
        
        NES_VK_FAIL_RETURN(m_device, vkCreateSemaphore(m_device, &semaphoreCreateInfo, m_device.GetVkAllocationCallbacks(), &m_handle));

        // If non-zero, then it is fixed.
        if (initialValue != 0)
        {
            m_fixedValue = initialValue;
        }
        // Otherwise, it is considered dynamic.
        else
        {
            m_dynamicValue = std::make_shared<std::atomic<uint64>>(0);
        }
        
        return EGraphicsResult::Success;
    }

    void SemaphoreState::SetDebugName(const char* name)
    {
        m_device.SetDebugNameToTrivialObject(m_handle, name);
    }

    void SemaphoreState::SetDynamicValue(const uint64 value)
    {
        // Must be dynamic and its dynamic value shouldn't have been set yet.
        NES_GRAPHICS_ASSERT(m_device, IsDynamic() && m_dynamicValue->load() == 0);

        // Update the shared_ptr value so that every copy of this semaphore
        // state has access to it.
        m_dynamicValue->store(value);

        // TryFixate() afterwards, to cache it in the fixed value.
        TryFixate();
    }

    bool SemaphoreState::IsValid() const
    {
        return m_handle && (m_fixedValue != 0 || m_dynamicValue);
    }

    bool SemaphoreState::IsDynamic() const
    {
        return m_handle && (m_dynamicValue);
    }

    bool SemaphoreState::IsFixed() const
    {
        return m_handle && (m_fixedValue != 0);
    }

    bool SemaphoreState::CanWait() const
    {
        return m_handle && (m_fixedValue != 0 || (m_dynamicValue && m_dynamicValue->load() != 0));
    }

    bool SemaphoreState::CanWait()
    {
        TryFixate();
        return static_cast<const SemaphoreState*>(this)->CanWait();
    }

    bool SemaphoreState::IsSignaled() const
    {
        const uint64 timelineValue = GetTimelineValue();
        if (timelineValue == 0)
            return false;

        uint64 currentValue;
        const VkResult result = vkGetSemaphoreCounterValue(m_device, m_handle, &currentValue);
        if (result == VK_SUCCESS)
            return currentValue >= timelineValue;

        return false;
    }

    bool SemaphoreState::IsSignaled()
    {
        TryFixate();
        return static_cast<const SemaphoreState*>(this)->IsSignaled();
    }

    uint64 SemaphoreState::GetTimelineValue() const
    {
        if (m_fixedValue)
            return m_fixedValue;

        if (m_dynamicValue)
            return m_dynamicValue->load();

        // The semaphore is invalid.
        return 0;
    }

    EGraphicsResult SemaphoreState::Wait(const uint64 timeout)
    {
        TryFixate();
        return static_cast<const SemaphoreState*>(this)->Wait(timeout);
    }

    EGraphicsResult SemaphoreState::Wait(const uint64 timeout) const
    {
        const uint64 timelineValue = GetTimelineValue();

        // If zero, then the dynamic state hasn't been set!
        if (timelineValue == 0)
            return EGraphicsResult::InitializationFailed;

        const VkSemaphoreWaitInfo waitInfo
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .pNext = nullptr,
            .flags = 0,
            .semaphoreCount = 1,
            .pSemaphores = &m_handle,
            .pValues = &timelineValue,
        };

        NES_VK_FAIL_RETURN(m_device, vkWaitSemaphores(m_device, &waitInfo, timeout));
        
        return EGraphicsResult::Success;
    }

    // VkSemaphoreSubmitInfo SemaphoreState::CreateSubmitInfo(const EPipelineStageBits stages) const
    // {
    //     NES_GRAPHICS_ASSERT(m_device, IsValid());
    //     
    //     VkSemaphoreSubmitInfo submitInfo
    //     {
    //         .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
    //         .pNext = nullptr,
    //         .semaphore = m_handle,
    //         .value = GetTimelineValue(),
    //         .stageMask = stageMask,
    //         .deviceIndex = 0,
    //     };
    //
    //     NES_GRAPHICS_ASSERT(m_device, submitInfo.value != 0);
    //     return submitInfo;
    // }

    void SemaphoreState::TryFixate()
    {
        if (m_fixedValue == 0 && m_dynamicValue)
        {
            // Get the current value of the dynamic state.
            m_fixedValue = m_dynamicValue->load();

            // If non-zero, then the dynamic value has been set, we
            // can cache it in the fixed value.
            if (m_fixedValue != 0)
            {
                // We can release the dynamic value here because the dynamic value
                // is only transitioned from 0 once, and we cache the value in the fixed value.
                m_dynamicValue = nullptr;
            }
        }
    }
}
