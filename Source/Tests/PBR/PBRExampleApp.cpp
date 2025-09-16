// PBRExampleApp.cpp
#include "PBRExampleApp.h"

#include <iostream>

#include "Nessie/Application/EntryPoint.h"
#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Asset/AssetManager.h"
#include "Nessie/Graphics/CommandBuffer.h"
#include "Nessie/Graphics/DataUploader.h"
#include "Nessie/Graphics/RenderDevice.h"
#include "Nessie/Graphics/Renderer.h"
#include "Nessie/Graphics/Shader.h"
#include "Nessie/Graphics/Texture.h"
#include "Nessie/Input/InputManager.h"

bool PBRExampleApp::Internal_AppInit()
{
    if (!LoadScene())
        return false;
    
    // Initial Camera Position:
    m_camera.m_position = nes::Vec3(-10.f, 5.f, -10.f);
    m_camera.m_rotation = nes::Rotation(20.f, 45.f, 0.f);
    m_camera.m_FOVY = nes::math::ToRadians(65.f);

    auto& device = nes::DeviceManager::GetRenderDevice();
    m_frames.resize(nes::Renderer::GetMaxFramesInFlight());
    const auto swapchainExtent = nes::Renderer::GetSwapchainExtent();

    m_depthFormat = device.GetSupportedDepthFormat(24);

    // Create the GBuffer:
    // The first color image will be our msaa image that we render to,
    // and we create a depth buffer as well.
    nes::GBufferDesc gBufferDesc = nes::GBufferDesc()
        .SetColors({ nes::Renderer::GetSwapchainFormat()} )
        .SetDepth(m_depthFormat)
        .SetMaxSampleCount();
    m_gBuffer = nes::GBuffer(gBufferDesc);
    
    CreateBuffersAndImages(device);
    UpdateGBuffer(device, swapchainExtent.width, swapchainExtent.height);
    CreateGridPipeline(device);
    CreateSkyboxPipeline(device);
    CreateGeometryPipeline(device);
    CreateDescriptorPool(device);
    CreateDescriptorSets(device);

    return true;
}

void PBRExampleApp::Internal_AppUpdate(const float timeStep)
{
    UpdateCamera(timeStep);
}

void PBRExampleApp::OnEvent(nes::Event& e)
{
    // When right click is down, allow camera turning.
    if (auto* pMouseButtonEvent = e.Cast<nes::MouseButtonEvent>())
    {
        if (pMouseButtonEvent->GetButton() == nes::EMouseButton::Right)
        {
            if (pMouseButtonEvent->GetAction() == nes::EMouseAction::Pressed)
            {
                m_cameraRotationEnabled = true;
                nes::InputManager::SetCursorMode(nes::ECursorMode::Disabled);
            }
                    
            else if (pMouseButtonEvent->GetAction() == nes::EMouseAction::Released)
            {
                m_cameraRotationEnabled = false;
                nes::InputManager::SetCursorMode(nes::ECursorMode::Visible);
            }
        }
    }
}

void PBRExampleApp::Internal_OnResize(const uint32 width, const uint32 height)
{
    // Create the MSAA image.
    UpdateGBuffer(nes::Renderer::GetDevice(), width, height);
}

