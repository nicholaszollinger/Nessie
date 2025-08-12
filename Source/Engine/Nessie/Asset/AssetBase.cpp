// AssetBase.cpp
#include "AssetBase.h"

namespace nes
{
    AssetBase::AssetBase(AssetBase&& other) noexcept
        : m_lockCount(other.m_lockCount.load(std::memory_order_relaxed))
        , m_id(other.m_id)
        //, m_handle(other.m_handle)
    {
        other.m_id = kInvalidAssetID;
        //other.m_handle = kInvalidAssetHandle;
        other.m_lockCount.store(0);
    }

    AssetBase& AssetBase::operator=(AssetBase&& other) noexcept
    {
        if (this != &other)
        {
            m_id = other.m_id;
            //m_handle = other.m_handle;
            m_lockCount.store(other.m_lockCount.load(std::memory_order_relaxed));

            other.m_id = kInvalidAssetID;
            //other.m_handle = kInvalidAssetHandle;
            other.m_lockCount.store(0);
        }

        return *this;
    }
    
    void AssetBase::RemoveLock() const
    {
        if (HasLocks())
        {
            m_lockCount.fetch_sub(1, std::memory_order_relaxed);
        }
    }
}
