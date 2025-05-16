// RefCounter.inl
#pragma once

namespace nes::internal
{
    inline RefCounterBase::~RefCounterBase()
    {
#if NES_LOGGING_ENABLED
        // Check that there are no external references remaining.
        const uint32_t refCount = m_refCount.load(std::memory_order_relaxed);
        NES_ASSERT(refCount == 0 || refCount == kEmbedded);
#endif
    }
    
    inline bool RefCounterBase::RemoveRef() const
    {
        if (m_refCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            ReleaseObject();
            return true;
        }

        return false;
    }
    
    template <typename Type>
    template <typename To>
    inline RefCounter<To>* RefCounter<Type>::GetAs()
    {
        NES_ASSERT((m_pObject == nullptr) || (checked_cast<To*>(m_pObject) != nullptr));
        return reinterpret_cast<RefCounter<To>*>(this);
    }
    
    template <typename Type>
    inline void RefCounter<Type>::ReleaseObject() const
    {
        NES_SAFE_DELETE(m_pObject);
    }
}

namespace nes
{
    template <typename Derived>
    void RefTarget<Derived>::ReleaseObject() const
    {
        // This is a bit sketch, but I needed to have a non-const pointer for the JobSystem::Job
        // class to call into FreeJob().
        const Derived* pConstThis = static_cast<const Derived*>(this);
        Derived* pThis = const_cast<Derived*>(pConstThis);
        ReleaseObjectImpl(pThis);
    }
}