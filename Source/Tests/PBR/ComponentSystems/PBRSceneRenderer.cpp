// PBRSceneRenderer.cpp
#include "PBRSceneRenderer.h"

#include "Components/LightComponents.h"
#include "Components/MeshComponent.h"
#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Graphics/DataUploader.h"
#include "Nessie/Graphics/Renderer.h"
#include "Nessie/Graphics/Shader.h"
#include "Nessie/Graphics/Texture.h"
#include "Nessie/World/Components/CameraComponent.h"
#include "Nessie/World/ComponentSystems/TransformSystem.h"

namespace pbr
{
    void PBRSceneRenderer::RegisterComponentTypes()
    {
        NES_REGISTER_COMPONENT(nes::TransformComponent);
        NES_REGISTER_COMPONENT(nes::CameraComponent);
        NES_REGISTER_COMPONENT(MeshComponent);
        NES_REGISTER_COMPONENT(PointLightComponent);
        NES_REGISTER_COMPONENT(DirectionalLightComponent);
    }

    bool PBRSceneRenderer::Init()
    {
        auto& device = nes::DeviceManager::GetRenderDevice();
        m_frames.resize(nes::Renderer::GetMaxFramesInFlight());

        // Load Application Settings
        std::filesystem::path path = NES_CONFIG_DIR;
        path /= "PBRAppSettings.yaml";

        YAML::Node file = YAML::LoadFile(path.string());
        if (!file)
        {
            NES_ERROR("Failed to load Application Settings!");
            return false;
        }

        CreateDescriptorPool(device);
        CreateGraphicsResources(device);
        CreateAndLoadDefaultAssets(device, file);
        CreateDescriptorSets(device);
        
        return true;
    }

    void PBRSceneRenderer::Shutdown()
    {
        m_colorTarget = nullptr;
        m_depthTarget = nullptr;
        m_frames.clear();

        m_textureSampler = nullptr;
        m_depthSampler = nullptr;
    
        m_shadowPipelineLayout = nullptr;
        m_shadowPipeline = nullptr;
        m_shadowSampledImageView = nullptr;

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

    void PBRSceneRenderer::ProcessEnabledEntities()
    {
        auto& registry = GetRegistry();
        
        // Handle Camera Activation:
        {
            auto view = registry.GetAllEntitiesWith<nes::IDComponent, nes::PendingEnable, nes::CameraComponent>(entt::exclude<nes::DisabledComponent>);

            // [TODO]: Check whether it should be set active on enable.
            for (auto entity : view)
            {
                const auto id = view.get<nes::IDComponent>(entity).GetID();

                if (m_activeCameraID == nes::kInvalidEntityID || m_activeCameraID != id)
                {
                    m_activeCameraID = id;
                }
            }
        }
        
        // Handle enabled Entities with Meshes:
        {
            auto view = registry.GetAllEntitiesWith<nes::IDComponent, nes::PendingEnable, nes::TransformComponent, MeshComponent>();
            for (auto entity : view)
            {
                // Register a new Mesh geometry if not already:
                auto& meshComp = view.get<MeshComponent>(entity);
                auto pMesh = nes::AssetManager::GetAsset<MeshAsset>(meshComp.m_meshID);
                if (!m_scene.m_idToMeshIndex.contains(meshComp.m_meshID))
                {
                    if (pMesh != nullptr)
                        RegisterMeshAsset(pMesh);
                }
                
                if (meshComp.m_materialID == nes::kInvalidAssetID)
                {
                    // Get the default material for the asset.
                    meshComp.m_materialID = pMesh->GetDefaultMaterialID();

                    // Default Material if none present:
                    if (meshComp.m_materialID == nes::kInvalidAssetID)
                        meshComp.m_materialID = GetDefaultMaterialID();
                }

                // Register a new Material data if not already:
                if (!m_scene.m_idToMaterialIndex.contains(meshComp.m_materialID))
                {
                    auto pMaterial = nes::AssetManager::GetAsset<PBRMaterial>(meshComp.m_materialID);
                    if (pMaterial != nullptr)
                        RegisterMaterialAsset(pMaterial);
                }

                // Add the instance to our array.
                EntityInstance instance
                {
                    .m_entity = entity,
                    .m_meshIndex = m_scene.m_idToMeshIndex.at(meshComp.m_meshID),
                    .m_materialIndex = m_scene.m_idToMaterialIndex.at(meshComp.m_materialID)
                };
                m_scene.m_instances.emplace_back(instance);
                m_scene.m_entityToInstanceMap.emplace(entity, static_cast<uint32>(m_scene.m_instances.size() - 1));
            }
        }

        // Handle Enabled Point Lights
        {
            auto view = registry.GetAllEntitiesWith<nes::PendingEnable, PointLightComponent>();
            for ([[maybe_unused]] auto entity : view)
            {
                m_scene.m_pointLights.emplace_back();
            }
        }
        
        // Handle Enabled Directional Lights
        {
            auto view = registry.GetAllEntitiesWith<nes::PendingEnable, DirectionalLightComponent>();
            for ([[maybe_unused]] auto entity : view)
            {
                m_scene.m_directionalLights.emplace_back();
            }
        }
    }

    void PBRSceneRenderer::ProcessDisabledEntities()
    {
        // [TODO]: 
        //auto& registry = GetRegistry();
        
        // [TODO]: If a CameraComponent is disabled, and it is my Active Camera, print an Error? No Active Camera? Select the next active Camera?
        // [TODO]: Disable PointLights.
        // [TODO]: Disable DirectionalLights.
    }

    void PBRSceneRenderer::ProcessDestroyedEntities(const bool destroyingWorld)
    {
        if (!destroyingWorld)
        {
            // [TODO]: If a Mesh Component is destroyed, and there are no other entities using that mesh, remove it?
        }
    }

    void PBRSceneRenderer::ResizeRenderTargets(const uint32 width, const uint32 height)
    {
        // Resize the MSAA Set
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
                .SetAccess(nes::EAccessBits::None, nes::EAccessBits::ResolveSource)
                .SetBarrierStage(nes::EPipelineStageBits::TopOfPipe, nes::EPipelineStageBits::Copy);

            nes::ImageBarrierDesc depthBarrier = nes::ImageBarrierDesc()
                .SetImage(&depthImage, nes::EImagePlaneBits::Depth | nes::EImagePlaneBits::Stencil)
                .SetLayout(nes::EImageLayout::Undefined, nes::EImageLayout::DepthStencilAttachment);

            nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
                .SetImageBarriers({imageBarrier, depthBarrier});
        
            commandBuffer.SetBarriers(barrierGroup);

            nes::Renderer::SubmitAndWaitTempCommands(commandBuffer);
        }
    }

