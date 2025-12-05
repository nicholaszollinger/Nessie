// PBRExampleApp.cpp
#include "PBRExampleApp.h"

#include "Editor/DayNightSimComponentInspector.h"
#include "Editor/LightComponentInspectors.h"
#include "Editor/MeshComponentInspector.h"
#include "Editor/SkyboxComponentInspector.h"
#include "Nessie/Application/EntryPoint.h"
#include "Nessie/Editor/EditorWorld.h"
#include "Nessie/Editor/Inspectors/Components/CameraComponentInspector.h"
#include "Nessie/Editor/Inspectors/Components/FreeCamMovementComponentInspector.h"
#include "Nessie/Editor/Inspectors/Components/TransformComponentInspector.h"
#include "Nessie/Editor/Windows/EditorConsole.h"
#include "Nessie/Editor/Windows/HierarchyWindow.h"
#include "Nessie/Editor/Windows/InspectorWindow.h"
#include "Nessie/Editor/Windows/ViewportWindow.h"
#include "Nessie/Graphics/Renderer.h"
#include "Nessie/Graphics/Texture.h"
#include "Nessie/Graphics/Shader.h"
#include "PBRExampleWorld.h"
#include "Nessie/Input/InputManager.h"

PBRExampleApp::PBRExampleApp(nes::ApplicationDesc&& appDesc, nes::WindowDesc&& windowDesc, nes::RendererDesc&& rendererDesc)
    : nes::Application(std::move(appDesc), std::move(windowDesc), std::move(rendererDesc))
{
    //
}

void PBRExampleApp::PushEvent(nes::Event& e)
{

    if (m_pEditorWorld)
        m_pEditorWorld->OnEvent(e);
}

bool PBRExampleApp::Init()
{
    NES_REGISTER_ASSET_TYPE(nes::Shader);
    NES_REGISTER_ASSET_TYPE(nes::Texture);
    NES_REGISTER_ASSET_TYPE(nes::TextureCube);
    NES_REGISTER_ASSET_TYPE(pbr::MeshAsset);
    NES_REGISTER_ASSET_TYPE(pbr::PBRMaterial);
    NES_REGISTER_ASSET_TYPE(nes::WorldAsset);

    // Register Inspectors
    nes::EditorInspectorRegistry::RegisterInspector<nes::TransformComponentInspector>();
    nes::EditorInspectorRegistry::RegisterInspector<nes::CameraComponentInspector>();
    nes::EditorInspectorRegistry::RegisterInspector<pbr::DirectionalLightComponentInspector>();
    nes::EditorInspectorRegistry::RegisterInspector<pbr::PointLightComponentInspector>();
    nes::EditorInspectorRegistry::RegisterInspector<pbr::MeshComponentInspector>();
    nes::EditorInspectorRegistry::RegisterInspector<nes::FreeCamMovementComponentInspector>();
    nes::EditorInspectorRegistry::RegisterInspector<pbr::DayNightSimComponentInspector>();
    nes::EditorInspectorRegistry::RegisterInspector<pbr::SkyboxComponentInspector>();

    // Setup ImGui:
    nes::ImGuiDesc desc{};
    desc.m_pRenderQueue = nes::Renderer::GetRenderQueue();
    desc.m_pWindow = &GetWindow();
    desc.m_swapchainFormat = nes::Renderer::GetSwapchainFormat();
    desc.m_framesInFlight = nes::Renderer::GetMaxFramesInFlight();
    m_imgui.Init(nes::Renderer::GetDevice(), desc);

    // Register Editor Window Types:
    m_viewportWindow = m_windowManager.RegisterWindow<nes::ViewportWindow>();
    auto pHierarchyWindow = m_windowManager.RegisterWindow<nes::HierarchyWindow>();
    auto pInspectorWindow = m_windowManager.RegisterWindow<nes::InspectorWindow>();
    m_windowManager.RegisterWindow<nes::EditorConsole>();

    // Initialize the Window Manager, after all Windows have been registered.
    if (!m_windowManager.Init())
    {
        NES_ERROR("Failed to initialize EditorWindowManager!");
        return false;
    }
    
    // Create the Editor and Runtime worlds
    m_pEditorWorld = nes::Create<nes::EditorWorld>();
    auto pWorld = nes::Create<pbr::PBRExampleWorld>();
    m_pEditorWorld->SetRuntimeWorld(pWorld);

    // Load the World Asset
    std::filesystem::path path = NES_CONTENT_DIR;
    path /= "Worlds/PBRTestWorld.yaml";
    if (nes::AssetManager::LoadSync<nes::WorldAsset>(m_worldAssetID, path) != nes::ELoadResult::Success)
    {
        NES_ERROR("Failed to load World Asset!");
        return false;
    }

    auto pWorldAsset = nes::AssetManager::GetAsset<nes::WorldAsset>(m_worldAssetID);
    NES_ASSERT(pWorldAsset);
    auto& assetPack = pWorldAsset->GetAssetPack();

    auto onAssetLoaded = [](const nes::AsyncLoadResult& loadResult)
    {
        [[maybe_unused]] auto& metadata = loadResult.GetAssetMetadata();
        NES_LOG("Loaded: {} Success: {}", metadata.m_assetName, loadResult.IsValid());
    };

    // Load the World's Assets, asynchronously:
    auto onComplete = [this](const bool succeeded)
    {
        if (succeeded)
        {
            NES_LOG("World load successful!");
            m_pEditorWorld->SetWorldAsset(m_worldAssetID);
        }
        else
        {
            NES_ERROR("Failed to load World!");
            Quit();
        }
    };
    
    nes::AssetManager::LoadAssetPackAsync(assetPack, onComplete, onAssetLoaded);
    m_windowManager.SetWorld(m_pEditorWorld);
    
    return true;
}

