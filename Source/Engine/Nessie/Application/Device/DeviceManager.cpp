// DeviceManager.cpp
#include "DeviceManager.h"

#include "Nessie/Application/Platform.h"
#include "Nessie/Graphics/RenderDevice.h"
#include "GLFW/glfw3.h"

namespace nes
{
    NES_DEFINE_LOG_TAG(kGLFWLogTag, "GLFW", Warn);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Callback used to handle errors in GLFW.
    //----------------------------------------------------------------------------------------------------
    static void ErrorCallback([[maybe_unused]] int error, [[maybe_unused]] const char* description)
    {
        NES_ERROR(nes::kGLFWLogTag, "Error: {} - {}", error, description);
    }

    RenderDevice& DeviceManager::GetRenderDevice()
    {
        return *(Platform::GetDeviceManager().m_pDevice);
    }

    bool DeviceManager::Init()
    {
        // Initialize GLFW
        if (!glfwInit())
        {
            NES_ERROR(kGLFWLogTag, "GLFW could not be initialized!");
            return false;
        }

        glfwSetErrorCallback(ErrorCallback);

        // [TODO]: Initialize system info, what input devices are connected, gpu info, display, etc.
        
        return true;
    }

    bool DeviceManager::CreateRenderDevice(const ApplicationDesc& appDesc, const RendererDesc& rendererDesc)
    {
        m_pDevice = std::make_unique<RenderDevice>();
        if (!m_pDevice->Init(appDesc, rendererDesc))
        {
            NES_ERROR("Failed to Create Render Device!");
            return false;
        }

        return true;
    }

    void DeviceManager::Shutdown()
    {
        if (m_pDevice)
        {
            m_pDevice->Destroy();
            m_pDevice.reset();
        }

        // Terminate GLFW
        glfwTerminate();
    }
    
}