    void PBRSceneRenderer::RenderScene(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
    {
        // No Camera!
        if (m_activeCameraID == nes::kInvalidEntityID)
            return;

        auto& device = nes::DeviceManager::GetRenderDevice();
        BuildSceneData(device, commandBuffer);
        UpdateUniformBuffers(context);
    
        // Shadow Pass
        RenderShadows(commandBuffer, context);

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
    }

    bool PBRSceneRenderer::CreateAndLoadDefaultAssets(nes::RenderDevice& device, const YAML::Node& file)
    {
        // Load the Asset Pack of default assets and shaders.
        {
            const auto& assets = file["Assets"];
            nes::AssetPack pack{};
            if (!nes::AssetPack::LoadFromYAML(assets, pack))
            {
                NES_ERROR("Failed to load default Asset Pack!");
                return false;
            }

            // Load all the assets immediately.
            if (nes::AssetManager::LoadAssetPackSync(pack) != nes::ELoadResult::Success)
            {
                NES_ERROR("Failed to initialize SceneRenderer! Failed to load Assets in default Asset Pack!");
                return false;
            }
        }

        // Shaders are loaded, Create the render targets and pipelines:
        CreateRenderTargetsAndPipelines(device, file);

        // Set the Default AssetIDs:
        {
            s_cubeMeshID = 1;
            s_planeMeshID = 2;
            s_sphereMeshID = 3;
            
            const auto& defaultAssetIDs = file["DefaultAssetIDs"];
            s_errorTextureID = defaultAssetIDs["ErrorTextureID"].as<uint64>();
            s_blackTextureID = defaultAssetIDs["BlackTextureID"].as<uint64>();
            s_whiteTextureID = defaultAssetIDs["WhiteTextureID"].as<uint64>();
            s_flatNormalTextureID = defaultAssetIDs["FlatNormalTextureID"].as<uint64>();
            s_defaultMaterialID = defaultAssetIDs["DefaultMaterialID"].as<uint64>();
            s_defaultSkyboxID = defaultAssetIDs["DefaultSkyboxID"].as<uint64>();
            
            // Set our default skybox to the scene.
            m_scene.m_skyboxTextureID = s_defaultSkyboxID;

            // Register the Default Textures:
            auto pTexture = nes::AssetManager::GetAsset<nes::Texture>(s_errorTextureID);
            RegisterTextureAsset(device, pTexture);

            pTexture = nes::AssetManager::GetAsset<nes::Texture>(s_blackTextureID);
            RegisterTextureAsset(device, pTexture);

            pTexture = nes::AssetManager::GetAsset<nes::Texture>(s_whiteTextureID);
            RegisterTextureAsset(device, pTexture);

            pTexture = nes::AssetManager::GetAsset<nes::Texture>(s_flatNormalTextureID);
            RegisterTextureAsset(device, pTexture);

            // Create the Descriptor for the Skybox, and add it to the scene.
            auto pTextureCube = nes::AssetManager::GetAsset<nes::TextureCube>(s_defaultSkyboxID);
            NES_ASSERT(pTextureCube);
            RegisterTextureCubeAsset(device, pTextureCube);
            
            // Register the Default Material.
            auto pMaterial = nes::AssetManager::GetAsset<pbr::PBRMaterial>(s_defaultMaterialID);
            NES_ASSERT(pMaterial);
            RegisterMaterialAsset(pMaterial);
        }
        
        // Create the Default Meshes:
        Mesh sceneMesh;
        m_scene.m_indices.reserve(6400);
        m_scene.m_vertices.reserve(6400);

        std::vector<Vertex> vertices;
        std::vector<uint32> indices;
        indices.reserve(6400);
        vertices.reserve(6400);

        // Cube:
        {
            helpers::AppendCubeMeshData(vertices, indices, sceneMesh);
            MeshAsset asset(&vertices[sceneMesh.m_firstVertex], sceneMesh.m_vertexCount, &indices[sceneMesh.m_firstIndex], sceneMesh.m_indexCount, s_defaultMaterialID);
            nes::AssetManager::AddMemoryAsset<MeshAsset>(s_cubeMeshID, std::move(asset));

            auto pAsset = nes::AssetManager::GetAsset<MeshAsset>(s_cubeMeshID);
            RegisterMeshAsset(pAsset);
            
            //m_scene.m_meshes.emplace_back(sceneMesh);
            //m_scene.m_idToMeshIndex.emplace(s_cubeMeshID, static_cast<uint32>(m_scene.m_meshes.size() - 1));
        }

        // Sphere:
        {
            helpers::SphereGenDesc desc;
            desc.m_latitudeBands = 30.f;
            desc.m_longitudeBands = 30.f;
            desc.m_radius = 0.5f;
            
            helpers::AppendSphereMeshData(desc, m_scene.m_vertices, m_scene.m_indices, sceneMesh);
            MeshAsset asset(&m_scene.m_vertices[sceneMesh.m_firstVertex], sceneMesh.m_vertexCount, &m_scene.m_indices[sceneMesh.m_firstIndex], sceneMesh.m_indexCount, s_defaultMaterialID);
            nes::AssetManager::AddMemoryAsset<MeshAsset>(s_sphereMeshID, std::move(asset));

            auto pAsset = nes::AssetManager::GetAsset<MeshAsset>(s_sphereMeshID);
            RegisterMeshAsset(pAsset);
            //m_scene.m_meshes.emplace_back(sceneMesh);
            //m_scene.m_idToMeshIndex.emplace(s_sphereMeshID, static_cast<uint32>(m_scene.m_meshes.size() - 1));
        }

        // Plane:
        {
            helpers::PlaneGenDesc desc;
            desc.m_width = 10.f;
            desc.m_height = 10.f;
            desc.m_subdivisionsX = 10;
            desc.m_subdivisionsZ = 10;
            
            helpers::AppendPlaneData(desc, m_scene.m_vertices, m_scene.m_indices, sceneMesh);
            MeshAsset asset(&m_scene.m_vertices[sceneMesh.m_firstVertex], sceneMesh.m_vertexCount, &m_scene.m_indices[sceneMesh.m_firstIndex], sceneMesh.m_indexCount, s_defaultMaterialID);
            nes::AssetManager::AddMemoryAsset<MeshAsset>(s_planeMeshID, std::move(asset));

            auto pAsset = nes::AssetManager::GetAsset<MeshAsset>(s_planeMeshID);
            RegisterMeshAsset(pAsset);
            
            //m_scene.m_meshes.emplace_back(sceneMesh);
            //m_scene.m_idToMeshIndex.emplace(s_planeMeshID, static_cast<uint32>(m_scene.m_meshes.size() - 1));
        }

        return true;
    }

    void PBRSceneRenderer::CreateRenderTargetsAndPipelines(nes::RenderDevice& device, const YAML::Node& file)
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

        // PBR Geometry Pipeline
        {
            path = NES_CONTENT_DIR;
            path /= pipelines["PBR"].as<std::string>();

            YAML::Node pipelineFile = YAML::LoadFile(path.string());
            NES_ASSERT(pipelineFile);
            auto graphicsPipeline = pipelineFile["GraphicsPipeline"];
            NES_ASSERT(graphicsPipeline);
            LoadGraphicsPipeline(graphicsPipeline, device, m_pbrPipelineLayout, m_pbrPipeline);
        }

        // Shadow Pipeline:
        {
            auto shadowSettings = file["ShadowSettings"];
            uint32 minBits = shadowSettings["FormatMinBits"].as<uint32>(32);
            m_shadowImageFormat = device.GetSupportedDepthFormat(minBits, false);
            m_shadowMapResolution = shadowSettings["ImageResolution"].as<uint32>(2048);
            m_shadowCascadeCount = shadowSettings["NumCascades"].as<uint32>(1);
            m_shadowMaxDistance = shadowSettings["MaxShadowDistance"].as<float>(100.f);
            m_shadowCascadeSplitLambda = shadowSettings["CascadeSplitLambda"].as<float>(0.5f);
            m_shadowDepthBiasConstant = shadowSettings["DepthBiasConstant"].as<float>(1.25f);
            m_shadowDepthBiasSlope = shadowSettings["DepthBiasSlope"].as<float>(1.75f);

            auto shaderID = shadowSettings["DepthShader"].as<uint64>(nes::kInvalidAssetID.GetValue());
            CreateDepthPassResources(device, shaderID);
        }
    }

