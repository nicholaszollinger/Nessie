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
    m_frames.resize(nes::Renderer::GetMaxFramesInFlight());
    
    // Load the Scene, and all assets.
    {
        NES_LOG("Loading Scene...");
        
        std::filesystem::path path = NES_CONTENT_DIR;
        path /= "Scenes\\PBRTestScene.yaml";
        if (!helpers::LoadScene(path, device, m_scene, m_loadedAssets))
            return false;
        
        NES_LOG("Scene Loaded.");
    }

    // Load Application Settings
    std::filesystem::path path = NES_CONFIG_DIR;
    path /= "PBRAppSettings.yaml";

    YAML::Node file = YAML::LoadFile(path.string());
    if (!file)
    {
        NES_ERROR("Failed to load Application Settings!");
        return false;
    }

    // Camera Settings:
    {
        auto cameraSettings = file["CameraSettings"];

        auto position = cameraSettings["Position"];
        m_camera.m_position.x = position[0].as<float>(0.f);
        m_camera.m_position.y = position[1].as<float>(0.f);
        m_camera.m_position.z = position[2].as<float>(0.f);

        auto rotation = cameraSettings["Rotation"];
        m_camera.m_rotation.m_pitch = rotation[0].as<float>(0.f);
        m_camera.m_rotation.m_yaw = rotation[1].as<float>(0.f);
        m_camera.m_rotation.m_roll = rotation[2].as<float>(0.f);
        
        m_camera.m_FOVY = nes::math::ToRadians(cameraSettings["FOV"].as<float>(65.f));
        m_cameraMoveSpeed = cameraSettings["MoveSpeed"].as<float>(50.f);
        m_cameraSensitivity = cameraSettings["Sensitivity"].as<float>(1.0f);
        m_cameraAperture = cameraSettings["Aperture"].as<float>(1.0f);
        m_cameraShutterSpeed = 1.f / cameraSettings["ShutterSpeedSeconds"].as<float>(125.f);
        m_cameraISO = cameraSettings["ISO"].as<float>(100.f);
    }
    
    CreateRenderTargetsAndPipelines(device, file);
    CreateBuffersAndImages(device);
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
    ResizeRenderTargets(width, height);
}

