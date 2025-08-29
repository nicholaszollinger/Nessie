// GraphicsCommon.cpp
#include "GraphicsCommon.h"

namespace nes
{
//============================================================================================================================================================================================
#pragma region [ Common ]
//============================================================================================================================================================================================

    Viewport::Viewport(const uint32 width, const uint32 height, const uint32 xOffset, const uint32 yOffset)
    {
        SetOffset(xOffset, yOffset);
        SetExtent(width, height);
    }

    Viewport& Viewport::SetOffset(const uint32 x, const uint32 y)
    {
        m_offset.x = static_cast<float>(x);
        m_offset.y = static_cast<float>(y);
        return *this;
    }

    Viewport& Viewport::SetExtent(const uint32 width, const uint32 height)
    {
        m_extent.x = static_cast<float>(width);
        m_extent.y = static_cast<float>(height);
        return *this;
    }

    Scissor::Scissor(const uint32 width, const uint32 height, const int xOffset, const int yOffset)
    {
        m_extent = { width, height };
        m_offset = { xOffset, yOffset };
    }

    Scissor::Scissor(const Viewport& viewport)
    {
        FillViewport(viewport);
    }

    Scissor& Scissor::FillViewport(const Viewport& viewport)
    {
        m_offset = IVec2(static_cast<int>(viewport.m_offset.x), static_cast<int>(viewport.m_offset.y));
        m_extent = UVec2(static_cast<uint32>(viewport.m_extent.x), static_cast<uint32>(viewport.m_extent.y));
        return *this;
    }

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Graphics Pipeline: Input Assembly ]
//============================================================================================================================================================================================
    
    VertexStreamDesc& VertexStreamDesc::SetStride(const uint32 stride)
    {
        m_stride = stride;
        return *this;
    }

    VertexStreamDesc& VertexStreamDesc::SetBinding(const uint32 index)
    {
        m_bindingIndex = static_cast<uint16>(index);
        return *this;
    }

    VertexStreamDesc& VertexStreamDesc::SetStepRate(const EVertexStreamStepRate stepRate)
    {
        m_stepRate = stepRate;
        return *this;
    }

    VertexInputDesc& VertexInputDesc::SetAttributes(const vk::ArrayProxy<VertexAttributeDesc>& attributes)
    {
        m_attributes = attributes;
        return *this;
    }

    VertexInputDesc& VertexInputDesc::SetStreams(const vk::ArrayProxy<VertexStreamDesc>& streams)
    {
        m_streams = streams;
        return *this;
    }

    VertexBufferDesc::VertexBufferDesc(const DeviceBuffer* pBuffer, const uint32 stride, const uint64 offset)
        : m_pBuffer(pBuffer)
        , m_offset(offset)
        , m_stride(stride)
    {
        //
    }

    VertexBufferDesc& VertexBufferDesc::SetBuffer(const DeviceBuffer* pBuffer)
    {
        m_pBuffer = pBuffer;
        return *this;
    }

    VertexBufferDesc& VertexBufferDesc::SetOffset(const uint64 offset)
    {
        m_offset = offset;
        return *this;
    }

    VertexBufferDesc& VertexBufferDesc::SetStride(const uint32 stride)
    {
        m_stride = stride;
        return *this;
    }

#pragma endregion
    
//============================================================================================================================================================================================
#pragma region [ Graphics Pipeline: Output Merger ]
//============================================================================================================================================================================================
    
    RenderTargetsDesc& RenderTargetsDesc::SetColorTargets(const vk::ArrayProxy<const Descriptor*>& colors)
    {
        m_colors = colors;
        return *this;
    }

    RenderTargetsDesc& RenderTargetsDesc::SetDepthStencilTargets(const Descriptor* depthStencil)
    {
        m_pDepthStencil = depthStencil;
        return *this;
    }

    ClearDesc& ClearDesc::SetColorValue(const vk::ClearColorValue& color, const uint32 attachmentIndex)
    {
        m_clearValue.color = color;
        m_aspect |= vk::ImageAspectFlagBits::eColor;
        m_colorAttachmentIndex = attachmentIndex;
        return *this;
    }

    ClearDesc& ClearDesc::SetDepthValue(const float depth)
    {
        m_clearValue.depthStencil.depth = depth;
        m_aspect |= vk::ImageAspectFlagBits::eDepth;
        return *this;
    }

    ClearDesc& ClearDesc::SetStencilValue(const uint32 stencil)
    {
        m_clearValue.depthStencil.stencil = stencil;
        m_aspect |= vk::ImageAspectFlagBits::eStencil;
        return *this;
    }

    ClearDesc& ClearDesc::SetDepthStencilValue(const float depth, const uint32 stencil)
    {
        SetDepthValue(depth);
        SetStencilValue(stencil);
        return *this;
    }

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Pipelines ]
//============================================================================================================================================================================================

    GraphicsPipelineDesc& GraphicsPipelineDesc::SetShaderStages(const std::vector<ShaderDesc>& shaderStages)
    {
        m_shaderStages = shaderStages;
        return *this;
    }

    GraphicsPipelineDesc& GraphicsPipelineDesc::SetVertexInput(const VertexInputDesc& vertexInput)
    {
        m_vertexInput = vertexInput;
        return *this;
    }

    GraphicsPipelineDesc& GraphicsPipelineDesc::SetInputAssemblyDesc(const InputAssemblyDesc& desc)
    {
        m_inputAssembly = desc;
        return *this;
    }

    GraphicsPipelineDesc& GraphicsPipelineDesc::SetRasterizationDesc(const RasterizationDesc& desc)
    {
        m_rasterization = desc;
        return *this;
    }

    GraphicsPipelineDesc& GraphicsPipelineDesc::SetMultisampleDesc(const MultisampleDesc& desc)
    {
        m_multisample = desc;
        m_enableMultisample = true;
        return *this;
    }

    GraphicsPipelineDesc& GraphicsPipelineDesc::SetMultisampleEnabled(const bool enabled)
    {
        m_enableMultisample = enabled;
        return *this;
    }

    GraphicsPipelineDesc& GraphicsPipelineDesc::SetOutputMergerDesc(const OutputMergerDesc& desc)
    {
        m_outputMerger = desc;
        return *this;
    }

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Command Signatures ]
//============================================================================================================================================================================================

    DrawDesc::DrawDesc(const uint32 numVertices, const uint32 firstVertex, const uint32 numInstances, const uint32 firstInstance)
        : m_vertexCount(numVertices)
        , m_firstVertex(firstVertex)
        , m_instanceCount(numInstances)
        , m_firstInstance(firstInstance)
    {
        //
    }

    DrawIndexedDesc::DrawIndexedDesc(const uint32 numIndices, const uint32 firstIndex, const uint32 firstVertex, const uint32 numInstances, const uint32 firstInstance)
        : m_firstVertex(firstVertex)
        , m_indexCount(numIndices)
        , m_firstIndex(firstIndex)
        , m_instanceCount(numInstances)
        , m_firstInstance(firstInstance)
    {
        //
    }

#pragma endregion
    
}