    void PBRSceneRenderer::CreateDepthPassResources(nes::RenderDevice& device, const nes::AssetID& shaderID)
    {
        // Allocate the Depth Image, with each layer being a new cascade.
        {
            nes::ImageDesc imageDesc{};
            imageDesc.m_type = nes::EImageType::Image2D;
            imageDesc.m_usage = nes::EImageUsageBits::DepthStencilAttachment | nes::EImageUsageBits::ShaderResource;
            imageDesc.m_format = m_shadowImageFormat;
            imageDesc.m_width = m_shadowMapResolution;
            imageDesc.m_height = m_shadowMapResolution;
            imageDesc.m_depth = 1;
            imageDesc.m_sampleCount = 1;
            imageDesc.m_layerCount = m_shadowCascadeCount;
            imageDesc.m_clearValue = nes::ClearDepthStencilValue(1.0, 0);
            
            nes::AllocateImageDesc allocDesc{};
            allocDesc.m_imageDesc = imageDesc;
            allocDesc.m_memoryLocation = nes::EMemoryLocation::Device;
            m_shadowMap = nes::DeviceImage(device, allocDesc);
        }

        // Full depth map view (all layers)
        {
            nes::Image2DViewDesc imageViewDesc{};
            imageViewDesc.m_layerCount = m_shadowCascadeCount;
            imageViewDesc.m_pImage = &m_shadowMap;
            imageViewDesc.m_viewType = nes::EImage2DViewType::ShaderResource2DArray;
            imageViewDesc.m_format = m_shadowImageFormat;
            m_shadowSampledImageView = nes::Descriptor(device, imageViewDesc);
        }

        // Image View Per Cascade for the depth pass
        {
            for (uint32 i = 0; i < m_shadowCascadeCount; ++i)
            {
                nes::Image2DViewDesc imageViewDesc{};
                imageViewDesc.m_baseLayer = i;
                imageViewDesc.m_layerCount = 1;
                imageViewDesc.m_pImage = &m_shadowMap;
                imageViewDesc.m_viewType = nes::EImage2DViewType::DepthStencilAttachment;
                imageViewDesc.m_format = m_shadowImageFormat;
                m_shadowImageViews.emplace_back(device, imageViewDesc);
            }
        }
        
        // Shadow Pipeline Layout
        {
            nes::DescriptorBindingDesc binding = nes::DescriptorBindingDesc()
                .SetShaderStages(nes::EPipelineStageBits::VertexShader)
                .SetBindingIndex(0)
                .SetDescriptorType(nes::EDescriptorType::UniformBuffer);
            
            nes::DescriptorSetDesc descriptorSetDesc = nes::DescriptorSetDesc()
                .SetBindings(&binding, 1);

            nes::PushConstantDesc pushConstantDesc{};
            pushConstantDesc.m_offset = 0;
            pushConstantDesc.m_size = sizeof(DepthPassPushConstants);
            pushConstantDesc.m_shaderStages = nes::EPipelineStageBits::VertexShader;
            
            nes::PipelineLayoutDesc pipelineLayoutDesc = nes::PipelineLayoutDesc()
                .SetDescriptorSets(descriptorSetDesc)
                .SetPushConstants(pushConstantDesc)
                .SetShaderStages(nes::EPipelineStageBits::VertexShader);

            m_shadowPipelineLayout = nes::PipelineLayout(device, pipelineLayoutDesc);
        }

        // Shadow Pipeline
        {
            // Shader Stages:
            nes::AssetPtr<nes::Shader> shader = nes::AssetManager::GetAsset<nes::Shader>(shaderID);
            NES_ASSERT(shader, "Failed to create Pipeline! Shader not present!");
            auto shaderStages = shader->GetGraphicsShaderStages();

            // Vertex Input
            auto vertexAttributes = Vertex::GetBindingDescs();

            nes::VertexStreamDesc vertexStreamDesc = nes::VertexStreamDesc()
                .SetBinding(0)
                .SetStepRate(nes::EVertexStreamStepRate::PerVertex)
                .SetStride(sizeof(Vertex));
            
            nes::VertexInputDesc vertexInputDesc = nes::VertexInputDesc()
                .SetAttributes(vertexAttributes)
                .SetStreams(vertexStreamDesc);

            // Input Assembly
            nes::InputAssemblyDesc inputAssemblyDesc{};
            inputAssemblyDesc.m_topology = nes::ETopology::TriangleList;

            // Rasterizer:
            nes::RasterizationDesc rasterDesc = {};
            rasterDesc.m_cullMode = nes::ECullMode::None;
            rasterDesc.m_enableDepthClamp = false;
            rasterDesc.m_fillMode = nes::EFillMode::Solid;
            rasterDesc.m_frontFace = nes::EFrontFaceWinding::CounterClockwise;
            rasterDesc.m_depthBias.m_enabled = true;

            // Output Merger
            nes::OutputMergerDesc outputMergerDesc{};
            outputMergerDesc.m_colorCount = 0;
            outputMergerDesc.m_pColors = nullptr;
            outputMergerDesc.m_depth.m_compareOp = nes::ECompareOp::Less;
            outputMergerDesc.m_depth.m_enableWrite = true;
            outputMergerDesc.m_depthStencilFormat = m_shadowImageFormat;

            // Create the pipeline:
            nes::GraphicsPipelineDesc pipelineDesc = nes::GraphicsPipelineDesc()
                .SetShaderStages(shaderStages)
                .SetVertexInput(vertexInputDesc)
                .SetInputAssemblyDesc(inputAssemblyDesc)
                .SetRasterizationDesc(rasterDesc)
                .SetOutputMergerDesc(outputMergerDesc);

            m_shadowPipeline = nes::Pipeline(device, m_shadowPipelineLayout, pipelineDesc);
        }
    }

