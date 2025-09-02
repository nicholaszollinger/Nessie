// SemaphoreState.cpp
#include "DeviceSemaphore.h"
#include "RenderDevice.h"
#include "Renderer.h"

namespace nes
{
    DeviceSemaphore::DeviceSemaphore(DeviceSemaphore&& other) noexcept
        : m_pDevice(other.m_pDevice)
        , m_semaphore(std::move(other.m_semaphore))
    {
        other.m_pDevice = nullptr;
    }

    DeviceSemaphore& DeviceSemaphore::operator=(std::nullptr_t)
    {
        FreeSemaphore();
        return *this;
    }

    DeviceSemaphore& DeviceSemaphore::operator=(DeviceSemaphore&& other) noexcept
    {
        if (this != &other)
        {
            m_pDevice = other.m_pDevice;
            m_semaphore = std::move(other.m_semaphore);
            
            other.m_pDevice = nullptr;
        }

        return *this;
    }

    DeviceSemaphore::~DeviceSemaphore()
    {
        FreeSemaphore();
    }

    DeviceSemaphore::DeviceSemaphore(RenderDevice& device, const uint64 initialValue)
        : m_pDevice(&device)
    {
        vk::SemaphoreTypeCreateInfo timelineCreateInfo = vk::SemaphoreTypeCreateInfo()
            .setSemaphoreType(vk::SemaphoreType::eTimeline)
            .setInitialValue(initialValue);

        vk::SemaphoreCreateInfo semaphoreCreateInfo = vk::SemaphoreCreateInfo()
            .setPNext(&timelineCreateInfo);
        
        m_semaphore = vk::raii::Semaphore(device, semaphoreCreateInfo, device.GetVkAllocationCallbacks());
    }

    EGraphicsResult DeviceSemaphore::Wait(const uint64 value, const uint64 timeout) const
    {
        // If null, return.
        if (m_semaphore == nullptr)
            return EGraphicsResult::Failure;
        
        const vk::SemaphoreWaitInfo waitInfo = vk::SemaphoreWaitInfo()
            .setSemaphores(*m_semaphore)
            .setValues(value);

        NES_VK_FAIL_RETURN(*m_pDevice, m_pDevice->GetVkDevice().waitSemaphores(waitInfo, timeout));
        
        return EGraphicsResult::Success;
    }

    void DeviceSemaphore::Signal(const uint64 value) const
    {
        if (m_semaphore == nullptr)
            return;
        
        vk::SemaphoreSignalInfo signalInfo = vk::SemaphoreSignalInfo()
            .setSemaphore(*m_semaphore)
            .setValue(value);
        
        m_pDevice->GetVkDevice().signalSemaphore(signalInfo);
    }

    uint64 DeviceSemaphore::GetValue() const
    {
        if (m_semaphore == nullptr)
            return 0;
        
        return m_semaphore.getCounterValue();
    }

    void DeviceSemaphore::SetDebugName(const std::string& name)
    {
        m_pDevice->SetDebugNameVkObject(GetNativeVkObject(), name);
    }

    NativeVkObject DeviceSemaphore::GetNativeVkObject() const
    {
        return NativeVkObject(*m_semaphore, vk::ObjectType::eSemaphore);
    }

    void DeviceSemaphore::FreeSemaphore()
    {
        if (m_semaphore != nullptr)
        {
            Renderer::SubmitResourceFree([semaphore = std::move(m_semaphore)]() mutable
            {
                semaphore = nullptr;   
            });
        }
    }

    SemaphoreValue::SemaphoreValue(DeviceSemaphore* pSemaphore, const uint64 initialValue)
        : m_pSemaphore(pSemaphore)
    {
        // If non-zero, then it is fixed.
        if (initialValue != 0)
            m_fixedValue = initialValue;
        
        // Otherwise, it is considered dynamic.
        else
            m_dynamicValue = std::make_shared<std::atomic<uint64>>(0);
    }

    void SemaphoreValue::SetDynamicValue(const uint64 value)
    {
        // Must be dynamic and its dynamic value shouldn't have been set yet.
        NES_ASSERT(IsDynamic() && m_dynamicValue->load() == 0);

        // Update the shared_ptr value so that every copy of this semaphore
        // state has access to it.
        m_dynamicValue->store(value);

        // TryFixate() afterwards, to cache it in the fixed value.
        TryFixate();
    }

    bool SemaphoreValue::CanWait()
    {
        TryFixate();
        return static_cast<const SemaphoreValue*>(this)->CanWait();
    }

    bool SemaphoreValue::IsSignaled() const
    {
        const uint64 timelineValue = GetTimelineValue();
        if (timelineValue == 0)
            return false;

        const uint64 currentValue = m_pSemaphore->GetValue();
        return currentValue >= timelineValue;
    }

    bool SemaphoreValue::IsSignaled()
    {
        TryFixate();
        return static_cast<const SemaphoreValue*>(this)->IsSignaled();
    }

    uint64 SemaphoreValue::GetTimelineValue() const
    {
        if (m_fixedValue)
            return m_fixedValue;

        if (m_dynamicValue)
            return m_dynamicValue->load();

        // The semaphore is invalid.
        return 0;
    }

    EGraphicsResult SemaphoreValue::Wait(const uint64 timeout)
    {
        TryFixate();
        return static_cast<const SemaphoreValue*>(this)->Wait(timeout);
    }

    EGraphicsResult SemaphoreValue::Wait(const uint64 timeout) const
    {
        const uint64 timelineValue = GetTimelineValue();

        // If zero, then the fixed state hasn't been set!
        if (!m_pSemaphore || timelineValue == 0)
            return EGraphicsResult::InitializationFailed;

        return m_pSemaphore->Wait(timelineValue, timeout);
    }

    void SemaphoreValue::TryFixate()
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
