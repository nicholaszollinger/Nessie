// PipelineLayout.cpp
#include "PipelineLayout.h"

#include "RenderDevice.h"
#include "Renderer.h"

namespace nes
{
    PipelineLayout::PipelineLayout(PipelineLayout&& other) noexcept
        : m_pDevice(other.m_pDevice)
        , m_layout(std::move(other.m_layout))
        , m_bindPoint(other.m_bindPoint)
    {
        //
    }

    PipelineLayout& PipelineLayout::operator=(std::nullptr_t)
    {
        FreeLayout();
        return *this;
    }

    PipelineLayout& PipelineLayout::operator=(PipelineLayout&& other) noexcept
    {
        if (this != &other)
        {
            FreeLayout();

            m_pDevice = other.m_pDevice;
            m_layout = std::move(other.m_layout);
            m_bindPoint = other.m_bindPoint;

            other.m_pDevice = nullptr;
        }

        return *this;
    }

    PipelineLayout::~PipelineLayout()
    {
        FreeLayout();
    }

    PipelineLayout::PipelineLayout(RenderDevice& device, const PipelineLayoutDesc& desc)
        : m_pDevice(&device)
    {
        CreatePipelineLayout(device, desc);
    }
    
    void PipelineLayout::SetDebugName(const std::string& name)
    {
        m_pDevice->SetDebugNameVkObject(GetNativeVkObject(), name);
    }

    NativeVkObject PipelineLayout::GetNativeVkObject() const
    {
        return NativeVkObject(*m_layout, vk::ObjectType::ePipelineLayout);
    }

    void PipelineLayout::CreatePipelineLayout(const RenderDevice& device, const PipelineLayoutDesc& desc)
    {
        // Binding point:
        if (desc.m_shaderStages & EPipelineStageBits::GraphicsShaders)
            m_bindPoint = vk::PipelineBindPoint::eGraphics;
        else if (desc.m_shaderStages & EPipelineStageBits::ComputeShader)
            m_bindPoint = vk::PipelineBindPoint::eCompute;
        else if (desc.m_shaderStages & EPipelineStageBits::RayTracingShaders)
            m_bindPoint = vk::PipelineBindPoint::eRayTracingKHR;
        
        // [TODO]: Descriptor Set Layouts
        std::vector<vk::DescriptorSetLayout> descriptorSets{};
        
        // [TODO]: Push constant ranges:
        std::vector<vk::PushConstantRange> pushConstants{};
        
        // Create the pipeline layout:
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
            .setSetLayouts(descriptorSets)
            .setPushConstantRanges(pushConstants);
        
        m_layout = vk::raii::PipelineLayout(device, pipelineLayoutInfo, device.GetVkAllocationCallbacks());
    }

    void PipelineLayout::FreeLayout()
    {
        if (m_layout != nullptr)
        {
            Renderer::SubmitResourceFree([layout = std::move(m_layout)]() mutable
            {
                layout = nullptr;
            });
        }

        m_pDevice = nullptr;
    }
}
