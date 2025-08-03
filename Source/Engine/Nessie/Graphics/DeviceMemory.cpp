// DeviceMemory.cpp
#include "DeviceMemory.h"

#include "RenderDevice.h"

namespace nes
{
    DeviceMemory::~DeviceMemory()
    {
        if (m_ownsNativeObjects)
        {
            vkFreeMemory(m_device, m_handle, m_device.GetVkAllocationCallbacks());
        }
    }

    EGraphicsResult DeviceMemory::Init(const DeviceMemoryCreateInfo& /*info*/)
    {
        // [TODO]: 

        return EGraphicsResult::Success;
    }

    EGraphicsResult DeviceMemory::InitDedicated(const DeviceBuffer& /*buffer*/)
    {
        // [TODO]: 
        
        return EGraphicsResult::Success;
    }

    EGraphicsResult DeviceMemory::InitDedicated(const Texture& /*texture*/)
    {
        // [TODO]: 
        
        return EGraphicsResult::Success;
    }

    void DeviceMemory::SetDebugName(const char* name)
    {
        m_device.SetDebugNameToTrivialObject(m_handle, name);
    }
}
