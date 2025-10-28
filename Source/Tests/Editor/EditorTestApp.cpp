// EditorTestApp.cpp
#include "EditorTestApp.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "Nessie/Application/EntryPoint.h"
#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Editor/Windows/EditorConsole.h"
#include "Nessie/Editor/Windows/HierarchyWindow.h"
#include "Nessie/Editor/Windows/InspectorWindow.h"
#include "Nessie/Editor/Windows/ViewportWindow.h"
#include "Nessie/Graphics/RenderDevice.h"
#include "Nessie/Graphics/Renderer.h"
#include "Nessie/Input/InputEvents.h"

EditorTestApp::EditorTestApp(nes::ApplicationDesc&& appDesc, nes::WindowDesc&& windowDesc, nes::RendererDesc&& rendererDesc)
    : nes::Application(std::move(appDesc), std::move(windowDesc), std::move(rendererDesc))
{
    //
}

void EditorTestApp::PushEvent(nes::Event& event)
{
    // [TEST]: Used to show logs in the console window in the editor. 
    if (nes::KeyEvent* keyEvent = event.Cast<nes::KeyEvent>())
    {
        if (keyEvent->GetKeyCode() == nes::EKeyCode::Space && keyEvent->GetAction() == nes::EKeyAction::Pressed)
        {
            NES_LOG("This is a test log!");
        }
    }
}

bool EditorTestApp::Init()
{
    std::filesystem::path iniSettingsPath = NES_CONFIG_DIR;
    iniSettingsPath /= "imgui.ini";
    
    nes::ImGuiDesc desc{};
    desc.m_pRenderQueue = nes::Renderer::GetRenderQueue();
    desc.m_pWindow = &GetWindow();
    desc.m_swapchainFormat = nes::Renderer::GetSwapchainFormat();
    desc.m_framesInFlight = nes::Renderer::GetMaxFramesInFlight();
    desc.m_iniSettingsPath = iniSettingsPath;
    m_imgui = nes::ImGuiRenderer(nes::Renderer::GetDevice(), desc);

    // Register Editor Window Types:
    m_windowManager.RegisterWindow<nes::ViewportWindow>();
    m_windowManager.RegisterWindow<nes::HierarchyWindow>();
    m_windowManager.RegisterWindow<nes::InspectorWindow>();
    m_windowManager.RegisterWindow<nes::EditorConsole>();
    
    if (!m_windowManager.Init())
    {
        NES_ERROR("Failed to initialize EditorWindowManager!");
        return false;
    }
    
    return true;
}

void EditorTestApp::PreShutdown()
{
    m_windowManager.Shutdown();
    
    // Close ImGui.
    m_imgui = nullptr;
}

void EditorTestApp::Update(const float)
{
    //
}

void EditorTestApp::OnResize(const uint32, const uint32)
{
    //auto& device = nes::DeviceManager::GetRenderDevice();
    //auto& window = GetWindow().GetNativeWindow();
    //auto pQueue = nes::Renderer::GetRenderQueue();
    
    // [TODO]: Handle DPI/Content scaling appropriately with ImGui?
}

void EditorTestApp::Render(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
{
    // Record ImGui Draw calls:
    m_imgui.BeginFrame();
    RenderImGuiEditor();
    m_imgui.CreateRenderData();
    
    // Transition the swapchain image to color attachment
    {
        nes::ImageBarrierDesc swapchainBarrier = nes::ImageBarrierDesc()
            .SetImage(context.GetSwapchainImage())
            .SetLayout(nes::EImageLayout::Undefined, nes::EImageLayout::ColorAttachment);

        nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
            .SetImageBarriers({swapchainBarrier } );
        
        commandBuffer.SetBarriers(barrierGroup);
    }

    // Set the swapchain image as our color render target:
    nes::RenderTargetsDesc renderTargetsDesc = nes::RenderTargetsDesc()
        .SetColorTargets(&context.GetSwapchainImageDescriptor());

    // Get the viewport and scissor that will encompass the entire image.
    const nes::Viewport viewport = context.GetSwapchainViewport();
    const nes::Scissor scissor(viewport);
    
    // Begin Rendering to the Swapchain image.
    commandBuffer.BeginRendering(renderTargetsDesc);
    {
        // Clear the screen to a dark grey color:
        nes::ClearDesc clearDesc = nes::ClearDesc::Color(nes::LinearColor(0.01f, 0.01f, 0.01f, 1.0f));
        commandBuffer.ClearRenderTargets(clearDesc);
        commandBuffer.SetViewports(viewport);
        commandBuffer.SetScissors(scissor);
        
        m_imgui.RenderToSwapchain(commandBuffer, context);
        
        // End Rendering to the Swapchain image.
        commandBuffer.EndRendering();
    }
    
    // Transition the Swapchain image to Present layout to present!
    {
        nes::ImageBarrierDesc imageBarrier = nes::ImageBarrierDesc()
            .SetImage(context.GetSwapchainImage())
            .SetLayout(nes::EImageLayout::ColorAttachment, nes::EImageLayout::Present);
    
        nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
            .SetImageBarriers(imageBarrier);
        
        commandBuffer.SetBarriers(barrierGroup);
    }
    
    m_imgui.EndFrame();
}

void EditorTestApp::RenderImGuiEditor()
{
    m_windowManager.SetupMainWindowAndDockSpace();
    
    // Menu Bar (fixed at top)
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            //if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Handle save */ }
            //ImGui::Separator();
            if (ImGui::MenuItem("Quit")) { Quit(); }
            ImGui::EndMenu();
        }

        m_windowManager.RenderWindowMenu();
        ImGui::EndMenuBar();
    }
    
    ImGui::End();
    m_windowManager.RenderWindows();
}

std::unique_ptr<nes::Application> nes::CreateApplication(const nes::CommandLineArgs& args)
{
    ApplicationDesc appDesc = ApplicationDesc(args)
        .SetApplicationName("Editor Test")
        .SetIsHeadless(false);
       
    WindowDesc windowDesc = WindowDesc()
        .SetResolution(1920, 1080)
        .SetLabel("Editor Test")
        .SetWindowMode(EWindowMode::Windowed)
        .EnableResize(true)
        .EnableVsync(false);

    RendererDesc rendererDesc = RendererDesc()
        .EnableValidationLayer()
        .RequireQueueType(EQueueType::Graphics)
        .RequireQueueType(EQueueType::Transfer);

    return std::make_unique<EditorTestApp>(std::move(appDesc), std::move(windowDesc), std::move(rendererDesc));
}

NES_MAIN()