// EditorTestApp.h
#pragma once
#include "Nessie/Application/Application.h"
#include "Nessie/Editor/EditorWindowManager.h"
#include "Nessie/Graphics/ImGui/ImGuiRenderer.h"

class EditorTestApp final : public nes::Application
{
public:
    EditorTestApp(nes::ApplicationDesc&& appDesc, nes::WindowDesc&& windowDesc, nes::RendererDesc&& rendererDesc);
    virtual void PushEvent(nes::Event& e) override;

private:
    virtual bool    Init() override;
    virtual void    PreShutdown() override;
    virtual void    Update(const float deltaTime) override;
    virtual void    OnResize(const uint32 width, const uint32 height) override;
    virtual void    Render(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context) override;

private:
    void            RenderImGuiEditor();
    
private:
    nes::ImGuiRenderer          m_imgui = nullptr;
    nes::EditorWindowManager    m_windowManager{};
};