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
    if (!LoadAssets())
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
    
    CreateUniformBuffers(device);
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
        .SetDepthStencilTargets(&m_gBuffer.GetDepthImageView());
    
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

        // [TODO]: Render the Skybox

        // [TODO]: Render the Geometry

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
    m_skyboxPipeline = nullptr;
    m_gridPipeline = nullptr;
    m_gridPipelineLayout = nullptr;
    m_geometryPipeline = nullptr;
    m_geometryPipelineLayout = nullptr;
    m_descriptorPool = nullptr;
}

bool PBRExampleApp::LoadAssets()
{
    NES_LOG("Loading Assets...");
    std::filesystem::path assetPath{};
    
    // Grid Vert Shader:
    {
        assetPath = NES_SHADER_DIR;
        assetPath /= "Grid.vert.spv";

        const auto result = nes::AssetManager::LoadSync<nes::Shader>(m_gridVertShader, assetPath);
        if (result != nes::ELoadResult::Success)
        {
            NES_ERROR("Failed to load Grid Frag Shader!");
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
    
    NES_LOG("Assets Loaded.");
    return true;
}

void PBRExampleApp::CreateUniformBuffers(nes::RenderDevice& device)
{
    // A single constant buffer that different frames will use. The Descriptors will
    // have access to a section of the buffer.
    nes::AllocateBufferDesc desc;
    desc.m_size = sizeof(GlobalConstants) * nes::Renderer::GetMaxFramesInFlight();
    desc.m_usage = nes::EBufferUsageBits::UniformBuffer;
    desc.m_location = nes::EMemoryLocation::HostUpload; // We are updating the data each frame, so we need to write to it.
    m_globalConstantBuffer = nes::DeviceBuffer(device, desc);
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
        
        nes::ImageBarrierDesc imageBarrier = nes::ImageBarrierDesc()
            .SetImage(&msaaImage)
            .SetLayout(nes::EImageLayout::Undefined, nes::EImageLayout::ResolveSource)
            .SetAccess(nes::EAccessBits::None, nes::EAccessBits::ResolveSource);

        nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
            .SetImageBarriers(imageBarrier);
        
        commandBuffer.SetBarriers(barrierGroup);

        nes::Renderer::SubmitAndWaitTempCommands(commandBuffer);
    }
}

void PBRExampleApp::CreateGridPipeline(nes::RenderDevice& device)
{
    // Pipeline Layout:
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

void PBRExampleApp::CreateSkyboxPipeline(nes::RenderDevice& /*device*/)
{
    // [TODO]: 
}

void PBRExampleApp::CreateGeometryPipeline(nes::RenderDevice& /*device*/)
{
    // [TODO]: 
}

void PBRExampleApp::CreateDescriptorPool(nes::RenderDevice& device)
{
    // [TODO]: I need:
    // - 5 images per material: Base Color, Normal, Roughness, Metallic and Emission Maps. 
    // - At least 1 Sampler for Mesh Texture Maps.
    // - Transform uniform buffers for each frame.
    // - Scene uniform buffers for each frame.
    // - Descriptor Set for each frame.
    // - Descriptor for Texture Cube - 6 more images?
    // - Descriptor Set for each Mesh's PBR material parameters.
    
    // Create a descriptor pool that will only be able to allocate the
    // exact number of constant buffer descriptors that we need (1 per frame).
    const uint32 numFrames = static_cast<uint32>(m_frames.size());
    nes::DescriptorPoolDesc poolDesc{};
    poolDesc.m_descriptorSetMaxNum = numFrames;
    poolDesc.m_uniformBufferMaxNum = numFrames * 2;
    poolDesc.m_samplerMaxNum = 1;
    poolDesc.m_imageMaxNum = 1;
    
    m_descriptorPool = nes::DescriptorPool(device, poolDesc);
}

void PBRExampleApp::CreateDescriptorSets(nes::RenderDevice& device)
{
    // View into the single uniform buffer:
    nes::BufferViewDesc bufferViewDesc{};
    bufferViewDesc.m_pBuffer = &m_globalConstantBuffer;
    bufferViewDesc.m_viewType = nes::EBufferViewType::Uniform;
    bufferViewDesc.m_size = sizeof(GlobalConstants);
    
    // Create the Buffer View Descriptors:
    for (size_t i = 0; i < m_frames.size(); ++i)
    {
        // Set the offset for this view:
        bufferViewDesc.m_offset = i * sizeof(GlobalConstants);
        m_frames[i].m_globalBufferView = nes::Descriptor(device, bufferViewDesc);
        m_frames[i].m_globalBufferOffset = bufferViewDesc.m_offset;

        // [TODO]: This is only for the grid pipeline layout.
        // Allocate the Descriptor Set:
        m_descriptorPool.AllocateDescriptorSets(m_gridPipelineLayout, 0, &m_frames[i].m_descriptorSet);

        std::array updateDescs =
        {
            nes::DescriptorBindingUpdateDesc(&m_frames[i].m_globalBufferView, 1),
        };
        m_frames[i].m_descriptorSet.UpdateBindings(updateDescs.data(), 0, static_cast<uint32>(updateDescs.size()));
    } 
}

void PBRExampleApp::UpdateUniformBuffers(const nes::RenderFrameContext& context)
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
    m_globalConstantBuffer.CopyToMappedMemory(&constants, m_frames[context.GetFrameIndex()].m_globalBufferOffset, sizeof(GlobalConstants));
}

void PBRExampleApp::RenderGrid(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context) const
{
    NES_ASSERT(m_gridPipeline != nullptr);
    NES_ASSERT(m_gridPipelineLayout != nullptr);
    NES_ASSERT(m_frames.size() > context.GetFrameIndex());
    
    commandBuffer.BindPipelineLayout(m_gridPipelineLayout);
    commandBuffer.BindPipeline(m_gridPipeline);
    commandBuffer.BindDescriptorSet(0, m_frames[context.GetFrameIndex()].m_descriptorSet);
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
    const float heading = m_inputRotation.y * 0.5f;
    const float pitch = nes::math::Clamp(m_inputRotation.x * 0.5f, -0.49f * nes::math::Pi(), 0.49f * nes::math::Pi());
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