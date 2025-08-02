// Shader.cpp
#include "Shader.h"

#include "RenderDevice.h"

namespace nes
{
    Shader::~Shader()
    {
        if (m_handle != nullptr)
        {
            vkDestroyShaderModule(m_device, m_handle, m_device.GetVulkanAllocationCallbacks());
        }
    }

    EGraphicsResult Shader::Init(const ShaderDesc& desc)
    {
        if (!desc.IsValid())
            return EGraphicsResult::InvalidArgument;
        
        const VkShaderModuleCreateInfo createInfo
        {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = static_cast<VkShaderModuleCreateFlags>(0),
            .codeSize = desc.m_size,
            .pCode = static_cast<const uint32*>(desc.m_pByteCode),
        };
        NES_VK_FAIL_RETURN(m_device, vkCreateShaderModule(m_device, &createInfo, m_device.GetVulkanAllocationCallbacks(), &m_handle));
        
        m_desc = desc;
        return EGraphicsResult::Success;
    }

    void Shader::SetDebugName(const char* name)
    {
        m_device.SetDebugNameToTrivialObject(m_handle, name);
    }
}
