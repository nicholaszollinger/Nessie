// Pipeline.cpp
#include "Pipeline.h"
#include "Vulkan/VulkanConversions.h"
#include "RenderDevice.h"
#include "PipelineLayout.h"
#include "Renderer.h"
#include "Nessie/Application/Device/DeviceManager.h"

namespace nes
{
    Pipeline::~Pipeline()
    {
        Renderer::SubmitResourceFree([pipeline = std::move(m_pipeline)]() mutable
        {
            pipeline = nullptr;
        });
    }

    EGraphicsResult Pipeline::Init(PipelineLayout& layout, const GraphicsPipelineDesc& desc)
    {
        // Shaders
        const auto& descShaders = desc.m_shaderStages;
        std::vector<vk::PipelineShaderStageCreateInfo> stages(descShaders.size());
        
        std::vector<vk::raii::ShaderModule> shaderModules;
        shaderModules.reserve(descShaders.size());
        
        for (size_t i = 0; i < descShaders.size(); ++i)
        {
            shaderModules.emplace_back(nullptr);
            auto& stage = stages[i];
            EGraphicsResult result = SetupShaderStage(stage, descShaders[i], shaderModules[i]);
            if (result != nes::EGraphicsResult::Success)
                return result;
        }

        // Vertex Input
        std::vector<vk::VertexInputAttributeDescription> vertexAttributes(desc.m_vertexInput.m_attributes.size());
        std::vector<vk::VertexInputBindingDescription> vertexBindings(desc.m_vertexInput.m_streams.size());
        
        vk::PipelineVertexInputStateCreateInfo vertexInputState = vk::PipelineVertexInputStateCreateInfo()
            .setPVertexAttributeDescriptions(vertexAttributes.data())
            .setVertexAttributeDescriptionCount(desc.m_vertexInput.m_attributes.size())
            .setPVertexBindingDescriptions(vertexBindings.data())
            .setVertexBindingDescriptionCount(desc.m_vertexInput.m_streams.size());

        // Fill out attributes:
        for (uint32 i = 0; i < desc.m_vertexInput.m_attributes.size(); ++i)
        {
            const auto& attribute = *(desc.m_vertexInput.m_attributes.begin() + i);
            auto& vkAttribute = vertexAttributes[i];

            vkAttribute.setFormat(GetVkFormat(attribute.m_format))
                .setOffset(attribute.m_offset)
                .setBinding(attribute.m_streamIndex)
                .setLocation(attribute.m_location);
        }

        // Fill out bindings:
        for (uint32 i = 0; i < desc.m_vertexInput.m_streams.size(); ++i)
        {
            const auto& stream = *(desc.m_vertexInput.m_streams.begin() + i);
            auto& vkBinding = vertexBindings[i];

            vkBinding.setBinding(stream.m_bindingIndex)
                .setStride(stream.m_stride)
                .setInputRate(stream.m_stepRate == EVertexStreamStepRate::PerVertex? vk::VertexInputRate::eVertex : vk::VertexInputRate::eInstance);
        }

        // Input Assembly
        // [TODO]: Current is hacked in.
        const auto& descInputAssembly = desc.m_inputAssembly;
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo()
            .setTopology(vk::PrimitiveTopology::eTriangleList);//(GetVkTopology(descInputAssembly.m_topology))
            //.setPrimitiveRestartEnable(descInputAssembly.m_primitiveRestart != EPrimitiveRestart::Disabled);

        vk::PipelineTessellationStateCreateInfo tessellationState = vk::PipelineTessellationStateCreateInfo()
            .setPatchControlPoints(descInputAssembly.m_tessControlPointCount);

        // Multisampling
        // [TODO]: Current is just hacked in.
        vk::PipelineMultisampleStateCreateInfo multisampleState = vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(vk::SampleCountFlagBits::e1)
            .setSampleShadingEnable(vk::False);

        // Rasterization
        // [TODO]: Conversions:
        // [TODO]: ConservativeStateCreateInfo
        // [TODO]: LineStateCreateInfoKHR
        //const auto& descRasterizer = desc.m_rasterization;
        vk::PipelineRasterizationStateCreateInfo rasterizationState = vk::PipelineRasterizationStateCreateInfo()
            .setDepthClampEnable(vk::False)//(descRasterizer.m_enableDepthClamp)
            .setRasterizerDiscardEnable(vk::False)
            .setPolygonMode(vk::PolygonMode::eFill)//(GetVkPolygonMode(descRasterizer.m_fillMode))
            .setCullMode(vk::CullModeFlagBits::eBack)//(GetVkCullMode(descRasterizer.m_cullMode))
            .setFrontFace(vk::FrontFace::eClockwise)//(GetVkFrontFace(descRasterizer.m_frontFace))
            .setDepthBiasEnable(vk::False)//(descRasterizer.m_depthBias.IsEnabled())
            //.setDepthBiasClamp(descRasterizer.m_depthBias.m_clamp)
            //.setDepthBiasConstantFactor(descRasterizer.m_depthBias.m_constant)
            .setDepthBiasSlopeFactor(1.f)//(descRasterizer.m_depthBias.m_slope)
            .setLineWidth(1.f);//(descRasterizer.m_lineWidth);
        
        // Viewport State (will be dynamic).
        vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo()
            .setViewportCount(1)
            .setScissorCount(1);

        // Depth-Stencil
        const DepthAttachmentDesc& depth = desc.m_outputMerger.m_depth;
        const StencilAttachmentDesc& stencil = desc.m_outputMerger.m_stencil;

        vk::StencilOpState front = vk::StencilOpState()
            .setPassOp(GetVkStencilOp(stencil.m_front.m_passOp))
            .setFailOp(GetVkStencilOp(stencil.m_front.m_failOp))
            .setDepthFailOp(GetVkStencilOp(stencil.m_front.m_depthFailOp))
            .setCompareMask(stencil.m_front.m_compareMask)
            .setWriteMask(stencil.m_front.m_writeMask);

        vk::StencilOpState back = vk::StencilOpState()
            .setPassOp(GetVkStencilOp(stencil.m_back.m_passOp))
            .setFailOp(GetVkStencilOp(stencil.m_back.m_failOp))
            .setDepthFailOp(GetVkStencilOp(stencil.m_back.m_depthFailOp))
            .setCompareMask(stencil.m_back.m_compareMask)
            .setWriteMask(stencil.m_back.m_writeMask);
        
        vk::PipelineDepthStencilStateCreateInfo depthStencilState = vk::PipelineDepthStencilStateCreateInfo()
            .setDepthTestEnable(depth.m_compareOp != ECompareOp::None)
            .setDepthWriteEnable(depth.m_enableWrite)
            .setDepthCompareOp(GetVkCompareOp(depth.m_compareOp))
            .setDepthBoundsTestEnable(depth.m_enableBoundsTest)
            .setStencilTestEnable((stencil.m_front.m_compareOp == ECompareOp::None && stencil.m_back.m_compareOp != ECompareOp::None)? vk::False : vk::True)
            .setMinDepthBounds(0.f)
            .setMaxDepthBounds(1.f)
            .setFront(front)
            .setBack(back);

        // Blending
        // [TODO]: Current is just hacked in. 
        //std::vector<vk::PipelineColorBlendAttachmentState> colors(desc.m_outputMerger.m_colorNum);
        //const auto& descOutputMerger = desc.m_outputMerger;
        vk::PipelineColorBlendAttachmentState colorAttachment = vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(false)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

        vk::PipelineColorBlendStateCreateInfo colorBlendState = vk::PipelineColorBlendStateCreateInfo()
            .setLogicOpEnable(vk::False)//(desc.m_outputMerger.m_logicOp != ELogicOp::None)
            .setLogicOp(vk::LogicOp::eCopy)//(desc.m_outputMerger.m_logicOp)
            .setAttachmentCount(1)//(desc.m_outputMerger.m_colorNum)
            .setPAttachments(&colorAttachment);

        // Formats
        // [TODO]: Color array in output merger needs to change.
        //const FormatProps& depthStencilFormatProps = GetFormatProps(descOutputMerger.m_depthStencilFormat);

        // [TODO]: Format data for the attachments:
        vk::Format colorFormats = vk::Format::eB8G8R8A8Srgb;
        // std::vector<vk::Format> colorFormats(1);// (descOutputMerger.m_colorNum);
        // for (size_t i = 0; i < colorFormats.size(); i++)
        // {
        //     colorFormats[i] = GetVkFormat(descOutputMerger.m_pColors[i].m_format);
        // }
        
        vk::PipelineRenderingCreateInfo pipelineRenderingInfo = vk::PipelineRenderingCreateInfo()
            //.setViewMask(descOutputMerger.m_viewMask)
            .setColorAttachmentCount(1)//(descOutputMerger.m_colorNum)
            .setPColorAttachmentFormats(&colorFormats);//(colorFormats.data())
            //.setDepthAttachmentFormat(GetVkFormat(descOutputMerger.m_depthStencilFormat))
            //.setStencilAttachmentFormat(depthStencilFormatProps.m_isStencil? GetVkFormat(descOutputMerger.m_depthStencilFormat) : vk::Format::eUndefined);

        // Dynamic State
        // [TODO]: Other dynamic states
        uint32 dynamicStateCount = 0;
        std::array<vk::DynamicState, 16> dynamicStates;
        dynamicStates[dynamicStateCount++] = vk::DynamicState::eViewport; // [TODO]: WithCount
        dynamicStates[dynamicStateCount++] = vk::DynamicState::eScissor;  // [TODO]: WithCount

        if (vertexInputState.pVertexAttributeDescriptions)
            dynamicStates[dynamicStateCount++] = vk::DynamicState::eVertexInputBindingStride;

        vk::PipelineDynamicStateCreateInfo dynamicState = vk::PipelineDynamicStateCreateInfo()
            .setDynamicStateCount(dynamicStateCount)
            .setPDynamicStates(dynamicStates.data());
        

        // Create
        vk::PipelineCreateFlags flags{};
        // [TODO]: 
        //if (rasterization.m_shadingRate)
            //flags |= vk::PipelineCreateFlagBits::eRenderingFragmentShadingRateAttachmentKHR;
        
        vk::GraphicsPipelineCreateInfo info = vk::GraphicsPipelineCreateInfo()
            .setPNext(&pipelineRenderingInfo)
            .setFlags(flags)
            .setStageCount(static_cast<uint32>(descShaders.size()))
            .setPStages(stages.data())
            .setPVertexInputState(&vertexInputState)
            .setPInputAssemblyState(&inputAssemblyState)
            //.setPTessellationState(&tessellationState)
            .setPViewportState(&viewportState)
            .setPRasterizationState(&rasterizationState)
            .setPMultisampleState(&multisampleState)
            //.setPDepthStencilState(&depthStencilState)
            .setPColorBlendState(&colorBlendState)
            .setPDynamicState(&dynamicState)
            .setLayout(layout.GetVkPipelineLayout())
            .setRenderPass(nullptr);
            //.setSubpass(0)
            //.setBasePipelineHandle(nullptr)
            //.setBasePipelineIndex(-1);

        // [TODO]: Pipeline Cache object.
        m_pipeline = vk::raii::Pipeline(m_device, nullptr, info, m_device.GetVkAllocationCallbacks());
        
        return EGraphicsResult::Success;
    }

    void Pipeline::SetDebugName(const std::string& name)
    {
        vk::Pipeline pipeline = *m_pipeline;
        m_device.SetDebugNameToTrivialObject(pipeline, name);
    }

    EGraphicsResult Pipeline::SetupShaderStage(vk::PipelineShaderStageCreateInfo& outStage, const ShaderDesc& desc, vk::raii::ShaderModule& outModule)
    {
        vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo()
            .setCodeSize(desc.m_size)
            .setPCode(static_cast<const uint32_t*>(desc.m_pByteCode));
        
        auto& device = DeviceManager::GetRenderDevice();
        outModule = vk::raii::ShaderModule(device, createInfo);

        outStage = vk::PipelineShaderStageCreateInfo()
            .setStage(GetVkShaderStageFlagBits(desc.m_stage))
            .setModule(outModule);


        // Entry Point:
        // [TODO]: This is only valid for glsl; slang is based on stage, "vertMain", "fragMain", etc.
        // - I should handle this within the Shader Desc / Shader itself.
        outStage.pName = desc.m_entryPointName == nullptr ? "main" : desc.m_entryPointName;
        
        return EGraphicsResult::Success;
    }
}
