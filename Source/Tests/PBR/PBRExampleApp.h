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
        nes::Descriptor             m_globalBufferView = nullptr;      // View into the World uniform buffer.
        nes::DescriptorSet          m_descriptorSet = nullptr;
        uint64                      m_globalBufferOffset = 0;           // Offset to the GlobalConstants struct for this frame.
    };

private:
    //----------------------------------------------------------------------------------------------------
    /// @brief : Load the Shaders, Textures and Meshes for the App.  
    //----------------------------------------------------------------------------------------------------
    bool                            LoadAssets();
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Create the uniform buffers that will have their data updated each frame.
    //----------------------------------------------------------------------------------------------------
    void                            CreateUniformBuffers(nes::RenderDevice& device);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Create the MSAA and Depth images and descriptors that we will be rendering to before resolve to the
    ///     swapchain's image.
    //----------------------------------------------------------------------------------------------------
    void                            UpdateGBuffer(nes::RenderDevice& device, const uint32 width, const uint32 height);

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
    /// @brief : Update this frame's uniform buffers.
    //----------------------------------------------------------------------------------------------------
    void                            UpdateUniformBuffers(const nes::RenderFrameContext& context);

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
    nes::AssetID                    m_gridVertShader = nes::kInvalidAssetID;
    nes::AssetID                    m_gridFragShader = nes::kInvalidAssetID;
    nes::GBuffer                    m_gBuffer = nullptr;                        // Color and depth render targets.
    nes::DescriptorPool             m_descriptorPool = nullptr;
    nes::Pipeline                   m_geometryPipeline = nullptr;
    nes::PipelineLayout             m_geometryPipelineLayout = nullptr;
    nes::Pipeline                   m_skyboxPipeline = nullptr;
    nes::Pipeline                   m_gridPipeline = nullptr;
    nes::PipelineLayout             m_gridPipelineLayout = nullptr;
    nes::DeviceBuffer               m_globalConstantBuffer = nullptr;
    std::vector<FrameData>          m_frames{};
    nes::EFormat                    m_depthFormat = nes::EFormat::Unknown;

    // Camera Data:
    CameraState                     m_camera{};
    nes::Vec3                       m_inputMovement = nes::Vec3::Zero();
    nes::Vec2                       m_inputRotation = nes::Vec2::Zero();
    float                           m_cameraMoveSpeed = 50.f;
    bool                            m_cameraRotationEnabled = false;
};
