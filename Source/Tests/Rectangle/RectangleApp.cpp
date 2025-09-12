// Rectangle.cpp
#include "RectangleApp.h"

#include "Nessie/Application/EntryPoint.h"
#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Asset/AssetManager.h"
#include "Nessie/Graphics/CommandBuffer.h"
#include "Nessie/Graphics/DataUploader.h"
#include "Nessie/Graphics/RenderDevice.h"
#include "Nessie/Graphics/Renderer.h"
#include "Nessie/Graphics/Shader.h"
#include "Nessie/Graphics/Texture.h"

bool RectangleApp::Internal_AppInit()
{
    // Load the Simple Shader
    {
        std::filesystem::path shaderPath = NES_SHADER_DIR;
        shaderPath /= "Rectangle.spv";

        const auto result = nes::AssetManager::LoadSync<nes::Shader>(m_shaderID, shaderPath);
        if (result != nes::ELoadResult::Success)
        {
            NES_ERROR("Failed to load Shader!");
            return false;
        }
        NES_LOG("Shader Loaded Successfully!");
    }

    // Load Image
    {
        std::filesystem::path texturePath = NES_CONTENT_DIR;
        texturePath /= "StatueTestImage.jpg";

        const auto result = nes::AssetManager::LoadSync<nes::Texture>(m_textureID, texturePath);
        if (result != nes::ELoadResult::Success)
        {
            NES_ERROR("Failed to load Texture!");
            return false;
        }
        NES_LOG("Texture Loaded Successfully!");
    }

    auto& device = nes::DeviceManager::GetRenderDevice();
    m_frames.resize(nes::Renderer::GetMaxFramesInFlight());
    const auto swapchainExtent = nes::Renderer::GetSwapchainExtent();
    
    CreateGeometryBuffer(device);
    CreateUniformBuffer(device);
    CreateMSAAImage(device, swapchainExtent.width, swapchainExtent.height);
    CreatePipeline(device);
    CreateDescriptorPool(device);
    CreateDescriptorSets(device);

    return true;
}

void RectangleApp::Internal_AppUpdate([[maybe_unused]] const float timeStep)
{
    // nothing to do.
}

void RectangleApp::Internal_OnResize(const uint32 width, const uint32 height)
{
    // Recreate the MSAA image:
    m_msaaImageView = nullptr;
    m_msaaImage = nullptr;

    // Create the MSAA image.
    CreateMSAAImage(nes::Renderer::GetDevice(), width, height);
}

