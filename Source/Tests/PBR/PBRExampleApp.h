// PBRExampleApp.h
#pragma once

#include "Nessie/Application/Application.h"
#include "Nessie/Asset/AssetBase.h"
#include "Nessie/Graphics/Camera.h"
#include "Nessie/Graphics/DeviceBuffer.h"
#include "Nessie/Graphics/Pipeline.h"
#include "Nessie/Graphics/PipelineLayout.h"
#include "Nessie/Graphics/DescriptorPool.h"
#include "Nessie/Graphics/RenderTarget.h"
#include "Helpers/Scene.h"
#include "Helpers/Camera.h"
#include "Helpers/LightSpace.h"

struct CameraState
{
    nes::Vec3       m_position;         // Position of the camera.
    nes::Rotation   m_rotation{};       // Rotation of the camera.
    float           m_FOVY;             // Field of view in radians in the up direction.
    float           m_nearPlane = 0.1f; 
    float           m_farPlane = 1000.f; 
};

//----------------------------------------------------------------------------------------------------
/// @brief : This example application renders a 3D Scene with a single mesh. It uses a Physically Based
///     Rendering pipeline to render the 3D object more akin to the real world.
//----------------------------------------------------------------------------------------------------
class PBRExampleApp final : public nes::Application
{
public:
    explicit PBRExampleApp(const nes::ApplicationDesc& appDesc) : nes::Application(appDesc) {}

    // Application Interface:
    virtual void                    OnEvent(nes::Event& e) override;
    virtual bool                    Internal_AppInit() override;
    virtual void                    Internal_AppUpdate(const float timeStep) override;
    virtual void                    Internal_OnResize(const uint32 width, const uint32 height) override;
    virtual void                    Internal_AppRender(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context) override;
    virtual void                    Internal_AppShutdown() override;

private:
    //----------------------------------------------------------------------------------------------------
    /// @brief : Struct containing the number of lights per type in the variable sized storage buffers. 
    //----------------------------------------------------------------------------------------------------
    struct alignas(64) LightCountUBO
    {
        static constexpr uint32     kMaxPointLights = 1024;
        static constexpr uint32     kMaxDirectionalLights = 4;
        static constexpr uint32     kMaxSpotLights = 256;
        static constexpr uint32     kMaxAreaLights = 128;
        
        uint32                      m_directionalCount = 0;
        uint32                      m_pointCount = 0;
        uint32                      m_spotCount = 0;
        uint32                      m_areaCount = 0;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Push constants for a single depth pass. 
    //----------------------------------------------------------------------------------------------------
    struct DepthPassPushConstants
    {
        nes::Mat44                  m_model;            // Object model matrix.
        uint32                      m_cascadeIndex = 0; // The cascade index for this depth pass.
    };
    
    // All types in the buffer are 64-byte aligned.
    static constexpr uint32         kGlobalUBOElementSize = sizeof(CameraUBO) + sizeof(LightCountUBO) + sizeof(helpers::CascadedShadowMapsUBO);
    
    struct FrameData
    {
        // Buffers & Resource Views:
        nes::Descriptor             m_cameraUBOView = nullptr;              // View of the Camera info for this frame.
        nes::Descriptor             m_lightCountUBOView = nullptr;          // View of the Light Counts for this frame.
        nes::Descriptor             m_shadowUBOView = nullptr;              // View of the Shadow data for this frame.
        nes::Descriptor             m_pointLightsView = nullptr;            // View of the Storage Buffer of Point Lights.
        nes::Descriptor             m_directionalLightsView = nullptr;      // View of the Storage Buffer of Directional Lights.
        nes::Descriptor             m_materialUBOView = nullptr;            // View of the MaterialUBO buffer.
        nes::DeviceBuffer           m_materialUBOBuffer = nullptr;          // Device Buffer containing Material parameters.
        nes::DeviceBuffer           m_pointLightsBuffer = nullptr;          // Point Lights SSBO buffer.
        nes::DeviceBuffer           m_directionalLightsBuffer = nullptr;    // Directional Lights SSBO buffer.

        // Descriptor Set Values:
        nes::DescriptorSet          m_cameraSet = nullptr;                  // Value for the CameraUBO
        nes::DescriptorSet          m_lightDataSet = nullptr;               // Value for LightCount and Light Array values.
        nes::DescriptorSet          m_materialDataSet = nullptr;            // Value for the ObjectBuffer.
        nes::DescriptorSet          m_shadowPassDataSet = nullptr;          // Value for the Shadow Pass.
        nes::DescriptorSet          m_sampledShadowDataSet = nullptr;       // Value for shadow data used in the PBR pipeline.
        
