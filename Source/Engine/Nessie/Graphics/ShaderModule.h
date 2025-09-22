// ShaderModule.h
#pragma once
#include "GraphicsCommon.h"
#include "DeviceObject.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A shader module contains the shader code for a single pipeline stage.
    //----------------------------------------------------------------------------------------------------
    class ShaderModule
    {
    public:
        ShaderModule(std::nullptr_t) {}
        ShaderModule(const ShaderModule&) = delete;
        ShaderModule(ShaderModule&& other) noexcept;
        ShaderModule& operator=(std::nullptr_t) noexcept;
        ShaderModule& operator=(const ShaderModule&) = delete;
        ShaderModule& operator=(ShaderModule&& other) noexcept;
        ~ShaderModule();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a new shader module.
        //----------------------------------------------------------------------------------------------------
        ShaderModule(RenderDevice& device, ShaderModuleDesc&& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the properties of the shader module, including the stage that it will be executed. 
        //----------------------------------------------------------------------------------------------------
        const ShaderModuleDesc&         GetDesc() const { return m_desc; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this module.
        //----------------------------------------------------------------------------------------------------
        void                            SetDebugName(const std::string& name);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the vulkan module object.
        //----------------------------------------------------------------------------------------------------
        const vk::raii::ShaderModule&   GetVkShaderModule() const { return m_module; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Advanced use. Get the native vulkan object handle, and the type.
        //----------------------------------------------------------------------------------------------------
        NativeVkObject                  GetNativeVkObject() const;
        
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Submits the module to be freed by the Renderer, and resets the description. 
        //----------------------------------------------------------------------------------------------------
        void                            FreeShaderModule();

    private:
        RenderDevice*                   m_pDevice = nullptr;
        vk::raii::ShaderModule          m_module = nullptr;
        ShaderModuleDesc                m_desc{};
    };
    static_assert(DeviceObjectType<ShaderModule>);
}
