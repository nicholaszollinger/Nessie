// SimpleRenderer.h
#pragma once
#include "Nessie/Asset/AssetBase.h"
#include "Nessie/Graphics/DescriptorPool.h"
#include "Nessie/Graphics/DeviceBuffer.h"
#include "Nessie/Graphics/Pipeline.h"
#include "Nessie/Graphics/PipelineLayout.h"
#include "Nessie/Graphics/RenderTarget.h"
#include "Nessie/World/WorldRenderer.h"
#include "Nessie/World/Components/IDComponent.h"

class SimpleRenderer final : public nes::WorldRenderer
{
public:
    SimpleRenderer(nes::WorldBase& world) : WorldRenderer(world) {}
    
    // Component System API:
    virtual bool                Init() override;
    virtual void                Shutdown() override;
    virtual void                RegisterComponentTypes() override;
    virtual void                ProcessNewEntities() override;

    // WorldRenderer API:
    virtual void                RenderWorldWithCamera(const nes::WorldCamera& worldCamera, nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context) override;
    virtual nes::WorldCamera    GetActiveCamera() const override;
    virtual nes::RenderTarget*  GetFinalColorTarget() override { return &m_colorTarget; }
    virtual nes::RenderTarget*  GetFinalDepthTarget() override { return nullptr; }
    virtual void                OnViewportResize(const uint32 width, const uint32 height) override;

private:
    struct Vertex
    {
        nes::Float2                 m_position{};
        nes::Float2                 m_texCoord{};
        alignas (16) nes::Float3    m_color{};
    };

    struct UniformBufferObject
    {
        nes::Mat44                  m_model{};
        nes::Mat44                  m_view{};
        nes::Mat44                  m_proj{};
    };

    struct FrameData
    {
        nes::Descriptor             m_uniformBufferView = nullptr;
        nes::DescriptorSet          m_descriptorSet = nullptr;
        uint64                      m_uniformBufferViewOffset = 0;
    };

private:
    //----------------------------------------------------------------------------------------------------
    /// @brief : Creates the device buffer that will contain the vertices and indices.
    //----------------------------------------------------------------------------------------------------
    void                            CreateGeometryBuffer(nes::RenderDevice& device);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Create the uniform buffers that will have their data updated each frame.
    //----------------------------------------------------------------------------------------------------
    void                            CreateUniformBuffer(nes::RenderDevice& device);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Create the pipeline object use to render the Rectangle.
    //----------------------------------------------------------------------------------------------------
    void                            CreatePipeline(nes::RenderDevice& device);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Create the Descriptor Pool that will allow us to allocate DescriptorSets.
    //----------------------------------------------------------------------------------------------------
    void                            CreateDescriptorPool(nes::RenderDevice& device);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Allocate the DescriptorSets that will be used to store the current uniform buffer value to
    ///     send to the shader.
    //----------------------------------------------------------------------------------------------------
    void                            CreateDescriptorSets(nes::RenderDevice& device);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Update this frame's uniform buffer data.
    //----------------------------------------------------------------------------------------------------
    void                            UpdateUniformBuffer(const nes::WorldCamera& camera, const nes::RenderFrameContext& context);

private:
    nes::AssetID                    m_shaderID = nes::kInvalidAssetID;
    nes::AssetID                    m_textureID = nes::kInvalidAssetID;
    
    nes::RenderTarget               m_colorTarget = nullptr;
    nes::PipelineLayout             m_pipelineLayout = nullptr;
    nes::Pipeline                   m_pipeline = nullptr;
    nes::DeviceBuffer               m_geometryBuffer = nullptr;
    nes::IndexBufferRange           m_indexBufferDesc = {};
    nes::VertexBufferRange          m_vertexBufferDesc = {};
    nes::DeviceBuffer               m_uniformBuffer = nullptr; 
    nes::DescriptorPool             m_descriptorPool = nullptr;
    std::vector<FrameData>          m_frames{};
    nes::Descriptor                 m_imageView = nullptr;          // View of our texture.
    nes::Descriptor                 m_sampler = nullptr;            // Sampler for our texture.
    nes::EntityID                   m_activeCameraID = nes::kInvalidEntityID;
};
