// PBRSceneRenderer.cpp
#include "PBRSceneRenderer.h"

#include "Components/SkyboxComponent.h"
#include "Components/LightComponents.h"
#include "Components/MeshComponent.h"
#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Graphics/DataUploader.h"
#include "Nessie/Graphics/Renderer.h"
#include "Nessie/Graphics/Shader.h"
#include "Nessie/Graphics/Texture.h"
#include "Nessie/World/Components/CameraComponent.h"
#include "Nessie/World/ComponentSystems/TransformSystem.h"

#include "Nessie/FileIO/YAML/Serializers/YamlCoreSerializers.h"
#include "Nessie/FileIO/YAML/Serializers/YamlMathSerializers.h"
#include "Nessie/FileIO/YAML/Serializers/YamlGraphicsSerializers.h"

namespace pbr
{
    void PBRSceneRenderer::RegisterComponentTypes()
    {
        NES_REGISTER_COMPONENT(nes::TransformComponent);
        NES_REGISTER_COMPONENT(nes::CameraComponent);
        NES_REGISTER_COMPONENT(MeshComponent);
        NES_REGISTER_COMPONENT(PointLightComponent);
        NES_REGISTER_COMPONENT(DirectionalLightComponent);
        NES_REGISTER_COMPONENT(SkyboxComponent);
    }

    bool PBRSceneRenderer::Init()
    {
        auto& device = nes::DeviceManager::GetRenderDevice();
        m_frames.resize(nes::Renderer::GetMaxFramesInFlight());

        // Load Application Settings
        std::filesystem::path path = NES_CONFIG_DIR;
        path /= "PBRAppSettings.yaml";

        nes::YamlInStream file(path);
        if (!file.IsOpen())
        {
            NES_ERROR("Failed to load Application Settings!");
            return false;
        }

        CreateDescriptorPool(device);
        CreateGraphicsResources(device);
        CreateAndLoadDefaultAssets(device, file);
        CreateDescriptorSets(device);

        NES_LOG("PBR Renderer Initialized!");
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
    
        m_gridPipeline = nullptr;
        m_gridPipelineLayout = nullptr;
    
        m_pbrPipeline = nullptr;
        m_pbrPipelineLayout = nullptr;

        m_materialDescriptorSets.clear();
        m_descriptorPool = nullptr;
    }

    void PBRSceneRenderer::ProcessEnabledEntities()
    {
        auto* pRegistry = GetEntityRegistry();
        if (!pRegistry)
            return;
        
        // Handle Camera Activation:
        {
            auto view = pRegistry->GetAllEntitiesWith<nes::IDComponent, nes::PendingEnable, nes::CameraComponent>(entt::exclude<nes::DisabledComponent>);

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
            auto view = pRegistry->GetAllEntitiesWith<nes::IDComponent, nes::PendingEnable, nes::TransformComponent, MeshComponent>();
            for (auto entity : view)
            {
                // Register a new Mesh geometry if not already:
                auto& meshComp = view.get<MeshComponent>(entity);
                RegisterMeshComponent(entity, meshComp);
            }
        }
    }

    void PBRSceneRenderer::ProcessDisabledEntities()
    {
        // [TODO]: Disabling entities isn't implemented just yet in the editor, so I don't have to worry about it at the moment.
    }

    void PBRSceneRenderer::ProcessDestroyedEntities(const bool destroyingWorld)
    {
        auto* pRegistry = GetEntityRegistry();
        if (!destroyingWorld && pRegistry)
        {
            // Handle removing meshes:
            {
                auto view = pRegistry->GetAllEntitiesWith<nes::IDComponent, nes::PendingDestruction, MeshComponent>(entt::exclude<nes::DisabledComponent>);
                for (auto entity : view)
                {
                    auto& meshComp = view.get<MeshComponent>(entity);
                    UnregisterMeshComponent(entity, meshComp);
                }
            }
        }
    }

