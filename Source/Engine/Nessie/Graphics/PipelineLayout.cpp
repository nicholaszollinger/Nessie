// PipelineLayout.cpp
#include "PipelineLayout.h"

#include "RenderDevice.h"
#include "Renderer.h"

namespace nes
{
    PipelineLayout::~PipelineLayout()
    {
        Renderer::SubmitResourceFree([layout = std::move(m_layout)]() mutable
        {
            layout = nullptr;
        });
    }

    EGraphicsResult PipelineLayout::Init(const PipelineLayoutDesc& desc)
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
        
        m_layout = vk::raii::PipelineLayout(m_device, pipelineLayoutInfo, m_device.GetVkAllocationCallbacks());
        
        return EGraphicsResult::Success;
    }

    void PipelineLayout::SetDebugName(const std::string& name)
    {
        vk::PipelineLayout pipelineLayout = *m_layout;
        m_device.SetDebugNameToTrivialObject(pipelineLayout, name);
    }
}