void PBRExampleApp::Update(const float deltaTime)
{
    if (m_viewportWindow)
        m_viewportWindow->Tick(deltaTime);
    
    if (m_pEditorWorld)
    {
        const float worldDeltaTime = (m_pEditorWorld->IsSimulating() && m_pEditorWorld->IsPaused())? 0.f : deltaTime;
        m_pEditorWorld->Tick(worldDeltaTime);
    }
}

void PBRExampleApp::OnResize(const uint32, const uint32)
{
    // Resizing is handled in the viewport window.
    // if (m_pWorld)
    //     m_pWorld->OnResize(width, height);
}

void PBRExampleApp::Render(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
{
    ImGuiIO& io = ImGui::GetIO();
    const nes::ECursorMode cursorMode = nes::InputManager::GetCursorMode();
    if (cursorMode == nes::ECursorMode::Disabled)
    {
        // Mouse is locked - tell ImGui the mouse is unavailable
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    }
    
    // Record ImGui Draw calls:
    m_imgui.BeginFrame();
    RenderImGuiEditor();
    m_imgui.CreateRenderData();
    
    // Render the World into the offscreen targets (non-swapchain targets)
    if (m_viewportWindow)
    {
        m_viewportWindow->RenderWorld(commandBuffer, context);
    }

    // Render ImGui data into the Swapchain:
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
            .SetLayout(nes::EImageLayout::ColorAttachment, nes::EImageLayout::Present)
            .SetBarrierStage(nes::EPipelineStageBits::ColorAttachment, nes::EPipelineStageBits::All);
    
        nes::BarrierGroupDesc barrierGroup = nes::BarrierGroupDesc()
            .SetImageBarriers(imageBarrier);
        
        commandBuffer.SetBarriers(barrierGroup);
    }
    
    m_imgui.EndFrame();
}

void PBRExampleApp::RenderImGuiEditor()
{
    m_windowManager.BeginMainWindowAndDockSpace();
    
    // Menu Bar (fixed at top)
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Quit"))
                Quit();
            
            ImGui::EndMenu();
        }

        m_windowManager.RenderWindowMenu();
        ImGui::EndMenuBar();
    }
    
    RenderSimulationControls();
    m_windowManager.EndMainWindowAndDockSpace();

    // Render windows within the dockspace.
    m_windowManager.RenderWindows();
}

