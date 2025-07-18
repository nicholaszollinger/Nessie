// Device.cpp

#include "Device.h"

namespace nes
{
    bool Device::Init(const ApplicationDesc&, ApplicationWindow*, const RendererDesc& rendererDesc)
    {
        m_allocationCallbacks = rendererDesc.m_allocationCallbacks;
        m_allocationCallbacks.EnsureValidCallbacksOrReset();
    
        return true;
    }
}

