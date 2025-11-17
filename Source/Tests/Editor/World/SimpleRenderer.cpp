// SimpleRenderer.cpp
#include "SimpleRenderer.h"

#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Graphics/Renderer.h"
#include "Nessie/Graphics/Shader.h"
#include "Nessie/Graphics/Texture.h"
#include "Nessie/Asset/AssetManager.h"
#include "Nessie/Graphics/DataUploader.h"

#include "Nessie/World.h"
#include "Nessie/World/Components/CameraComponent.h"
#include "Nessie/World/ComponentSystems/TransformSystem.h"

bool SimpleRenderer::Init()
{
    // Load the Simple Shader
    {
        std::filesystem::path shaderPath = NES_SHADER_DIR;
        shaderPath /= "RectangleShader.yaml";

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
        texturePath /= "Images/StatueTestImage.jpg";

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

    // Create the Render Targets - A single MSAA image.
    {
        nes::RenderTargetDesc desc;
        desc.m_name = "Color Target";
        desc.m_format = nes::Renderer::GetSwapchainFormat();
        desc.m_sampleCount = 1; // =0; Leaving as zero sets it to the maximum.
        desc.m_planes = nes::EImagePlaneBits::Color;
        desc.m_usage = nes::EImageUsageBits::ColorAttachment | nes::EImageUsageBits::ShaderResource;
        desc.m_size.x = swapchainExtent.width;
        desc.m_size.y = swapchainExtent.height;
        desc.m_clearValue = nes::ClearColorValue(0.01f, 0.01f, 0.01f, 1.0f);
        m_colorTarget = nes::RenderTarget(device, desc);
    }
    
    CreateGeometryBuffer(device);
    CreateUniformBuffer(device);
    CreatePipeline(device);
    CreateDescriptorPool(device);
    CreateDescriptorSets(device);

    return true;
}

void SimpleRenderer::Shutdown()
{
    m_colorTarget = nullptr;
    m_imageView = nullptr;
    m_sampler = nullptr;
    m_frames.clear();
    m_geometryBuffer = nullptr;
    m_pipeline = nullptr;
    m_pipelineLayout = nullptr;
}

void SimpleRenderer::RegisterComponentTypes()
{
    NES_REGISTER_COMPONENT(nes::CameraComponent);
    NES_REGISTER_COMPONENT(nes::TransformComponent);
}

void SimpleRenderer::ProcessNewEntities()
{
    // [TODO]: Set the Active Camera
}

void SimpleRenderer::RenderWorldWithCamera(const nes::WorldCamera& worldCamera, nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
{
    UpdateUniformBuffer(worldCamera, context);

    auto& colorImage = m_colorTarget.GetImage();

    // Transition the color image to Color Attachment so that we can render to it.
    {
        nes::ImageBarrierDesc colorBarrier = nes::ImageBarrierDesc()
            .SetImage(&colorImage)
            .SetLayout(nes::EImageLayout::Undefined, nes::EImageLayout::ColorAttachment)
            .SetAccess(nes::EAccessBits::None, nes::EAccessBits::ColorAttachment)
            .SetBarrierStage(nes::EPipelineStageBits::None, nes::EPipelineStageBits::ColorAttachment);
        
        nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
            .SetImageBarriers({ colorBarrier } );
        
        commandBuffer.SetBarriers(barrierGroup);
    }

    // Set the image as our color render target:
    nes::RenderTargetsDesc renderTargetsDesc = nes::RenderTargetsDesc()
        .SetColorTargets(&m_colorTarget.GetView());

    // Get the viewport and scissor that will encompass the entire world render area.
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
}

nes::WorldCamera SimpleRenderer::GetActiveCamera() const
{
    nes::WorldCamera worldCamera{};
    auto* pRegistry = GetEntityRegistry();
    if (!pRegistry)
        return worldCamera;
    
    auto activeCameraEntity = pRegistry->GetEntity(m_activeCameraID);
    if (activeCameraEntity == nes::kInvalidEntityHandle)
    {
        NES_WARN("No Camera in World!");
        return worldCamera;
    }
    
    nes::CameraComponent& camera = pRegistry->GetComponent<nes::CameraComponent>(activeCameraEntity);
    worldCamera.m_camera = camera.m_camera;
    
    nes::TransformComponent& transform = pRegistry->GetComponent<nes::TransformComponent>(activeCameraEntity);
    worldCamera.m_position = transform.GetWorldPosition();
    const auto& worldMatrix = transform.GetWorldTransformMatrix(); 
    worldCamera.m_forward = worldMatrix.GetForward();
    worldCamera.m_up = worldMatrix.GetUp();
    
    return worldCamera;
}

void SimpleRenderer::OnViewportResize(const uint32 width, const uint32 height)
{
    m_colorTarget.Resize(width, height);

    // // Convert the image to the resolve source layout:
    // {
    //     auto commandBuffer = nes::Renderer::BeginTempCommands();
    //
    //     nes::ImageBarrierDesc imageBarrier = nes::ImageBarrierDesc()
    //         .SetImage(&m_colorTarget.GetImage())
    //         .SetLayout(nes::EImageLayout::Undefined, nes::EImageLayout::ColorAttachment)
    //         .SetAccess(nes::EAccessBits::None, nes::EAccessBits::ColorAttachment);
    //
    //     nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
    //         .SetImageBarriers(imageBarrier);
    //     
    //     commandBuffer.SetBarriers(barrierGroup);
    //
    //     nes::Renderer::SubmitAndWaitTempCommands(commandBuffer);
    // }
}

void SimpleRenderer::CreateGeometryBuffer(nes::RenderDevice& device)
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

void SimpleRenderer::CreateUniformBuffer(nes::RenderDevice& device)
{
    // A single constant buffer that different frames will use. The Descriptors will
    // have access to a section of the buffer.
    nes::AllocateBufferDesc desc;
    desc.m_size = sizeof(UniformBufferObject) * nes::Renderer::GetMaxFramesInFlight();
    desc.m_usage = nes::EBufferUsageBits::UniformBuffer;
    desc.m_location = nes::EMemoryLocation::HostUpload; // We are updating the data each frame, so we need to write to it.
    m_uniformBuffer = nes::DeviceBuffer(device, desc);
}

void SimpleRenderer::CreatePipeline(nes::RenderDevice& device)
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
    nes::AssetPtr<nes::Shader> shader = nes::AssetManager::GetAsset<nes::Shader>(m_shaderID);
    NES_ASSERT(shader, "Failed to create Pipeline! Shader not present!");
    auto shaderStages = shader->GetGraphicsShaderStages();

    // Multisample:
    nes::MultisampleDesc multisampleDesc;
    multisampleDesc.m_sampleCount = m_colorTarget.GetSampleCount();

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
        .SetShaderStages(shaderStages)
        .SetVertexInput(vertexInputDesc)
        .SetMultisampleDesc(multisampleDesc)
        .SetRasterizationDesc(rasterDesc)
        .SetOutputMergerDesc(outputMergerDesc);

    NES_ASSERT(m_pipelineLayout != nullptr);
    m_pipeline = nes::Pipeline(device, m_pipelineLayout, pipelineDesc);
}

void SimpleRenderer::CreateDescriptorPool(nes::RenderDevice& device)
{
    // Create a descriptor pool that will only be able to allocate the
    // exact number of constant buffer descriptors that we need (1 per frame).
    const uint32 numDescriptors = static_cast<uint32>(m_frames.size());
    nes::DescriptorPoolDesc poolDesc{};
    poolDesc.m_descriptorSetMaxNum = numDescriptors;
    poolDesc.m_uniformBufferMaxNum = numDescriptors;
    poolDesc.m_samplerMaxNum = 16;
    poolDesc.m_imageMaxNum = 16;
    
    m_descriptorPool = nes::DescriptorPool(device, poolDesc);
}

void SimpleRenderer::CreateDescriptorSets(nes::RenderDevice& device)
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
    nes::Descriptor* pImageView = &m_imageView;

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
    nes::Descriptor* pSamplerView = &m_sampler;
    
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

        nes::Descriptor* pBufferView = &m_frames[i].m_uniformBufferView;
        
        // Allocate the Descriptor Set:
        m_descriptorPool.AllocateDescriptorSets(m_pipelineLayout, 0, &m_frames[i].m_descriptorSet);

        std::array updateDescs =
        {
            nes::DescriptorBindingUpdateDesc(&pBufferView, 1),
            nes::DescriptorBindingUpdateDesc(&pImageView, 1),
            nes::DescriptorBindingUpdateDesc(&pSamplerView, 1),
        };
        
        m_frames[i].m_descriptorSet.UpdateBindings(updateDescs.data(), 0, static_cast<uint32>(updateDescs.size()));
    }
}

void SimpleRenderer::UpdateUniformBuffer(const nes::WorldCamera& camera, const nes::RenderFrameContext& context)
{
    UniformBufferObject ubo;

    // The Rectangle will be at the origin and rotate around the Y-Axis based on time.
    ubo.m_model = nes::Mat44::MakeRotation(nes::Quat::FromAxisAngle(nes::Vec3::AxisY(), nes::math::ToRadians(180.f)));
    ubo.m_view = camera.CalculateViewMatrix();
    
    const auto swapchainExtent = context.GetSwapchainExtent();
    ubo.m_proj = camera.m_camera.CalculateProjectionMatrix(swapchainExtent.width, swapchainExtent.height);

    // Set the updated data:
    m_uniformBuffer.CopyToMappedMemory(&ubo, m_frames[context.GetFrameIndex()].m_uniformBufferViewOffset, sizeof(UniformBufferObject));
}
