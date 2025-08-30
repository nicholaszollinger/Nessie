// SemaphoreState.cpp
#include "SemaphoreState.h"
#include "RenderDevice.h"
#include "Renderer.h"

namespace nes
{
    SemaphoreState::SemaphoreState(SemaphoreState&& other) noexcept
        : m_pDevice(other.m_pDevice)
        , m_semaphore(std::move(other.m_semaphore))
        , m_dynamicValue(std::move(other.m_dynamicValue))
        , m_fixedValue(other.m_fixedValue)
    {
        other.m_pDevice = nullptr;
        other.m_fixedValue = 0;
    }

    SemaphoreState& SemaphoreState::operator=(std::nullptr_t)
    {
        FreeSemaphore();
        return *this;
    }

    SemaphoreState& SemaphoreState::operator=(SemaphoreState&& other) noexcept
    {
        if (this != &other)
        {
            m_pDevice = other.m_pDevice;
            m_semaphore = std::move(other.m_semaphore);
            m_dynamicValue = std::move(other.m_dynamicValue);
            m_fixedValue = other.m_fixedValue;

            other.m_pDevice = nullptr;
            other.m_fixedValue = 0;
        }

        return *this;
    }

    void SemaphoreState::FreeSemaphore()
    {
        if (m_semaphore != nullptr)
        {
            Renderer::SubmitResourceFree([semaphore = std::move(m_semaphore)]() mutable
            {
                semaphore = nullptr;   
            });
        }

        m_dynamicValue = nullptr;
        m_fixedValue = 0;
    }

    SemaphoreState::~SemaphoreState()
    {
        FreeSemaphore();
    }

    SemaphoreState::SemaphoreState(RenderDevice& device, const uint64 initialValue)
        : m_pDevice(&device)
    {
        vk::SemaphoreTypeCreateInfo timelineSemaphoreCreateInfo = vk::SemaphoreTypeCreateInfo()
            .setSemaphoreType(vk::SemaphoreType::eTimeline)
            .setInitialValue(initialValue);

        vk::SemaphoreCreateInfo semaphoreCreateInfo = vk::SemaphoreCreateInfo()
            .setPNext(&timelineSemaphoreCreateInfo);
        
        m_semaphore = vk::raii::Semaphore(device, semaphoreCreateInfo, device.GetVkAllocationCallbacks());

        // If non-zero, then it is fixed.
        if (initialValue != 0)
            m_fixedValue = initialValue;
        
        // Otherwise, it is considered dynamic.
        else
            m_dynamicValue = std::make_shared<std::atomic<uint64>>(0);
    }

    void SemaphoreState::SetDebugName(const std::string& name)
    {
        m_pDevice->SetDebugNameVkObject(GetNativeVkObject(), name);
    }

    void SemaphoreState::SetDynamicValue(const uint64 value)
    {
        // Must be dynamic and its dynamic value shouldn't have been set yet.
        NES_ASSERT(IsDynamic() && m_dynamicValue->load() == 0);

        // Update the shared_ptr value so that every copy of this semaphore
        // state has access to it.
        m_dynamicValue->store(value);

        // TryFixate() afterwards, to cache it in the fixed value.
        TryFixate();
    }

    bool SemaphoreState::IsValid() const
    {
        return m_semaphore != nullptr && (m_fixedValue != 0 || m_dynamicValue);
    }

    bool SemaphoreState::IsDynamic() const
    {
        return m_semaphore != nullptr && (m_dynamicValue);
    }

    bool SemaphoreState::IsFixed() const
    {
        return m_semaphore != nullptr && (m_fixedValue != 0);
    }

    bool SemaphoreState::CanWait() const
    {
        return m_semaphore != nullptr && (m_fixedValue != 0 || (m_dynamicValue && m_dynamicValue->load() != 0));
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

        const uint64 currentValue = m_semaphore.getCounterValue();
        return currentValue >= timelineValue;
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
        
        const vk::SemaphoreWaitInfo waitInfo = vk::SemaphoreWaitInfo()
            .setSemaphores(*m_semaphore)
            .setValues(timelineValue);

        NES_VK_FAIL_RETURN(*m_pDevice, m_pDevice->GetVkDevice().waitSemaphores(waitInfo, timeout));
        
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

    NativeVkObject SemaphoreState::GetNativeVkObject() const
    {
        return NativeVkObject(*m_semaphore, vk::ObjectType::eSemaphore);
    }

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
