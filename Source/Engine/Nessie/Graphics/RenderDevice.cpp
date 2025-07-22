// RenderDevice.cpp

#include "RenderDevice.h"

namespace nes
{
    bool RenderDevice::Init(const ApplicationDesc&, ApplicationWindow*, const RendererDesc& rendererDesc)
    {
        m_allocationCallbacks = rendererDesc.m_allocationCallbacks;
        m_allocationCallbacks.EnsureValidCallbacksOrReset();
    
        return true;
    }
}