    void PBRSceneRenderer::CreateGraphicsResources(nes::RenderDevice& device)
    {
        // Texture Sampler Descriptor
        {
            nes::SamplerDesc samplerDesc{};
            samplerDesc.m_addressModes = {nes::EAddressMode::ClampToEdge, nes::EAddressMode::ClampToEdge, nes::EAddressMode::ClampToEdge};
            samplerDesc.m_filters = {nes::EFilterType::Linear, nes::EFilterType::Linear, nes::EFilterType::Linear};
            samplerDesc.m_anisotropy = static_cast<uint8>(device.GetDesc().m_other.m_maxSamplerAnisotropy);
            samplerDesc.m_mipMax = 16.f;
            m_textureSampler = nes::Descriptor(device, samplerDesc);
        }

        // Depth Sampler Descriptor
        {
            nes::SamplerDesc samplerDesc{};
            samplerDesc.m_addressModes = {nes::EAddressMode::ClampToEdge, nes::EAddressMode::ClampToEdge, nes::EAddressMode::ClampToEdge};
            samplerDesc.m_filters = {nes::EFilterType::Linear, nes::EFilterType::Linear, nes::EFilterType::Linear};
            samplerDesc.m_anisotropy = static_cast<uint8>(1.f);
            samplerDesc.m_mipMin = 0.f;
            samplerDesc.m_mipMax = 1.f;
            samplerDesc.m_mipBias = 0.f;
            samplerDesc.m_compareOp = nes::ECompareOp::None;
            samplerDesc.m_borderColor = nes::ClearColorValue(1.f, 1.f, 1.f, 1.f);
            m_depthSampler = nes::Descriptor(device, samplerDesc);
        }
        
        // Globals Buffer: Contains CameraUBO + LightCountUBO + ShadowUBO. 
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
            static constexpr uint32 kMaxIndices = 64'000;
            
            nes::AllocateBufferDesc desc;
            desc.m_size = kMaxIndices * sizeof(uint32); //static_cast<uint32>(m_scene.m_indices.size()) * sizeof(uint32);
            desc.m_usage = nes::EBufferUsageBits::IndexBuffer;
            desc.m_location = nes::EMemoryLocation::Device;
            m_indicesBuffer = nes::DeviceBuffer(device, desc);
            m_indicesBuffer.SetDebugName("Indices Buffer");
        }

        // Vertex Device Buffer
        {
            static constexpr uint32 kMaxVertices = 6'400;
            
            nes::AllocateBufferDesc desc;
            desc.m_size = kMaxVertices * sizeof(Vertex); //static_cast<uint32>(m_scene.m_vertices.size()) * sizeof(Vertex);
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

                static constexpr uint32 kMaxMaterials = 64;
            
                nes::AllocateBufferDesc desc;
                desc.m_size = static_cast<uint32>(kMaxMaterials * sizeof(MaterialUBO));
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

    void PBRSceneRenderer::CreateDescriptorPool(nes::RenderDevice& device)
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

    void PBRSceneRenderer::CreateDescriptorSets(nes::RenderDevice& device)
    {
        nes::Descriptor* pTextureSampler = &m_textureSampler;
    
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

        nes::BufferViewDesc shadowDataView{};
        shadowDataView.m_viewType = nes::EBufferViewType::Uniform;
        shadowDataView.m_pBuffer = &m_globalsBuffer;
        shadowDataView.m_size = sizeof(CascadedShadowMapsUBO);

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

            // Global Buffer: CameraUBO + LightCountUBO + ShadowUBO for each frame.
            {
                // Set the offsets in the globals buffer.
                // - All are 64 byte aligned.
                cameraView.m_offset = i * kGlobalUBOElementSize; 
                lightCountView.m_offset = cameraView.m_offset + sizeof(CameraUBO);
                shadowDataView.m_offset = lightCountView.m_offset + sizeof(LightCountUBO);
                frame.m_cameraBufferOffset = cameraView.m_offset; 
                frame.m_lightCountOffset = lightCountView.m_offset;
                frame.m_shadowDataOffset = shadowDataView.m_offset;

                // Create the views:
                frame.m_cameraUBOView = nes::Descriptor(device, cameraView);
                frame.m_lightCountUBOView = nes::Descriptor(device, lightCountView);
                frame.m_shadowUBOView = nes::Descriptor(device, shadowDataView);
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
            
                frame.m_lightDataSet.UpdateBindings(updateDescs.data(), 0, static_cast<uint32>(updateDescs.size()));
            }

            // Shadow Pass Data Set: Used only by the Shadow pass.
            {
                m_descriptorPool.AllocateDescriptorSets(m_shadowPipelineLayout, 0, &m_frames[i].m_shadowPassDataSet);
            
                nes::Descriptor* pShadowUBO = &frame.m_shadowUBOView;
                nes::DescriptorBindingUpdateDesc updateDesc(&pShadowUBO, 1);
                frame.m_shadowPassDataSet.UpdateBindings(&updateDesc, 0, 1);
            }

            // PBR Shadow Data Set: Used only in the fragment shader. Used to sample the shadow map.
            {
                m_descriptorPool.AllocateDescriptorSets(m_pbrPipelineLayout, 3, &m_frames[i].m_sampledShadowDataSet);
            
                nes::Descriptor* pDepthSampler = &m_depthSampler;
                nes::Descriptor* pShadowMapImage = &m_shadowSampledImageView;
                nes::Descriptor* pShadowData = &frame.m_shadowUBOView; 
            
                std::array shadowUpdateDescs
                {
                    nes::DescriptorBindingUpdateDesc(&pDepthSampler, 1),
                    nes::DescriptorBindingUpdateDesc(&pShadowMapImage, 1),
                    nes::DescriptorBindingUpdateDesc(&pShadowData, 1),
                };
                frame.m_sampledShadowDataSet.UpdateBindings(shadowUpdateDescs.data(), 0, static_cast<uint32>(shadowUpdateDescs.size()));
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
                nes::DescriptorBindingUpdateDesc(&pTextureSampler, 1),
                nes::DescriptorBindingUpdateDesc(&pSkyboxTexture, 1),
            };
            m_skyboxDescriptorSet.UpdateBindings(updateDescs.data(), 0, static_cast<uint32>(updateDescs.size()));
        }
    }

