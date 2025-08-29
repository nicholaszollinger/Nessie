// SimpleTriangle.h
#pragma once
#include "Nessie/Application/Application.h"
#include "Nessie/Asset/AssetBase.h"

//----------------------------------------------------------------------------------------------------
/// @brief : This example application renders a Triangle to the screen.
//----------------------------------------------------------------------------------------------------
class SimpleTriangle final : public nes::Application
{
public:
    explicit SimpleTriangle(const nes::ApplicationDesc& appDesc) : nes::Application(appDesc) {}

    virtual bool Internal_AppInit() override;
    virtual void Internal_AppUpdate([[maybe_unused]] const float timeStep) override;
    virtual void Internal_AppRender(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context) override;
    virtual void Internal_AppShutdown() override;

private:
    struct Vertex
    {
        nes::Vec2 m_position{};
        nes::Vec3 m_color{};
    };
    
    nes::AssetID            m_shaderID = nes::kInvalidAssetID;
    nes::PipelineLayout*    m_pPipelineLayout = nullptr;
    nes::Pipeline*          m_pPipeline = nullptr;
    nes::DeviceBuffer*      m_pVertexBuffer = nullptr;
    nes::VertexBufferDesc   m_vertexBufferDesc = {};
};