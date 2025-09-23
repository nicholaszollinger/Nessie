// PBRExampleApp.cpp
#include "PBRExampleApp.h"

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
    auto& device = nes::DeviceManager::GetRenderDevice();
    
    // Load the Scene, and all assets.
    {
        NES_LOG("Loading Scene...");

        helpers::SceneConfig config{};
        
        std::filesystem::path path = NES_CONTENT_DIR;
        path /= "Scenes\\PBRTestScene.yaml";
        if (!helpers::LoadScene(path, device, m_scene, m_loadedAssets, config))
            return false;

        m_skyboxTexture = config.m_skyboxTextureID;
        m_skyboxShader = config.m_skyboxShaderID;
        m_pbrShader = config.m_pbrShaderID;
        m_gridShader = config.m_gridShaderID;
        
        NES_LOG("Scene Loaded.");
    }
    
    // Initial Camera Position:
    m_camera.m_position = nes::Vec3(-10.f, 5.f, -10.f);
    m_camera.m_rotation = nes::Rotation(20.f, 45.f, 0.f);
    m_camera.m_FOVY = nes::math::ToRadians(65.f);
    
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
    
    m_skyboxPipeline = nullptr;
    m_skyboxPipelineLayout = nullptr;
    m_skyboxDescriptorSet = nullptr;
    
    m_gridPipeline = nullptr;
    m_gridPipelineLayout = nullptr;
    
    m_pbrPipeline = nullptr;
    m_pbrPipelineLayout = nullptr;

    m_materialDescriptorSets.clear();
    
    m_descriptorPool = nullptr;
}

