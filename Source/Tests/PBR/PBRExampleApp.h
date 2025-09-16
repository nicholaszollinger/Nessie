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
    struct GlobalConstants
    {
        // Camera Data:
        nes::Mat44                  m_projection = nes::Mat44::Identity();
        nes::Mat44                  m_view = nes::Mat44::Identity();
    };

    struct FrameData
    {
        nes::Descriptor             m_globalConstantBuffer = nullptr;       // Camera, lighting (todo).
        nes::DescriptorSet          m_globalConstantSet = nullptr;          // Value for the global constants.
        uint64                      m_globalBufferOffset = 0;               // Offset to the GlobalConstants value in the 
    };

private:
    //----------------------------------------------------------------------------------------------------
    /// @brief : Load the Shaders, Textures and Meshes for the App.  
    //----------------------------------------------------------------------------------------------------
    bool                            LoadScene();

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
    // Assets:
    nes::AssetID                    m_gridVertShader = nes::kInvalidAssetID;
    nes::AssetID                    m_gridFragShader = nes::kInvalidAssetID;
    nes::AssetID                    m_pbrVertShader = nes::kInvalidAssetID;
    nes::AssetID                    m_pbrFragShader = nes::kInvalidAssetID;
    nes::AssetID                    m_skyboxTexture = nes::kInvalidAssetID;
    nes::AssetID                    m_skyboxVertShader = nes::kInvalidAssetID;
    nes::AssetID                    m_skyboxFragShader = nes::kInvalidAssetID;
    nes::AssetID                    m_whiteTexture = nes::kInvalidAssetID;
    nes::GBuffer                    m_gBuffer = nullptr;                        // Color and depth render targets.
    nes::DescriptorPool             m_descriptorPool = nullptr;

    // Geometry Pipeline
    nes::PipelineLayout             m_pbrPipelineLayout = nullptr;
    nes::Pipeline                   m_pbrPipeline = nullptr;

    // Skybox Pipeline
    nes::PipelineLayout             m_skyboxPipelineLayout = nullptr;
    nes::Pipeline                   m_skyboxPipeline = nullptr;
    nes::DescriptorSet              m_skyboxDescriptorSet = nullptr;        // ImageCube.


    // Grid Pipeline
    nes::PipelineLayout             m_gridPipelineLayout = nullptr;
    nes::Pipeline                   m_gridPipeline = nullptr;
    
    nes::Descriptor                 m_sampler = nullptr;
    nes::Descriptor                 m_skyboxTextureView = nullptr;

    // Scene Data:
    Scene                           m_scene{};

    // Device Buffers:
    std::vector<FrameData>          m_frames{};
    nes::DeviceBuffer               m_globalConstantsBuffer = nullptr;      // Global Scene Data 
    nes::DeviceBuffer               m_indicesBuffer = nullptr;              // Index Data
    nes::DeviceBuffer               m_verticesBuffer = nullptr;             // Vertex Data
    nes::DeviceBuffer               m_instanceBuffer = nullptr;            // Transform Buffer.
    nes::DescriptorSet              m_materialDescriptorSet = nullptr;      // Images for the Materials.
    nes::EFormat                    m_depthFormat = nes::EFormat::Unknown;
    
    // Camera Data:
    CameraState                     m_camera{};
    nes::Vec3                       m_inputMovement = nes::Vec3::Zero();
    nes::Vec2                       m_inputRotation = nes::Vec2::Zero();
    float                           m_cameraMoveSpeed = 50.f;
    float                           m_cameraSensitivity = 1.f;
    bool                            m_cameraRotationEnabled = false;
};
