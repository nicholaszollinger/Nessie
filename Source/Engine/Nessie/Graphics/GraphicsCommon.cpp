// GraphicsCommon.cpp
#include "GraphicsCommon.h"
#include "DeviceBuffer.h"

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
#pragma region [ Pipline Stages and Barriers ]
//============================================================================================================================================================================================

    ImageBarrierDesc& ImageBarrierDesc::SetImage(DeviceImage* pImage)
    {
        NES_ASSERT(pImage != nullptr);
        m_pImage = pImage;
        return *this;
    }

    ImageBarrierDesc& ImageBarrierDesc::SetLayout(const EImageLayout before, const EImageLayout after)
    {
        m_before.m_layout = before;
        m_after.m_layout = after;
        return *this;
    }

    ImageBarrierDesc& ImageBarrierDesc::SetBarrierStage(const EPipelineStageBits before, const EPipelineStageBits after)
    {
        m_before.m_stages = before;
        m_after.m_stages = after;
        return *this;
    }

    ImageBarrierDesc& ImageBarrierDesc::SetAccess(const EAccessBits before, const EAccessBits after)
    {
        m_before.m_access = before;
        m_after.m_access = after;
        return *this;
    }

    ImageBarrierDesc& ImageBarrierDesc::SetRegion(const EImagePlaneBits planes, const uint32 baseMip, const uint32 numMips, const uint32 baseLayer, const uint32 numLayers)
    {
        m_planes = planes;
        m_baseMip = baseMip;
        m_baseLayer = baseLayer;
        m_mipCount = numMips;
        m_layerCount = numLayers;
        return *this;
    }

    ImageBarrierDesc& ImageBarrierDesc::SetQueueAccess(DeviceQueue* pSrcQueue, DeviceQueue* pDstQueue)
    {
        NES_ASSERT(pSrcQueue != nullptr);
        NES_ASSERT(pDstQueue != nullptr);
        
        m_pSrcQueue = pSrcQueue;
        m_pDstQueue = pDstQueue;
        return *this;
    }

    BarrierGroupDesc& BarrierGroupDesc::SetImageBarriers(const vk::ArrayProxy<ImageBarrierDesc>& barriers)
    {
        m_imageBarriers.insert(m_imageBarriers.begin(), barriers.begin(), barriers.end());
        return *this;
    }

#pragma endregion
    
//============================================================================================================================================================================================
#pragma region [ Resources: creation ]
//============================================================================================================================================================================================

    void ImageDesc::Validate()
    {
        m_height = math::Max(m_height, 1u);
        m_depth = math::Max(m_depth, 1u);
        m_mipCount = math::Max(m_mipCount, 1u);
        m_layerCount = math::Max(m_layerCount, 1u);
        m_sampleCount = math::Max(m_sampleCount, 1u);
    }

    ImageRegionDesc& ImageRegionDesc::SetOffset(const uint32 x, const uint32 y, const uint32 z)
    {
        m_offset.x = x;
        m_offset.y = y;
        m_offset.z = z;
        return *this;
    }

    ImageRegionDesc& ImageRegionDesc::SetSize(const uint32 width, const uint32 height, const uint32 depth)
    {
        m_width = math::Max(width, 1u);
        m_height = math::Max(height, 1u);
        m_depth = math::Max(depth, 1u);
        return *this;
    }

    ImageRegionDesc& ImageRegionDesc::SetMipLevel(const uint32 mipLevel)
    {
        m_mipLevel = mipLevel;
        return *this;
    }

    ImageRegionDesc& ImageRegionDesc::SetLayer(const uint32 layer)
    {
        m_layer = layer;
        return *this;
    }

    ImageRegionDesc& ImageRegionDesc::SetImagePlanes(const EImagePlaneBits planes)
    {
        m_planes = planes;
        return *this;
    }

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Resources: binding to memory ]
//============================================================================================================================================================================================
    
    // AllocateBufferDesc AllocateBufferDesc::VertexBuffer(const uint64 vertexCount, const uint32 vertexSize)
    // {
    //     AllocateBufferDesc desc;
    //     desc.m_size = vertexCount * vertexSize;
    //     desc.m_usage = EBufferUsageBits::VertexBuffer;
    //     desc.m_location = EMemoryLocation::Device;
    //     return desc;
    // }
    //
    // AllocateBufferDesc AllocateBufferDesc::IndexBuffer(const uint64 indexCount, const EIndexType type)
    // {
    //     AllocateBufferDesc desc;
    //     desc.m_size = indexCount * (type == EIndexType::U32? sizeof(uint32) : sizeof(uint16));
    //     desc.m_usage = EBufferUsageBits::IndexBuffer;
    //     desc.m_location = EMemoryLocation::Device;
    //     return desc;
    // }

#pragma endregion