void RectangleApp::Internal_AppRender(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
{
    // Update our uniform buffer:
    UpdateUniformBuffer(context);
    
    // Transition the MSAA image to Color Attachment so that we can render to it,
    // and the Swapchain image to Resolve Destination so that we can resolve our rendered MSAA image to it.
    {
        nes::ImageBarrierDesc msaaBarrier = nes::ImageBarrierDesc()
            .SetImage(&m_msaaImage)
            .SetLayout(nes::EImageLayout::ResolveSource, nes::EImageLayout::ColorAttachment)
            .SetBarrierStage(nes::EPipelineStageBits::None, nes::EPipelineStageBits::ColorAttachment)
            .SetAccess(nes::EAccessBits::ResolveSource, nes::EAccessBits::ColorAttachment);

        nes::ImageBarrierDesc swapchainBarrier = nes::ImageBarrierDesc()
            .SetImage(context.GetSwapchainImage())
            .SetLayout(nes::EImageLayout::Undefined, nes::EImageLayout::ResolveDestination);

        nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
            .SetImageBarriers({ msaaBarrier, swapchainBarrier } );
        
        commandBuffer.SetBarriers(barrierGroup);
    }
    
    // Set the msaa image as our color render target:
    nes::RenderTargetsDesc renderTargetsDesc = nes::RenderTargetsDesc()
        .SetColorTargets(&m_msaaImageView);

    // Get the viewport and scissor that will encompass the entire image.
    const nes::Viewport viewport = context.GetSwapchainViewport();
    const nes::Scissor scissor(viewport);

    // Render a rectangle using the pipeline.
    commandBuffer.BeginRendering(renderTargetsDesc);
    {
        // Clear the screen to a dark grey color:
        nes::ClearDesc clearDesc = nes::ClearDesc::Color(nes::LinearColor(0.01f, 0.01f, 0.01f, 1.0f));
        commandBuffer.ClearRenderTargets(clearDesc);

        // Set our pipeline and render area:
        commandBuffer.BindPipelineLayout(m_pipelineLayout);
        commandBuffer.BindPipeline(m_pipeline);
        commandBuffer.SetViewports(viewport);
        commandBuffer.SetScissors(scissor);

        // Bind the descriptor set that contains our uniform buffer data:
        commandBuffer.BindDescriptorSet(0, m_frames[context.GetFrameIndex()].m_descriptorSet);
        
        // Draw the rectangle:
        commandBuffer.BindIndexBuffer(m_indexBufferDesc);
        commandBuffer.BindVertexBuffers(m_vertexBufferDesc);
        commandBuffer.DrawIndexed(m_indexBufferDesc.GetNumIndices());

        // Finish.
        commandBuffer.EndRendering();
    }

    // Transition the MSAA Image to the Resolve Source layout:
    {
        nes::ImageBarrierDesc imageBarrier = nes::ImageBarrierDesc()
            .SetImage(&m_msaaImage)
            .SetLayout(nes::EImageLayout::ColorAttachment, nes::EImageLayout::ResolveSource)
            .SetAccess(nes::EAccessBits::ColorAttachment, nes::EAccessBits::ResolveSource);

        nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
            .SetImageBarriers(imageBarrier);
        
        commandBuffer.SetBarriers(barrierGroup);
    }

    // Resolve the Swapchain image from the MSAA image:
    {
        commandBuffer.ResolveImage(m_msaaImage, *context.GetSwapchainImage());
    }

    // Transition the Swapchain image to Present layout to present!
    {
        nes::ImageBarrierDesc imageBarrier = nes::ImageBarrierDesc()
            .SetImage(context.GetSwapchainImage())
            .SetLayout(nes::EImageLayout::ResolveDestination, nes::EImageLayout::Present)
            .SetAccess(nes::EAccessBits::ResolveDestination, nes::EAccessBits::None);
    
        nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
            .SetImageBarriers(imageBarrier);
        
        commandBuffer.SetBarriers(barrierGroup);
    }
}

void RectangleApp::Internal_AppShutdown()
{
    m_msaaImage = nullptr;
    m_msaaImageView = nullptr;
    m_imageView = nullptr;
    m_sampler = nullptr;
    m_frames.clear();
    m_geometryBuffer = nullptr;
    m_pipeline = nullptr;
    m_pipelineLayout = nullptr;
}

void RectangleApp::CreateGeometryBuffer(nes::RenderDevice& device)
{
    const std::vector<Vertex> vertices =
    {
        {{-0.5f, 0.5f}, {1.f, 0.f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.f, 0.f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.f, 1.f},{0.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f}, {1.f, 1.f}, {1.0f, 1.0f, 1.0f}}
    };
    const uint64 vertexBufferSize = (sizeof(Vertex) * vertices.size());
        
    const std::vector<uint16> indices =
    {
        0, 1, 2, 2, 3, 0
    };
    const uint64 indexBufferSize = (sizeof(uint16) * indices.size());

    // Allocate the Geometry Buffer:
    // - This single device buffer will contain both the vertices and the indices. The indices are stored after the vertices.
    {
        nes::AllocateBufferDesc desc;
        desc.m_size = vertexBufferSize + indexBufferSize;
        desc.m_location = nes::EMemoryLocation::Device;
        desc.m_usage = nes::EBufferUsageBits::IndexBuffer | nes::EBufferUsageBits::VertexBuffer;
        m_geometryBuffer = nes::DeviceBuffer(device, desc);
            
        m_vertexBufferDesc = nes::VertexBufferRange(&m_geometryBuffer, sizeof(Vertex), vertices.size());
        m_indexBufferDesc = nes::IndexBufferRange(&m_geometryBuffer, indices.size(), 0, nes::EIndexType::U16, vertexBufferSize);
    }

    // Upload the vertex and index data to the buffer.
    {
        nes::CommandBuffer buffer = nes::Renderer::BeginTempCommands();
        nes::DataUploader uploader(device);
        
        // Vertex Buffer data
        nes::UploadBufferDesc vertexUpload;
        vertexUpload.m_pBuffer = &m_geometryBuffer;
        vertexUpload.m_pData = vertices.data();
        vertexUpload.m_uploadOffset = 0;
        vertexUpload.m_uploadSize = vertexBufferSize;
        uploader.AppendUploadBuffer(vertexUpload);

        // Index Buffer data
        nes::UploadBufferDesc indexUpload;
        indexUpload.m_pBuffer = &m_geometryBuffer;
        indexUpload.m_pData = indices.data();
        indexUpload.m_uploadOffset = vertexBufferSize;
        indexUpload.m_uploadSize = indexBufferSize;
        uploader.AppendUploadBuffer(indexUpload);
            
        uploader.RecordCommands(buffer);
        nes::Renderer::SubmitAndWaitTempCommands(buffer);

        // Release staging buffer resources.
        uploader.Destroy();
    }
}

void RectangleApp::CreateUniformBuffer(nes::RenderDevice& device)
{
    // A single constant buffer that different frames will use. The Descriptors will
    // have access to a section of the buffer.
    nes::AllocateBufferDesc desc;
    desc.m_size = sizeof(UniformBufferObject) * nes::Renderer::GetMaxFramesInFlight();
    desc.m_usage = nes::EBufferUsageBits::UniformBuffer;
    desc.m_location = nes::EMemoryLocation::HostUpload; // We are updating the data each frame, so we need to write to it.
    m_uniformBuffer = nes::DeviceBuffer(device, desc);
}

void RectangleApp::CreateMSAAImage(nes::RenderDevice& device, const uint32 width, const uint32 height)
{
    // Get the format of the swapchain, and the number of samples that we can use for it.
    nes::EFormat swapchainFormat = nes::Renderer::GetSwapchainFormat();
    nes::EFormatFeatureBits features = device.GetFormatFeatures(swapchainFormat);
    const uint32 samples = nes::GetMaxSampleCount(features);

    // Create the image desc:
    nes::ImageDesc imageDesc{};
    imageDesc.m_mipCount = 1;
    imageDesc.m_format = swapchainFormat;
    imageDesc.m_layerCount = 1;
    imageDesc.m_sampleCount = samples;
    imageDesc.m_type = nes::EImageType::Image2D;
    imageDesc.m_usage = nes::EImageUsageBits::ColorAttachment;
    imageDesc.m_width = width;
    imageDesc.m_height = height;
    imageDesc.m_depth = 1;

    // Allocate the image
    nes::AllocateImageDesc allocDesc;
    allocDesc.m_desc = imageDesc;
    allocDesc.m_memoryLocation = nes::EMemoryLocation::Device;
    m_msaaImage = nes::DeviceImage(device, allocDesc);

    // Create the image descriptor (image view):
    nes::Image2DViewDesc imageViewDesc{};
    imageViewDesc.m_format = swapchainFormat;
    imageViewDesc.m_pImage = &m_msaaImage;
    imageViewDesc.m_viewType = nes::EImage2DViewType::ColorAttachment;
    m_msaaImageView = nes::Descriptor(device, imageViewDesc);

    // Convert the msaa image to the resolve source layout:
    {
        auto commandBuffer = nes::Renderer::BeginTempCommands();

        nes::ImageBarrierDesc imageBarrier = nes::ImageBarrierDesc()
            .SetImage(&m_msaaImage)
            .SetLayout(nes::EImageLayout::Undefined, nes::EImageLayout::ResolveSource)
            .SetAccess(nes::EAccessBits::None, nes::EAccessBits::ResolveSource);

        nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
            .SetImageBarriers(imageBarrier);
        
        commandBuffer.SetBarriers(barrierGroup);

        nes::Renderer::SubmitAndWaitTempCommands(commandBuffer);
    }
}

void RectangleApp::CreatePipeline(nes::RenderDevice& device)
{
    // Create the Pipeline Layout:
    {
        std::array bindings = 
        {
            // Binding for the UBO object.
            nes::DescriptorBindingDesc()
                .SetBindingIndex(0)
                .SetDescriptorType(nes::EDescriptorType::UniformBuffer)
                .SetShaderStages(nes::EPipelineStageBits::VertexShader),

            // Image Resource
            nes::DescriptorBindingDesc()
                .SetBindingIndex(1)
                .SetDescriptorType(nes::EDescriptorType::Image)
                .SetShaderStages(nes::EPipelineStageBits::FragmentShader),

            // Sampler Resource
            nes::DescriptorBindingDesc()
                .SetBindingIndex(2)
                .SetDescriptorType(nes::EDescriptorType::Sampler)
                .SetShaderStages(nes::EPipelineStageBits::FragmentShader)
        };
        
        nes::DescriptorSetDesc descriptorSetDesc = nes::DescriptorSetDesc()
            .SetBindings(bindings.data(), static_cast<uint32>(bindings.size()));
        
        // Add this set to the Pipeline Layout.
        nes::PipelineLayoutDesc layoutDesc = nes::PipelineLayoutDesc()
            .SetDescriptorSets(descriptorSetDesc)
            .SetShaderStages(nes::EPipelineStageBits::VertexShader);
            
        m_pipelineLayout = nes::PipelineLayout(device, layoutDesc);
    }
    
    // Attributes of the Vertex struct
    std::array<nes::VertexAttributeDesc, 3> attributes =
    {
        nes::VertexAttributeDesc(0, offsetof(Vertex, m_position), nes::EFormat::RG32_SFLOAT, 0),
        nes::VertexAttributeDesc(1, offsetof(Vertex, m_texCoord), nes::EFormat::RG32_SFLOAT, 0),
        nes::VertexAttributeDesc(2, offsetof(Vertex, m_color), nes::EFormat::RGB32_SFLOAT, 0),
    };

    // A single stream of Vertex elements:
    nes::VertexStreamDesc vertexStreamDesc = nes::VertexStreamDesc()
        .SetStride(sizeof(Vertex));

    nes::VertexInputDesc vertexInputDesc = nes::VertexInputDesc()
        .SetAttributes(attributes)
        .SetStreams(vertexStreamDesc);

    // Shader Stages:
    nes::AssetPtr<nes::Shader> triangleShader = nes::AssetManager::GetAsset<nes::Shader>(m_shaderID);
    NES_ASSERT(triangleShader, "Failed to create Pipeline! Shader not present!");

    const auto& byteCode = triangleShader->GetByteCode();
    nes::ShaderDesc vertStage
    {
        .m_stage = nes::EPipelineStageBits::VertexShader,
        .m_pByteCode = byteCode.data(),
        .m_size = byteCode.size(),
        .m_entryPointName = "vertMain",
    };
    nes::ShaderDesc fragStage
    {
        .m_stage = nes::EPipelineStageBits::FragmentShader,
        .m_pByteCode = byteCode.data(),
        .m_size = byteCode.size(),
        .m_entryPointName = "fragMain",
    };

    // Get the maximum samples for the swapchain format:
    nes::EFormat swapchainFormat = nes::Renderer::GetSwapchainFormat();
    nes::EFormatFeatureBits features = device.GetFormatFeatures(swapchainFormat);
    const uint32 maxSamples = nes::GetMaxSampleCount(features);

    // Multisample:
    nes::MultisampleDesc multisampleDesc;
    multisampleDesc.m_sampleCount = maxSamples;

    // Rasterizer:
    nes::RasterizationDesc rasterDesc = {};
    rasterDesc.m_cullMode = nes::ECullMode::Back;
    rasterDesc.m_enableDepthClamp = false;
    rasterDesc.m_fillMode = nes::EFillMode::Solid;
    rasterDesc.m_frontFace = nes::EFrontFaceWinding::CounterClockwise;

    // Color attachment:
    nes::ColorAttachmentDesc colorAttachment = {};
    colorAttachment.m_format = nes::Renderer::GetSwapchainFormat();
    colorAttachment.m_enableBlend = false;
    
    // OutputMerger:
    nes::OutputMergerDesc outputMergerDesc = nes::OutputMergerDesc();
    outputMergerDesc.m_colorCount = 1;
    outputMergerDesc.m_pColors = &colorAttachment;
    
    // Create the Pipeline:
    nes::GraphicsPipelineDesc pipelineDesc = nes::GraphicsPipelineDesc()
        .SetShaderStages({ vertStage, fragStage })
        .SetVertexInput(vertexInputDesc)
        .SetMultisampleDesc(multisampleDesc)
        .SetRasterizationDesc(rasterDesc)
        .SetOutputMergerDesc(outputMergerDesc);

    NES_ASSERT(m_pipelineLayout != nullptr);
    m_pipeline = nes::Pipeline(device, m_pipelineLayout, pipelineDesc);
}

void RectangleApp::CreateDescriptorPool(nes::RenderDevice& device)
{
    // Create a descriptor pool that will only be able to allocate the
    // exact number of constant buffer descriptors that we need (1 per frame).
    const uint32 numDescriptors = static_cast<uint32>(m_frames.size());
    nes::DescriptorPoolDesc poolDesc{};
    poolDesc.m_descriptorSetMaxNum = numDescriptors;
    poolDesc.m_uniformBufferMaxNum = numDescriptors;
    poolDesc.m_samplerMaxNum = 1;
    poolDesc.m_imageMaxNum = 1;
    
    m_descriptorPool = nes::DescriptorPool(device, poolDesc);
}

void RectangleApp::CreateDescriptorSets(nes::RenderDevice& device)
{
    auto pTexture = nes::AssetManager::GetAsset<nes::Texture>(m_textureID);
    NES_ASSERT(pTexture != nullptr);

    auto& image = pTexture->GetDeviceImage();
    const auto& desc = image.GetDesc();

    // Create the image view descriptor:
    nes::Image2DViewDesc imageViewDesc;
    imageViewDesc.m_pImage = &image;
    imageViewDesc.m_baseLayer = 0;
    imageViewDesc.m_layerCount = 1;
    imageViewDesc.m_baseMipLevel = 0;
    imageViewDesc.m_mipCount = static_cast<uint16>(desc.m_mipCount);
    imageViewDesc.m_format = desc.m_format;
    imageViewDesc.m_viewType = nes::EImage2DViewType::ShaderResource2D;
    m_imageView = nes::Descriptor(device, imageViewDesc);

    // Create the Sampler descriptor:
    nes::SamplerDesc samplerDesc{};
    samplerDesc.m_filters.m_mag = nes::EFilterType::Linear;
    samplerDesc.m_filters.m_min = nes::EFilterType::Linear;
    samplerDesc.m_filters.m_mip = nes::EFilterType::Linear;
    samplerDesc.m_addressModes.u = nes::EAddressMode::Repeat;
    samplerDesc.m_addressModes.v = nes::EAddressMode::Repeat;
    samplerDesc.m_addressModes.w = nes::EAddressMode::Repeat;
    samplerDesc.m_mipBias = 0.f;
    samplerDesc.m_borderColor = nes::ClearColorValue(0.f, 0.f, 0.f);
    samplerDesc.m_compareOp = nes::ECompareOp::None;
    samplerDesc.m_anisotropy = static_cast<uint8>(device.GetDesc().m_other.m_maxSamplerAnisotropy);
    m_sampler = nes::Descriptor(device, samplerDesc);
    
    // View into the single uniform buffer:
    nes::BufferViewDesc bufferViewDesc{};
    bufferViewDesc.m_pBuffer = &m_uniformBuffer;
    bufferViewDesc.m_viewType = nes::EBufferViewType::Uniform;
    bufferViewDesc.m_size = sizeof(UniformBufferObject);
    
    // Create the Buffer View Descriptors:
    for (size_t i = 0; i < m_frames.size(); ++i)
    {
        // Set the offset for this view:
        bufferViewDesc.m_offset = i * sizeof(UniformBufferObject);
        m_frames[i].m_uniformBufferView = nes::Descriptor(device, bufferViewDesc);
        m_frames[i].m_uniformBufferViewOffset = bufferViewDesc.m_offset;

        // Allocate the Descriptor Set:
        m_descriptorPool.AllocateDescriptorSets(m_pipelineLayout, 0, &m_frames[i].m_descriptorSet);

        std::array updateDescs =
        {
            nes::DescriptorBindingUpdateDesc(&m_frames[i].m_uniformBufferView, 1),
            nes::DescriptorBindingUpdateDesc(&m_imageView, 1),
            nes::DescriptorBindingUpdateDesc(&m_sampler, 1),
        };
        
        m_frames[i].m_descriptorSet.UpdateBindings(updateDescs.data(), 0, static_cast<uint32>(updateDescs.size()));
    }
}

void RectangleApp::UpdateUniformBuffer(const nes::RenderFrameContext& context)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    const float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo;

    // The Rectangle will be at the origin and rotate around the Z-Axis based on time.
    ubo.m_model = nes::Mat44::MakeRotation(nes::Quat::FromAxisAngle(nes::Vec3::AxisZ(), time * nes::math::ToRadians(90.f)));

    // Look at the geometry from above at a 45-degree angle:
    const auto viewport = context.GetSwapchainViewport();
    ubo.m_view = nes::Mat44::LookAt(nes::Vec3(2.f, 2.f, 2.f), nes::Vec3::Zero(), nes::Vec3::Forward());
    ubo.m_proj = nes::Mat44::Perspective(nes::math::ToRadians(45.f), viewport.m_extent.x / viewport.m_extent.y, 0.1f, 10.f);

    // Flip the Y coordinate.
    ubo.m_proj[1][1] *= -1.f;

    // Set the new data:
    m_uniformBuffer.CopyToMappedMemory(&ubo, m_frames[context.GetFrameIndex()].m_uniformBufferViewOffset, sizeof(UniformBufferObject));
}

std::unique_ptr<nes::Application> nes::CreateApplication(ApplicationDesc& outAppDesc, WindowDesc& outWindowDesc, RendererDesc& outRendererDesc)
{
    outAppDesc.SetApplicationName("Rectangle")
        .SetIsHeadless(false);
       
    outWindowDesc.SetResolution(720, 720)
        .SetLabel("Rectangle")
        .SetWindowMode(EWindowMode::Windowed)
        .EnableResize(true)
        .EnableVsync(false);

    outRendererDesc.EnableValidationLayer()
        .RequireQueueType(EQueueType::Graphics)
        .RequireQueueType(EQueueType::Transfer);

    return std::make_unique<RectangleApp>(outAppDesc);
}

NES_MAIN()