void PBRExampleApp::Internal_AppRender(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
{
    // Update our uniform buffer:
    UpdateUniformBuffers(context);

    // We render to this higher sampled image - we will resolve this with the swapchain image at the end of the frame.
    auto& msaaImage = m_gBuffer.GetColorImage();

    // Transition the MSAA image to Color Attachment so that we can render to it,
    // and the Swapchain image to Resolve Destination so that we can resolve our rendered MSAA image to it.
    {
        nes::ImageBarrierDesc msaaBarrier = nes::ImageBarrierDesc()
            .SetImage(&msaaImage)
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
        .SetColorTargets(&m_gBuffer.GetColorImageView())
        .SetDepthStencilTarget(&m_gBuffer.GetDepthImageView());
    
    // Record the Render commands:
    commandBuffer.BeginRendering(renderTargetsDesc);
    {
        // Clear the screen to a dark grey color:
        // Clear the depth to 1.
        const nes::ClearDesc colorClear = nes::ClearDesc::Color(nes::LinearColor(0.01f, 0.01f, 0.01f));
        const nes::ClearDesc depthClear = nes::ClearDesc::DepthStencil(1.f, 0);
        commandBuffer.ClearRenderTargets({ colorClear, depthClear });

        // Set the viewport and scissor to encompass the entire image.
        const nes::Viewport viewport = context.GetSwapchainViewport();
        const nes::Scissor scissor(viewport);
        commandBuffer.SetViewports(viewport);
        commandBuffer.SetScissors(scissor);
        
        RenderSkybox(commandBuffer, context);
        RenderInstances(commandBuffer, context);
        RenderGrid(commandBuffer, context);

        // Finish.
        commandBuffer.EndRendering();
    }

    // Transition the MSAA Image to the Resolve Source layout:
    {
        nes::ImageBarrierDesc imageBarrier = nes::ImageBarrierDesc()
            .SetImage(&msaaImage)
            .SetLayout(nes::EImageLayout::ColorAttachment, nes::EImageLayout::ResolveSource)
            .SetAccess(nes::EAccessBits::ColorAttachment, nes::EAccessBits::ResolveSource);

        nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
            .SetImageBarriers(imageBarrier);
        
        commandBuffer.SetBarriers(barrierGroup);
    }

    // Resolve the Swapchain image from the MSAA image:
    {
        commandBuffer.ResolveImage(msaaImage, *context.GetSwapchainImage());
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

void PBRExampleApp::Internal_AppShutdown()
{
    m_gBuffer = nullptr;
    m_frames.clear();

    m_verticesBuffer = nullptr;
    m_indicesBuffer = nullptr;
    m_instanceBuffer = nullptr;
    
    m_skyboxPipeline = nullptr;
    m_skyboxPipelineLayout = nullptr;
    m_skyboxDescriptorSet = nullptr;
    
    m_gridPipeline = nullptr;
    m_gridPipelineLayout = nullptr;
    
    m_pbrPipeline = nullptr;
    m_pbrPipelineLayout = nullptr;
    
    m_descriptorPool = nullptr;
}

bool PBRExampleApp::LoadScene()
{
    NES_LOG("Loading Scene...");
    std::filesystem::path assetPath{};
    
    // Grid Vert Shader:
    {
        assetPath = NES_SHADER_DIR;
        assetPath /= "Grid.vert.spv";

        const auto result = nes::AssetManager::LoadSync<nes::Shader>(m_gridVertShader, assetPath);
        if (result != nes::ELoadResult::Success)
        {
            NES_ERROR("Failed to load Grid Vert Shader!");
            return false;
        }
    }

    // Grid Frag Shader:
    {
        assetPath = NES_SHADER_DIR;
        assetPath /= "Grid.frag.spv";

        const auto result = nes::AssetManager::LoadSync<nes::Shader>(m_gridFragShader, assetPath);
        if (result != nes::ELoadResult::Success)
        {
            NES_ERROR("Failed to load Grid Frag Shader!");
            return false;
        }
    }

    // Skybox Vert Shader:
    {
        assetPath = NES_SHADER_DIR;
        assetPath /= "Skybox.vert.spv";

        const auto result = nes::AssetManager::LoadSync<nes::Shader>(m_skyboxVertShader, assetPath);
        if (result != nes::ELoadResult::Success)
        {
            NES_ERROR("Failed to load Skybox Vert Shader!");
            return false;
        }
    }

    // Skybox Frag Shader:
    {
        assetPath = NES_SHADER_DIR;
        assetPath /= "Skybox.frag.spv";

        const auto result = nes::AssetManager::LoadSync<nes::Shader>(m_skyboxFragShader, assetPath);
        if (result != nes::ELoadResult::Success)
        {
            NES_ERROR("Failed to load Skybox Frag Shader!");
            return false;
        }
    }

    // PBR Vert Shader:
    {
        assetPath = NES_SHADER_DIR;
        assetPath /= "PBR.vert.spv";

        const auto result = nes::AssetManager::LoadSync<nes::Shader>(m_pbrVertShader, assetPath);
        if (result != nes::ELoadResult::Success)
        {
            NES_ERROR("Failed to load PBR Vert Shader!");
            return false;
        }
    }

    // PBR Frag Shader:
    {
        assetPath = NES_SHADER_DIR;
        assetPath /= "PBR.frag.spv";

        const auto result = nes::AssetManager::LoadSync<nes::Shader>(m_pbrFragShader, assetPath);
        if (result != nes::ELoadResult::Success)
        {
            NES_ERROR("Failed to load PBR Frag Shader!");
            return false;
        }
    }

    // Skybox Texture:
    {
        assetPath = NES_CONTENT_DIR;
        assetPath /= "Skybox\\Skybox.yaml";

        const auto result = nes::AssetManager::LoadSync<nes::TextureCube>(m_skyboxTexture, assetPath);
        if (result != nes::ELoadResult::Success)
        {
            NES_ERROR("Failed to load Grid Frag Shader!");
            return false;
        }
    }

    // Load the Scene Data:
    // Gather vertices and indices:
    m_scene.m_indices.reserve(128);
    m_scene.m_vertices.reserve(128);

    // Cube Mesh:
    Mesh cubeMesh;
    helpers::AppendCubeMeshData(m_scene.m_vertices, m_scene.m_indices, cubeMesh);
    m_scene.m_meshes.emplace_back(cubeMesh);

    // Cube Material:
    PBRMaterialInstance cubeMaterial{};
    cubeMaterial.m_baseColorMapIndex = 0;
    m_scene.m_materials.emplace_back(cubeMaterial);
    
    // Cube Mesh Instance:
    Instance cubeInstance{};
    cubeInstance.m_meshIndex = 0;
    cubeInstance.m_materialIndex = 0;
    // At the origin, uniformScale == 1, no rotation. 
    cubeInstance.m_matRow0 = nes::Vec4(1.0f, 0.0f, 0.0f, 0.0f);
    cubeInstance.m_matRow1 = nes::Vec4(0.0f, 1.0f, 0.0f, 0.0f);
    cubeInstance.m_matRow2 = nes::Vec4(0.0f, 0.0f, 1.0f, 0.0f);
    m_scene.m_instances.emplace_back(cubeInstance);
    //m_scene.m_instances.emplace_back(cubeInstance);
    
    NES_LOG("Scene Loaded.");
    return true;
}

void PBRExampleApp::CreateBuffersAndImages(nes::RenderDevice& device)
{
    // Global Constant Buffer: 
    {
        // A single constant buffer that different frames will use. The Descriptors will
        // have access to a section of the buffer.
        nes::AllocateBufferDesc desc;
        desc.m_size = sizeof(GlobalConstants) * nes::Renderer::GetMaxFramesInFlight();
        desc.m_usage = nes::EBufferUsageBits::UniformBuffer;
        desc.m_location = nes::EMemoryLocation::HostUpload;         // We are updating the data each frame, so we need to write to it.
        m_globalConstantsBuffer = nes::DeviceBuffer(device, desc);
    }

    // Index Device Buffer
    {
        nes::AllocateBufferDesc desc;
        desc.m_size = static_cast<uint32>(m_scene.m_indices.size()) * sizeof(uint32);
        desc.m_usage = nes::EBufferUsageBits::IndexBuffer;
        desc.m_location = nes::EMemoryLocation::Device;
        m_indicesBuffer = nes::DeviceBuffer(device, desc);
    }

    // Vertex Device Buffer
    {
        nes::AllocateBufferDesc desc;
        desc.m_size = static_cast<uint32>(m_scene.m_vertices.size()) * sizeof(Vertex);
        desc.m_usage = nes::EBufferUsageBits::VertexBuffer;
        desc.m_location = nes::EMemoryLocation::Device;
        m_verticesBuffer = nes::DeviceBuffer(device, desc);
    }

    // Transform Device Buffer
    {
        nes::AllocateBufferDesc desc;
        desc.m_size = static_cast<uint32>(m_scene.m_instances.size()) * sizeof(Instance);
        desc.m_usage = nes::EBufferUsageBits::VertexBuffer; // 
        desc.m_location = nes::EMemoryLocation::HostUpload; // We are updating the instance information each frame.
        m_instanceBuffer = nes::DeviceBuffer(device, desc);
    }
    
    // 32x32 White Texture:
    static constexpr size_t kImageSize = 32;
    nes::DeviceImage whiteImage = nullptr;
    nes::Color* pWhiteImageData = nullptr;
    
    {
        // White Image Data:
        // - White in a metallic PBR workflow isn't full 255.
        //   Base Color values range from [30, 240]
        pWhiteImageData = static_cast<nes::Color*>(NES_ALLOC(kImageSize * kImageSize * sizeof(nes::Color)));
        constexpr nes::Color kPhysicalWhite = nes::Color(240, 240, 240); 
        for (size_t i = 0; i < kImageSize * kImageSize; ++i)
        {
            pWhiteImageData[i] = kPhysicalWhite;
        }

        // Create the Device Image:
        nes::ImageDesc imageDesc{};
        imageDesc.m_width = kImageSize;
        imageDesc.m_height = kImageSize;
        imageDesc.m_depth = 1;
        imageDesc.m_usage = nes::EImageUsageBits::ShaderResource;
        imageDesc.m_layerCount = 1;
        imageDesc.m_mipCount = 1;
        imageDesc.m_sampleCount = 1;
        imageDesc.m_sharingMode = nes::ESharingMode::Exclusive;
        imageDesc.m_format = nes::EFormat::RGBA8_UNORM;

        nes::AllocateImageDesc allocDesc{};
        allocDesc.m_desc = imageDesc;
        allocDesc.m_memoryLocation = nes::EMemoryLocation::Device;
        whiteImage = nes::DeviceImage(device, allocDesc);
    }

    // Upload Data
    {
        nes::DataUploader uploader(device);
        nes::CommandBuffer cmdBuffer = nes::Renderer::BeginTempCommands();

        // Indices:
        nes::UploadBufferDesc desc;
        desc.m_pBuffer = &m_indicesBuffer;
        desc.m_pData = m_scene.m_indices.data();
        desc.m_uploadOffset = 0;
        desc.m_uploadSize = m_scene.m_indices.size() * sizeof(uint32);
        uploader.AppendUploadBuffer(desc);

        // Vertices:
        desc.m_pBuffer = &m_verticesBuffer;
        desc.m_uploadOffset = 0;
        desc.m_pData = m_scene.m_vertices.data();
        desc.m_uploadSize = m_scene.m_vertices.size() * sizeof(Vertex);
        uploader.AppendUploadBuffer(desc);

        // Instances:
        desc.m_pBuffer = &m_instanceBuffer;
        desc.m_uploadOffset = 0;
        desc.m_pData = m_scene.m_instances.data();
        desc.m_uploadSize = m_scene.m_instances.size() * sizeof(Instance);
        uploader.AppendUploadBuffer(desc);

        // White Image:
        nes::UploadImageDesc imageUploadDesc{};
        imageUploadDesc.m_pImage = &whiteImage;
        imageUploadDesc.m_layerCount = 1;
        imageUploadDesc.m_planes = nes::EImagePlaneBits::Color;
        imageUploadDesc.m_pSrcData = pWhiteImageData;
        imageUploadDesc.m_newLayout = nes::EImageLayout::ShaderResource;
        uploader.AppendUploadImage(imageUploadDesc);

        // Submit:
        uploader.RecordCommands(cmdBuffer);
        nes::Renderer::SubmitAndWaitTempCommands(cmdBuffer);
    }

    // Add the White Texture to the Asset Manager:
    nes::Texture whiteTexture = nes::Texture(std::move(whiteImage), nes::Buffer(pWhiteImageData, kImageSize * kImageSize * sizeof(nes::Color)));
    nes::AssetManager::AddMemoryAsset(m_whiteTexture, std::move(whiteTexture));
}

void PBRExampleApp::UpdateGBuffer(nes::RenderDevice& device, const uint32 width, const uint32 height)
{
    // Resize the GBuffer.
    m_gBuffer.Resize(device, width, height);

    // Set Debug Names
    {
        m_gBuffer.GetColorImage().SetDebugName("MSAA Image");
        m_gBuffer.GetColorImageView().SetDebugName("MSAA Image View");
        m_gBuffer.GetDepthImage().SetDebugName("Depth/Stencil Image");
        m_gBuffer.GetDepthImageView().SetDebugName("Depth/Stencil Image View");
    }

    // After resize, each image is in the Undefined layout.
    // Convert the msaa image to the resolve source layout:
    {
        auto commandBuffer = nes::Renderer::BeginTempCommands();
        auto& msaaImage = m_gBuffer.GetColorImage();
        auto& depthImage = m_gBuffer.GetDepthImage();
        
        nes::ImageBarrierDesc imageBarrier = nes::ImageBarrierDesc()
            .SetImage(&msaaImage)
            .SetLayout(nes::EImageLayout::Undefined, nes::EImageLayout::ResolveSource)
            .SetAccess(nes::EAccessBits::None, nes::EAccessBits::ResolveSource);

        nes::ImageBarrierDesc depthBarrier = nes::ImageBarrierDesc()
            .SetImage(&depthImage, nes::EImagePlaneBits::Depth | nes::EImagePlaneBits::Stencil)
            .SetLayout(nes::EImageLayout::Undefined, nes::EImageLayout::DepthStencilAttachment);

        nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
            .SetImageBarriers({imageBarrier, depthBarrier});
        
        commandBuffer.SetBarriers(barrierGroup);

        nes::Renderer::SubmitAndWaitTempCommands(commandBuffer);
    }
}

void PBRExampleApp::CreateGridPipeline(nes::RenderDevice& device)
{
    // Pipeline Layout : Just the Camera information:
    {
        // Binding for the Camera data:
        nes::DescriptorBindingDesc bindingDesc = nes::DescriptorBindingDesc()
            .SetBindingIndex(0)
            .SetDescriptorType(nes::EDescriptorType::UniformBuffer)
            .SetShaderStages(nes::EPipelineStageBits::VertexShader | nes::EPipelineStageBits::FragmentShader);

        nes::DescriptorSetDesc descriptorSetDesc = nes::DescriptorSetDesc()
            .SetBindings(&bindingDesc, 1);

        // Add this set to the Pipeline Layout.
        nes::PipelineLayoutDesc layoutDesc = nes::PipelineLayoutDesc()
            .SetDescriptorSets(descriptorSetDesc)
            .SetShaderStages(nes::EPipelineStageBits::VertexShader | nes::EPipelineStageBits::FragmentShader);

        m_gridPipelineLayout = nes::PipelineLayout(device, layoutDesc);
    }
    
    // Shader Stages:
    nes::AssetPtr<nes::Shader> gridVert = nes::AssetManager::GetAsset<nes::Shader>(m_gridVertShader);
    NES_ASSERT(gridVert);

    nes::AssetPtr<nes::Shader> gridFrag = nes::AssetManager::GetAsset<nes::Shader>(m_gridFragShader);
    NES_ASSERT(gridFrag);

    const nes::ShaderDesc vertStage
    {
        .m_stage = nes::EPipelineStageBits::VertexShader,
        .m_pByteCode = gridVert->GetByteCode().data(),
        .m_size = gridVert->GetByteCode().size(),
        .m_entryPointName = "main",
    };

    const nes::ShaderDesc fragStage
    {
        .m_stage = nes::EPipelineStageBits::FragmentShader,
        .m_pByteCode = gridFrag->GetByteCode().data(),
        .m_size = gridFrag->GetByteCode().size(),
        .m_entryPointName = "main",
    };

    // Rasterizer:
    nes::RasterizationDesc rasterDesc = {};
    rasterDesc.m_cullMode = nes::ECullMode::None;
    rasterDesc.m_enableDepthClamp = false;
    rasterDesc.m_fillMode = nes::EFillMode::Solid;
    rasterDesc.m_frontFace = nes::EFrontFaceWinding::CounterClockwise;

    // Use the GBuffer Sample count.
    nes::MultisampleDesc multisampleDesc = {};
    multisampleDesc.m_sampleCount = m_gBuffer.GetSampleCount();

    // Color attachment - to the GBuffer's color attachment.
    nes::ColorAttachmentDesc colorAttachment = {};
    colorAttachment.m_format = m_gBuffer.GetColorFormat();
    colorAttachment.m_enableBlend = true; // Default blending behavior is based on alpha.
    
    // OutputMerger:
    nes::OutputMergerDesc outputMergerDesc = nes::OutputMergerDesc();
    outputMergerDesc.m_colorCount = 1;
    outputMergerDesc.m_pColors = &colorAttachment;
    outputMergerDesc.m_depthStencilFormat = m_gBuffer.GetDepthFormat();
    outputMergerDesc.m_depth.m_compareOp = nes::ECompareOp::Less;
    outputMergerDesc.m_depth.m_enableWrite = false; // Test, but don't write.
    
    // Create the Pipeline:
    nes::GraphicsPipelineDesc pipelineDesc = nes::GraphicsPipelineDesc()
        .SetShaderStages({ vertStage, fragStage })
        .SetRasterizationDesc(rasterDesc)
        .SetOutputMergerDesc(outputMergerDesc)
        .SetMultisampleDesc(multisampleDesc);

    NES_ASSERT(m_gridPipelineLayout != nullptr);
    m_gridPipeline = nes::Pipeline(device, m_gridPipelineLayout, pipelineDesc);
    m_gridPipeline.SetDebugName("Grid Graphics Pipeline");
}

void PBRExampleApp::CreateSkyboxPipeline(nes::RenderDevice& device)
{
    // Pipeline Layout :
    //  Set 0: "Global"
    //      Binding 0: Camera data
    //  Set 1: "Skybox"
    //      Binding 0: Sampler
    //      Binding 1: ImageCube
    {
        // Global:
        nes::DescriptorBindingDesc globalBindings = nes::DescriptorBindingDesc()
            .SetBindingIndex(0)
            .SetDescriptorType(nes::EDescriptorType::UniformBuffer)
            .SetShaderStages(nes::EPipelineStageBits::VertexShader | nes::EPipelineStageBits::FragmentShader);

        nes::DescriptorSetDesc globalSetDesc = nes::DescriptorSetDesc()
            .SetBindings(&globalBindings, 1);
        
        // Skybox:
        std::array skyboxBindings =
        {
            // Linear Sampler
            nes::DescriptorBindingDesc()
                .SetBindingIndex(0)
                .SetDescriptorType(nes::EDescriptorType::Sampler)
                .SetShaderStages(nes::EPipelineStageBits::FragmentShader),
            
            // Skybox Image
            nes::DescriptorBindingDesc()
                .SetBindingIndex(1)
                .SetDescriptorType(nes::EDescriptorType::Image)
                .SetShaderStages(nes::EPipelineStageBits::FragmentShader),
        };
        nes::DescriptorSetDesc skyboxSetDesc = nes::DescriptorSetDesc()
            .SetBindings(skyboxBindings.data(), static_cast<uint32>(skyboxBindings.size()));
        
        // Create the layout:
        nes::PipelineLayoutDesc layoutDesc = nes::PipelineLayoutDesc()
            .SetDescriptorSets({globalSetDesc, skyboxSetDesc})
            .SetShaderStages(nes::EPipelineStageBits::VertexShader | nes::EPipelineStageBits::FragmentShader);

        m_skyboxPipelineLayout = nes::PipelineLayout(device, layoutDesc);
    }
    
    // Shader Stages:
    nes::AssetPtr<nes::Shader> vertShader = nes::AssetManager::GetAsset<nes::Shader>(m_skyboxVertShader);
    NES_ASSERT(vertShader);

    nes::AssetPtr<nes::Shader> fragShader = nes::AssetManager::GetAsset<nes::Shader>(m_skyboxFragShader);
    NES_ASSERT(fragShader);

    const nes::ShaderDesc vertStage
    {
        .m_stage = nes::EPipelineStageBits::VertexShader,
        .m_pByteCode = vertShader->GetByteCode().data(),
        .m_size = vertShader->GetByteCode().size(),
        .m_entryPointName = "main",
    };

    const nes::ShaderDesc fragStage
    {
        .m_stage = nes::EPipelineStageBits::FragmentShader,
        .m_pByteCode = fragShader->GetByteCode().data(),
        .m_size = fragShader->GetByteCode().size(),
        .m_entryPointName = "main",
    };

    // Vertex Input: Vertex object.
    std::array vertexAttributeDescs = Vertex::GetBindingDescs();

    // Per-Vertex Stream.
    nes::VertexStreamDesc vertexStreamDesc = nes::VertexStreamDesc()
        .SetBinding(0)
        .SetStepRate(nes::EVertexStreamStepRate::PerVertex)
        .SetStride(sizeof(Vertex));

    nes::VertexInputDesc vertexInputDesc = nes::VertexInputDesc()
        .SetAttributes(vertexAttributeDescs)
        .SetStreams(vertexStreamDesc);
    
    // Rasterizer:
    nes::RasterizationDesc rasterDesc = {};
    rasterDesc.m_cullMode = nes::ECullMode::None;
    rasterDesc.m_enableDepthClamp = false;
    rasterDesc.m_fillMode = nes::EFillMode::Solid;
    rasterDesc.m_frontFace = nes::EFrontFaceWinding::CounterClockwise;

    // Use the GBuffer Sample count.
    nes::MultisampleDesc multisampleDesc = {};
    multisampleDesc.m_sampleCount = m_gBuffer.GetSampleCount();

    // Color attachment - to the GBuffer's color attachment.
    nes::ColorAttachmentDesc colorAttachment = {};
    colorAttachment.m_format = m_gBuffer.GetColorFormat();
    colorAttachment.m_enableBlend = false; // Default blending behavior is based on alpha.
    
    // OutputMerger:
    nes::OutputMergerDesc outputMergerDesc = nes::OutputMergerDesc();
    outputMergerDesc.m_depthStencilFormat = m_gBuffer.GetDepthFormat();
    outputMergerDesc.m_colorCount = 1;
    outputMergerDesc.m_pColors = &colorAttachment;
    
    // Create the Pipeline:
    nes::GraphicsPipelineDesc pipelineDesc = nes::GraphicsPipelineDesc()
        .SetShaderStages({ vertStage, fragStage })
        .SetVertexInput(vertexInputDesc)
        .SetRasterizationDesc(rasterDesc)
        .SetOutputMergerDesc(outputMergerDesc)
        .SetMultisampleDesc(multisampleDesc);

    NES_ASSERT(m_skyboxPipelineLayout != nullptr);
    m_skyboxPipeline = nes::Pipeline(device, m_skyboxPipelineLayout, pipelineDesc);
    m_skyboxPipeline.SetDebugName("Skybox Graphics Pipeline");
}

void PBRExampleApp::CreateGeometryPipeline(nes::RenderDevice& device)
{
    // Pipeline Layout :
    //  Set 0: "Global"
    //      Binding 0: Camera data
    {
        // Global:
        nes::DescriptorBindingDesc globalBindings = nes::DescriptorBindingDesc()
            .SetBindingIndex(0)
            .SetDescriptorType(nes::EDescriptorType::UniformBuffer)
            .SetShaderStages(nes::EPipelineStageBits::VertexShader | nes::EPipelineStageBits::FragmentShader);

        nes::DescriptorSetDesc globalSetDesc = nes::DescriptorSetDesc()
            .SetBindings(&globalBindings, 1);
        
        // Create the layout:
        nes::PipelineLayoutDesc layoutDesc = nes::PipelineLayoutDesc()
            .SetDescriptorSets({globalSetDesc})
            .SetShaderStages(nes::EPipelineStageBits::VertexShader | nes::EPipelineStageBits::FragmentShader);

        m_pbrPipelineLayout = nes::PipelineLayout(device, layoutDesc);
    }
    
    // Shader Stages:
    nes::AssetPtr<nes::Shader> vertShader = nes::AssetManager::GetAsset<nes::Shader>(m_pbrVertShader);
    NES_ASSERT(vertShader);

    nes::AssetPtr<nes::Shader> fragShader = nes::AssetManager::GetAsset<nes::Shader>(m_pbrFragShader);
    NES_ASSERT(fragShader);

    const nes::ShaderDesc vertStage
    {
        .m_stage = nes::EPipelineStageBits::VertexShader,
        .m_pByteCode = vertShader->GetByteCode().data(),
        .m_size = vertShader->GetByteCode().size(),
        .m_entryPointName = "main",
    };

    const nes::ShaderDesc fragStage
    {
        .m_stage = nes::EPipelineStageBits::FragmentShader,
        .m_pByteCode = fragShader->GetByteCode().data(),
        .m_size = fragShader->GetByteCode().size(),
        .m_entryPointName = "main",
    };

    // Vertex Input: Vertex object.
    std::array vertexAttributeDescs = Vertex::GetBindingDescs();
    std::vector<nes::VertexAttributeDesc> vertexAttributes;
    vertexAttributes.reserve(10);
    vertexAttributes.insert(vertexAttributes.end(), vertexAttributeDescs.begin(), vertexAttributeDescs.end());

    // Vertex Input: Instance data
    vertexAttributes.emplace_back(5, static_cast<uint32>(offsetof(Instance, m_matRow0)), nes::EFormat::RGBA32_SFLOAT, 1);
    vertexAttributes.emplace_back(6, static_cast<uint32>(offsetof(Instance, m_matRow1)), nes::EFormat::RGBA32_SFLOAT, 1);
    vertexAttributes.emplace_back(7, static_cast<uint32>(offsetof(Instance, m_matRow2)), nes::EFormat::RGBA32_SFLOAT, 1);
    vertexAttributes.emplace_back(8, static_cast<uint32>(offsetof(Instance, m_meshIndex)), nes::EFormat::R32_UINT, 1);
    vertexAttributes.emplace_back(9, static_cast<uint32>(offsetof(Instance, m_materialIndex)), nes::EFormat::R32_UINT, 1);
    
    std::array vertexStreamDescs =
    {
        // Per Vertex
        nes::VertexStreamDesc()
            .SetBinding(0)
            .SetStepRate(nes::EVertexStreamStepRate::PerVertex)
            .SetStride(sizeof(Vertex)),

        // Per Instance
        nes::VertexStreamDesc()
            .SetBinding(1)
            .SetStepRate(nes::EVertexStreamStepRate::PerInstance)
            .SetStride(sizeof(Instance))
    };
    
    nes::VertexInputDesc vertexInputDesc = nes::VertexInputDesc()
        .SetAttributes(vertexAttributes)
        .SetStreams(vertexStreamDescs);
    
    // Rasterizer:
    nes::RasterizationDesc rasterDesc = {};
    rasterDesc.m_cullMode = nes::ECullMode::Back;
    rasterDesc.m_enableDepthClamp = false;
    rasterDesc.m_fillMode = nes::EFillMode::Solid;
    rasterDesc.m_frontFace = nes::EFrontFaceWinding::CounterClockwise;

    // Use the GBuffer Sample count.
    nes::MultisampleDesc multisampleDesc = {};
    multisampleDesc.m_sampleCount = m_gBuffer.GetSampleCount();

    // Color attachment - to the GBuffer's color attachment.
    nes::ColorAttachmentDesc colorAttachment = {};
    colorAttachment.m_format = m_gBuffer.GetColorFormat();
    colorAttachment.m_enableBlend = false;
    
    // OutputMerger:
    nes::OutputMergerDesc outputMergerDesc = nes::OutputMergerDesc();
    outputMergerDesc.m_depthStencilFormat = m_gBuffer.GetDepthFormat();
    outputMergerDesc.m_depth.m_compareOp = nes::ECompareOp::Less;
    outputMergerDesc.m_depth.m_enableWrite = true;
    outputMergerDesc.m_colorCount = 1;
    outputMergerDesc.m_pColors = &colorAttachment;
    
    // Create the Pipeline:
    nes::GraphicsPipelineDesc pipelineDesc = nes::GraphicsPipelineDesc()
        .SetShaderStages({ vertStage, fragStage })
        .SetVertexInput(vertexInputDesc)
        .SetRasterizationDesc(rasterDesc)
        .SetOutputMergerDesc(outputMergerDesc)
        .SetMultisampleDesc(multisampleDesc);

    NES_ASSERT(m_pbrPipelineLayout != nullptr);
    m_pbrPipeline = nes::Pipeline(device, m_pbrPipelineLayout, pipelineDesc);
    m_pbrPipeline.SetDebugName("PBR Graphics Pipeline");
}

void PBRExampleApp::CreateDescriptorPool(nes::RenderDevice& device)
{
    // [TODO]: I need:
    // - 5 images per material: Base Color, Normal, Roughness, Metallic and Emission Maps. 
    // - At least 1 Sampler for Mesh Texture Maps.
    // - Transform uniform buffers for each frame.
    // - Scene uniform buffers for each frame.
    // - Descriptor Set for each frame.
    // - Descriptor for Texture Cube Image.
    // - Descriptor Set for each Mesh's PBR material parameters.
    
    //const uint32 numFrames = nes::Renderer::GetMaxFramesInFlight();
    nes::DescriptorPoolDesc poolDesc{};
    poolDesc.m_descriptorSetMaxNum = 64;//numFrames * 2; 
    poolDesc.m_uniformBufferMaxNum = 64;//numFrames * 2;
    poolDesc.m_samplerMaxNum = 64;//numFrames;      
    poolDesc.m_imageMaxNum = 64;                    
    m_descriptorPool = nes::DescriptorPool(device, poolDesc);
}

void PBRExampleApp::CreateDescriptorSets(nes::RenderDevice& device)
{
    // Sampler Descriptor
    {
        nes::SamplerDesc samplerDesc{};
        samplerDesc.m_addressModes = {nes::EAddressMode::Repeat, nes::EAddressMode::Repeat};
        samplerDesc.m_filters = {nes::EFilterType::Linear, nes::EFilterType::Linear, nes::EFilterType::Linear};
        samplerDesc.m_anisotropy = static_cast<uint8>(device.GetDesc().m_other.m_maxSamplerAnisotropy);
        samplerDesc.m_mipMax = 16.f;
        m_sampler = nes::Descriptor(device, samplerDesc);
    }
    
    // Skybox Texture Descriptor
    {
        auto pTexture = nes::AssetManager::GetAsset<nes::TextureCube>(m_skyboxTexture);
        NES_ASSERT(pTexture != nullptr);

        auto& image = pTexture->GetDeviceImage();
        const auto& desc = image.GetDesc();

        // Create the image view descriptor:
        nes::Image2DViewDesc imageViewDesc;
        imageViewDesc.m_pImage = &image;
        imageViewDesc.m_baseLayer = 0;
        imageViewDesc.m_layerCount = desc.m_layerCount;
        imageViewDesc.m_baseMipLevel = 0;
        imageViewDesc.m_mipCount = static_cast<uint16>(desc.m_mipCount);
        imageViewDesc.m_format = desc.m_format;
        imageViewDesc.m_viewType = nes::EImage2DViewType::ShaderResourceCube;
        m_skyboxTextureView = nes::Descriptor(device, imageViewDesc);    
    }
    
    // Global Constant Descriptors:
    nes::BufferViewDesc bufferViewDesc{};
    bufferViewDesc.m_pBuffer = &m_globalConstantsBuffer;
    bufferViewDesc.m_viewType = nes::EBufferViewType::Uniform;
    bufferViewDesc.m_size = sizeof(GlobalConstants);
    
    for (size_t i = 0; i < m_frames.size(); ++i)
    {
        // Create the GlobalConstant Descriptor for this frame:
        bufferViewDesc.m_offset = i * sizeof(GlobalConstants);
        m_frames[i].m_globalConstantBuffer = nes::Descriptor(device, bufferViewDesc);
        m_frames[i].m_globalBufferOffset = bufferViewDesc.m_offset;
        
        // Global Constants Descriptor Set: Camera Data 
        {
            // Global Constant Descriptor Set - Allocated from the grid pipeline layout, which just needs the global constants.
            m_descriptorPool.AllocateDescriptorSets(m_gridPipelineLayout, 0, &m_frames[i].m_globalConstantSet);
        
            std::array updateDescs =
            {
                nes::DescriptorBindingUpdateDesc(&m_frames[i].m_globalConstantBuffer, 1),
            };
            m_frames[i].m_globalConstantSet.UpdateBindings(updateDescs.data(), 0, static_cast<uint32>(updateDescs.size()));
        }
    }
    
    // Skybox Descriptor Set: Sampler and CubeImage.
    {
        m_descriptorPool.AllocateDescriptorSets(m_skyboxPipelineLayout, 1, &m_skyboxDescriptorSet);
    
        std::array updateDescs =
        {
            nes::DescriptorBindingUpdateDesc(&m_sampler, 1),
            nes::DescriptorBindingUpdateDesc(&m_skyboxTextureView, 1),
        };
        m_skyboxDescriptorSet.UpdateBindings(updateDescs.data(), 0, static_cast<uint32>(updateDescs.size()));
    }
}

void PBRExampleApp::UpdateUniformBuffers(const nes::RenderFrameContext& context)
{
    // Update Globals
    {
        // Calculate forward & up vectors.
        const nes::Vec3 forward = m_camera.m_rotation.RotatedVector(nes::Vec3::Forward());
        const nes::Vec3 up = m_camera.m_rotation.RotatedVector(nes::Vec3::Up());
    
        // Set the Camera Constants:
        GlobalConstants constants;
        const auto viewport = context.GetSwapchainViewport();
        constants.m_view = nes::Mat44::LookAt(m_camera.m_position, m_camera.m_position + forward, up);
        constants.m_projection = nes::Mat44::Perspective(m_camera.m_FOVY, viewport.m_extent.x / viewport.m_extent.y, 0.1f, 1000.f);

        // Flip the Y projection.
        constants.m_projection[1][1] *= -1.f;

        // Set the scene uniform data:
        m_globalConstantsBuffer.CopyToMappedMemory(&constants, m_frames[context.GetFrameIndex()].m_globalBufferOffset, sizeof(GlobalConstants));
    }

    // [TODO]: 
    // Update Instance Positions
    // {
    //     for (uint32 i = 0; i < m_scene.m_instances.size(); ++i)
    //     {
    //         
    //     }
    // }
    
}

void PBRExampleApp::RenderSkybox(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
{
    NES_ASSERT(m_skyboxPipeline != nullptr);
    NES_ASSERT(m_skyboxPipelineLayout != nullptr);
    NES_ASSERT(m_frames.size() > context.GetFrameIndex());

    commandBuffer.BindPipelineLayout(m_skyboxPipelineLayout);
    commandBuffer.BindPipeline(m_skyboxPipeline);
    commandBuffer.BindDescriptorSet(0, m_frames[context.GetFrameIndex()].m_globalConstantSet);
    commandBuffer.BindDescriptorSet(1, m_skyboxDescriptorSet);

    const auto cubeMesh = m_scene.m_meshes[0];
    
    // Bind the vertex buffer range for the cube.
    nes::VertexBufferRange vertexBuffer(&m_verticesBuffer, sizeof(Vertex), cubeMesh.m_vertexCount, cubeMesh.m_firstVertex * sizeof(Vertex));
    commandBuffer.BindVertexBuffers(vertexBuffer);

    // Bind the index buffer range for the cube.
    nes::IndexBufferRange indexBuffer = nes::IndexBufferRange(&m_indicesBuffer, cubeMesh.m_indexCount, cubeMesh.m_firstIndex);
    commandBuffer.BindIndexBuffer(indexBuffer);

    nes::DrawIndexedDesc drawDesc{};
    drawDesc.m_firstIndex = indexBuffer.GetFirstIndex();
    drawDesc.m_indexCount = indexBuffer.GetNumIndices();
    drawDesc.m_vertexOffset = vertexBuffer.GetOffset();
    commandBuffer.DrawIndexed(drawDesc);
}

void PBRExampleApp::RenderInstances(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
{
    NES_ASSERT(m_pbrPipeline != nullptr);
    NES_ASSERT(m_pbrPipelineLayout != nullptr);
    NES_ASSERT(m_frames.size() > context.GetFrameIndex());
    
    commandBuffer.BindPipelineLayout(m_pbrPipelineLayout);
    commandBuffer.BindPipeline(m_pbrPipeline);
    commandBuffer.BindDescriptorSet(0, m_frames[context.GetFrameIndex()].m_globalConstantSet);

    // Bind the index buffer for the entire range:
    nes::IndexBufferRange indexBuffer = nes::IndexBufferRange(&m_indicesBuffer, m_scene.m_indices.size(), 0);
    commandBuffer.BindIndexBuffer(indexBuffer);
    
    for (uint32 i = 0; i < static_cast<uint32>(m_scene.m_instances.size()); ++i)
    {
        const auto& instance = m_scene.m_instances[i];
        const Mesh& mesh = m_scene.m_meshes[instance.m_meshIndex];

        // [TODO]: 
        //const PBRMaterialInstance& material = m_scene.m_materials[instance.m_materialIndex];

        // Vertex Buffers - First is the mesh, second is the instance.
        nes::VertexBufferRange meshVertexBuffer(&m_verticesBuffer, sizeof(Vertex), mesh.m_vertexCount, mesh.m_firstVertex * sizeof(Vertex));
        nes::VertexBufferRange instanceVertexBuffer(&m_instanceBuffer, sizeof(Instance), 1, i * sizeof(Instance));
        commandBuffer.BindVertexBuffers({ meshVertexBuffer, instanceVertexBuffer}, 0);

        //auto& vkCommandBuffer = commandBuffer.GetVkCommandBuffer();
        //vkCommandBuffer.bindVertexBuffers(1, instanceVertexBuffer.GetBuffer()->GetVkBuffer(), instanceVertexBuffer.GetOffset());
        //vkCommandBuffer.bindVertexBuffers(0, meshVertexBuffer.GetBuffer()->GetVkBuffer(), meshVertexBuffer.GetOffset());

        nes::DrawIndexedDesc drawDesc{};
        drawDesc.m_firstIndex = mesh.m_firstIndex;
        drawDesc.m_indexCount = mesh.m_indexCount;
        drawDesc.m_vertexOffset = meshVertexBuffer.GetOffset();
        commandBuffer.DrawIndexed(drawDesc);
    }
}

void PBRExampleApp::RenderGrid(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context) const
{
    NES_ASSERT(m_gridPipeline != nullptr);
    NES_ASSERT(m_gridPipelineLayout != nullptr);
    NES_ASSERT(m_frames.size() > context.GetFrameIndex());
    
    commandBuffer.BindPipelineLayout(m_gridPipelineLayout);
    commandBuffer.BindPipeline(m_gridPipeline);
    commandBuffer.BindDescriptorSet(0, m_frames[context.GetFrameIndex()].m_globalConstantSet);
    commandBuffer.DrawVertices(6);
}

void PBRExampleApp::UpdateCamera(const float deltaTime)
{
    ProcessInput();

    // Speed:
    float speed = m_cameraMoveSpeed * deltaTime;
    const bool shift = nes::InputManager::IsKeyDown(nes::EKeyCode::LeftShift) || nes::InputManager::IsKeyDown(nes::EKeyCode::RightShift);
    if (shift)
        speed *= 10.f;

    // Delta Rotation:
    const float heading = m_inputRotation.y * m_cameraSensitivity;
    const float pitch = nes::math::Clamp(m_inputRotation.x * m_cameraSensitivity, -0.49f * nes::math::Pi(), 0.49f * nes::math::Pi());
    const nes::Vec3 deltaPitchYawRoll = nes::Vec3(pitch, heading, 0.f);

    // Delta Movement.
    const nes::Vec3 deltaMovement = m_inputMovement * speed;

    // If there is enough change:
    if (deltaPitchYawRoll.LengthSqr() > 0.f || deltaMovement.LengthSqr() > 0.f)
    {
        // Apply rotation:
        nes::Rotation localRotation = m_camera.m_rotation;
        if (deltaPitchYawRoll.LengthSqr() > 0.f)
        {
            const nes::Rotation deltaRotation(deltaPitchYawRoll);
            localRotation += deltaRotation;
        }

        // Translation:
        nes::Vec3 localPosition = m_camera.m_position;
        localPosition += localRotation.RotatedVector(nes::Vec3(deltaMovement.x, 0.f, deltaMovement.z));
        localPosition.y += deltaMovement.y;

        m_camera.m_position = localPosition;
        m_camera.m_rotation = localRotation;
    }
}

void PBRExampleApp::ProcessInput()
{
    m_inputMovement = nes::Vec3::Zero();
    m_inputRotation = nes::Vec2::Zero();

    // Process Movement:
    if (nes::InputManager::IsKeyDown(nes::EKeyCode::W))
        m_inputMovement.z += 1.f;
            
    if (nes::InputManager::IsKeyDown(nes::EKeyCode::S))
        m_inputMovement.z -= 1.f;
            
    if (nes::InputManager::IsKeyDown(nes::EKeyCode::A))
        m_inputMovement.x -= 1.f;
        
    if (nes::InputManager::IsKeyDown(nes::EKeyCode::D))
        m_inputMovement.x += 1.f;
        
    if (nes::InputManager::IsKeyDown(nes::EKeyCode::Space))
        m_inputMovement.y += 1.f;  
            
    if (nes::InputManager::IsKeyDown(nes::EKeyCode::LeftControl) || nes::InputManager::IsKeyDown(nes::EKeyCode::RightControl))
        m_inputMovement.y -= 1.f;

    // Normalize movement vector
    m_inputMovement.NormalizedOr(nes::Vec3::Zero());

    // Process Rotation:
    if (m_cameraRotationEnabled)
    {
        const nes::Vec2 delta = nes::InputManager::GetCursorDelta();
        m_inputRotation.x = delta.y;
        m_inputRotation.y = delta.x;
        m_inputRotation.Normalize();
    }
}

// void TestCallback(nes::ELogLevel, const char*, const uint32, const char* message, const nes::LogTag&, void*)
// {
//     std::cout << message << "\n";
// }

std::unique_ptr<nes::Application> nes::CreateApplication(ApplicationDesc& outAppDesc, WindowDesc& outWindowDesc, RendererDesc& outRendererDesc)
{
    outAppDesc.SetApplicationName("PBRExampleApp")
        .SetIsHeadless(false);
       
    outWindowDesc.SetResolution(1920, 1080)
        .SetLabel("PBR Example")
        .SetWindowMode(EWindowMode::Windowed)
        .EnableResize(true)
        .EnableVsync(false);

    outRendererDesc.EnableValidationLayer()
        .RequireQueueType(EQueueType::Graphics)
        .RequireQueueType(EQueueType::Transfer);
    
// #if NES_RELEASE
//         outRendererDesc.EnableValidationLayer(true);
//         outRendererDesc.SetDebugMessageCallback(TestCallback);
// #endif
    

    return std::make_unique<PBRExampleApp>(outAppDesc);
}

NES_MAIN()