    void PBRSceneRenderer::UpdateUniformBuffers(const nes::RenderFrameContext& context)
    {
        auto& registry = GetRegistry();
        auto& frame = m_frames[context.GetFrameIndex()];
    
        const auto viewport = context.GetSwapchainViewport();
        const float aspectRatio = viewport.m_extent.x / viewport.m_extent.y;

        // Update Camera Data:
        CameraUBO cameraConstants;
        {
            auto activeCameraEntity = registry.GetEntity(m_activeCameraID);
            nes::CameraComponent& camera = registry.GetComponent<nes::CameraComponent>(activeCameraEntity);
            nes::TransformComponent& transform = registry.GetComponent<nes::TransformComponent>(activeCameraEntity);
    
            const auto worldPosition = transform.GetWorldPosition();
            cameraConstants.m_position = nes::Float3(worldPosition.x, worldPosition.y, worldPosition.z);

            const nes::Vec3 forward = transform.GetWorldTransformMatrix().GetForward();
            const nes::Vec3 up = transform.GetWorldTransformMatrix().GetUp();
    
            cameraConstants.m_view = nes::Mat44::LookAt(worldPosition, worldPosition + forward, up);
            cameraConstants.m_projection = camera.CalculateProjectionMatrix(static_cast<uint32>(viewport.m_extent.x), static_cast<uint32>(viewport.m_extent.y));
            cameraConstants.m_viewProjection = cameraConstants.m_projection * cameraConstants.m_view;
            cameraConstants.m_exposureFactor = camera.CalculateExposureFactor();

            m_globalsBuffer.CopyToMappedMemory(&cameraConstants, frame.m_cameraBufferOffset, sizeof(CameraUBO));
        }
    
        // Update Lighting Data:
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
                frame.m_pointLightsBuffer.CopyToMappedMemory(m_scene.m_pointLights.data(), 0, m_scene.m_pointLights.size() * sizeof(PointLight));
            }
        }

        // Update Shadow Cascades:
        {
            NES_ASSERT(!m_scene.m_directionalLights.empty());

            GenShadowCascadesDesc desc;
            desc.m_shadowMapResolution = static_cast<float>(m_shadowMapResolution);
            desc.m_cameraNear = 0.5f;//m_camera.m_nearPlane;
            desc.m_cameraFar = m_shadowMaxDistance;
            desc.m_cameraView = cameraConstants.m_view;

            // Projection matrix with altered near/far plane:
            desc.m_cameraProj = nes::Mat44::Perspective(nes::math::ToRadians(45.f), aspectRatio, desc.m_cameraNear, desc.m_cameraFar);
            desc.m_numCascades = m_shadowCascadeCount;
            desc.m_splitLambda = m_shadowCascadeSplitLambda;
            CascadedShadowMapsUBO csm = helpers::GenerateShadowCascadesForLight(m_scene.m_directionalLights[0], desc);

            m_globalsBuffer.CopyToMappedMemory(&csm, frame.m_shadowDataOffset, sizeof(CascadedShadowMapsUBO));
        }
        
        // Update Material Data:
        if (!m_scene.m_materials.empty())
        {
            frame.m_materialUBOBuffer.CopyToMappedMemory(m_scene.m_materials.data());
        }
    }

    void PBRSceneRenderer::BuildSceneData(nes::RenderDevice& device, nes::CommandBuffer& commandBuffer)
    {
        // Update all model matrices for the instances:
        auto& registry = GetRegistry();
        for (auto& instance : m_scene.m_instances)
        {
            auto& transform = registry.GetComponent<nes::TransformComponent>(instance.m_entity);
            instance.m_model = transform.GetWorldTransformMatrix();
        }

        // Point Light data:
        {
            auto view = registry.GetAllEntitiesWith<PointLightComponent, nes::TransformComponent>(entt::exclude<nes::DisabledComponent>);
            m_scene.m_pointLights.clear();

            for (auto entity : view)
            {
                auto& lightComp = view.get<PointLightComponent>(entity);
                auto& transform = view.get<nes::TransformComponent>(entity);

                PointLight light{};
                const nes::Vec3 position = transform.GetWorldPosition();
                light.m_position = nes::Float3(position.x, position.y, position.z);
                light.m_color = nes::Float3(lightComp.m_color.r, lightComp.m_color.g, lightComp.m_color.b);
                light.m_intensity = lightComp.m_intensity;
                light.m_radius = lightComp.m_radius;
                m_scene.m_pointLights.push_back(light);
            }
        }

        // Directional Light data:
        {
            auto view = registry.GetAllEntitiesWith<DirectionalLightComponent>(entt::exclude<nes::DisabledComponent>);
            m_scene.m_directionalLights.clear();

            for (auto entity : view)
            {
                auto& lightComp = view.get<DirectionalLightComponent>(entity);

                DirectionalLight light{};
                light.m_direction = nes::Float3(lightComp.m_direction.x, lightComp.m_direction.y, lightComp.m_direction.z);
                light.m_color = nes::Float3(lightComp.m_color.r, lightComp.m_color.g, lightComp.m_color.b);
                light.m_intensity = lightComp.m_intensity;
                m_scene.m_directionalLights.push_back(light);
            }
        }

        // [TODO]: Should probably add to a member Uploader variable, and 
        // Upload the geometry data to the GPU:
        {
            nes::DataUploader uploader(device);
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
            uploader.RecordCommands(commandBuffer);
            //nes::Renderer::SubmitAndWaitTempCommands(cmdBuffer);
        }
    }

    void PBRSceneRenderer::RenderShadows(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
    {
        auto& registry = GetRegistry();
        auto& depthImage = m_shadowMap;
    
        // Transition the Shadow Target's image to the DepthStencilAttachment.
        {
            nes::ImageBarrierDesc imageBarrier = nes::ImageBarrierDesc()
                .SetImage(&depthImage, nes::EImagePlaneBits::Depth)
                .SetRegion(nes::EImagePlaneBits::Depth, 0, 1, 0, m_shadowCascadeCount)
                .SetLayout(nes::EImageLayout::Undefined, nes::EImageLayout::DepthStencilAttachment)
                .SetBarrierStage(nes::EPipelineStageBits::None, nes::EPipelineStageBits::DepthStencilAttachment)
                .SetAccess(nes::EAccessBits::None, nes::EAccessBits::DepthStencilAttachmentWrite);

            nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
                .SetImageBarriers({ imageBarrier } );
        
            commandBuffer.SetBarriers(barrierGroup);
        }
    
        auto& frame = m_frames[context.GetFrameIndex()];

        // Render the scene into the depth image layer
        for (uint32 i = 0; i < m_shadowCascadeCount; ++i)
        {
            // Set the Shadow Image as our depth target.
            nes::RenderTargetsDesc renderTargetsDesc = nes::RenderTargetsDesc()
                 .SetDepthStencilTarget(&m_shadowImageViews[i]);

            // Record Render Commands:
            commandBuffer.BeginRendering(renderTargetsDesc);
            {
                // Clear the Color and Depth Targets:
                const nes::ClearDesc depthClear = nes::ClearDesc::DepthStencil(depthImage.GetDesc().m_clearValue);
                commandBuffer.ClearRenderTargets({ depthClear });

                // Set the viewport and scissor to encompass the Shadow Map Image:
                const nes::Viewport viewport(m_shadowMapResolution, m_shadowMapResolution);
                const nes::Scissor scissor(viewport);
                commandBuffer.SetViewports(viewport);
                commandBuffer.SetScissors(scissor);

                // Bind the Shadow data:
                commandBuffer.BindPipelineLayout(m_shadowPipelineLayout);
                commandBuffer.BindPipeline(m_shadowPipeline);
                commandBuffer.BindDescriptorSet(0, frame.m_shadowPassDataSet);
                commandBuffer.SetDepthBias(m_shadowDepthBiasConstant, m_shadowDepthBiasSlope, 0.f);
            
                DepthPassPushConstants pushConstants{};
                pushConstants.m_cascadeIndex = i;

                // Bind the index buffer for the entire range:
                nes::IndexBufferRange indexBuffer = nes::IndexBufferRange(&m_indicesBuffer, m_scene.m_indices.size(), 0);
                commandBuffer.BindIndexBuffer(indexBuffer);
        
                for (auto& instance : m_scene.m_instances)
                {
                    if (!registry.IsValidEntity(instance.m_entity))
                        continue;
                    
                    // Push the object's position and the cascade index:
                    pushConstants.m_model = instance.m_model;
                    commandBuffer.SetPushConstant(0, &pushConstants, sizeof(DepthPassPushConstants));
                
                    // Bind Mesh Vertex Buffer.
                    const Mesh& mesh = m_scene.m_meshes[instance.m_meshIndex];
                    nes::VertexBufferRange meshVertexBuffer(&m_verticesBuffer, sizeof(Vertex), mesh.m_vertexCount, mesh.m_firstVertex * sizeof(Vertex));
                    commandBuffer.BindVertexBuffers({ meshVertexBuffer}, 0);
                
                    // Draw
                    nes::DrawIndexedDesc drawDesc{};
                    drawDesc.m_firstIndex = mesh.m_firstIndex;
                    drawDesc.m_indexCount = mesh.m_indexCount;
                    commandBuffer.DrawIndexed(drawDesc);
                }

                // Finish.
                commandBuffer.EndRendering();
            }
        }

        // Transition the Shadow Target's image to be accessed by the geometry shader. 
        {
            nes::ImageBarrierDesc imageBarrier = nes::ImageBarrierDesc()
                .SetImage(&depthImage, nes::EImagePlaneBits::Depth)
                .SetRegion(nes::EImagePlaneBits::Depth, 0, 1, 0, m_shadowCascadeCount)
                .SetLayout(nes::EImageLayout::DepthStencilAttachment, nes::EImageLayout::ShaderResource)
                .SetAccess(nes::EAccessBits::DepthStencilAttachmentWrite, nes::EAccessBits::ShaderResourceRead);

            nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
                .SetImageBarriers(imageBarrier);
        
            commandBuffer.SetBarriers(barrierGroup);
        }
    }

    void PBRSceneRenderer::RenderSkybox(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
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

    void PBRSceneRenderer::RenderInstances(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
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
        commandBuffer.BindDescriptorSet(3, frame.m_sampledShadowDataSet);

        // Bind the index buffer for the entire range:
        nes::IndexBufferRange indexBuffer = nes::IndexBufferRange(&m_indicesBuffer, m_scene.m_indices.size(), 0);
        commandBuffer.BindIndexBuffer(indexBuffer);

        for (const auto& instance : m_scene.m_instances)
        {
            // Push the instance data:
            InstanceUBO object;
            object.SetTransform(instance.m_model);
            object.SetMesh(instance.m_meshIndex);
            object.SetMaterial(instance.m_materialIndex);
            commandBuffer.SetPushConstant(0, &object, sizeof(InstanceUBO));

            // Bind the Material Textures:
            commandBuffer.BindDescriptorSet(4, m_materialDescriptorSets[instance.m_materialIndex]);

            // Bind Mesh Vertex Buffer.
            const Mesh& mesh = m_scene.m_meshes[instance.m_meshIndex];
            nes::VertexBufferRange meshVertexBuffer(&m_verticesBuffer, sizeof(Vertex), mesh.m_vertexCount, mesh.m_firstVertex * sizeof(Vertex));
            commandBuffer.BindVertexBuffers({ meshVertexBuffer}, 0);

            // Draw
            nes::DrawIndexedDesc drawDesc{};
            drawDesc.m_firstIndex = mesh.m_firstIndex;
            drawDesc.m_indexCount = mesh.m_indexCount;
            commandBuffer.DrawIndexed(drawDesc);
        }
    }

    void PBRSceneRenderer::RenderGrid(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context) const
    {
        NES_ASSERT(m_gridPipeline != nullptr);
        NES_ASSERT(m_gridPipelineLayout != nullptr);
        NES_ASSERT(m_frames.size() > context.GetFrameIndex());
    
        commandBuffer.BindPipelineLayout(m_gridPipelineLayout);
        commandBuffer.BindPipeline(m_gridPipeline);
        commandBuffer.BindDescriptorSet(0, m_frames[context.GetFrameIndex()].m_cameraSet);
        commandBuffer.DrawVertices(6);
    }

    void PBRSceneRenderer::RegisterMeshAsset(const nes::AssetPtr<MeshAsset>& pMesh)
    {
        NES_ASSERT(pMesh != nullptr);

        const auto id = pMesh->GetAssetID();
        if (m_scene.m_idToMeshIndex.contains(id))
            return;

        const auto& meshVertices = pMesh->GetVertices();
        const auto& meshIndices = pMesh->GetIndices();

        // Set the Index/Vertex information.
        Mesh sceneMesh;
        sceneMesh.m_firstVertex = static_cast<uint32>(m_scene.m_vertices.size());
        sceneMesh.m_firstIndex = static_cast<uint32>(m_scene.m_indices.size());
        sceneMesh.m_vertexCount = static_cast<uint32>(meshVertices.size());
        sceneMesh.m_indexCount = static_cast<uint32>(meshIndices.size());

        // Insert the data:
        m_scene.m_vertices.insert(m_scene.m_vertices.end(), meshVertices.begin(), meshVertices.end());
        m_scene.m_indices.insert(m_scene.m_indices.end(), meshIndices.begin(), meshIndices.end());
        m_scene.m_meshes.emplace_back(sceneMesh);
        m_scene.m_idToMeshIndex.emplace(id, static_cast<uint32>(m_scene.m_meshes.size() - 1));
    }

    void PBRSceneRenderer::RegisterMaterialAsset(nes::AssetPtr<PBRMaterial>& pMaterial)
    {
        NES_ASSERT(pMaterial != nullptr);
        auto& device = nes::DeviceManager::GetRenderDevice();

        const auto id = pMaterial->GetAssetID();
        if (m_scene.m_idToMaterialIndex.contains(id))
            return;

        const auto& materialDesc = pMaterial->GetDesc();

        // Base Color Map:
        if (materialDesc.m_baseColorMap != nes::kInvalidAssetID && !m_scene.m_idToTextureIndex.contains(materialDesc.m_baseColorMap))
        {
            auto pTexture = nes::AssetManager::GetAsset<nes::Texture>(materialDesc.m_baseColorMap);
            NES_ASSERT(pTexture != nullptr);
            RegisterTextureAsset(device, pTexture);
        }

        // Normal Map
        if (materialDesc.m_normalMap != nes::kInvalidAssetID && !m_scene.m_idToTextureIndex.contains(materialDesc.m_normalMap))
        {
            auto pTexture = nes::AssetManager::GetAsset<nes::Texture>(materialDesc.m_normalMap);
            NES_ASSERT(pTexture != nullptr);
            RegisterTextureAsset(device, pTexture);
        }

        // Material/Roughness Map
        if (materialDesc.m_roughnessMetallicMap != nes::kInvalidAssetID && !m_scene.m_idToTextureIndex.contains(materialDesc.m_roughnessMetallicMap))
        {
            auto pTexture = nes::AssetManager::GetAsset<nes::Texture>(materialDesc.m_roughnessMetallicMap);
            NES_ASSERT(pTexture != nullptr);
            RegisterTextureAsset(device, pTexture);
        }

        // Emissive Map
        if (materialDesc.m_emissionMap != nes::kInvalidAssetID && !m_scene.m_idToTextureIndex.contains(materialDesc.m_emissionMap))
        {
            auto pTexture = nes::AssetManager::GetAsset<nes::Texture>(materialDesc.m_emissionMap);
            NES_ASSERT(pTexture != nullptr);
            RegisterTextureAsset(device, pTexture);
        }

        // Create the Material Instance:
        MaterialUBO materialInstance{};
        materialInstance.m_baseColorScale = nes::Float3(materialDesc.m_baseColor.x, materialDesc.m_baseColor.y, materialDesc.m_baseColor.z);
        materialInstance.m_metallicScale = materialDesc.m_metallic;
        materialInstance.m_emissionScale = nes::Float3(materialDesc.m_emission.x, materialDesc.m_emission.y, materialDesc.m_emission.z);
        materialInstance.m_roughnessScale = materialDesc.m_roughness;

        // Base Color:
        if (materialDesc.m_baseColorMap != nes::kInvalidAssetID)
        {
            NES_ASSERT(m_scene.m_idToTextureIndex.contains(materialDesc.m_baseColorMap));
            materialInstance.m_baseColorIndex = m_scene.m_idToTextureIndex.at(materialDesc.m_baseColorMap);
        }

        // Normal:
        if (materialDesc.m_normalMap != nes::kInvalidAssetID)
        {
            NES_ASSERT(m_scene.m_idToTextureIndex.contains(materialDesc.m_normalMap));
            materialInstance.m_normalIndex = m_scene.m_idToTextureIndex.at(materialDesc.m_normalMap);
        }
        
        // Roughness Metallic:
        if (materialDesc.m_roughnessMetallicMap != nes::kInvalidAssetID)
        {
            NES_ASSERT(m_scene.m_idToTextureIndex.contains(materialDesc.m_roughnessMetallicMap));
            materialInstance.m_roughnessMetallicIndex = m_scene.m_idToTextureIndex.at(materialDesc.m_roughnessMetallicMap);
        }

        // Emission:
        if (materialDesc.m_emissionMap != nes::kInvalidAssetID)
        {
            NES_ASSERT(m_scene.m_idToTextureIndex.contains(materialDesc.m_emissionMap));
            materialInstance.m_emissionIndex = m_scene.m_idToTextureIndex.at(materialDesc.m_emissionMap);
        }
        
        m_scene.m_materials.emplace_back(materialInstance);
        m_scene.m_idToMaterialIndex.emplace(id, static_cast<uint32>(m_scene.m_materials.size() - 1));

        // Create the Material Descriptor Set:
        auto& descriptorSet = m_materialDescriptorSets.emplace_back(nullptr);
        m_descriptorPool.AllocateDescriptorSets(m_pbrPipelineLayout, 4, &descriptorSet, 1);
        
        nes::Descriptor* materialTextures[] =
        {
            &m_scene.m_textures[materialInstance.m_baseColorIndex],  
            &m_scene.m_textures[materialInstance.m_normalIndex],  
            &m_scene.m_textures[materialInstance.m_roughnessMetallicIndex],  
            &m_scene.m_textures[materialInstance.m_emissionIndex],  
        };

        nes::Descriptor* pTextureSampler = &m_textureSampler;
        
        std::array updateDescs =
        {
            nes::DescriptorBindingUpdateDesc(&pTextureSampler, 1),
            nes::DescriptorBindingUpdateDesc(materialTextures, 4),
        };
        
        descriptorSet.UpdateBindings(updateDescs.data(), 0, static_cast<uint32>(updateDescs.size()));
    }

    void PBRSceneRenderer::RegisterTextureAsset(nes::RenderDevice& device, const nes::AssetPtr<nes::Texture>& pTexture)
    {
        NES_ASSERT(pTexture != nullptr);

        auto& image = pTexture->GetDeviceImage();
        const auto& imageDesc = image.GetDesc();

        // Create the image view descriptor:
        nes::Image2DViewDesc imageViewDesc;
        imageViewDesc.m_pImage = &image;
        imageViewDesc.m_baseLayer = 0;
        imageViewDesc.m_layerCount = imageDesc.m_layerCount;
        imageViewDesc.m_baseMipLevel = 0;
        imageViewDesc.m_mipCount = static_cast<uint16>(imageDesc.m_mipCount);
        imageViewDesc.m_format = imageDesc.m_format;
        imageViewDesc.m_viewType = nes::EImage2DViewType::ShaderResource2D;
        m_scene.m_textures.emplace_back(device, imageViewDesc);
        m_scene.m_idToTextureIndex.emplace(pTexture->GetAssetID(), static_cast<uint32>(m_scene.m_textures.size() - 1));
    }

    void PBRSceneRenderer::RegisterTextureCubeAsset(nes::RenderDevice& device, const nes::AssetPtr<nes::TextureCube>& pTextureCube)
    {
        NES_ASSERT(pTextureCube != nullptr);
                
        auto& image = pTextureCube->GetDeviceImage();
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
        m_scene.m_textures.emplace_back(device, imageViewDesc);
        m_scene.m_idToTextureIndex.emplace(pTextureCube->GetAssetID(), static_cast<uint32>(m_scene.m_textures.size() - 1));
    }

    nes::RenderTarget PBRSceneRenderer::LoadColorRenderTarget(const YAML::Node& targetNode, const std::string& name, nes::RenderDevice& device, const nes::EFormat swapchainFormat, const nes::UInt2 swapchainExtent)
    {
        nes::RenderTargetDesc desc{};
        desc.m_name = name;
        desc.m_planes = nes::EImagePlaneBits::Color;

        // Format
        desc.m_format = static_cast<nes::EFormat>(targetNode["Format"].as<uint32>(0));
        if (desc.m_format == nes::EFormat::Unknown)
            desc.m_format = swapchainFormat;

        // Usage
        desc.m_usage = static_cast<nes::EImageUsageBits>(targetNode["Usage"].as<uint32>(0));
        desc.m_usage |= nes::EImageUsageBits::ColorAttachment;

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

    nes::RenderTarget PBRSceneRenderer::LoadDepthRenderTarget(const YAML::Node& targetNode, const std::string& name, nes::RenderDevice& device, const nes::UInt2 swapchainExtent)
    {
        nes::RenderTargetDesc desc{};
        desc.m_name = name;
        
        // Format
        const uint32 minBits = targetNode["FormatMinBits"].as<uint32>(16);
        const bool requireStencil = targetNode["FormatRequireStencil"].as<bool>(false);
        desc.m_format = device.GetSupportedDepthFormat(minBits, requireStencil);
        NES_ASSERT(desc.m_format != nes::EFormat::Unknown);

        // Usage
        desc.m_usage = static_cast<nes::EImageUsageBits>(targetNode["Usage"].as<uint32>(0));
        desc.m_usage |= nes::EImageUsageBits::DepthStencilAttachment;
    
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

    void PBRSceneRenderer::LoadGraphicsPipeline(const YAML::Node& pipelineNode, nes::RenderDevice& device, nes::PipelineLayout& outLayout, nes::Pipeline& outPipeline) const
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
            rasterState.m_enableDepthClamp = rasterizationNode["EnableDepthClamp"].as<bool>(false);
            rasterState.m_frontFace = static_cast<nes::EFrontFaceWinding>(rasterizationNode["FrontFace"].as<uint32>(kDefaultFrontFace));
            rasterState.m_lineWidth = rasterizationNode["LineWidth"].as<float>(1.0f);

            if (auto depthBiasNode = rasterizationNode["DepthBias"])
            {
                rasterState.m_depthBias.m_constant = depthBiasNode["Constant"].as<float>(0.f);
                rasterState.m_depthBias.m_clamp = depthBiasNode["Clamp"].as<float>(0.f);
                rasterState.m_depthBias.m_slope = depthBiasNode["Slope"].as<float>(0.f);
                rasterState.m_depthBias.m_enabled = depthBiasNode["Enabled"].as<bool>(false);
            }
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

    nes::AssetID PBRSceneRenderer::GetDefaultMeshID(const EDefaultMeshType type)
    {
        switch (type)
        {
            case EDefaultMeshType::Cube:    return s_cubeMeshID;
            case EDefaultMeshType::Plane:   return s_planeMeshID;
            case EDefaultMeshType::Sphere:  return s_sphereMeshID;
        }

        return nes::kInvalidAssetID;
    }

    nes::AssetID PBRSceneRenderer::GetDefaultTextureID(const EDefaultTextureType type)
    {
        switch (type)
        {
            case EDefaultTextureType::Error:        return s_errorTextureID;
            case EDefaultTextureType::Black:        return s_blackTextureID;
            case EDefaultTextureType::White:        return s_whiteTextureID;
            case EDefaultTextureType::FlatNormal:   return s_flatNormalTextureID;
        }

        return nes::kInvalidAssetID;
    }

    nes::AssetID PBRSceneRenderer::GetDefaultMaterialID()
    {
        return s_defaultMaterialID;
    }
}