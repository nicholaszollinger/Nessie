// Pipeline.cpp
#include "Pipeline.h"
#include "Vulkan/VulkanConversions.h"
#include "RenderDevice.h"
#include "PipelineLayout.h"
#include "Renderer.h"
#include "ShaderModule.h"
#include "Nessie/Application/Device/DeviceManager.h"

namespace nes
{
    Pipeline::Pipeline(Pipeline&& other) noexcept
        : m_pDevice(other.m_pDevice)
        , m_pipeline(std::move(other.m_pipeline))
        , m_bindPoint(other.m_bindPoint)
    {
        other.m_pDevice = nullptr;
    }

    Pipeline& Pipeline::operator=(std::nullptr_t)
    {
        FreePipeline();
        return *this;
    }

    Pipeline& Pipeline::operator=(Pipeline&& other) noexcept
    {
        if (this != &other)
        {
            FreePipeline();

            m_pDevice = other.m_pDevice;
            m_pipeline = std::move(other.m_pipeline);
            m_bindPoint = other.m_bindPoint;

            other.m_pDevice = nullptr;
        }
        
        return *this;
    }

    Pipeline::Pipeline(RenderDevice& device, PipelineLayout& layout, const GraphicsPipelineDesc& desc)
        : m_pDevice(&device)
    {
        CreateGraphicsPipeline(device, layout, desc);
    }

    NativeVkObject Pipeline::GetNativeVkObject() const
    {
        return NativeVkObject(*m_pipeline, vk::ObjectType::ePipeline);
    }