void PBRExampleApp::CreateBuffersAndImages(nes::RenderDevice& device)
{
    // Globals Buffer: Contains CameraUBO + LightCountUBO. 
    {
        nes::AllocateBufferDesc desc;
        desc.m_size = static_cast<uint64>(kGlobalUBOElementSize) * nes::Renderer::GetMaxFramesInFlight();
        desc.m_usage = nes::EBufferUsageBits::UniformBuffer;
        desc.m_location = nes::EMemoryLocation::HostUpload;         // We are updating the data each frame, so we need to write to it.
        m_globalsBuffer = nes::DeviceBuffer(device, desc);
        m_globalsBuffer.SetDebugName("GlobalUBO Buffer");
    }

    // Index Device Buffer
    {
        nes::AllocateBufferDesc desc;
        desc.m_size = static_cast<uint32>(m_scene.m_indices.size()) * sizeof(uint32);
        desc.m_usage = nes::EBufferUsageBits::IndexBuffer;
        desc.m_location = nes::EMemoryLocation::Device;
        m_indicesBuffer = nes::DeviceBuffer(device, desc);
        m_indicesBuffer.SetDebugName("Indices Buffer");
    }

    // Vertex Device Buffer
    {
        nes::AllocateBufferDesc desc;
        desc.m_size = static_cast<uint32>(m_scene.m_vertices.size()) * sizeof(Vertex);
        desc.m_usage = nes::EBufferUsageBits::VertexBuffer;
        desc.m_location = nes::EMemoryLocation::Device;
        m_verticesBuffer = nes::DeviceBuffer(device, desc);
        m_verticesBuffer.SetDebugName("Vertices Buffer");
    }

    // Material Storage Buffer per frame.
    {
        for (uint32 i = 0; i < nes::Renderer::GetMaxFramesInFlight(); ++i)
        {
            auto& frame = m_frames[i];
            
            nes::AllocateBufferDesc desc;
            desc.m_size = static_cast<uint32>(m_scene.m_materials.size()) * sizeof(MaterialUBO);
            desc.m_usage = nes::EBufferUsageBits::ShaderResourceStorage; 
            desc.m_location = nes::EMemoryLocation::HostUpload;
            desc.m_structureStride = sizeof(MaterialUBO);
            frame.m_materialUBOBuffer = nes::DeviceBuffer(device, desc);
            frame.m_materialUBOBuffer.SetDebugName(fmt::format("Materials SSBO({})", i));
        }
    }
    
    // Light Storage Buffers for each Frame:
    {
        for (uint32 i = 0; i < nes::Renderer::GetMaxFramesInFlight(); ++i)
        {
            auto& frame = m_frames[i];

            // Directional Lights:
            nes::AllocateBufferDesc desc;
            desc.m_size = sizeof(DirectionalLight) * LightCountUBO::kMaxDirectionalLights;
            desc.m_usage = nes::EBufferUsageBits::ShaderResourceStorage;
            desc.m_location = nes::EMemoryLocation::HostUpload;
            desc.m_structureStride = sizeof(DirectionalLight);
            frame.m_directionalLightsBuffer = nes::DeviceBuffer(device, desc);
            frame.m_directionalLightsBuffer.SetDebugName(fmt::format("DirectionalLights SSBO({})", i));
            
            // Point Lights:
            desc.m_size = sizeof(PointLight) * LightCountUBO::kMaxPointLights;
            desc.m_usage = nes::EBufferUsageBits::ShaderResourceStorage;
            desc.m_location = nes::EMemoryLocation::HostUpload;
            desc.m_structureStride = sizeof(PointLight);
            frame.m_pointLightsBuffer = nes::DeviceBuffer(device, desc);
            frame.m_pointLightsBuffer.SetDebugName(fmt::format("PointLights SSBO({})", i));
        }
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
        
        // Submit:
        uploader.RecordCommands(cmdBuffer);
        nes::Renderer::SubmitAndWaitTempCommands(cmdBuffer);
    }
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
    // Pipeline Layout : Just the Global information:
    {
        // Camera:
        std::array bindings =
        {
            nes::DescriptorBindingDesc()
                .SetBindingIndex(0)
                .SetDescriptorType(nes::EDescriptorType::UniformBuffer)
                .SetShaderStages(nes::EPipelineStageBits::VertexShader | nes::EPipelineStageBits::FragmentShader),
        };

        nes::DescriptorSetDesc cameraSet = nes::DescriptorSetDesc()
            .SetBindings(bindings.data(), static_cast<uint32>(bindings.size()));

        // Add this set to the Pipeline Layout.
        nes::PipelineLayoutDesc layoutDesc = nes::PipelineLayoutDesc()
            .SetDescriptorSets(cameraSet)
            .SetShaderStages(nes::EPipelineStageBits::VertexShader | nes::EPipelineStageBits::FragmentShader);

        m_gridPipelineLayout = nes::PipelineLayout(device, layoutDesc);
    }
    
    // Shader Stages:
    nes::AssetPtr<nes::Shader> gridShader = nes::AssetManager::GetAsset<nes::Shader>(m_gridShader);
    NES_ASSERT(gridShader);
    auto shaderStages = gridShader->GetGraphicsShaderStages();
    
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
        .SetShaderStages(shaderStages)
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
        // Camera:
        std::array bindings =
        {
            nes::DescriptorBindingDesc()
                .SetBindingIndex(0)
                .SetDescriptorType(nes::EDescriptorType::UniformBuffer)
                .SetShaderStages(nes::EPipelineStageBits::VertexShader | nes::EPipelineStageBits::FragmentShader),
        };

        nes::DescriptorSetDesc cameraSet = nes::DescriptorSetDesc()
            .SetBindings(bindings.data(), static_cast<uint32>(bindings.size()));
        
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
            .SetDescriptorSets({cameraSet, skyboxSetDesc})
            .SetShaderStages(nes::EPipelineStageBits::VertexShader | nes::EPipelineStageBits::FragmentShader);

        m_skyboxPipelineLayout = nes::PipelineLayout(device, layoutDesc);
    }

    // Shader Stages:
    nes::AssetPtr<nes::Shader> skyboxShader = nes::AssetManager::GetAsset<nes::Shader>(m_skyboxShader);
    NES_ASSERT(skyboxShader);
    auto shaderStages = skyboxShader->GetGraphicsShaderStages();

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
        .SetShaderStages(shaderStages)
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
    //  Set 0: "Camera"
    //      Binding 0: CameraUBO.
    //  Set 1: "Material Params" - Fragment
    //      Binding 0: Materials SSBO
    //  Set 2: "Lighting" - Only Fragment
    //      Binding 0: LightCountUBO
    //      Binding 1: DirectionLights SSBO
    //      Binding 2: PointLights SSBO
    //      (TODO) Binding 3: SpotLights SSBO 
    //      (TODO) Binding 4: AreaLights SSBO
    //  Set 3: "Sampler and Textures" - Only Fragment
    //      Binding 0: Sampler
    //      Binding 1: BaseColor Texture
    //      Binding 2: Normal Texture
    //      Binding 3: RoughnessMetallic Texture
    //      Binding 4: Emission Texture
    // PushConstants:
    //      Instance Data.
    {
        // Camera:
        nes::DescriptorBindingDesc cameraBinding = nes::DescriptorBindingDesc()
                .SetBindingIndex(0)
                .SetDescriptorType(nes::EDescriptorType::UniformBuffer)
                .SetShaderStages(nes::EPipelineStageBits::VertexShader | nes::EPipelineStageBits::FragmentShader);
        
        nes::DescriptorSetDesc cameraSet = nes::DescriptorSetDesc()
            .SetBindings(&cameraBinding, 1u);

        // Material
        nes::DescriptorBindingDesc materialBinding = nes::DescriptorBindingDesc()
                .SetBindingIndex(0)
                .SetDescriptorType(nes::EDescriptorType::StorageBuffer)
                .SetShaderStages(nes::EPipelineStageBits::FragmentShader);
        
        nes::DescriptorSetDesc materialSet = nes::DescriptorSetDesc()
            .SetBindings(&materialBinding, 1u);

        // Light Set:
        std::array lightBindings = 
        {
            nes::DescriptorBindingDesc()
                .SetBindingIndex(0)
                .SetDescriptorType(nes::EDescriptorType::UniformBuffer)
                .SetShaderStages(nes::EPipelineStageBits::FragmentShader),

            nes::DescriptorBindingDesc()
                .SetBindingIndex(1)
                .SetDescriptorType(nes::EDescriptorType::StorageBuffer)
                .SetShaderStages(nes::EPipelineStageBits::FragmentShader),

            nes::DescriptorBindingDesc()
                .SetBindingIndex(2)
                .SetDescriptorType(nes::EDescriptorType::StorageBuffer)
                .SetShaderStages(nes::EPipelineStageBits::FragmentShader),
        };
        
        nes::DescriptorSetDesc lightSet = nes::DescriptorSetDesc()
            .SetBindings(lightBindings.data(), static_cast<uint32_t>(lightBindings.size()));
        
        // Sampler & Materials
        std::array textureBindings =
        {
            nes::DescriptorBindingDesc()
                .SetBindingIndex(0)
                .SetDescriptorType(nes::EDescriptorType::Sampler)
                .SetShaderStages(nes::EPipelineStageBits::FragmentShader),

            nes::DescriptorBindingDesc()
                .SetBindingIndex(1)
                .SetDescriptorType(nes::EDescriptorType::Image)
                .SetShaderStages(nes::EPipelineStageBits::FragmentShader),
            
            nes::DescriptorBindingDesc()
                .SetBindingIndex(2)
                .SetDescriptorType(nes::EDescriptorType::Image)
                .SetShaderStages(nes::EPipelineStageBits::FragmentShader),

            nes::DescriptorBindingDesc()
                .SetBindingIndex(3)
                .SetDescriptorType(nes::EDescriptorType::Image)
                .SetShaderStages(nes::EPipelineStageBits::FragmentShader),

            nes::DescriptorBindingDesc()
                .SetBindingIndex(4)
                .SetDescriptorType(nes::EDescriptorType::Image)
                .SetShaderStages(nes::EPipelineStageBits::FragmentShader),
        };

        nes::DescriptorSetDesc textureSet = nes::DescriptorSetDesc()
            .SetBindings(textureBindings.data(), static_cast<uint32>(textureBindings.size()));

        nes::PushConstantDesc pushConstantDesc{};
        pushConstantDesc.m_offset = 0;
        pushConstantDesc.m_size = sizeof(ObjectUBO);
        pushConstantDesc.m_shaderStages = nes::EPipelineStageBits::VertexShader | nes::EPipelineStageBits::FragmentShader;
        
        // Create the layout:
        nes::PipelineLayoutDesc layoutDesc = nes::PipelineLayoutDesc()
            .SetDescriptorSets({cameraSet, materialSet, lightSet, textureSet})
            .SetPushConstants(pushConstantDesc)
            .SetShaderStages(nes::EPipelineStageBits::VertexShader | nes::EPipelineStageBits::FragmentShader);

        m_pbrPipelineLayout = nes::PipelineLayout(device, layoutDesc);
    }
    
    // Shader:
    nes::AssetPtr<nes::Shader> pbrShader = nes::AssetManager::GetAsset<nes::Shader>(m_pbrShader);
    NES_ASSERT(pbrShader);
    auto shaderStages = pbrShader->GetGraphicsShaderStages();

    // Vertex Input:
    std::array vertexBindings = Vertex::GetBindingDescs();
    const auto vertexStreamDesc = nes::VertexStreamDesc()
        .SetBinding(0)
        .SetStepRate(nes::EVertexStreamStepRate::PerVertex)
        .SetStride(sizeof(Vertex));
    
    nes::VertexInputDesc vertexInputDesc = nes::VertexInputDesc()
        .SetAttributes(vertexBindings)
        .SetStreams(vertexStreamDesc);
    
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
        .SetShaderStages(shaderStages)
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
    // Some default values for the time being:
    nes::DescriptorPoolDesc poolDesc{};
    poolDesc.m_descriptorSetMaxNum = 64; 
    poolDesc.m_uniformBufferMaxNum = 64;
    poolDesc.m_storageBufferMaxNum = 64;
    poolDesc.m_samplerMaxNum = 64;      
    poolDesc.m_imageMaxNum = 64;
    m_descriptorPool = nes::DescriptorPool(device, poolDesc);
}