//============================================================================================================================================================================================
#pragma region [ Pipeline Layout and Descriptors Management ]
//============================================================================================================================================================================================

    DescriptorBindingDesc& DescriptorBindingDesc::SetBindingIndex(const uint32 index)
    {
        m_bindingIndex = index;
        return *this;
    }

    DescriptorBindingDesc& DescriptorBindingDesc::SetDescriptorType(const EDescriptorType type, const uint32 count)
    {
        m_descriptorType = type;
        m_descriptorCount = count;
        return *this;
    }

    DescriptorBindingDesc& DescriptorBindingDesc::SetShaderStages(const EPipelineStageBits stages)
    {
        // [TODO]: Validate that they are valid shader stages?
        // - When I convert to the vulkan value, they are validated. Perhaps have a debug warning here. 
        m_shaderStages = stages;
        return *this;
    }

    DescriptorBindingDesc& DescriptorBindingDesc::SetFlags(const EDescriptorBindingBits flags)
    {
        m_flags = flags;
        return *this;
    }

    DescriptorSetDesc& DescriptorSetDesc::SetBindings(const DescriptorBindingDesc* pBindings, const uint32 count)
    {
        m_pBindings = pBindings;
        m_numBindings = count;
        return *this;
    }

    PipelineLayoutDesc& PipelineLayoutDesc::SetDescriptorSets(const vk::ArrayProxy<DescriptorSetDesc>& sets)
    {
        m_descriptorSets = sets;
        return *this;
    }

    PipelineLayoutDesc& PipelineLayoutDesc::SetPushConstants(const vk::ArrayProxy<PushConstantDesc>& pushConstants)
    {
        m_pushConstants = pushConstants;
        return *this;
    }

    PipelineLayoutDesc& PipelineLayoutDesc::SetShaderStages(const EPipelineStageBits stages)
    {
        m_shaderStages = stages;
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

    DeviceBufferRange::DeviceBufferRange(DeviceBuffer* pBuffer, const uint64 offset, const uint64 size)
        : m_pBuffer(pBuffer)
        , m_offset(offset)
        , m_size(size)
    {
        NES_ASSERT(pBuffer != nullptr);

        // Use the remaining size of the buffer from the offset.
        if (size == graphics::kWholeSize)
        {
            m_size = pBuffer->GetSize() - offset;
        }
    }
    
    uint8* DeviceBufferRange::GetMappedMemory() const
    {
        NES_ASSERT(m_pBuffer != nullptr);
        
        if (!m_pBuffer->m_pMappedMemory)
            return nullptr;
        
        return m_pBuffer->m_pMappedMemory + m_offset;
    }

    uint64 DeviceBufferRange::GetDeviceAddress() const
    {
        NES_ASSERT(m_pBuffer != nullptr);
        return m_pBuffer->m_deviceAddress;
    }

    VertexBufferRange::VertexBufferRange(DeviceBuffer* pBuffer, const uint64 stride, const uint64 vertexCount, const uint64 bufferOffset)
        : DeviceBufferRange(pBuffer, bufferOffset, stride * vertexCount)
        , m_stride(static_cast<uint32>(stride))
        , m_vertexCount(static_cast<uint32>(vertexCount))
    {
        NES_ASSERT(stride > 0);
    }

    IndexBufferRange::IndexBufferRange(DeviceBuffer* pBuffer, const uint64 indexCount, const EIndexType type, const uint64 bufferOffset)
        : DeviceBufferRange(pBuffer, bufferOffset, indexCount * (type == EIndexType::U16 ? sizeof(uint16) : sizeof(uint32)))
        , m_indexCount(static_cast<uint32>(indexCount))
        , m_indexType(type)
    {
        //
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

    ClearDesc ClearDesc::Color(const LinearColor color, const uint32 attachmentIndex)
    {
        ClearDesc result;
        result.m_clearValue = ClearColorValue(color.r, color.g, color.b, color.a);
        result.m_colorAttachmentIndex = attachmentIndex;
        result.m_planes = EImagePlaneBits::Color;
        return result;
    }

    ClearDesc ClearDesc::Depth(const float depth)
    {
        ClearDesc result{};
        result.m_clearValue = ClearDepthStencilValue(depth);
        result.m_planes = EImagePlaneBits::Depth;
        return result;
    }

    ClearDesc ClearDesc::Stencil(const uint32 stencil)
    {
        ClearDesc result{};
        result.m_clearValue = ClearDepthStencilValue(1.f, stencil);
        result.m_planes = EImagePlaneBits::Stencil;
        return result;
    }

    ClearDesc ClearDesc::DepthStencil(const float depth, const uint32 stencil)
    {
        ClearDesc result{};
        result.m_clearValue = ClearDepthStencilValue(depth, stencil);
        result.m_planes = EImagePlaneBits::Depth | EImagePlaneBits::Stencil;
        return result;
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
