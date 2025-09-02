// SimpleTriangle.h
#pragma once
#include "Nessie/Application/Application.h"
#include "Nessie/Asset/AssetBase.h"
#include "Nessie/Graphics/DeviceBuffer.h"
#include "Nessie/Graphics/DeviceImage.h"
#include "Nessie/Graphics/Pipeline.h"
#include "Nessie/Graphics/PipelineLayout.h"

//----------------------------------------------------------------------------------------------------
/// @brief : This example application renders a Rectangle to the screen.
//----------------------------------------------------------------------------------------------------
class RectangleApp final : public nes::Application
{
public:
    explicit RectangleApp(const nes::ApplicationDesc& appDesc) : nes::Application(appDesc) {}

    virtual bool Internal_AppInit() override;
    virtual void Internal_AppUpdate([[maybe_unused]] const float timeStep) override;
    virtual void Internal_AppRender(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context) override;
    virtual void Internal_AppShutdown() override;

private:
    struct Vertex
    {
        nes::Float2 m_position{};
        alignas (16) nes::Float3 m_color{};
    };
    
    nes::AssetID            m_shaderID = nes::kInvalidAssetID;
    nes::PipelineLayout     m_pipelineLayout = nullptr;
    nes::Pipeline           m_pipeline = nullptr;
    nes::DeviceBuffer       m_geometryBuffer = nullptr;
    nes::IndexBufferDesc    m_indexBufferDesc = {};
    nes::VertexBufferDesc   m_vertexBufferDesc = {};
    nes::DeviceQueue*       m_pTransferQueue = nullptr;
};