void PBRExampleApp::Internal_AppRender(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
{
    // Update our uniform buffer:
    UpdateUniformBuffers(context);

    // We render to this higher sampled image - we will resolve this with the swapchain image at the end of the frame.
    auto& msaaImage = m_colorTarget.GetImage();

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
         .SetColorTargets(&m_colorTarget.GetView())
         .SetDepthStencilTarget(&m_depthTarget.GetView());
    
    // Record Render Commands:
    commandBuffer.BeginRendering(renderTargetsDesc);
    {
        // Clear the Color and Depth Targets:
        const nes::ClearDesc colorClear = nes::ClearDesc::Color(m_colorTarget.GetClearValue(), 0);
        const nes::ClearDesc depthClear = nes::ClearDesc::DepthStencil(m_depthTarget.GetClearValue());
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
    m_colorTarget = nullptr;
    m_depthTarget = nullptr;
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

void PBRExampleApp::CreateRenderTargetsAndPipelines(nes::RenderDevice& device, const YAML::Node& file)
{
    const auto swapchainColorFormat = nes::Renderer::GetSwapchainFormat();
    const auto swapchainExtent = nes::Renderer::GetSwapchainExtent();

    // Load Render Targets:
    auto renderTargets = file["RenderTargets"];
    NES_ASSERT(renderTargets);
    NES_ASSERT(renderTargets.size() > 0);
    {
        const nes::UInt2 swapchainSize = nes::UInt2(swapchainExtent.width, swapchainExtent.height);
        
        m_colorTarget = LoadColorRenderTarget(renderTargets["Color"], "Color", device,  swapchainColorFormat, swapchainSize);
        m_depthTarget = LoadDepthRenderTarget(renderTargets["Depth"], "Depth", device, swapchainSize);

        // Add to the registry:
        m_renderTargetRegistry.emplace(m_colorTarget.GetName(), &m_colorTarget);
        m_renderTargetRegistry.emplace(m_depthTarget.GetName(), &m_depthTarget);
    }

    // Load Pipelines:
    std::filesystem::path path{};
    auto pipelines = file["Pipelines"];
    NES_ASSERT(pipelines);

    // Grid
    {
        path = NES_CONTENT_DIR;
        path /= pipelines["Grid"].as<std::string>();

        YAML::Node pipelineFile = YAML::LoadFile(path.string());
        NES_ASSERT(pipelineFile);
        auto graphicsPipeline = pipelineFile["GraphicsPipeline"];
        NES_ASSERT(graphicsPipeline);
        LoadGraphicsPipeline(graphicsPipeline, device, m_gridPipelineLayout, m_gridPipeline);
    }

    // Skybox
    {
        path = NES_CONTENT_DIR;
        path /= pipelines["Skybox"].as<std::string>();

        YAML::Node pipelineFile = YAML::LoadFile(path.string());
        NES_ASSERT(pipelineFile);
        auto graphicsPipeline = pipelineFile["GraphicsPipeline"];
        NES_ASSERT(graphicsPipeline);
        LoadGraphicsPipeline(graphicsPipeline, device, m_skyboxPipelineLayout, m_skyboxPipeline);
    }

    // PBR
    {
        path = NES_CONTENT_DIR;
        path /= pipelines["PBR"].as<std::string>();

        YAML::Node pipelineFile = YAML::LoadFile(path.string());
        NES_ASSERT(pipelineFile);
        auto graphicsPipeline = pipelineFile["GraphicsPipeline"];
        NES_ASSERT(graphicsPipeline);
        LoadGraphicsPipeline(graphicsPipeline, device, m_pbrPipelineLayout, m_pbrPipeline);
    }
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

void PBRExampleApp::ResizeRenderTargets(const uint32 width, const uint32 height)
{
    // Resize the MSAA Target Set
    m_colorTarget.Resize(width, height);
    m_depthTarget.Resize(width, height);
    
    // After resize, each image is in the Undefined layout.
    // Convert the msaa image to the resolve source layout:
    {
        auto commandBuffer = nes::Renderer::BeginTempCommands();
        auto& msaaImage = m_colorTarget.GetImage();
        auto& depthImage = m_depthTarget.GetImage();
        
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
        const auto textureIndex = m_scene.m_idToTextureIndex[m_scene.m_skyboxTextureID];
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

nes::RenderTarget PBRExampleApp::LoadColorRenderTarget(const YAML::Node& targetNode, const std::string& name, nes::RenderDevice& device, const nes::EFormat swapchainFormat, const nes::UInt2 swapchainExtent)
{
    nes::RenderTargetDesc desc{};
    desc.m_name = name;
    desc.m_planes = nes::EImagePlaneBits::Color;

    // Format
    desc.m_format = static_cast<nes::EFormat>(targetNode["Format"].as<uint32>(0));
    if (desc.m_format == nes::EFormat::Unknown)
        desc.m_format = swapchainFormat;

    // Sample Count
    desc.m_sampleCount = targetNode["SampleCount"].as<uint32>(1);

    // Clear Value
    auto clearColorNode = targetNode["ClearColor"];
    nes::ClearColorValue clearColorValue{};
    clearColorValue.m_float32[0] = clearColorNode[0].as<float>(0.f);
    clearColorValue.m_float32[1] = clearColorNode[1].as<float>(0.f);
    clearColorValue.m_float32[2] = clearColorNode[2].as<float>(0.f);
    clearColorValue.m_float32[3] = clearColorNode[3].as<float>(1.f);
    desc.m_clearValue = clearColorValue;

    // Size
    auto sizeNode = targetNode["Size"];
    desc.m_size.x = sizeNode[0].as<uint32>(0);
    desc.m_size.y = sizeNode[1].as<uint32>(0);

    // If either dimension is zero, use the swapchain extent.
    if (desc.m_size.x == 0 || desc.m_size.y == 0)
        desc.m_size = swapchainExtent;
    
    return nes::RenderTarget(device, desc);
}

nes::RenderTarget PBRExampleApp::LoadDepthRenderTarget(const YAML::Node& targetNode, const std::string& name, nes::RenderDevice& device, const nes::UInt2 swapchainExtent)
{
    nes::RenderTargetDesc desc{};
    desc.m_name = name;
        
    // Format
    const uint32 minBits = targetNode["FormatMinBits"].as<uint32>(16);
    const bool requireStencil = targetNode["FormatRequireStencil"].as<bool>(false);
    desc.m_format = device.GetSupportedDepthFormat(minBits, requireStencil);
    NES_ASSERT(desc.m_format != nes::EFormat::Unknown);
        
    // Image Planes based on the Format
    desc.m_planes = nes::EImagePlaneBits::Depth;
    if (requireStencil)
        desc.m_planes |= nes::EImagePlaneBits::Stencil;
        
    // Sample Count
    desc.m_sampleCount = targetNode["SampleCount"].as<uint32>(1);

    // Clear Value
    nes::ClearDepthStencilValue clearDepthStencil;
    clearDepthStencil.m_depth = targetNode["ClearDepth"].as<float>(1.f);
    clearDepthStencil.m_stencil = targetNode["ClearDepth"].as<uint32>(0);
    desc.m_clearValue = clearDepthStencil;

    // Size
    auto sizeNode = targetNode["Size"];
    desc.m_size.x = sizeNode[0].as<uint32>(0);
    desc.m_size.y = sizeNode[1].as<uint32>(0);

    // If either dimension is zero, use the swapchain extent.
    if (desc.m_size.x == 0 || desc.m_size.y == 0)
        desc.m_size = swapchainExtent;

    return nes::RenderTarget(device, desc);
}

void PBRExampleApp::LoadGraphicsPipeline(const YAML::Node& pipelineNode, nes::RenderDevice& device, nes::PipelineLayout& outLayout, nes::Pipeline& outPipeline) const
{
    // Pipeline Layout
    {
        auto layoutNode = pipelineNode["Layout"];

        // Stages
        static constexpr uint32 kDefaultStages = static_cast<uint32>(nes::EPipelineStageBits::GraphicsShaders);
        nes::EPipelineStageBits stages = static_cast<nes::EPipelineStageBits>(layoutNode["Stages"].as<uint32>(kDefaultStages));

        // Descriptor Sets:
        std::vector<std::vector<nes::DescriptorBindingDesc>> setBindings{};
        std::vector<nes::DescriptorSetDesc> setDescs{};
        
        auto descriptorSetsNode = layoutNode["DescriptorSets"];
        for (auto descriptorSet : descriptorSetsNode)
        {
            auto& bindingsArray = setBindings.emplace_back();
            auto bindings = descriptorSet["Bindings"];
            bindingsArray.reserve(bindings.size());
            
            // Bindings
            for (auto bindingNode : bindings)
            {
                nes::DescriptorBindingDesc desc{};
                desc.m_bindingIndex = bindingNode["Index"].as<uint32>(0);
                desc.m_descriptorCount = bindingNode["DescriptorCount"].as<uint32>(1);
                desc.m_descriptorType = static_cast<nes::EDescriptorType>(bindingNode["DescriptorType"].as<uint32>());
                desc.m_shaderStages = static_cast<nes::EPipelineStageBits>(bindingNode["Stages"].as<uint32>(kDefaultStages));
                bindingsArray.emplace_back(desc);
            }
            
            setDescs.emplace_back(bindingsArray.data(), static_cast<uint32>(bindingsArray.size()));
        }

        std::vector<nes::PushConstantDesc> pushConstantDescs{};
        
        // PushConstants
        auto pushConstantsNode = layoutNode["PushConstants"];
        for (auto pushConstant : pushConstantsNode)
        {
            nes::PushConstantDesc desc{};
            desc.m_offset = pushConstant["Offset"].as<uint32>(0);
            desc.m_size = pushConstant["Size"].as<uint32>();
            desc.m_shaderStages = static_cast<nes::EPipelineStageBits>(pushConstant["Stages"].as<uint32>(kDefaultStages));
            pushConstantDescs.emplace_back(desc);
        }

        // Create the Pipeline Layout:
        nes::PipelineLayoutDesc layoutDesc = nes::PipelineLayoutDesc()
            .SetShaderStages(stages)
            .SetDescriptorSets(setDescs)
            .SetPushConstants(pushConstantDescs);

        outLayout = nes::PipelineLayout(device, layoutDesc);
    }
    
    // Graphics Pipeline:
    nes::GraphicsPipelineDesc desc{};
    
    // Shader Stages:
    {
        nes::AssetID shaderID = pipelineNode["Shader"].as<uint64>(); 
        nes::AssetPtr<nes::Shader> shader = nes::AssetManager::GetAsset<nes::Shader>(shaderID);
        NES_ASSERT(shader);
        desc.m_shaderStages = shader->GetGraphicsShaderStages();
    }
    
    // Vertex Input State
    std::vector<nes::VertexAttributeDesc> attributeDescs{};
    std::vector<nes::VertexStreamDesc> streamDescs{};
    {
        auto vertexInputNode = pipelineNode["VertexInputState"];

        // Attributes
        auto attributesNode = vertexInputNode["Attributes"];
        attributeDescs.reserve(attributesNode.size());
        for (auto attribute : attributesNode)
        {
            nes::VertexAttributeDesc attributeDesc{};
            attributeDesc.m_location = attribute["Location"].as<uint32>();
            attributeDesc.m_offset = attribute["Offset"].as<uint32>();
            attributeDesc.m_format = static_cast<nes::EFormat>(attribute["Format"].as<uint32>());
            attributeDesc.m_streamIndex = attribute["Stream"].as<uint32>(0);
            attributeDescs.emplace_back(attributeDesc);
        }

        // Streams
        auto streamsNode = vertexInputNode["Streams"];
        streamDescs.reserve(streamsNode.size());
        for (auto stream : streamsNode)
        {
            nes::VertexStreamDesc streamDesc{};
            streamDesc.m_stride = stream["Stride"].as<uint32>();
            streamDesc.m_bindingIndex = stream["BindingIndex"].as<uint32>(0);
            streamDesc.m_stepRate = static_cast<nes::EVertexStreamStepRate>(stream["StepRate"].as<uint32>(0));
            streamDescs.emplace_back(streamDesc);
        }

        desc.m_vertexInput.m_attributes = attributeDescs;
        desc.m_vertexInput.m_streams = streamDescs;
    }

    // Input Assembly
    {
        auto inputAssembly = pipelineNode["InputAssembly"];

        static constexpr uint32 kDefaultTopologyMode = static_cast<uint32>(nes::ETopology::TriangleList);
        desc.m_inputAssembly.m_primitiveRestart = static_cast<nes::EPrimitiveRestart>(inputAssembly["PrimitiveRestart"].as<uint32>(0));
        desc.m_inputAssembly.m_tessControlPointCount = inputAssembly["TesselationPointCount"].as<uint8>(static_cast<uint8>(0));
        desc.m_inputAssembly.m_topology = static_cast<nes::ETopology>(inputAssembly["Topology"].as<uint32>(kDefaultTopologyMode));
    }

    // Rasterization
    {
        auto rasterizationNode = pipelineNode["Rasterization"];
        auto& rasterState = desc.m_rasterization;

        static constexpr uint32 kDefaultCullMode = static_cast<uint32>(nes::ECullMode::Back);
        static constexpr uint32 kDefaultFillMode = static_cast<uint32>(nes::EFillMode::Solid);
        static constexpr uint32 kDefaultFrontFace = static_cast<uint32>(nes::EFrontFaceWinding::CounterClockwise);
        
        rasterState.m_cullMode = static_cast<nes::ECullMode>(rasterizationNode["CullMode"].as<uint32>(kDefaultCullMode));
        rasterState.m_fillMode = static_cast<nes::EFillMode>(rasterizationNode["FillMode"].as<uint32>(kDefaultFillMode));
        // [TODO]: DepthBias
        rasterState.m_enableDepthClamp = rasterizationNode["EnableDepthClamp"].as<bool>(false);
        rasterState.m_frontFace = static_cast<nes::EFrontFaceWinding>(rasterizationNode["FrontFace"].as<uint32>(kDefaultFrontFace));
        rasterState.m_lineWidth = rasterizationNode["LineWidth"].as<float>(1.0f);
    }

    // Output Merger
    std::vector<nes::RenderTarget*> targets{};
    std::vector<nes::ColorAttachmentDesc> colorAttachments{};
    {
        auto outputMergerNode = pipelineNode["OutputMerger"];
        auto& outputMerger = desc.m_outputMerger;

        static constexpr uint32 kDefaultColorWriteMask = static_cast<uint32>(nes::EColorComponentBits::RGBA);

        // Color Attachments
        auto colorAttachmentsNode = outputMergerNode["ColorAttachments"];
        colorAttachments.reserve(colorAttachmentsNode.size());
        for (auto attachment : colorAttachmentsNode)
        {
            nes::ColorAttachmentDesc colorAttachmentDesc{};
            
            // Get the render target format this attachment is for.
            const std::string renderTargetName = attachment["RenderTarget"].as<std::string>();
            NES_ASSERT(m_renderTargetRegistry.contains(renderTargetName));
            auto* pTarget = m_renderTargetRegistry.at(renderTargetName);
            NES_ASSERT(pTarget->IsColorTarget());
            colorAttachmentDesc.m_format = pTarget->GetFormat();
            targets.emplace_back(pTarget);

            colorAttachmentDesc.m_enableBlend = attachment["EnableBlend"].as<bool>(true);
            colorAttachmentDesc.m_colorWriteMask = static_cast<nes::EColorComponentBits>(attachment["ColorWriteMask"].as<uint32>(kDefaultColorWriteMask));

            // Color Blend
            auto colorBlendState = attachment["ColorBlend"];
            colorAttachmentDesc.m_colorBlend.m_srcFactor = static_cast<nes::EBlendFactor>(colorBlendState["SrcFactor"].as<uint32>());
            colorAttachmentDesc.m_colorBlend.m_dstFactor = static_cast<nes::EBlendFactor>(colorBlendState["DstFactor"].as<uint32>());
            colorAttachmentDesc.m_colorBlend.m_op = static_cast<nes::EBlendOp>(colorBlendState["BlendOp"].as<uint32>());

            // Alpha Blend
            auto alphaBlendState = attachment["AlphaBlend"];
            colorAttachmentDesc.m_alphaBlend.m_srcFactor = static_cast<nes::EBlendFactor>(colorBlendState["SrcFactor"].as<uint32>());
            colorAttachmentDesc.m_alphaBlend.m_dstFactor = static_cast<nes::EBlendFactor>(colorBlendState["DstFactor"].as<uint32>());
            colorAttachmentDesc.m_alphaBlend.m_op = static_cast<nes::EBlendOp>(colorBlendState["BlendOp"].as<uint32>());

            colorAttachments.emplace_back(colorAttachmentDesc);
        }

        outputMerger.m_pColors = colorAttachments.data();
        outputMerger.m_colorCount = static_cast<uint32>(colorAttachments.size());

        // Depth Attachment (optional)
        if (auto depthAttachmentNode = outputMergerNode["DepthAttachment"])
        {
            // Get depth/stencil target this attachment is for.
            const std::string renderTargetName = depthAttachmentNode["RenderTarget"].as<std::string>();
            NES_ASSERT(m_renderTargetRegistry.contains(renderTargetName));
            auto* pTarget = m_renderTargetRegistry.at(renderTargetName);
            NES_ASSERT(pTarget->IsDepthTarget());
            targets.emplace_back(pTarget);
            outputMerger.m_depthStencilFormat = pTarget->GetFormat();

            static constexpr uint32 kDefaultDepthCompareOp = static_cast<uint32>(nes::ECompareOp::Less);
            outputMerger.m_depth.m_compareOp = static_cast<nes::ECompareOp>(depthAttachmentNode["CompareOp"].as<uint32>(kDefaultDepthCompareOp));
            outputMerger.m_depth.m_enableWrite = depthAttachmentNode["EnableWrite"].as<bool>(true);
        }

        // Stencil Attachment (optional)
        if (auto stencilAttachmentNode = outputMergerNode["StencilAttachment"])
        {
            static constexpr uint32 kDefaultStencilCompareOp = static_cast<uint32>(nes::ECompareOp::None);

            // Front
            auto frontNode = stencilAttachmentNode["Front"];
            auto& front = outputMerger.m_stencil.m_front;
            front.m_compareOp = static_cast<nes::ECompareOp>(frontNode["CompareOp"].as<uint32>(kDefaultStencilCompareOp));
            front.m_failOp = static_cast<nes::EStencilOp>(frontNode["FailOp"].as<uint32>());
            front.m_passOp = static_cast<nes::EStencilOp>(frontNode["PassOp"].as<uint32>());
            front.m_depthFailOp = static_cast<nes::EStencilOp>(frontNode["DepthFailOp"].as<uint32>());
            front.m_compareMask = frontNode["CompareMask"].as<uint8>(static_cast<uint8>(0));
            front.m_writeMask = frontNode["WriteMask"].as<uint8>(static_cast<uint8>(0));

            // Back
            auto backNode = stencilAttachmentNode["Back"];
            auto& back = outputMerger.m_stencil.m_back;
            back.m_compareOp = static_cast<nes::ECompareOp>(backNode["CompareOp"].as<uint32>(kDefaultStencilCompareOp));
            back.m_failOp = static_cast<nes::EStencilOp>(backNode["FailOp"].as<uint32>());
            back.m_passOp = static_cast<nes::EStencilOp>(backNode["PassOp"].as<uint32>());
            back.m_depthFailOp = static_cast<nes::EStencilOp>(backNode["DepthFailOp"].as<uint32>());
            back.m_compareMask = backNode["CompareMask"].as<uint8>(static_cast<uint8>(0));
            back.m_writeMask = backNode["WriteMask"].as<uint8>(static_cast<uint8>(0));
        }
    }

    // Multisample Behavior
    {
        auto& multisample = desc.m_multisample;
        desc.m_enableMultisample = false;

        if (auto multisampleNode = pipelineNode["Multisample"])
        {
            desc.m_enableMultisample = multisampleNode["Enabled"].as<bool>(false);
            if (desc.m_enableMultisample)
            {
                // If enabled, get the max sample count for the selected targets:
                multisample.m_sampleCount = nes::GetMaxSampleCountForTargets(targets);
            }
        }
    }
    
    // Create the Pipeline:
    outPipeline = nes::Pipeline(device, outLayout, desc);

    // Debug Name
    const std::string debugName = pipelineNode["DebugName"].as<std::string>();
    outPipeline.SetDebugName(debugName);
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