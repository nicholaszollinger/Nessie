// RenderDevice.cpp
#include "RenderDevice.h"

namespace nes
{
    bool RenderDevice::Init(const ApplicationDesc& /*appDesc*/, ApplicationWindow* /*pWindow*/, const RendererDesc& /*rendererDesc*/)
    {
//         // Device Description:
//         nri::DeviceCreationDesc deviceCreationDesc = {};
//         deviceCreationDesc.graphicsAPI = nri::GraphicsAPI::VK;
//         deviceCreationDesc.enableD3D11CommandBufferEmulation = false;
//         deviceCreationDesc.vkBindingOffsets = {};
//
//         // [TODO]: Add GLFW vulkan Extensions:
//         //deviceCreationDesc.vkExtensions.;
//         
//         deviceCreationDesc.adapterDesc = {};
//         deviceCreationDesc.allocationCallbacks = {};
//         deviceCreationDesc.callbackInterface.MessageCallback = DeviceMessageCallback;
//         deviceCreationDesc.callbackInterface.AbortExecution = nullptr;
//         
// #if NES_DEBUG
//         deviceCreationDesc.enableGraphicsAPIValidation = true;
//         deviceCreationDesc.enableNRIValidation = true;
// #else
//         deviceCreationDesc.enableNRIValidation = false;
//         deviceCreationDesc.enableGraphicsAPIValidation = false;
// #endif
//
//         // Create the Device
//         if (nri::nriCreateDevice(deviceCreationDesc, m_pDeviceHandle) != nri::Result::SUCCESS)
//         {
//             NES_ERROR("Failed to create the Render device!");
//             return false;
//         }
//
//         // Core interface:
//         if (nri::nriGetInterface(*m_pDeviceHandle, NRI_INTERFACE(nri::CoreInterface), &m_coreInterface) != nri::Result::SUCCESS)
//         {
//             NES_ERROR("Failed to get the core nri interface!");
//             // The device will be cleaned up on Destroy().
//             return false;
//         }
        
        return true;
    }

    void RenderDevice::Destroy()
    {
        // if (m_pDeviceHandle != nullptr)
        // {
        //     nri::nriDestroyDevice(m_pDeviceHandle);
        //     m_pDeviceHandle = nullptr;
        // }
    }

    // const nri::DeviceDesc& RenderDevice::GetDesc() const
    // {
    //     NES_ASSERT(m_pDeviceHandle != nullptr);
    //     NES_ASSERT(m_coreInterface.GetDeviceDesc != nullptr);
    //     
    //     return m_coreInterface.GetDeviceDesc(*m_pDeviceHandle);
    // }
}