        uint64                      m_cameraBufferOffset = 0;               // Byte offset in the Global buffer for the CameraUBO for this frame. 
        uint64                      m_lightCountOffset  = 0;                // Byte offset in the Global buffer for the LightCountUBO for this frame. 
        uint64                      m_shadowDataOffset  = 0;                // Byte offset in the Global buffer for the ShadowUBO for this frame. 
    };

private:
    using RenderTargetRegistry = std::unordered_map<std::string, nes::RenderTarget*>;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Loads and creates the sets of images that can be rendered to, and creates the pipelines
    ///     based on configuration data.
    //----------------------------------------------------------------------------------------------------
    void                            CreateRenderTargetsAndPipelines(nes::RenderDevice& device, const YAML::Node& file);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Create the depth images (1 per cascade), and the shadow pipeline.
    //----------------------------------------------------------------------------------------------------
    void                            CreateDepthPassResources(nes::RenderDevice& device, const nes::AssetID& shaderID);
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Create the global constants buffer and the vertex/index buffers for all geometry.
    //----------------------------------------------------------------------------------------------------
    void                            CreateBuffersAndImages(nes::RenderDevice& device);
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Create the Descriptor Pool that will allow us to allocate DescriptorSets.
    //----------------------------------------------------------------------------------------------------
    void                            CreateDescriptorPool(nes::RenderDevice& device);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Allocate the DescriptorSets that will be used to store the current uniform buffer values to
    ///     send to the shaders.
    //----------------------------------------------------------------------------------------------------
    void                            CreateDescriptorSets(nes::RenderDevice& device);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Resizes the Color and Depth targets to match the swapchain dimensions.
    //----------------------------------------------------------------------------------------------------
    void                            ResizeRenderTargets(const uint32 width, const uint32 height);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Update this frame's uniform buffers.
    //----------------------------------------------------------------------------------------------------
    void                            UpdateUniformBuffers(const nes::RenderFrameContext& context);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Renders the scene from the perspective of the directional light, storing the depth data in
    ///     the ShadowTarget's image.
    //----------------------------------------------------------------------------------------------------
    void                            RenderShadows(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context);
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Render the skybox.
    //----------------------------------------------------------------------------------------------------
    void                            RenderSkybox(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Render each instance in the scene.
    //----------------------------------------------------------------------------------------------------
    void                            RenderInstances(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context);
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Render the 3D grid.
    //----------------------------------------------------------------------------------------------------
    void                            RenderGrid(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context) const;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Update the world camera state:
    //----------------------------------------------------------------------------------------------------
    void                            UpdateCamera(const float deltaTime);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Set the input movement and rotation vector. 
    //----------------------------------------------------------------------------------------------------
    void                            ProcessInput();
    
    nes::RenderTarget               LoadColorRenderTarget(const YAML::Node& targetNode, const std::string& name, nes::RenderDevice& device, const nes::EFormat swapchainFormat, const nes::UInt2 swapchainExtent);
    nes::RenderTarget               LoadDepthRenderTarget(const YAML::Node& targetNode, const std::string& name, nes::RenderDevice& device, const nes::UInt2 swapchainExtent);
    void                            LoadGraphicsPipeline(const YAML::Node& pipelineNode, nes::RenderDevice& device, nes::PipelineLayout& outLayout, nes::Pipeline& outPipeline) const;

private:
    // Render Targets
    nes::RenderTarget               m_colorTarget = nullptr;
    nes::RenderTarget               m_depthTarget = nullptr;
    RenderTargetRegistry            m_renderTargetRegistry{};
    
    // Assets:
    std::vector<nes::AssetID>       m_loadedAssets;

    // Shadow Pipeline
    nes::PipelineLayout             m_shadowPipelineLayout = nullptr;
    nes::Pipeline                   m_shadowPipeline = nullptr;
    nes::DeviceImage                m_shadowMap = nullptr;
    std::vector<nes::Descriptor>    m_shadowImageViews{};
    nes::EFormat                    m_shadowImageFormat = nes::EFormat::D32_SFLOAT;
    uint32                          m_shadowCascadeCount = 1;
    uint32                          m_shadowMapResolution = 4096;
    float                           m_shadowMaxDistance = 100.f;
    float                           m_shadowCascadeSplitLambda = 0.5f;
    float                           m_shadowDepthBiasConstant = 1.25f;
    float                           m_shadowDepthBiasSlope = 1.75f;

    // Geometry Pipeline
    nes::PipelineLayout             m_pbrPipelineLayout = nullptr;
    nes::Pipeline                   m_pbrPipeline = nullptr;

    // Skybox Pipeline
    nes::PipelineLayout             m_skyboxPipelineLayout = nullptr;
    nes::Pipeline                   m_skyboxPipeline = nullptr;
    nes::DescriptorSet              m_skyboxDescriptorSet = nullptr;
    
    // Grid Pipeline
    nes::PipelineLayout             m_gridPipelineLayout = nullptr;
    nes::Pipeline                   m_gridPipeline = nullptr;

    // Scene Data:
    Scene                           m_scene{};
    nes::DescriptorPool             m_descriptorPool = nullptr;
    nes::Descriptor                 m_textureSampler = nullptr;
    nes::Descriptor                 m_depthSampler = nullptr;
    nes::Descriptor                 m_shadowSampledImageView = nullptr;
    std::vector<FrameData>          m_frames{};

    // Buffers
    nes::DeviceBuffer               m_indicesBuffer = nullptr;              // Index Data
    nes::DeviceBuffer               m_verticesBuffer = nullptr;             // Vertex Data
    nes::DeviceBuffer               m_globalsBuffer = nullptr;              // Contains the Camera, LightCount and Shadow Data for each frame.  
    std::vector<nes::DescriptorSet> m_materialDescriptorSets{};
    
    // Camera Data:
    CameraState                     m_camera{};
    nes::Vec3                       m_inputMovement = nes::Vec3::Zero();
    nes::Vec2                       m_inputRotation = nes::Vec2::Zero();
    float                           m_cameraMoveSpeed = 50.f;
    float                           m_cameraSensitivity = 1.25f;
    float                           m_cameraAperture = 8.f;
    float                           m_cameraShutterSpeed = 1.f / 125.f;
    float                           m_cameraISO = 100.f;
    bool                            m_cameraRotationEnabled = false;
};