void PBRExampleApp::CreateDescriptorSets(nes::RenderDevice& device)
{
    nes::Descriptor* pDefaultSampler = &m_sampler;
    // Sampler Descriptor
    {
        nes::SamplerDesc samplerDesc{};
        samplerDesc.m_addressModes = {nes::EAddressMode::ClampToEdge, nes::EAddressMode::ClampToEdge, nes::EAddressMode::ClampToEdge};
        samplerDesc.m_filters = {nes::EFilterType::Linear, nes::EFilterType::Linear, nes::EFilterType::Linear};
        samplerDesc.m_anisotropy = static_cast<uint8>(device.GetDesc().m_other.m_maxSamplerAnisotropy);
        samplerDesc.m_mipMax = 16.f;
        m_sampler = nes::Descriptor(device, samplerDesc);
    }
    
    // Camera Descriptors
    nes::BufferViewDesc cameraView{};
    cameraView.m_pBuffer = &m_globalsBuffer;
    cameraView.m_viewType = nes::EBufferViewType::Uniform;
    cameraView.m_size = sizeof(CameraUBO);
    
    // Light Count Descriptor
    nes::BufferViewDesc lightCountView{};
    lightCountView.m_viewType = nes::EBufferViewType::Uniform;
    lightCountView.m_pBuffer = &m_globalsBuffer;
    lightCountView.m_size = sizeof(LightCountUBO);

    // Object Descriptors
    nes::BufferViewDesc objectViewDesc{};
    objectViewDesc.m_viewType = nes::EBufferViewType::ShaderResourceStorage;

    // Point Light Descriptor:
    nes::BufferViewDesc pointLightView{};
    pointLightView.m_viewType = nes::EBufferViewType::ShaderResourceStorage;

    // Directional Light Descriptor:
    nes::BufferViewDesc directionalLightView{};
    directionalLightView.m_viewType = nes::EBufferViewType::ShaderResourceStorage;

    // [TODO]: SpotLights, AreaLights 
    
    for (size_t i = 0; i < m_frames.size(); ++i)
    {
        auto& frame = m_frames[i];

        // Global Buffer: CameraUBO + LightCountUBO
        {
            // Set the offsets in the globals buffer.
            // - Both are 64 byte aligned.
            cameraView.m_offset = i * kGlobalUBOElementSize; 
            lightCountView.m_offset = cameraView.m_offset + sizeof(CameraUBO);
            frame.m_cameraBufferOffset = cameraView.m_offset; 
            frame.m_lightCountOffset = lightCountView.m_offset;

            // Create the views:
            frame.m_cameraUBOView = nes::Descriptor(device, cameraView);
            frame.m_lightCountUBOView = nes::Descriptor(device, lightCountView);
        }

        // Materials View
        {
            objectViewDesc.m_pBuffer = &frame.m_materialUBOBuffer;
            frame.m_materialUBOView = nes::Descriptor(device, objectViewDesc);
        }

        // Light Type Views
        // [TODO]: SpotLights and AreaLights.
        {
            pointLightView.m_pBuffer = &frame.m_pointLightsBuffer;
            frame.m_pointLightsView = nes::Descriptor(device, pointLightView);
            
            directionalLightView.m_pBuffer = &frame.m_directionalLightsBuffer;
            frame.m_directionalLightsView = nes::Descriptor(device, directionalLightView);
        }
        
        // Camera Descriptor Set: Used by all pipeline layouts at "set 0. We are using the grid layout to allocate it.
        {
            m_descriptorPool.AllocateDescriptorSets(m_gridPipelineLayout, 0, &frame.m_cameraSet);
            nes::Descriptor* pView = &frame.m_cameraUBOView;
            auto updateDesc = nes::DescriptorBindingUpdateDesc(&pView, 1);
            frame.m_cameraSet.UpdateBindings(&updateDesc, 0);
        }

        // Material Data Set: Used by PBR Pipeline Layout only.
        {
            m_descriptorPool.AllocateDescriptorSets(m_pbrPipelineLayout, 1, &frame.m_materialDataSet);
            nes::Descriptor* pView = &frame.m_materialUBOView;
            auto updateDesc = nes::DescriptorBindingUpdateDesc(&pView, 1);
            frame.m_materialDataSet.UpdateBindings(&updateDesc, 0);
        }

        // Light Data Set: Used by PBR Pipeline Layout only.
        {
            m_descriptorPool.AllocateDescriptorSets(m_pbrPipelineLayout, 2, &m_frames[i].m_lightDataSet);

            nes::Descriptor* pLightCounts = &frame.m_lightCountUBOView;
            nes::Descriptor* pDirectLights = &frame.m_directionalLightsView;
            nes::Descriptor* pPointLights = &frame.m_pointLightsView;
            //nes::Descriptor* pSpotLights = &frame.m_spotLightsView;
            //nes::Descriptor* pAreaLights = &frame.m_areaLightsView;

            std::array updateDescs
            {
                nes::DescriptorBindingUpdateDesc(&pLightCounts, 1),
                nes::DescriptorBindingUpdateDesc(&pDirectLights, 1),
                nes::DescriptorBindingUpdateDesc(&pPointLights, 1),
                //nes::DescriptorBindingUpdateDesc(&pSpotLights, 1),
                //nes::DescriptorBindingUpdateDesc(&pAreaLights, 1),
            };
            
            frame.m_lightDataSet.UpdateBindings(updateDescs.data(), 0, static_cast<uint32_t>(updateDescs.size()));
        }
    }
    
    // Skybox Descriptor Set: Sampler and CubeImage.
    {
        m_descriptorPool.AllocateDescriptorSets(m_skyboxPipelineLayout, 1, &m_skyboxDescriptorSet);

        // Get the Skybox Texture View:
        const auto textureIndex = m_scene.m_idToTextureIndex[m_skyboxTexture];
        NES_ASSERT(textureIndex < m_scene.m_textures.size());
        nes::Descriptor* pSkyboxTexture = &m_scene.m_textures[textureIndex];
        NES_ASSERT(*pSkyboxTexture != nullptr);

        std::array updateDescs =
        {
            nes::DescriptorBindingUpdateDesc(&pDefaultSampler, 1),
            nes::DescriptorBindingUpdateDesc(&pSkyboxTexture, 1),
        };
        m_skyboxDescriptorSet.UpdateBindings(updateDescs.data(), 0, static_cast<uint32>(updateDescs.size()));
    }

    // Sampler and Images Set - Per Material.
    {
        uint32 numMaterials = static_cast<uint32>(m_scene.m_materials.size());
        m_materialDescriptorSets.reserve(numMaterials);
        
        for (uint32 i = 0; i < numMaterials; ++i)
        {
            const auto& material = m_scene.m_materials[i];

            m_materialDescriptorSets.emplace_back(nullptr);
            m_descriptorPool.AllocateDescriptorSets(m_pbrPipelineLayout, 3, &m_materialDescriptorSets[i], 1);
            
            nes::Descriptor* materialTextures[] =
            {
                &m_scene.m_textures[material.m_baseColorIndex],  
                &m_scene.m_textures[material.m_normalIndex],  
                &m_scene.m_textures[material.m_roughnessMetallicIndex],  
                &m_scene.m_textures[material.m_emissionIndex],  
            };

            std::array updateDescs =
            {
                nes::DescriptorBindingUpdateDesc(&pDefaultSampler, 1),
                nes::DescriptorBindingUpdateDesc(materialTextures, 4),
            };

            m_materialDescriptorSets[i].UpdateBindings(updateDescs.data(), 0, static_cast<uint32>(updateDescs.size()));
        }
    }
}