    void PBRSceneRenderer::RenderWorldWithCamera(const nes::WorldCamera& worldCamera, nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
    {
        auto& device = nes::DeviceManager::GetRenderDevice();
        auto* pRegistry = GetEntityRegistry();
        
        if (pRegistry)
        {
            BuildSceneData(*pRegistry, device, commandBuffer, context);
        }
        
        UpdateUniformBuffers(worldCamera, context);

        if (pRegistry)
        {
            RenderShadows(*pRegistry, commandBuffer, context);
        }
        
        // Get the color target (multisampled or single-sampled) image that will be drawn into.
        auto& colorTarget = GetColorDrawTarget();
        auto& colorImage = colorTarget.GetImage();

        // Transition the color target to color attachment.
        {
            nes::ImageBarrierDesc colorBarrier = nes::ImageBarrierDesc()
                .SetImage(&colorImage)
                .SetLayout(nes::EImageLayout::Undefined, nes::EImageLayout::ColorAttachment);

            nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
                .SetImageBarriers({ colorBarrier } );
        
            commandBuffer.SetBarriers(barrierGroup);
        }

        // Set our render targets:
        nes::RenderTargetsDesc renderTargetsDesc = nes::RenderTargetsDesc()
             .SetColorTargets(&colorTarget.GetView())
             .SetDepthStencilTarget(&m_depthTarget.GetView());
    
        // Record Render Commands:
        commandBuffer.BeginRendering(renderTargetsDesc);
        {
            // Clear the Color and Depth Targets:
            const nes::ClearDesc colorClear = nes::ClearDesc::Color(colorTarget.GetClearValue(), 0);
            const nes::ClearDesc depthClear = nes::ClearDesc::DepthStencil(m_depthTarget.GetClearValue());
            commandBuffer.ClearRenderTargets({ colorClear, depthClear });

            // Set the viewport and scissor to encompass the entire image.
            const nes::Viewport viewport(m_colorTarget.GetSize().x, m_colorTarget.GetSize().y);
            const nes::Scissor scissor(viewport);
            commandBuffer.SetViewports(viewport);
            commandBuffer.SetScissors(scissor);
            
            RenderInstances(commandBuffer, context);
            RenderSkybox(commandBuffer, context);

            // [TODO]:
            // Should probably be moved to the Editor when you can.
            if (!GetWorld().IsSimulating())
                RenderGrid(commandBuffer, context);
 
            // Finish.
            commandBuffer.EndRendering();
        }

        // Resolve the m_msaa image into the final color target if enabled. 
        if (colorTarget.GetSampleCount() > 1)
        {
            // Transition images for resolution:
            {
                nes::ImageBarrierDesc imageBarrier = nes::ImageBarrierDesc()
                    .SetImage(&m_msaaTarget.GetImage())
                    .SetLayout(nes::EImageLayout::ColorAttachment, nes::EImageLayout::ResolveSource);

                nes::ImageBarrierDesc colorImageBarrier = nes::ImageBarrierDesc()
                    .SetImage(&m_colorTarget.GetImage())
                    .SetLayout(nes::EImageLayout::Undefined, nes::EImageLayout::ResolveDestination);
            
                nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
                    .SetImageBarriers({ imageBarrier, colorImageBarrier} );
                    
                commandBuffer.SetBarriers(barrierGroup);
            }
        
            commandBuffer.ResolveImage(m_msaaTarget.GetImage(), m_colorTarget.GetImage());

            // Transition to the color attachment after
            {
                nes::ImageBarrierDesc colorImageBarrier = nes::ImageBarrierDesc()
                    .SetImage(&m_colorTarget.GetImage())
                    .SetLayout(nes::EImageLayout::ResolveDestination, nes::EImageLayout::ColorAttachment);
            
                nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
                    .SetImageBarriers({ colorImageBarrier} );
                    
                commandBuffer.SetBarriers(barrierGroup);
            }
        }
    }

    nes::WorldCamera PBRSceneRenderer::GetActiveCamera() const
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

    void PBRSceneRenderer::OnViewportResize(const uint32 width, const uint32 height)
    {
        // Resize the MSAA Set
        m_colorTarget.Resize(width, height);
        m_depthTarget.Resize(width, height);

        if (m_sampleCount > 1)
        {
            m_msaaTarget.Resize(width, height);
        }

        // After resize, each image is in the undefined layout.
        {
            auto commandBuffer = nes::Renderer::BeginTempCommands();
            auto& depthImage = m_depthTarget.GetImage();

            nes::ImageBarrierDesc depthBarrier = nes::ImageBarrierDesc()
                .SetImage(&depthImage, nes::EImagePlaneBits::Depth | nes::EImagePlaneBits::Stencil)
                .SetLayout(nes::EImageLayout::Undefined, nes::EImageLayout::DepthStencilAttachment);

            nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
                .SetImageBarriers({depthBarrier});
        
            commandBuffer.SetBarriers(barrierGroup);

            nes::Renderer::SubmitAndWaitTempCommands(commandBuffer);
        }
    }

    bool PBRSceneRenderer::CreateAndLoadDefaultAssets(nes::RenderDevice& device, const nes::YamlInStream& file)
    {
        const auto root = file.GetRoot();
        
        // Load the Asset Pack of default assets and shaders.
        {
            const auto& assets = root["Assets"];
            nes::AssetPack pack{};
            if (!nes::AssetPack::Deserialize(assets, pack))
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
        CreateRenderTargetsAndPipelines(device, root);

        // Set the Default AssetIDs:
        {
            s_cubeMeshID = 1;
            s_planeMeshID = 2;
            s_sphereMeshID = 3;
            
            const auto& defaultAssetIDs = root["DefaultAssetIDs"];
            defaultAssetIDs["ErrorTextureID"].Read(s_errorTextureID);
            defaultAssetIDs["BlackTextureID"].Read(s_blackTextureID);
            defaultAssetIDs["WhiteTextureID"].Read(s_whiteTextureID);
            defaultAssetIDs["FlatNormalTextureID"].Read(s_flatNormalTextureID);
            defaultAssetIDs["DefaultMaterialID"].Read(s_defaultMaterialID);
            defaultAssetIDs["DefaultSkyboxID"].Read(s_defaultSkyboxID);

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
        MeshInstance sceneMesh;
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
            nes::AssetManager::AddMemoryAsset<MeshAsset>(s_cubeMeshID, std::move(asset), "Default Cube");

            auto pAsset = nes::AssetManager::GetAsset<MeshAsset>(s_cubeMeshID);
            RegisterMeshAsset(pAsset);
        }

        // Sphere:
        {
            helpers::SphereGenDesc desc;
            desc.m_latitudeBands = 30.f;
            desc.m_longitudeBands = 30.f;
            desc.m_radius = 0.5f;
            
            helpers::AppendSphereMeshData(desc, m_scene.m_vertices, m_scene.m_indices, sceneMesh);
            MeshAsset asset(&m_scene.m_vertices[sceneMesh.m_firstVertex], sceneMesh.m_vertexCount, &m_scene.m_indices[sceneMesh.m_firstIndex], sceneMesh.m_indexCount, s_defaultMaterialID);
            nes::AssetManager::AddMemoryAsset<MeshAsset>(s_sphereMeshID, std::move(asset), "Default Sphere");

            auto pAsset = nes::AssetManager::GetAsset<MeshAsset>(s_sphereMeshID);
            RegisterMeshAsset(pAsset);
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
            nes::AssetManager::AddMemoryAsset<MeshAsset>(s_planeMeshID, std::move(asset), "Default Plane");

            auto pAsset = nes::AssetManager::GetAsset<MeshAsset>(s_planeMeshID);
            RegisterMeshAsset(pAsset);
        }

        return true;
    }

    void PBRSceneRenderer::CreateRenderTargetsAndPipelines(nes::RenderDevice& device, const nes::YamlNode& root)
    {
        const auto swapchainColorFormat = nes::Renderer::GetSwapchainFormat();
        const auto swapchainExtent = nes::Renderer::GetSwapchainExtent();

        // Load Render Targets:
        auto renderTargets = root["RenderTargets"];
        NES_ASSERT(renderTargets);
        NES_ASSERT(renderTargets.size() > 0);
        {
            const nes::UInt2 swapchainSize = nes::UInt2(swapchainExtent.width, swapchainExtent.height);
            
            m_colorTarget = CreateColorRenderTarget(renderTargets["Color"], "Color", device,  swapchainColorFormat, swapchainSize);
            m_depthTarget = LoadDepthRenderTarget(renderTargets["Depth"], "Depth", device, swapchainSize);
            
            m_renderTargetRegistry.emplace(m_colorTarget.GetName(), &GetColorDrawTarget());
            m_renderTargetRegistry.emplace(m_depthTarget.GetName(), &m_depthTarget);
        }

        // Load Pipelines:
        std::filesystem::path path{};
        std::string relativePath{};
        auto pipelines = root["Pipelines"];
        NES_ASSERT(pipelines);

        // Grid
        {
            path = NES_CONTENT_DIR;
            pipelines["Grid"].Read(relativePath);
            path /= relativePath;

            nes::YamlInStream pipelineFile(path);
            NES_ASSERT(pipelineFile.IsOpen());
            auto graphicsPipeline = pipelineFile.GetRoot()["GraphicsPipeline"];
            NES_ASSERT(graphicsPipeline);
            LoadGraphicsPipeline(graphicsPipeline, device, m_gridPipelineLayout, m_gridPipeline);
        }

        // Skybox
        {
            path = NES_CONTENT_DIR;
            pipelines["Skybox"].Read(relativePath);
            path /= relativePath;

            nes::YamlInStream pipelineFile(path);
            NES_ASSERT(pipelineFile.IsOpen());
            auto graphicsPipeline = pipelineFile.GetRoot()["GraphicsPipeline"];
            NES_ASSERT(graphicsPipeline);
            LoadGraphicsPipeline(graphicsPipeline, device, m_skyboxPipelineLayout, m_skyboxPipeline);
        }

        // PBR Geometry Pipeline
        {
            path = NES_CONTENT_DIR;
            pipelines["PBR"].Read(relativePath);
            path /= relativePath;

            nes::YamlInStream pipelineFile(path);
            NES_ASSERT(pipelineFile.IsOpen());
            auto graphicsPipeline = pipelineFile.GetRoot()["GraphicsPipeline"];
            NES_ASSERT(graphicsPipeline);
            LoadGraphicsPipeline(graphicsPipeline, device, m_pbrPipelineLayout, m_pbrPipeline);
        }

        // Shadow Pipeline:
        {
            auto shadowSettings = root["ShadowSettings"];
            
            uint32 minBits;
            shadowSettings["FormatMinBits"].Read(minBits, 32u);
            m_shadowImageFormat = device.GetSupportedDepthFormat(minBits, false);

            shadowSettings["ImageResolution"].Read(m_shadowMapResolution, 2048u);
            shadowSettings["NumCascades"].Read(m_shadowCascadeCount, 1u);
            shadowSettings["MaxShadowDistance"].Read(m_shadowMaxDistance, 100.f);
            shadowSettings["CascadeSplitLambda"].Read(m_shadowCascadeSplitLambda, 0.5f);
            shadowSettings["DepthBiasConstant"].Read(m_shadowDepthBiasConstant, 1.25f);
            shadowSettings["DepthBiasSlope"].Read(m_shadowDepthBiasSlope, 1.75f);
            
            nes::AssetID shaderID;
            shadowSettings["DepthShader"].Read(shaderID, nes::kInvalidAssetID);
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
            static constexpr uint32 kMaxVertices = 25'600;
            
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

            // Skybox Descriptor Set: Sampler and CubeImage.
            {
                m_descriptorPool.AllocateDescriptorSets(m_skyboxPipelineLayout, 1, &frame.m_skyboxDescriptorSet);
                UpdateSkyboxTexture(frame.m_skyboxDescriptorSet);
            }
        }
    
        
    }

    void PBRSceneRenderer::UpdateUniformBuffers(const nes::WorldCamera& worldCamera, const nes::RenderFrameContext& context)
    {
        //auto& registry = GetRegistry();
        auto& frame = m_frames[context.GetFrameIndex()];
    
        const nes::Viewport viewport(m_colorTarget.GetSize().x, m_colorTarget.GetSize().y);
        const float aspectRatio = m_colorTarget.GetAspectRatio();

        // Update Camera Data:
        CameraUBO cameraConstants;
        {
            cameraConstants.m_position = nes::Float3(worldCamera.m_position.x, worldCamera.m_position.y, worldCamera.m_position.z);
            cameraConstants.m_view = worldCamera.CalculateViewMatrix();
            cameraConstants.m_exposureFactor = worldCamera.m_camera.CalculateExposureFactor();
            cameraConstants.m_projection = worldCamera.m_camera.CalculateProjectionMatrix(static_cast<uint32>(viewport.m_extent.x), static_cast<uint32>(viewport.m_extent.y));
            cameraConstants.m_viewProjection = cameraConstants.m_projection * cameraConstants.m_view;

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
        if (!m_scene.m_directionalLights.empty())
        {
            //NES_ASSERT(!m_scene.m_directionalLights.empty());

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

    void PBRSceneRenderer::BuildSceneData(nes::EntityRegistry& registry, nes::RenderDevice& device, nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
    {
        auto& frame = m_frames[context.GetFrameIndex()];
        
        // Update all model matrices for the instances:
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

            nes::EntityHandle directionalLight = nes::kInvalidEntityHandle; 
            int highestPriority = std::numeric_limits<int>::min();
            for (auto entity : view)
            {
                auto& lightComp = view.get<DirectionalLightComponent>(entity);

                if (lightComp.m_priority > highestPriority)
                {
                    directionalLight = entity;
                    highestPriority = lightComp.m_priority;
                }
            }

            // Only one directional light for now.
            // [TODO]: I should get rid of the array representation now that I am doing the priority check.
            if (directionalLight != nes::kInvalidEntityHandle)
            {
                auto& lightComp = registry.GetComponent<DirectionalLightComponent>(directionalLight);
                
                DirectionalLight light{};
                light.m_direction = nes::Float3(lightComp.m_direction.x, lightComp.m_direction.y, lightComp.m_direction.z);
                light.m_color = nes::Float3(lightComp.m_color.r, lightComp.m_color.g, lightComp.m_color.b);
                light.m_intensity = lightComp.m_intensity;
                m_scene.m_directionalLights.push_back(light);
            }
        }

        // Skybox
        {
            auto view = registry.GetAllEntitiesWith<SkyboxComponent>(entt::exclude<nes::DisabledComponent>);

            nes::AssetID skyboxAssetID = nes::kInvalidAssetID;
            int highestPriority = std::numeric_limits<int>::min();
            for (auto entity : view)
            {
                auto& skyboxComp = view.get<SkyboxComponent>(entity);

                if (skyboxComp.m_priority > highestPriority)
                {
                    highestPriority = skyboxComp.m_priority;
                    skyboxAssetID = skyboxComp.m_skyboxAssetID;
                }
            }

            // If there are no skyboxes in the world, set the default.
            if (skyboxAssetID == nes::kInvalidAssetID)
            {
                skyboxAssetID = GetDefaultSkyboxID();
            }

            // We need to update our current skybox:
            if (skyboxAssetID != m_scene.m_skyboxTextureID)
            {
                const bool needsRegister = !m_scene.m_idToTextureIndex.contains(skyboxAssetID);
                auto pTexture = nes::AssetManager::GetAsset<nes::TextureCube>(skyboxAssetID);
                if (needsRegister && pTexture)
                {
                    RegisterTextureCubeAsset(device, pTexture);
                }

                m_scene.m_skyboxTextureID = skyboxAssetID;
            }

            UpdateSkyboxTexture(frame.m_skyboxDescriptorSet);
        }

        // [TODO]: Should probably make the Uploader a member variable.
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

    void PBRSceneRenderer::RenderShadows(nes::EntityRegistry& registry, nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
    {
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

                    for (const auto& submesh : instance.m_submeshes)
                    {
                        // Push the object's position and the cascade index:
                        pushConstants.m_model = instance.m_model;
                        commandBuffer.SetPushConstant(0, &pushConstants, sizeof(DepthPassPushConstants));
                
                        // Bind Mesh Vertex Buffer.
                        const MeshInstance& mesh = m_scene.m_meshes[submesh.m_meshIndex];
                        nes::VertexBufferRange meshVertexBuffer(&m_verticesBuffer, sizeof(Vertex), mesh.m_vertexCount, mesh.m_firstVertex * sizeof(Vertex));
                        commandBuffer.BindVertexBuffers({ meshVertexBuffer}, 0);
                
                        // Draw
                        nes::DrawIndexedDesc drawDesc{};
                        drawDesc.m_firstIndex = mesh.m_firstIndex;
                        drawDesc.m_indexCount = mesh.m_indexCount;
                        commandBuffer.DrawIndexed(drawDesc);
                    }
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
        
        auto& frame = m_frames[context.GetFrameIndex()];
        commandBuffer.BindPipelineLayout(m_skyboxPipelineLayout);
        commandBuffer.BindPipeline(m_skyboxPipeline);
        commandBuffer.BindDescriptorSet(0, frame.m_cameraSet);
        commandBuffer.BindDescriptorSet(1, frame.m_skyboxDescriptorSet);

        // Draw 3 Vertices, 
        commandBuffer.DrawVertices(3);
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
            for (auto& submesh : instance.m_submeshes)
            {
                // Push the instance data:
                InstanceUBO object;
                object.SetTransform(instance.m_model);
                object.SetMesh(submesh.m_meshIndex);
                object.SetMaterial(submesh.m_materialIndex);
                commandBuffer.SetPushConstant(0, &object, sizeof(InstanceUBO));

                // Bind the Material Textures:
                commandBuffer.BindDescriptorSet(4, m_materialDescriptorSets[submesh.m_materialIndex]);

                // Bind Mesh Vertex Buffer.
                const MeshInstance& mesh = m_scene.m_meshes[submesh.m_meshIndex];
                nes::VertexBufferRange meshVertexBuffer(&m_verticesBuffer, sizeof(Vertex), mesh.m_vertexCount, mesh.m_firstVertex * sizeof(Vertex));
                commandBuffer.BindVertexBuffers({ meshVertexBuffer}, 0);

                // Draw
                nes::DrawIndexedDesc drawDesc{};
                drawDesc.m_firstIndex = mesh.m_firstIndex;
                drawDesc.m_indexCount = mesh.m_indexCount;
                commandBuffer.DrawIndexed(drawDesc);
            }
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

    void PBRSceneRenderer::RegisterMeshComponent(nes::EntityHandle entity, MeshComponent& meshComp)
    {
        auto pMesh = nes::AssetManager::GetAsset<MeshAsset>(meshComp.m_sourceMeshID);
        if (!m_scene.m_idToMeshInstanceIndices.contains(meshComp.m_sourceMeshID))
        {
            if (pMesh != nullptr)
                RegisterMeshAsset(pMesh);
        }

        // Register the Component Materials:
        const auto& sourceMeshMaterials = pMesh->GetMaterials();
        if (meshComp.m_materials.empty())
        {
            meshComp.m_materials = sourceMeshMaterials;
        }
        
        for (size_t i = 0; i < meshComp.m_materials.size(); ++i)
        {
            nes::AssetID& materialID = meshComp.m_materials[i];
            if (materialID == nes::kInvalidAssetID)
            {
                // Get the default material for the asset.
                if (i < sourceMeshMaterials.size())
                    materialID = sourceMeshMaterials[i];

                // Default Material if none present:
                if (materialID == nes::kInvalidAssetID)
                    materialID = GetDefaultMaterialID();
            }

            // Register the Material Asset data if not already:
            if (!m_scene.m_idToMaterialIndex.contains(materialID))
            {
                auto pMaterial = nes::AssetManager::GetAsset<PBRMaterial>(materialID);
                if (pMaterial != nullptr)
                    RegisterMaterialAsset(pMaterial);
            }
        }
        
        // Add the instance to our array.
        EntityInstance instance;
        instance.m_entity = entity;

        auto& meshInstanceIndices = m_scene.m_idToMeshInstanceIndices[meshComp.m_sourceMeshID];
        NES_ASSERT(meshInstanceIndices.size() == meshComp.m_materials.size());

        // Add all submesh data instance for this entity:
        for (size_t i = 0; i < meshInstanceIndices.size(); ++i)
        {
            SubMeshInstance submeshInstance;
            submeshInstance.m_meshIndex = meshInstanceIndices[i];
            submeshInstance.m_materialIndex = m_scene.m_idToMaterialIndex[meshComp.m_materials[i]];
            instance.m_submeshes.emplace_back(submeshInstance);
        }
        
        //.m_meshIndex = m_scene.m_idToMeshIndex.at(meshComp.m_sourceMeshID),
        //.m_materialIndex = m_scene.m_idToMaterialIndex.at(meshComp.m_materialID
        m_scene.m_instances.emplace_back(instance);
        m_scene.m_entityToInstanceMap.emplace(entity, m_scene.m_instances.empty()? 0 : static_cast<uint32>(m_scene.m_instances.size() - 1));
    }

    void PBRSceneRenderer::UnregisterMeshComponent(nes::EntityHandle entity, MeshComponent& /*meshComp*/)
    {
        // [TODO]: Unregister Mesh/Material Data if there are no more references to them in the scene.
        // - I would need to add a scene reference count in the maps for ID -> Mesh/Material Index.
        //auto pMeshAsset = nes::AssetManager::GetAsset<MeshAsset>(meshComponent.m_meshID);
        //auto pMaterialAsset = nes::AssetManager::GetAsset<PBRMaterial>(meshComponent.m_materialID);
        
        const auto instanceIndex = m_scene.m_entityToInstanceMap.at(entity);
        if (instanceIndex == m_scene.m_instances.size() - 1)
        {
            m_scene.m_instances.pop_back();
        }
        else
        {
            std::swap(m_scene.m_instances[instanceIndex], m_scene.m_instances.back());
            m_scene.m_instances.pop_back();
        
            // Update the mapping of entity to index.
            const auto& swappedInstance = m_scene.m_instances[instanceIndex];
            m_scene.m_entityToInstanceMap[swappedInstance.m_entity] = instanceIndex;
        }
        
        m_scene.m_entityToInstanceMap.erase(entity);
    }

    void PBRSceneRenderer::RegisterMeshAsset(const nes::AssetPtr<MeshAsset>& pMesh)
    {
        NES_ASSERT(pMesh != nullptr);

        const auto id = pMesh->GetAssetID();
        if (m_scene.m_idToMeshInstanceIndices.contains(id))
            return;

        const auto& meshVertices = pMesh->GetVertices();
        const auto& meshIndices = pMesh->GetIndices();

        const uint32 baseVertex = static_cast<uint32>(m_scene.m_vertices.size());
        const uint32 baseIndex = static_cast<uint32>(m_scene.m_indices.size());
        m_scene.m_vertices.insert(m_scene.m_vertices.end(), meshVertices.begin(), meshVertices.end());
        m_scene.m_indices.insert(m_scene.m_indices.end(), meshIndices.begin(), meshIndices.end());

        std::vector<uint32> meshInstanceIndices;
        meshInstanceIndices.reserve(meshIndices.size());

        uint32 subMeshVertexOffset = 0;
        
        for (auto& submesh : pMesh->GetSubMeshes())
        {
            // Set the Index/Vertex information.
            MeshInstance sceneMesh;
            sceneMesh.m_firstVertex = baseVertex + subMeshVertexOffset;
            sceneMesh.m_firstIndex = baseIndex + submesh.m_firstIndex;
            sceneMesh.m_vertexCount = submesh.m_vertexCount;
            sceneMesh.m_indexCount = submesh.m_indexCount;
            
            subMeshVertexOffset += submesh.m_vertexCount;

            // Add to the Mesh Instances:
            m_scene.m_meshes.emplace_back(sceneMesh);
            meshInstanceIndices.emplace_back(static_cast<uint32>(m_scene.m_meshes.size() - 1));
        }

        // Map the Asset ID of the source mesh to the instances within the scene.
        m_scene.m_idToMeshInstanceIndices.emplace(id, std::move(meshInstanceIndices));
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

    nes::RenderTarget& PBRSceneRenderer::GetColorDrawTarget()
    {
        if (m_sampleCount > 1)
        {
            NES_ASSERT(m_msaaTarget.GetImage() != nullptr, "Multisampling is enabled, but no msaa render target was created!");
            return m_msaaTarget;
        }

        return m_colorTarget;
    }

    void PBRSceneRenderer::UpdateSkyboxTexture(nes::DescriptorSet& set)
    {
        // Get the Skybox Texture View:
        const auto textureIndex = m_scene.m_idToTextureIndex[m_scene.m_skyboxTextureID];
        NES_ASSERT(textureIndex < m_scene.m_textures.size());
        nes::Descriptor* pSkyboxTexture = &m_scene.m_textures[textureIndex];
        NES_ASSERT(*pSkyboxTexture != nullptr);
        nes::Descriptor* pTextureSampler = &m_textureSampler;

        std::array updateDescs =
        {
            nes::DescriptorBindingUpdateDesc(&pTextureSampler, 1),
            nes::DescriptorBindingUpdateDesc(&pSkyboxTexture, 1),
        };
        set.UpdateBindings(updateDescs.data(), 0, static_cast<uint32>(updateDescs.size()));
    }

    void PBRSceneRenderer::OnWorldSet()
    {
        if (auto* pRegistry = GetEntityRegistry())
        {
            pRegistry->OnComponentCreated<MeshComponent>().connect<&PBRSceneRenderer::OnMeshComponentAdded>(this);
            pRegistry->OnComponentUpdated<MeshComponent>().connect<&PBRSceneRenderer::OnMeshComponentUpdated>(this);
            pRegistry->OnComponentDestroyed<MeshComponent>().connect<&PBRSceneRenderer::OnMeshComponentRemoved>(this);    
        }

        ClearSceneInstanceData();
    }

    void PBRSceneRenderer::OnWorldRemoved()
    {
        if (auto* pRegistry = GetEntityRegistry())
        {
            pRegistry->OnComponentCreated<MeshComponent>().disconnect<&PBRSceneRenderer::OnMeshComponentAdded>(this);
            pRegistry->OnComponentUpdated<MeshComponent>().disconnect<&PBRSceneRenderer::OnMeshComponentUpdated>(this);
            pRegistry->OnComponentDestroyed<MeshComponent>().disconnect<&PBRSceneRenderer::OnMeshComponentRemoved>(this);
        }
    }

    void PBRSceneRenderer::OnBeginSimulation()
    {
        ClearSceneInstanceData();
    }

    void PBRSceneRenderer::OnEndSimulation()
    {
        ClearSceneInstanceData();
    }

    void PBRSceneRenderer::OnEntityRegistryChanged(nes::EntityRegistry* pNewRegistry, nes::EntityRegistry* pOldRegistry)
    {
        ClearSceneInstanceData();
        
        if (pOldRegistry != nullptr)
        {
            pOldRegistry->OnComponentCreated<MeshComponent>().disconnect<&PBRSceneRenderer::OnMeshComponentAdded>(this);
            pOldRegistry->OnComponentUpdated<MeshComponent>().disconnect<&PBRSceneRenderer::OnMeshComponentUpdated>(this);
            pOldRegistry->OnComponentDestroyed<MeshComponent>().disconnect<&PBRSceneRenderer::OnMeshComponentRemoved>(this);
        }
        
        if (pNewRegistry != nullptr)
        {
            pNewRegistry->OnComponentCreated<MeshComponent>().connect<&PBRSceneRenderer::OnMeshComponentAdded>(this);
            pNewRegistry->OnComponentUpdated<MeshComponent>().connect<&PBRSceneRenderer::OnMeshComponentUpdated>(this);
            pNewRegistry->OnComponentDestroyed<MeshComponent>().connect<&PBRSceneRenderer::OnMeshComponentRemoved>(this);
        }
    }

    void PBRSceneRenderer::ClearSceneInstanceData()
    {
        m_scene.m_directionalLights.clear();
        m_scene.m_pointLights.clear();
        m_scene.m_entityToInstanceMap.clear();
        m_scene.m_instances.clear();
    }

    void PBRSceneRenderer::OnMeshComponentAdded(entt::registry& registry, const entt::entity entity)
    {
        if (entity == nes::kInvalidEntityHandle || m_scene.m_entityToInstanceMap.contains(entity))
            return;

        // If pending initialization, this will be handled later.
        // If disabled, we won't worry about it now.
        if (registry.any_of<nes::PendingInitialization, nes::DisabledComponent>(entity))
            return;

        // This is a new mesh component, for an entity that is visible and already exists.
        auto& meshComponent = registry.get<MeshComponent>(entity);

        // Set the initial mesh to a cube:
        if (meshComponent.m_sourceMeshID == nes::kInvalidAssetID)
        {
            meshComponent.m_sourceMeshID = GetDefaultMeshID(EDefaultMeshType::Cube);
        }
        
        RegisterMeshComponent(entity, meshComponent);
    }

    void PBRSceneRenderer::OnMeshComponentUpdated(entt::registry& registry, const entt::entity entity)
    {
        if (entity == nes::kInvalidEntityHandle || registry.any_of<nes::DisabledComponent>(entity) || !m_scene.m_entityToInstanceMap.contains(entity))
            return;

        auto& meshComponent = registry.get<MeshComponent>(entity);
        const uint32 index = m_scene.m_entityToInstanceMap.at(entity);
        NES_ASSERT(index < m_scene.m_instances.size());

        auto& instance = m_scene.m_instances[index];

        // Get the mesh and material indices:
        std::vector<uint32> meshInstanceIndices{};
        std::vector<uint32> materialIndices{};

        if (m_scene.m_idToMeshInstanceIndices.contains(meshComponent.m_sourceMeshID))
        {
            meshInstanceIndices = m_scene.m_idToMeshInstanceIndices.at(meshComponent.m_sourceMeshID);
        }
        else
        {
            auto pMeshAsset = nes::AssetManager::GetAsset<MeshAsset>(meshComponent.m_sourceMeshID);
            if (pMeshAsset != nullptr)
            {
                RegisterMeshAsset(pMeshAsset);
                meshInstanceIndices = m_scene.m_idToMeshInstanceIndices.at(meshComponent.m_sourceMeshID);
            }
        }

        // Get the indices for each material asset:
        for (auto& materialID : meshComponent.m_materials)
        {
            if (m_scene.m_idToMaterialIndex.contains(materialID))
            {
                materialIndices.emplace_back(m_scene.m_idToMaterialIndex.at(materialID));
            }
            else
            {
                // Register the material asset.
                auto pMaterialAsset = nes::AssetManager::GetAsset<PBRMaterial>(materialID);
                if (pMaterialAsset != nullptr)
                {
                    RegisterMaterialAsset(pMaterialAsset);
                    materialIndices.emplace_back(m_scene.m_idToMaterialIndex.at(materialID));
                }
            }
        }
        
        NES_ASSERT(meshInstanceIndices.size() == materialIndices.size());

        instance.m_submeshes.clear();
        for (size_t i = 0; i < meshInstanceIndices.size(); ++i)
        {
            instance.m_submeshes.emplace_back(meshInstanceIndices[i], materialIndices[i]);
        }
    }

    void PBRSceneRenderer::OnMeshComponentRemoved(entt::registry& registry, const entt::entity entity)
    {
        if (entity == nes::kInvalidEntityHandle || registry.any_of<nes::DisabledComponent>(entity) || !m_scene.m_entityToInstanceMap.contains(entity))
            return;

        auto& meshComponent = registry.get<MeshComponent>(entity);
        UnregisterMeshComponent(entity, meshComponent);
    }

    nes::RenderTarget PBRSceneRenderer::CreateColorRenderTarget(const nes::YamlNode& targetNode, const std::string& name, nes::RenderDevice& device, const nes::EFormat swapchainFormat, const nes::UInt2 swapchainExtent)
    {
        nes::RenderTargetDesc desc{};
        desc.m_planes = nes::EImagePlaneBits::Color;

        // Format
        targetNode["Format"].Read(desc.m_format, nes::EFormat::Unknown);
        if (desc.m_format == nes::EFormat::Unknown)
            desc.m_format = swapchainFormat;

        // Usage
        targetNode["Usage"].Read(desc.m_usage, nes::EImageUsageBits::None);
        desc.m_usage |= nes::EImageUsageBits::ColorAttachment | nes::EImageUsageBits::ShaderResource;

        // Sample Count
        targetNode["SampleCount"].Read<uint32>(desc.m_sampleCount, 1u);
        if (desc.m_sampleCount == 0)
        {
            // Get the max sample count for the format
            const auto features = device.GetFormatFeatures(desc.m_format);
            desc.m_sampleCount = nes::GetMaxSampleCount(features);
        }

        // Clear Value
        auto clearColorNode = targetNode["ClearColor"];
        nes::ClearColorValue clearColorValue{};
        clearColorNode[0].Read<float>(clearColorValue.m_float32[0], 0.f);
        clearColorNode[1].Read<float>(clearColorValue.m_float32[1], 0.f);
        clearColorNode[2].Read<float>(clearColorValue.m_float32[2], 0.f);
        clearColorNode[3].Read<float>(clearColorValue.m_float32[3], 1.f);
        desc.m_clearValue = clearColorValue;

        // Size
        targetNode["Size"].Read(desc.m_size, nes::UInt2::Zero());

        // If either dimension is zero, use the swapchain extent.
        if (desc.m_size.x == 0 || desc.m_size.y == 0)
            desc.m_size = swapchainExtent;

        // Hack: Create the Multisampled Target here if enabled.
        if (desc.m_sampleCount > 1)
        {
            // Create the msaa target
            desc.m_name = "MSAA Color";
            m_msaaTarget = nes::RenderTarget(device, desc);
            m_sampleCount = m_msaaTarget.GetSampleCount();
        }

        // Set the sample count to 1 for the output color target.
        desc.m_sampleCount = 1;
        desc.m_name = name;
        return nes::RenderTarget(device, desc);
    }

    nes::RenderTarget PBRSceneRenderer::LoadDepthRenderTarget(const nes::YamlNode& targetNode, const std::string& name, nes::RenderDevice& device, const nes::UInt2 swapchainExtent)
    {
        nes::RenderTargetDesc desc{};
        desc.m_name = name;
        
        // Format
        uint32 minBits;
        targetNode["FormatMinBits"].Read<uint32>(minBits, 16u);
        
        bool requireStencil;
        targetNode["FormatRequireStencil"].Read<bool>(requireStencil, false);
        
        desc.m_format = device.GetSupportedDepthFormat(minBits, requireStencil);
        NES_ASSERT(desc.m_format != nes::EFormat::Unknown);

        // Usage
        targetNode["Usage"].Read(desc.m_usage, nes::EImageUsageBits::None);
        desc.m_usage |= nes::EImageUsageBits::DepthStencilAttachment;
    
        // Image Planes based on the Format
        desc.m_planes = nes::EImagePlaneBits::Depth;
        if (requireStencil)
            desc.m_planes |= nes::EImagePlaneBits::Stencil;
    
        // Sample Count
        targetNode["SampleCount"].Read<uint32>(desc.m_sampleCount, 1u);

        // Clear Value
        nes::ClearDepthStencilValue clearDepthStencil;
        targetNode["ClearDepth"].Read<float>(clearDepthStencil.m_depth, 1.f);
        targetNode["ClearStencil"].Read<uint32>(clearDepthStencil.m_stencil, 0);
        desc.m_clearValue = clearDepthStencil;

        // Size
        targetNode["Size"].Read(desc.m_size, nes::UInt2::Zero());

        // If either dimension is zero, use the swapchain extent.
        if (desc.m_size.x == 0 || desc.m_size.y == 0)
            desc.m_size = swapchainExtent;
    
        return nes::RenderTarget(device, desc);
    }

    void PBRSceneRenderer::LoadGraphicsPipeline(const nes::YamlNode& pipelineNode, nes::RenderDevice& device, nes::PipelineLayout& outLayout, nes::Pipeline& outPipeline) const
    {
        // Pipeline Layout
        {
            auto layoutNode = pipelineNode["Layout"];

            // Stages
            static constexpr uint32 kDefaultStages = static_cast<uint32>(nes::EPipelineStageBits::GraphicsShaders);
            nes::EPipelineStageBits stages;
            layoutNode["Stages"].Read<nes::EPipelineStageBits>(stages, nes::EPipelineStageBits::GraphicsShaders);

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
                    bindingNode["Index"].Read<uint32>(desc.m_bindingIndex, 0u);
                    bindingNode["DescriptorCount"].Read<uint32>(desc.m_descriptorCount, 1u);
                    bindingNode["DescriptorType"].Read<nes::EDescriptorType>(desc.m_descriptorType, {});
                    bindingNode["Stages"].Read<nes::EPipelineStageBits>(desc.m_shaderStages, nes::EPipelineStageBits::GraphicsShaders);
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
                pushConstant["Offset"].Read<uint32>(desc.m_offset, 0u);
                pushConstant["Size"].Read<uint32>(desc.m_size, 0u);
                pushConstant["Stages"].Read<nes::EPipelineStageBits>(desc.m_shaderStages, nes::EPipelineStageBits::GraphicsShaders);
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
            nes::AssetID shaderID;
            pipelineNode["Shader"].Read(shaderID, nes::kInvalidAssetID); 
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
                attribute["Location"].Read(attributeDesc.m_location, 0u);
                attribute["Offset"].Read(attributeDesc.m_offset, 0u);
                attribute["Format"].Read(attributeDesc.m_format, nes::EFormat::Unknown);
                attribute["Stream"].Read(attributeDesc.m_streamIndex, 0u);
                attributeDescs.emplace_back(attributeDesc);
            }

            // Streams
            auto streamsNode = vertexInputNode["Streams"];
            streamDescs.reserve(streamsNode.size());
            for (auto stream : streamsNode)
            {
                nes::VertexStreamDesc streamDesc{};
                stream["Stride"].Read(streamDesc.m_stride, 0u);
                stream["BindingIndex"].Read(streamDesc.m_bindingIndex, 0u);
                stream["StepRate"].Read(streamDesc.m_stepRate, nes::EVertexStreamStepRate::PerVertex);
                streamDescs.emplace_back(streamDesc);
            }

            desc.m_vertexInput.m_attributes = attributeDescs;
            desc.m_vertexInput.m_streams = streamDescs;
        }

        // Input Assembly
        {
            auto inputAssembly = pipelineNode["InputAssembly"];
            inputAssembly["PrimitiveRestart"].Read(desc.m_inputAssembly.m_primitiveRestart );
            inputAssembly["TesselationPointCount"].Read(desc.m_inputAssembly.m_tessControlPointCount);
            inputAssembly["Topology"].Read(desc.m_inputAssembly.m_topology, nes::ETopology::TriangleList);
        }

        // Rasterization
        {
            auto rasterizationNode = pipelineNode["Rasterization"];
            auto& rasterState = desc.m_rasterization;
            
            rasterizationNode["CullMode"].Read(rasterState.m_cullMode, nes::ECullMode::Back);
            rasterizationNode["FillMode"].Read(rasterState.m_fillMode, nes::EFillMode::Solid);
            rasterizationNode["EnableDepthClamp"].Read(rasterState.m_enableDepthClamp, false);
            rasterizationNode["FrontFace"].Read(rasterState.m_frontFace, nes::EFrontFaceWinding::CounterClockwise);
            rasterizationNode["LineWidth"].Read(rasterState.m_lineWidth, 1.f);

            if (auto depthBiasNode = rasterizationNode["DepthBias"])
            {
                depthBiasNode["Constant"].Read(rasterState.m_depthBias.m_constant, 0.f);
                depthBiasNode["Clamp"].Read(rasterState.m_depthBias.m_clamp, 0.f);
                depthBiasNode["Slope"].Read(rasterState.m_depthBias.m_slope, 0.f);
                depthBiasNode["Enabled"].Read(rasterState.m_depthBias.m_enabled, false);
            }
        }

        // Output Merger
        std::vector<nes::RenderTarget*> targets{};
        std::vector<nes::ColorAttachmentDesc> colorAttachments{};
        {
            auto outputMergerNode = pipelineNode["OutputMerger"];
            auto& outputMerger = desc.m_outputMerger;
            
            // Color Attachments
            auto colorAttachmentsNode = outputMergerNode["ColorAttachments"];
            colorAttachments.reserve(colorAttachmentsNode.size());
            for (auto attachment : colorAttachmentsNode)
            {
                nes::ColorAttachmentDesc colorAttachmentDesc{};
            
                // Get the render target format this attachment is for.
                std::string renderTargetName;
                attachment["RenderTarget"].Read(renderTargetName);
                
                NES_ASSERT(m_renderTargetRegistry.contains(renderTargetName));
                auto* pTarget = m_renderTargetRegistry.at(renderTargetName);
                NES_ASSERT(pTarget->IsColorTarget());
                colorAttachmentDesc.m_format = pTarget->GetFormat();
                targets.emplace_back(pTarget);

                attachment["EnableBlend"].Read(colorAttachmentDesc.m_enableBlend, true);
                attachment["ColorWriteMask"].Read(colorAttachmentDesc.m_colorWriteMask, nes::EColorComponentBits::RGBA);
                
                // Color Blend
                auto colorBlendState = attachment["ColorBlend"];
                colorBlendState["SrcFactor"].Read(colorAttachmentDesc.m_colorBlend.m_srcFactor);
                colorBlendState["DstFactor"].Read(colorAttachmentDesc.m_colorBlend.m_dstFactor);
                colorBlendState["BlendOp"].Read(colorAttachmentDesc.m_colorBlend.m_op, nes::EBlendOp::Add);

                // Alpha Blend
                auto alphaBlendState = attachment["AlphaBlend"];
                alphaBlendState["SrcFactor"].Read(colorAttachmentDesc.m_alphaBlend.m_srcFactor);
                alphaBlendState["DstFactor"].Read(colorAttachmentDesc.m_alphaBlend.m_dstFactor);
                alphaBlendState["BlendOp"].Read(colorAttachmentDesc.m_colorBlend.m_op, nes::EBlendOp::Add);
                
                colorAttachments.emplace_back(colorAttachmentDesc);
            }

            outputMerger.m_pColors = colorAttachments.data();
            outputMerger.m_colorCount = static_cast<uint32>(colorAttachments.size());

            // Depth Attachment (optional)
            if (auto depthAttachmentNode = outputMergerNode["DepthAttachment"])
            {
                // Get depth/stencil target this attachment is for.
                std::string renderTargetName;
                depthAttachmentNode["RenderTarget"].Read(renderTargetName);
                NES_ASSERT(m_renderTargetRegistry.contains(renderTargetName));
                auto* pTarget = m_renderTargetRegistry.at(renderTargetName);
                NES_ASSERT(pTarget->IsDepthTarget());
                targets.emplace_back(pTarget);
                outputMerger.m_depthStencilFormat = pTarget->GetFormat();
                
                depthAttachmentNode["CompareOp"].Read(outputMerger.m_depth.m_compareOp, nes::ECompareOp::Less);
                depthAttachmentNode["EnableWrite"].Read(outputMerger.m_depth.m_enableWrite, true);
            }

            // Stencil Attachment (optional)
            if (auto stencilAttachmentNode = outputMergerNode["StencilAttachment"])
            {
                // Front
                auto frontNode = stencilAttachmentNode["Front"];
                auto& front = outputMerger.m_stencil.m_front;

                frontNode["CompareOp"].Read(front.m_compareOp, nes::ECompareOp::None);
                frontNode["FailOp"].Read(front.m_failOp);
                frontNode["PassOp"].Read(front.m_passOp);
                frontNode["DepthFailOp"].Read(front.m_depthFailOp);
                frontNode["CompareMask"].Read(front.m_compareMask);
                frontNode["WriteMask"].Read(front.m_writeMask);

                // Back
                auto backNode = stencilAttachmentNode["Back"];
                auto& back = outputMerger.m_stencil.m_back;
                backNode["CompareOp"].Read(back.m_compareOp, nes::ECompareOp::None);
                backNode["FailOp"].Read(back.m_failOp);
                backNode["PassOp"].Read(back.m_passOp);
                backNode["DepthFailOp"].Read(back.m_depthFailOp);
                backNode["CompareMask"].Read(back.m_compareMask);
                backNode["WriteMask"].Read(back.m_writeMask);
            }
        }

        // Multisample Behavior
        {
            auto& multisample = desc.m_multisample;
            desc.m_enableMultisample = false;

            if (auto multisampleNode = pipelineNode["Multisample"])
            {
                multisampleNode["Enabled"].Read(desc.m_enableMultisample, false);
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
        std::string debugName;
        pipelineNode["DebugName"].Read(debugName);
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

    nes::AssetID PBRSceneRenderer::GetDefaultSkyboxID()
    {
        return s_defaultSkyboxID;
    }
}