    void Pipeline::CreateGraphicsPipeline(const RenderDevice& device, PipelineLayout& layout, const GraphicsPipelineDesc& desc)
    {
        NES_ASSERT(layout != nullptr);
        
        // Shaders
        const auto& descShaders = desc.m_shaderStages;
        std::vector<vk::PipelineShaderStageCreateInfo> stages;

        #define TRY_ADD_STAGE(pModule)  \
        if (pModule)            \
        { \
            vk::PipelineShaderStageCreateInfo info; \
            SetupShaderStage(*(pModule), info); \
            stages.emplace_back(info); \
        }
        
        TRY_ADD_STAGE(descShaders.m_vertex);
        TRY_ADD_STAGE(descShaders.m_fragment);
        TRY_ADD_STAGE(descShaders.m_geometry);
        TRY_ADD_STAGE(descShaders.m_tessControl);
        TRY_ADD_STAGE(descShaders.m_tessEval);
        TRY_ADD_STAGE(descShaders.m_meshControl);
        TRY_ADD_STAGE(descShaders.m_meshEval);
        
        #undef TRY_ADD_STAGE
        
        // Vertex Input
        std::vector<vk::VertexInputAttributeDescription> vertexAttributes(desc.m_vertexInput.m_attributes.size());
        std::vector<vk::VertexInputBindingDescription> vertexBindings(desc.m_vertexInput.m_streams.size());
        vk::PipelineVertexInputStateCreateInfo vertexInputState = vk::PipelineVertexInputStateCreateInfo()
            .setPVertexAttributeDescriptions(vertexAttributes.data())
            .setPVertexBindingDescriptions(vertexBindings.data())
            .setVertexAttributeDescriptionCount(desc.m_vertexInput.m_attributes.size())
            .setVertexBindingDescriptionCount(desc.m_vertexInput.m_streams.size());

        VertexStreamDesc const* pStreams = desc.m_vertexInput.m_streams.data();
        VertexAttributeDesc const* pAttributes = desc.m_vertexInput.m_attributes.data();
        
        // Fill out attributes:
        for (uint32 i = 0; i < desc.m_vertexInput.m_attributes.size(); ++i)
        {
            const auto& attribute = pAttributes[i];
            auto& vkAttribute = vertexAttributes[i];

            vkAttribute = vk::VertexInputAttributeDescription()
                .setFormat(GetVkFormat(attribute.m_format))
                .setOffset(attribute.m_offset)
                .setBinding(attribute.m_streamIndex)
                .setLocation(attribute.m_location);
        }

        // Fill out bindings:
        for (uint32 i = 0; i < desc.m_vertexInput.m_streams.size(); ++i)
        {
            const VertexStreamDesc& stream = pStreams[i];
            auto& vkBinding = vertexBindings[i];

            vkBinding = vk::VertexInputBindingDescription();
            vkBinding.binding = stream.m_bindingIndex;
            vkBinding.stride = stream.m_stride;
            vkBinding.inputRate = stream.m_stepRate == EVertexStreamStepRate::PerVertex? vk::VertexInputRate::eVertex : vk::VertexInputRate::eInstance;
        }
        
        // Input Assembly:
        const auto& descInputAssembly = desc.m_inputAssembly;
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo()
            .setTopology(GetVkTopology(descInputAssembly.m_topology))
            .setPrimitiveRestartEnable(descInputAssembly.m_primitiveRestart != EPrimitiveRestart::Disabled);

        // Tesselation State
        vk::PipelineTessellationStateCreateInfo tessellationState = vk::PipelineTessellationStateCreateInfo()
            .setPatchControlPoints(descInputAssembly.m_tessControlPointCount);

        // Multisampling
        vk::PipelineSampleLocationsStateCreateInfoEXT sampleLocationsState = {};
        vk::PipelineMultisampleStateCreateInfo multisampleState = vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(vk::SampleCountFlagBits::e1);
        
        if (desc.m_enableMultisample)
        {
            multisampleState.setRasterizationSamples(GetVkSampleCountFlags(desc.m_multisample.m_sampleCount))
                .setPSampleMask(desc.m_multisample.m_sampleMask != 0? &desc.m_multisample.m_sampleMask : nullptr)
                .setMinSampleShading(0.f)
                .setSampleShadingEnable(vk::False)
                .setAlphaToCoverageEnable(desc.m_multisample.m_alphaToCoverage)
                .setAlphaToOneEnable(false);
        
            if (desc.m_multisample.m_sampleLocations)
            {
                sampleLocationsState.sampleLocationsEnable = vk::True;
                multisampleState.pNext = &sampleLocationsState;
            }
        }

        // Rasterization
        const auto& descRasterizer = desc.m_rasterization;
        vk::PipelineRasterizationStateCreateInfo rasterizationState = vk::PipelineRasterizationStateCreateInfo()
            .setDepthClampEnable(descRasterizer.m_enableDepthClamp)
            .setRasterizerDiscardEnable(vk::False)
            .setPolygonMode(GetVkPolygonMode(descRasterizer.m_fillMode))
            .setCullMode(GetVkCullMode(descRasterizer.m_cullMode))
            .setFrontFace(GetVkFrontFace(descRasterizer.m_frontFace))
            .setDepthBiasEnable(descRasterizer.m_depthBias.m_enabled)
            .setDepthBiasClamp(descRasterizer.m_depthBias.m_clamp)
            .setDepthBiasConstantFactor(descRasterizer.m_depthBias.m_constant)
            .setDepthBiasSlopeFactor(descRasterizer.m_depthBias.m_slope)
            .setLineWidth(1.f);

        // [TODO]: Conservative Raster State
        // vk::PipelineRasterizationConservativeStateCreateInfoEXT conservativeState = vk::PipelineRasterizationConservativeStateCreateInfoEXT();
        // if (descRasterizer.m_enableConservativeRaster)
        // {
        //     conservativeState.conservativeRasterizationMode = vk::ConservativeRasterizationModeEXT::eOverestimate;
        //     conservativeState.extraPrimitiveOverestimationSize = 0.0f;
        //     APPEND the extension to the rasterization state.
        // }
        
        // [TODO]: Line Smoothing State
        // vk::PipelineRasterizationLineStateCreateInfoKHR rasterizationLineState = vk::PipelineRasterizationLineStateCreateInfo();
        // if (descRasterizer.m_enableLineSmoothing)
        // {
        //     rasterizationLineState.lineRasterizationMode = vk::LineRasterizationMode::eRectangularSmoothKHR;
        //     APPEND the extension to the rasterization state.
        // }
        
        // Viewport State (will be dynamic).
        vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo()
            .setViewportCount(0)
            .setScissorCount(0);

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
            //.setDepthBoundsTestEnable(depth.m_enableBoundsTest)
            //.setStencilTestEnable((stencil.m_front.m_compareOp == ECompareOp::None && stencil.m_back.m_compareOp != ECompareOp::None)? vk::False : vk::True)
            .setMinDepthBounds(0.f)
            .setMaxDepthBounds(1.f)
            .setFront(front)
            .setBack(back);

        // Blending
        const auto& descOutputMerger = desc.m_outputMerger;
        std::vector<vk::PipelineColorBlendAttachmentState> colorAttachments(desc.m_outputMerger.m_colorCount);
        
        // [TODO]: Check for constant color reference for possible dynamic state.
        for (uint32 i = 0; i < desc.m_outputMerger.m_colorCount; ++i)
        {
            const ColorAttachmentDesc& attachmentDesc = descOutputMerger.m_pColors[i];
        
            colorAttachments[i] = vk::PipelineColorBlendAttachmentState()
                .setBlendEnable(attachmentDesc.m_enableBlend)
                .setSrcColorBlendFactor(GetVkBlendFactor(attachmentDesc.m_colorBlend.m_srcFactor))
                .setDstColorBlendFactor(GetVkBlendFactor(attachmentDesc.m_colorBlend.m_dstFactor))
                .setColorBlendOp(GetVkBlendOp(attachmentDesc.m_colorBlend.m_op))
                .setSrcAlphaBlendFactor(GetVkBlendFactor(attachmentDesc.m_alphaBlend.m_srcFactor))
                .setDstAlphaBlendFactor(GetVkBlendFactor(attachmentDesc.m_alphaBlend.m_dstFactor))
                .setAlphaBlendOp(GetVkBlendOp(attachmentDesc.m_colorBlend.m_op))
                .setColorWriteMask(GetVkColorComponentFlags(attachmentDesc.m_colorWriteMask));
        }
        
        vk::PipelineColorBlendStateCreateInfo colorBlendState = vk::PipelineColorBlendStateCreateInfo()
            .setLogicOpEnable(desc.m_outputMerger.m_logicOp != ELogicOp::None) // This requires a feature to be enabled!
            .setLogicOp(GetVkLogicOp(desc.m_outputMerger.m_logicOp))
            .setAttachments(colorAttachments);
        
        // Formats
        const FormatProps& depthStencilFormatProps = GetFormatProps(desc.m_outputMerger.m_depthStencilFormat);
        
        // Format data for the attachments:
        std::vector<vk::Format> colorFormats(descOutputMerger.m_colorCount);
        for (size_t i = 0; i < colorFormats.size(); i++)
        {
             colorFormats[i] = GetVkFormat(descOutputMerger.m_pColors[i].m_format);
        }
        
        vk::PipelineRenderingCreateInfo pipelineRenderingInfo = vk::PipelineRenderingCreateInfo()
            .setColorAttachmentCount(descOutputMerger.m_colorCount)
            .setColorAttachmentFormats(colorFormats)
            .setDepthAttachmentFormat(GetVkFormat(descOutputMerger.m_depthStencilFormat))
            .setStencilAttachmentFormat(depthStencilFormatProps.m_isStencil? GetVkFormat(descOutputMerger.m_depthStencilFormat) : vk::Format::eUndefined);

        // Dynamic State
        uint32 dynamicStateCount = 0;
        std::array<vk::DynamicState, 16> dynamicStates;
        dynamicStates[dynamicStateCount++] = vk::DynamicState::eViewportWithCount; 
        dynamicStates[dynamicStateCount++] = vk::DynamicState::eScissorWithCount;

        if (vertexInputState.pVertexAttributeDescriptions)
            dynamicStates[dynamicStateCount++] = vk::DynamicState::eVertexInputBindingStride;
        
        if (rasterizationState.depthBiasEnable)
            dynamicStates[dynamicStateCount++] = vk::DynamicState::eDepthBias;
        // if (depthStencilState.depthBoundsTestEnable)
        //     dynamicStates[dynamicStateCount++] = vk::DynamicState::eDepthBounds;
        // if (depthStencilState.stencilTestEnable)
        //     dynamicStates[dynamicStateCount++] = vk::DynamicState::eStencilReference;
        // if (sampleLocationsState.sampleLocationsEnable)
        //     dynamicStates[dynamicStateCount++] = vk::DynamicState::eSampleLocationsEXT;

        // [TODO]: Optional Shading Rate Dynamic State
        // [TODO]: Conditional Blend Constants Dynamic State

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
            .setStages(stages)
            .setPVertexInputState(&vertexInputState)
            .setPInputAssemblyState(&inputAssemblyState)
            .setPTessellationState(&tessellationState)
            .setPViewportState(&viewportState)
            .setPRasterizationState(&rasterizationState)
            .setPMultisampleState(&multisampleState)
            .setPDepthStencilState(&depthStencilState)
            .setPColorBlendState(&colorBlendState)
            .setPDynamicState(&dynamicState)
            .setLayout(layout.GetVkPipelineLayout())
            .setRenderPass(nullptr);

        // [TODO]: Pipeline Cache object.
        m_pipeline = vk::raii::Pipeline(device, nullptr, info, device.GetVkAllocationCallbacks());
    }

    void Pipeline::FreePipeline()
    {
        if (m_pipeline != nullptr)
        {
            Renderer::SubmitResourceFree([pipeline = std::move(m_pipeline)]() mutable
            {
                pipeline = nullptr;
            });
        }

        m_pDevice = nullptr;
    }

    Pipeline::~Pipeline()
    {
        Renderer::SubmitResourceFree([pipeline = std::move(m_pipeline)]() mutable
        {
            pipeline = nullptr;
        });
    }

    void Pipeline::SetDebugName(const std::string& name)
    {
        m_pDevice->SetDebugNameVkObject(GetNativeVkObject(), name);
    }

    EGraphicsResult Pipeline::SetupShaderStage(const ShaderModule& module, vk::PipelineShaderStageCreateInfo& outStage)
    {
        auto& desc = module.GetDesc();
        outStage = vk::PipelineShaderStageCreateInfo()
            .setStage(GetVkShaderStageFlagBits(desc.m_stage))
            .setModule(*module.GetVkShaderModule())
            .setPName(desc.m_entryPointName.c_str());
        
        return EGraphicsResult::Success;
    }
}
