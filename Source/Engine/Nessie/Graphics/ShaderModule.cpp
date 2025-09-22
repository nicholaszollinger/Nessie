// ShaderModule.cpp
#include "ShaderModule.h"
#include "RenderDevice.h"
#include "Renderer.h"

namespace nes
{
    ShaderModule::ShaderModule(ShaderModule&& other) noexcept
        : m_pDevice(other.m_pDevice)
        , m_module(std::move(other.m_module))
        , m_desc(std::move(other.m_desc))
    {
        other.m_pDevice = nullptr;
    }

    ShaderModule& ShaderModule::operator=(std::nullptr_t) noexcept
    {
        FreeShaderModule();
        return *this;
    }

    ShaderModule& ShaderModule::operator=(ShaderModule&& other) noexcept
    {
        if (this != &other)
        {
            FreeShaderModule();

            m_module = std::move(other.m_module);
            m_desc = other.m_desc;
            m_pDevice = other.m_pDevice;

            other.m_pDevice = nullptr;
        }
        
        return *this;
    }

    ShaderModule::~ShaderModule()
    {
        FreeShaderModule();
    }

    ShaderModule::ShaderModule(RenderDevice& device, ShaderModuleDesc&& desc)
        : m_pDevice(&device)
        , m_desc(std::move(desc))
    {
        vk::ShaderModuleCreateInfo moduleInfo = vk::ShaderModuleCreateInfo()
            .setPCode(reinterpret_cast<const uint32_t*>(m_desc.m_binary.data()))
            .setCodeSize(m_desc.m_binary.size());
        
        m_module = vk::raii::ShaderModule(device, moduleInfo, device.GetVkAllocationCallbacks());
    }

    void ShaderModule::SetDebugName(const std::string& name)
    {
        NES_ASSERT(m_pDevice != nullptr);
        m_pDevice->SetDebugNameVkObject(GetNativeVkObject(), name);
    }

    NativeVkObject ShaderModule::GetNativeVkObject() const
    {
        return {*m_module, vk::ObjectType::eShaderModule };
    }

    void ShaderModule::FreeShaderModule()
    {
        if (m_module != nullptr)
        {
            nes::Renderer::SubmitResourceFree([module = std::move(m_module)]() mutable
            {
                module = nullptr; 
            });

            m_desc = {};
        }
    }
}