void PBRExampleApp::RenderSimulationControls()
{
    // Toolbar configuration
    static constexpr float kToolbarHeight = 30.0f;
    static constexpr float kToolbarPaddingX = 8.0f;
    static constexpr float kToolbarPaddingY = 4.0f;
    static constexpr float kButtonSpacing = 8.0f;

    // Calculate button size based on available height (with padding)
    static constexpr float kButtonSize = kToolbarHeight - (kToolbarPaddingY * 2);
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(kToolbarPaddingX, kToolbarPaddingY));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(kButtonSpacing, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);

    // Get the content region for proper sizing
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();

    ImGui::BeginChild("##SimulationControls", ImVec2(contentRegion.x, kToolbarHeight));
        
    // Calculate total width and center position
    static constexpr int kButtonCount = 2;
    static constexpr float kTotalWidth = (kButtonSize * kButtonCount) + (kButtonSpacing * (kButtonCount - 1));
    const float centerPosX = (ImGui::GetContentRegionAvail().x - kTotalWidth) * 0.5f;

    if (centerPosX > 0)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + centerPosX);
    
    // Vertical centering (should already be centered due to padding, but ensure it)
    static constexpr float kCursorStartY = (kToolbarHeight - kButtonSize) * 0.5f;
    ImGui::SetCursorPosY(kCursorStartY);

    const nes::EWorldSimState editorWorldState = m_pEditorWorld->GetSimState();
    
    // Play/Pause Button (Green when stopped/paused, normal when playing)
    {
        const bool worldReadyToSimulate = m_pEditorWorld->GetCurrentWorldAsset() != nullptr;
        const bool showPlay = (editorWorldState == nes::EWorldSimState::Stopped || editorWorldState == nes::EWorldSimState::Paused);

        if (!worldReadyToSimulate)
        {
            // Grey button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        }
        else if (showPlay)
        {
            // Green play button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
        }
        else
        {
            // Normal pause button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        }

        ImGui::BeginDisabled(!worldReadyToSimulate);
        if (ImGui::Button("##PlayPause", ImVec2(kButtonSize, kButtonSize)))
        {
            if (showPlay)
            {
                if (editorWorldState == nes::EWorldSimState::Stopped)
                {
                    // Begin the simulation.
                    m_pEditorWorld->BeginSimulation();
                }
                else
                {
                    // Resume the simulation.
                    m_pEditorWorld->SetPaused(false);
                }
            }
            else
            {
                // Pause the simulation.
                m_pEditorWorld->SetPaused(true);
            }
        }
        
        // Draw icon
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonMax = ImGui::GetItemRectMax();
        const ImVec2 center = ImVec2((buttonMin.x + buttonMax.x) * 0.5f, (buttonMin.y + buttonMax.y) * 0.5f);
        const ImU32 iconColor = worldReadyToSimulate? IM_COL32(255, 255, 255, 255) : IM_COL32(100, 100, 100, 255);
        
        if (showPlay)
        {
            // Draw play triangle
            static constexpr float kTriangleSize = kButtonSize * 0.35f;
            const ImVec2 p1 = ImVec2(center.x - kTriangleSize * 0.3f, center.y - kTriangleSize * 0.6f);
            const ImVec2 p2 = ImVec2(center.x - kTriangleSize * 0.3f, center.y + kTriangleSize * 0.6f);
            const ImVec2 p3 = ImVec2(center.x + kTriangleSize * 0.7f, center.y);
            drawList->AddTriangleFilled(p1, p2, p3, iconColor);
        }
        else
        {
            // Draw pause bars
            static constexpr float kBarWidth = kButtonSize * 0.15f;
            static constexpr float kBarHeight = kButtonSize * 0.5f;
            static constexpr float kBarSpacing = kButtonSize * 0.1f;
            
            // Left bar
            drawList->AddRectFilled
            (
                ImVec2(center.x - kBarSpacing - kBarWidth, center.y - kBarHeight * 0.5f),
                ImVec2(center.x - kBarSpacing, center.y + kBarHeight * 0.5f),
                iconColor
            );
            
            // Right bar
            drawList->AddRectFilled
            (
                ImVec2(center.x + kBarSpacing, center.y - kBarHeight * 0.5f),
                ImVec2(center.x + kBarSpacing + kBarWidth, center.y + kBarHeight * 0.5f),
                iconColor
            );
        }

        ImGui::EndDisabled();
        ImGui::PopStyleColor(3);
        
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip(showPlay ? "Play" : "Pause");
    }
        
    ImGui::SameLine();

    // Stop Button (Red when running, grey when stopped)
    {
        const bool isActive = (editorWorldState != nes::EWorldSimState::Stopped);
        
        if (isActive)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        }
        
        ImGui::BeginDisabled(!isActive);
        
        if (ImGui::Button("##Stop", ImVec2(kButtonSize, kButtonSize)))
        {
            // End the simulation
            m_pEditorWorld->EndSimulation();
        }
        
        // Draw stop square
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonMax = ImGui::GetItemRectMax();
        const ImVec2 center = ImVec2((buttonMin.x + buttonMax.x) * 0.5f, (buttonMin.y + buttonMax.y) * 0.5f);
        
        static constexpr float kSquareSize = kButtonSize * 0.5f;
        
        ImU32 iconColor = isActive ? IM_COL32(255, 255, 255, 255) : IM_COL32(100, 100, 100, 255);
        drawList->AddRectFilled
        (
            ImVec2(center.x - kSquareSize * 0.5f, center.y - kSquareSize * 0.5f),
            ImVec2(center.x + kSquareSize * 0.5f, center.y + kSquareSize * 0.5f),
            iconColor
        );
        
        ImGui::EndDisabled();
        ImGui::PopStyleColor(3);
        
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Stop");
    }
        
    //ImGui::PopStyleColor();
    ImGui::EndChild();
    ImGui::PopStyleVar(3);
}

void PBRExampleApp::PreShutdown()
{
    if (m_pEditorWorld)
    {
        if (m_pEditorWorld->IsSimulating())
        {
            m_pEditorWorld->EndSimulation();
        }
        
        m_pEditorWorld->Destroy();
        m_pEditorWorld = nullptr;
    }

    m_viewportWindow = nullptr;
    m_windowManager.Shutdown();
    
    // Close ImGui.
    m_imgui.Shutdown();
}

std::unique_ptr<nes::Application> nes::CreateApplication(const nes::CommandLineArgs& args)
{
    ApplicationDesc appDesc(args);
    appDesc.SetApplicationName("PBRExampleApp")
        .SetIsHeadless(false);

    WindowDesc windowDesc = WindowDesc()
        .SetResolution(1920, 1080)
        .SetLabel("PBR Example")
        .SetWindowMode(EWindowMode::Windowed)
        .EnableResize(true)
        .EnableVsync(false);

    RendererDesc rendererDesc = nes::RendererDesc()
        .EnableValidationLayer()
        .RequireQueueType(EQueueType::Graphics)
        .RequireQueueType(EQueueType::Transfer);
    
    return std::make_unique<PBRExampleApp>(std::move(appDesc), std::move(windowDesc), std::move(rendererDesc));
}

NES_MAIN()