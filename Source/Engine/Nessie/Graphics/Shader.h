// Shader.h
#pragma once
#include "GraphicsCommon.h"
#include "DeviceAsset.h"

namespace nes
{
    class Shader final : public DeviceAsset
    {
    public:
        explicit            Shader(RenderDevice& device) : DeviceAsset(device) {}
        virtual             ~Shader() override;

        /// Operator to cast to the Vulkan Type.
        inline              operator VkShaderModule() const { return m_handle; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Creates the underlying VkShaderModule object.
        //----------------------------------------------------------------------------------------------------
        EGraphicsResult     Init(const ShaderDesc& desc);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a debug name for this shader resource.
        //----------------------------------------------------------------------------------------------------
        virtual void        SetDebugName(const char* name) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the shader description which includes the byte code, shader stage and entry point name.
        //----------------------------------------------------------------------------------------------------
        const ShaderDesc&   GetDesc() const { return m_desc; }

        

    private:
        ShaderDesc          m_desc;
        VkShaderModule      m_handle = nullptr;
    };
}
