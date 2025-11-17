// PBRExampleApp.h
#pragma once

#include "Nessie/Application/Application.h"
#include "Nessie/Editor/EditorWindowManager.h"
#include "Nessie/Graphics/ImGui/ImGuiRenderer.h"

namespace nes
{
    class EditorWorld;
    class ViewportWindow;
}

//----------------------------------------------------------------------------------------------------
/// @brief : This example application renders a 3D Scene with a single mesh. It uses a Physically Based
///     Rendering pipeline to render the 3D object more akin to the real world.
//----------------------------------------------------------------------------------------------------
class PBRExampleApp final : public nes::Application
{
public:
    PBRExampleApp(nes::ApplicationDesc&& appDesc, nes::WindowDesc&& windowDesc, nes::RendererDesc&& rendererDesc);
    
private:
    // Application Interface:
    virtual void                        PushEvent(nes::Event& e) override;
    virtual bool                        Init() override;
    virtual void                        OnResize(const uint32 width, const uint32 height) override;
    virtual void                        PreShutdown() override;
    virtual void                        Update(const float deltaTime) override;
    virtual void                        Render(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context) override;

private:
    // Editor-Specific Members.
    void                                RenderImGuiEditor();
    void                                RenderSimulationControls();
    
    nes::ImGuiRenderer                  m_imgui = nullptr;
    nes::EditorWindowManager            m_windowManager{};
    std::shared_ptr<nes::ViewportWindow> m_viewportWindow = nullptr;
    nes::StrongPtr<nes::EditorWorld>     m_pEditorWorld = nullptr; 
    nes::AssetID                        m_worldAssetID = nes::kInvalidAssetID;
};
