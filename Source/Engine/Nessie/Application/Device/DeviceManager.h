// DeviceManager.h
#pragma once
#include <memory>
#include <string>

namespace nes
{
    class   RenderDevice;
    class   Application;
    struct  ApplicationDesc;
    class   ApplicationWindow;
    struct  WindowDesc;
    struct  NativeWindow;
    struct  RendererDesc;

    //----------------------------------------------------------------------------------------------------
    // [TODO LATER]:
    // - This should be manager for all connected input devices, the Monitor, GPU, etc.
    // - Init should return a SystemDesc object that can be sent to the CreateApplication function.
    //
    /// @brief : Handles device creation/management.
    //----------------------------------------------------------------------------------------------------
    class DeviceManager
    {
    public:
        DeviceManager() = default;
        ~DeviceManager() = default;

        DeviceManager(const DeviceManager&) = delete;
        DeviceManager& operator=(const DeviceManager&) = delete;
        DeviceManager(DeviceManager&&) noexcept = delete;
        DeviceManager& operator=(DeviceManager&&) noexcept = delete;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Render Device.
        //----------------------------------------------------------------------------------------------------
        static RenderDevice&    GetRenderDevice();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the device manager, loading the Graphics API and getting device information.
        //----------------------------------------------------------------------------------------------------
        bool                    Init();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the RenderDevice.
        //----------------------------------------------------------------------------------------------------
        bool                    CreateRenderDevice(const ApplicationDesc& appDesc, const RendererDesc& rendererDesc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destroys the RenderDevice. 
        //----------------------------------------------------------------------------------------------------
        void                    Shutdown();
        
    private:
        std::unique_ptr<RenderDevice> m_pDevice;
    };
}
