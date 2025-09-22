// PBRExampleApp.h
#pragma once

#include "Nessie/Application/Application.h"
#include "Nessie/Asset/AssetBase.h"
#include "Nessie/Graphics/Camera.h"
#include "Nessie/Graphics/DeviceBuffer.h"
#include "Nessie/Graphics/Pipeline.h"
#include "Nessie/Graphics/PipelineLayout.h"
#include "Nessie/Graphics/DescriptorPool.h"
#include "Nessie/Graphics/GBuffer.h"
#include "Helpers/Scene.h"
#include "Helpers/Camera.h"
#include "Helpers/LightTypes.h."

struct CameraState
{
    nes::Vec3       m_position;         // Position of the camera.
    nes::Rotation   m_rotation{};       // Rotation of the camera.
    float           m_FOVY;             // Field of view in radians in the up direction.
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
    struct alignas (64) LightCountUBO
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

    // Both CameraUBO and LightUBO are 64-byte aligned.
    static constexpr uint32         kGlobalUBOElementSize = sizeof(CameraUBO) + sizeof(LightCountUBO);

    struct FrameData
    {
        // Buffers & Resource Views:
        nes::Descriptor             m_cameraUBOView = nullptr;              // Camera UBO view.
        nes::Descriptor             m_lightCountUBOView = nullptr;          // LightCount UBO view.
        nes::Descriptor             m_pointLightsView = nullptr;            // Point Lights SSBO view.
        nes::Descriptor             m_directionalLightsView = nullptr;      // Directional Lights SSBO view.
        nes::Descriptor             m_materialUBOView = nullptr;              // ObjectUBO view.
        nes::DeviceBuffer           m_materialUBOBuffer = nullptr;           // ObjectUBO device buffer.
        nes::DeviceBuffer           m_pointLightsBuffer = nullptr;          // Point Lights SSBO buffer.
        nes::DeviceBuffer           m_directionalLightsBuffer = nullptr;    // Directional Lights SSBO buffer.

        // Descriptor Set Values:
        nes::DescriptorSet          m_cameraSet = nullptr;                  // Value for the CameraUBO
        nes::DescriptorSet          m_lightDataSet = nullptr;               // Value for LightCount and Light Array values.
        nes::DescriptorSet          m_materialDataSet = nullptr;              // Value for the ObjectBuffer.
        
        uint64                      m_cameraBufferOffset = 0;               // Byte offset in the Global buffer for the CameraUBO for this frame. 
        uint64                      m_lightCountOffset  = 0;                // Byte offset in the Global buffer for the LightCountUBO for this frame. 
    };

private:
    //----------------------------------------------------------------------------------------------------
    /// @brief : Create the global constants buffer and the vertex/index buffers for all geometry.
    //----------------------------------------------------------------------------------------------------
    void                            CreateBuffersAndImages(nes::RenderDevice& device);
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Create the pipeline to render the grid plane.
    //----------------------------------------------------------------------------------------------------
    void                            CreateGridPipeline(nes::RenderDevice& device);
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Create the pipeline to render the Skybox.
    //----------------------------------------------------------------------------------------------------
    void                            CreateSkyboxPipeline(nes::RenderDevice& device);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Create the pipeline to render 3D geometry.
    //----------------------------------------------------------------------------------------------------
    void                            CreateGeometryPipeline(nes::RenderDevice& device);
    
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
    /// @brief : Create the MSAA and Depth images and descriptors that we will be rendering to before resolve to the
    ///     swapchain's image.
    //----------------------------------------------------------------------------------------------------
    void                            UpdateGBuffer(nes::RenderDevice& device, const uint32 width, const uint32 height);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Update this frame's uniform buffers.
    //----------------------------------------------------------------------------------------------------
    void                            UpdateUniformBuffers(const nes::RenderFrameContext& context);

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

private:
    std::vector<nes::AssetID>       m_loadedAssets;
    
    // Assets:
    nes::AssetID                    m_skyboxTexture = nes::kInvalidAssetID;
    nes::AssetID                    m_skyboxShader = nes::kInvalidAssetID;
    nes::AssetID                    m_gridShader = nes::kInvalidAssetID;
    nes::AssetID                    m_pbrShader = nes::kInvalidAssetID;
    nes::GBuffer                    m_gBuffer = nullptr;                        // Color and depth render targets.
    nes::DescriptorPool             m_descriptorPool = nullptr;

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
    nes::Descriptor                 m_sampler = nullptr;
    std::vector<FrameData>          m_frames{};
    nes::DeviceBuffer               m_indicesBuffer = nullptr;              // Index Data
    nes::DeviceBuffer               m_verticesBuffer = nullptr;             // Vertex Data

    // [TODO]: Combine the Light Count and Camera data into a single buffer?
    //      - They are both small, and will only have 1 per frame.
    
    nes::DeviceBuffer               m_globalsBuffer = nullptr;               // Contains the Camera information and  
    std::vector<nes::DescriptorSet> m_materialDescriptorSets{};
    nes::EFormat                    m_depthFormat = nes::EFormat::Unknown;
    
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
