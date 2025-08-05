// DeviceBuffer.cpp
#include "DeviceBuffer.h"
#include "RenderDevice.h"

namespace nes
{
    DeviceBuffer::~DeviceBuffer()
    {
        if (m_ownsNativeObjects)
        {
            if (m_allocation)
            {
                auto& allocator = m_device.GetResourceAllocator();
                allocator.FreeBuffer(*this);
            }
        }
    }

    EGraphicsResult DeviceBuffer::Init(const AllocateBufferDesc& desc)
    {
        auto& allocator = m_device.GetResourceAllocator();
        return allocator.AllocateBuffer(desc, *this);
    }

    void DeviceBuffer::SetDebugName(const char* name)
    {
        m_device.SetDebugNameToTrivialObject(m_handle, name);
    }
}