void PBRExampleApp::UpdateUniformBuffers(const nes::RenderFrameContext& context)
{
    auto& frame = m_frames[context.GetFrameIndex()];
    
    // Update the Camera:
    {
        CameraUBO cameraConstants;
        cameraConstants.m_exposureFactor = helpers::CalculateExposureFactor(m_cameraAperture, m_cameraShutterSpeed, m_cameraISO);
        
        // Set the world position.
        cameraConstants.m_position = nes::Float3(m_camera.m_position.x, m_camera.m_position.y, m_camera.m_position.z);

        // Calculate the View and Projection Matrices:
        const nes::Vec3 forward = m_camera.m_rotation.RotatedVector(nes::Vec3::Forward());
        const nes::Vec3 up = m_camera.m_rotation.RotatedVector(nes::Vec3::Up());
    
        const auto viewport = context.GetSwapchainViewport();
        cameraConstants.m_view = nes::Mat44::LookAt(m_camera.m_position, m_camera.m_position + forward, up);
        cameraConstants.m_projection = nes::Mat44::Perspective(m_camera.m_FOVY, viewport.m_extent.x / viewport.m_extent.y, 0.1f, 1000.f);

        // Flip the Y projection.
        cameraConstants.m_projection[1][1] *= -1.f;
        
        // Cache the view projection matrix.
        cameraConstants.m_viewProjection = cameraConstants.m_projection * cameraConstants.m_view;
        
        // Set the camera data:
        m_globalsBuffer.CopyToMappedMemory(&cameraConstants, frame.m_cameraBufferOffset, sizeof(CameraUBO));
    }

    // Update Lighting Information:
    {
        LightCountUBO lightCounts{};
        lightCounts.m_pointCount = static_cast<uint32>(m_scene.m_pointLights.size());
        lightCounts.m_directionalCount = static_cast<uint32>(m_scene.m_directionalLights.size());
        m_globalsBuffer.CopyToMappedMemory(&lightCounts, frame.m_lightCountOffset, sizeof(LightCountUBO));

        // Directional Lights:
        if (!m_scene.m_directionalLights.empty())
        {
            frame.m_directionalLightsBuffer.CopyToMappedMemory(m_scene.m_directionalLights.data());
        }

        // Point Lights
        if (!m_scene.m_pointLights.empty())
        {
            frame.m_pointLightsBuffer.CopyToMappedMemory(m_scene.m_pointLights.data());
        }

        // [TODO]: Spot Lights, Area Lights.
    }
    
    // Update Material Data:
    if (!m_scene.m_materials.empty())
    {
        frame.m_materialUBOBuffer.CopyToMappedMemory(m_scene.m_materials.data());
    }
}

void PBRExampleApp::RenderSkybox(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
{
    NES_ASSERT(m_skyboxPipeline != nullptr);
    NES_ASSERT(m_skyboxPipelineLayout != nullptr);
    NES_ASSERT(m_frames.size() > context.GetFrameIndex());

    commandBuffer.BindPipelineLayout(m_skyboxPipelineLayout);
    commandBuffer.BindPipeline(m_skyboxPipeline);
    commandBuffer.BindDescriptorSet(0, m_frames[context.GetFrameIndex()].m_cameraSet);
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
    commandBuffer.DrawIndexed(drawDesc);
}

void PBRExampleApp::RenderInstances(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
{
    NES_ASSERT(m_pbrPipeline != nullptr);
    NES_ASSERT(m_pbrPipelineLayout != nullptr);
    NES_ASSERT(m_frames.size() > context.GetFrameIndex());

    auto& frame = m_frames[context.GetFrameIndex()];
    
    commandBuffer.BindPipelineLayout(m_pbrPipelineLayout);
    commandBuffer.BindPipeline(m_pbrPipeline);
    commandBuffer.BindDescriptorSet(0, frame.m_cameraSet);
    commandBuffer.BindDescriptorSet(1, frame.m_materialDataSet);
    commandBuffer.BindDescriptorSet(2, frame.m_lightDataSet);

    // Bind the index buffer for the entire range:
    nes::IndexBufferRange indexBuffer = nes::IndexBufferRange(&m_indicesBuffer, m_scene.m_indices.size(), 0);
    commandBuffer.BindIndexBuffer(indexBuffer);
    
    for (auto& object : m_scene.m_objects)
    {
        // Push the Instance data
        commandBuffer.SetPushConstant(0, &object, sizeof(ObjectUBO));

        // Bind Material Textures:
        commandBuffer.BindDescriptorSet(3, m_materialDescriptorSets[object.m_materialIndex]);
        
        // Bind Mesh Vertex Buffer.
        const Mesh& mesh = m_scene.m_meshes[object.m_meshIndex];
        nes::VertexBufferRange meshVertexBuffer(&m_verticesBuffer, sizeof(Vertex), mesh.m_vertexCount, mesh.m_firstVertex * sizeof(Vertex));
        commandBuffer.BindVertexBuffers({ meshVertexBuffer}, 0);

        // Draw
        nes::DrawIndexedDesc drawDesc{};
        drawDesc.m_firstIndex = mesh.m_firstIndex;
        drawDesc.m_indexCount = mesh.m_indexCount;
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
    commandBuffer.BindDescriptorSet(0, m_frames[context.GetFrameIndex()].m_cameraSet);
    commandBuffer.DrawVertices(6);
}

void PBRExampleApp::UpdateCamera(const float deltaTime)
{
    ProcessInput();

    // Speed:
    float speed = m_cameraMoveSpeed * deltaTime;
    const bool shift = nes::InputManager::IsKeyDown(nes::EKeyCode::LeftShift) || nes::InputManager::IsKeyDown(nes::EKeyCode::RightShift);
    if (shift)
        speed *= 5.f;

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
    
    return std::make_unique<PBRExampleApp>(outAppDesc);
}

NES_MAIN()