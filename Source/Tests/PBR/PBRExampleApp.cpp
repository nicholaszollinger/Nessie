// PBRExampleApp.cpp
#include "PBRExampleApp.h"

#include <fstream>
#include "imgui_internal.h"
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
#include "Nessie/FileIO/YAML/Serializers/YamlMathSerializers.h"

static constexpr ImVec4 kGreenColorLinear = ImVec4(0.4f, 0.8f, 0.4f, 1.0f);

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

    LoadUserSettings();

    // Register Editor Window Types:
    m_viewportWindow = m_windowManager.RegisterWindow<nes::ViewportWindow>();
    auto pHierarchyWindow = m_windowManager.RegisterWindow<nes::HierarchyWindow>();
    auto pInspectorWindow = m_windowManager.RegisterWindow<nes::InspectorWindow>();
    m_windowManager.RegisterWindow<nes::EditorConsole>();

    std::filesystem::path worldAssetPath;
    // Initialize the Window Manager, after all Windows have been registered.
    if (!m_windowManager.Init(worldAssetPath))
    {
        NES_ERROR("Failed to initialize EditorWindowManager!");
        return false;
    }
    
    // Create the Editor and Runtime worlds
    m_pEditorWorld = nes::Create<nes::EditorWorld>();
    auto pWorld = nes::Create<pbr::PBRExampleWorld>();
    m_pEditorWorld->SetRuntimeWorld(pWorld);
    
    // Load the World Asset
    // std::filesystem::path path = NES_CONTENT_DIR;
    // path /= "Worlds/PBRTestWorld.yaml";
    if (nes::AssetManager::LoadSync<nes::WorldAsset>(m_worldAssetID, worldAssetPath) != nes::ELoadResult::Success)
    {
        NES_ERROR("Failed to load World Asset!");
        return false;
    }

    auto pWorldAsset = nes::AssetManager::GetAsset<nes::WorldAsset>(m_worldAssetID);
    NES_ASSERT(pWorldAsset);
    auto& assetPack = pWorldAsset->GetAssetPack();

    auto onAssetLoaded = [this](const nes::AsyncLoadResult& loadResult)
    {
        [[maybe_unused]] auto& metadata = loadResult.GetAssetMetadata();
        NES_LOG("Loaded: {} Success: {}", metadata.m_assetName, loadResult.IsValid());
        m_loadProgress = loadResult.GetRequestProgress();
    };

    // Load the World's Assets, asynchronously:
    auto onComplete = [this](const bool succeeded)
    {
        if (succeeded)
        {
            NES_LOG("World load successful!");
            m_loadingDisplayString = {};
            m_pEditorWorld->SetWorldAsset(m_worldAssetID);
        }
        else
        {
            NES_ERROR("Failed to load World!");
            Quit();
        }
    };
    
    m_loadingDisplayString = "Loading World...";
    
    nes::AssetManager::LoadAssetPackAsync(assetPack, onComplete, onAssetLoaded);
    m_windowManager.SetWorld(m_pEditorWorld);

    // Reveal the window:
    GetWindow().ShowWindow();
    
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

void PBRExampleApp::LoadUserSettings()
{
    std::filesystem::path path = NES_SAVED_DIR;
    path /= "User/UProjectConfig.yaml";

    auto& window = GetWindow();
    if (!std::filesystem::exists(path))
    {
        window.CenterWindow();
        return;
    }
    
    nes::YamlInStream stream(path);
    auto root = stream.GetRoot();
    auto applicationWindow = root["ApplicationWindow"];

    nes::WindowState restoreState;
    applicationWindow["Position"].Read(restoreState.m_position, nes::IVec2(std::numeric_limits<int>::max()));
    applicationWindow["Size"].Read(restoreState.m_resolution, nes::IVec2(std::numeric_limits<int>::max()));

    // Set defaults if invalid.
    if (restoreState.m_resolution.x < 0 || restoreState.m_resolution.y < 0)
        restoreState.m_resolution = window.GetResolution();
    
    bool fullscreen;
    applicationWindow["IsFullscreen"].Read(fullscreen, false);

    bool vsyncEnabled;
    applicationWindow["Vsync"].Read(vsyncEnabled, false);

    if (fullscreen)
    {
        if (restoreState.m_position.x < 0 || restoreState.m_position.y < 0)
            window.SetWindowRestoreStateCentered(restoreState.m_resolution.x, restoreState.m_position.y);
        else
            window.SetWindowRestoreState(restoreState);        
        
        window.SetMaximized(true);
    }
    else
    {
        if (restoreState.m_position.x < 0 || restoreState.m_position.y < 0)
            window.CenterWindow();
        else
            window.SetPosition(restoreState.m_position.x, restoreState.m_position.y);
    }
    
    window.SetVsync(vsyncEnabled);
}

void PBRExampleApp::SaveUserSettings()
{
    std::filesystem::path path = NES_SAVED_DIR;
    path /= "User/UProjectConfig.yaml";
    
    std::ofstream stream(path.string());
    if (!stream.is_open())
    {
        NES_ERROR("Failed to create User file for ProjectConfig! You probably didn't create the file directories correctly!\n\tPath:", path.string());
        return;
    }
    
    nes::YamlOutStream writer(path, stream);

    if (!m_desc.m_isHeadless)
    {
        auto& window = GetWindow();
        const auto& restoreState = window.GetWindowRestoreState();

        writer.BeginMap("ApplicationWindow");
        writer.Write("Position", restoreState.m_position);
        writer.Write("Size", restoreState.m_resolution);
        writer.Write("Vsync", window.IsVsyncEnabled());
        writer.Write("IsFullscreen", window.IsMaximized());
        
        writer.EndMap();
    }
}

void PBRExampleApp::RenderImGuiEditor()
{
    // Set up the main viewport window
    ImGuiViewport* imGuiViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(imGuiViewport->WorkPos);
    ImGui::SetNextWindowSize(imGuiViewport->WorkSize);
    ImGui::SetNextWindowViewport(imGuiViewport->ID);

    // Window flags for the main container
    static constexpr ImGuiWindowFlags kMainWindowFlags = ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    // Make the window background transparent
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    const bool mainWindowOpen = ImGui::Begin("MainWindow", nullptr, kMainWindowFlags);
    ImGui::PopStyleVar(3);

    if (mainWindowOpen)
    {
        static constexpr float kTopBarHeight = 40.f;
        static constexpr float kBottomBarHeight = 25.f;
        const auto windowPadding = ImGui::GetCurrentWindow()->WindowPadding;
    
        // Render the top bar, with menu, window buttons, etc.  
        RenderWindowTopBar(kTopBarHeight);
        ImGui::SetCursorPos(ImVec2(windowPadding.x, kTopBarHeight + windowPadding.y));

        // Set up the dockspace for each of the Editor Windows to dock in.
        ImVec2 regionAvailable = ImGui::GetContentRegionAvail();
        regionAvailable.y -= kBottomBarHeight;
        m_windowManager.SetupDockSpace(regionAvailable.x, regionAvailable.y);
    
        // Render the bottom bar:
        RenderWindowBottomBar(kBottomBarHeight);
    }
    ImGui::End();

    // Render windows within the dockspace:
    if (mainWindowOpen)
        m_windowManager.RenderWindows();
}

void PBRExampleApp::RenderWindowTopBar(const float topBarHeight)
{
    auto& appWindow = GetWindow();
    
    const ImVec2 windowPadding = ImGui::GetCurrentWindow()->WindowPadding;
    ImGui::SetCursorPos(ImVec2(windowPadding.x, windowPadding.y));
    const ImVec2 titlebarMin = ImGui::GetCursorScreenPos();
    const ImVec2 titlebarMax = { ImGui::GetCursorScreenPos().x + ImGui::GetWindowWidth() - windowPadding.x * 2.0f,
                                 ImGui::GetCursorScreenPos().y + topBarHeight };
    auto* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(titlebarMin, titlebarMax, IM_COL32(21, 21, 21, 255));
    
    // [~TODO]: Logo for the top left.

    // Calculate button dimensions
    static constexpr float kIconLineThickness = 1.0f;
    static constexpr auto kIconDefaultLineColor = IM_COL32(255, 255, 255, 255);
    const float buttonWidth = topBarHeight;
    const float buttonHeight = topBarHeight;
    float iconSize = topBarHeight * 0.25f;
    const float rounding = iconSize / 10.f;
    constexpr float kWindowButtonSpacing = 0.f;

    // Calculate positions for right-aligned buttons
    const float windowButtonsWidth = buttonWidth * 3 + kWindowButtonSpacing * 2;
    const float windowButtonsStartX = ImGui::GetCurrentWindow()->Size.x - windowButtonsWidth - windowPadding.x;

    //------------------------------
    // Window Buttons:
    //------------------------------
    ImGui::SetCursorPos(ImVec2(windowButtonsStartX, 0.f));

    static constexpr auto kButtonTransparentLinear = ImVec4(0, 0, 0, 0);
    static constexpr auto kButtonHoveredLinear = ImVec4(1.f, 1.f, 1.f, 0.15f);
    static constexpr auto kButtonActiveLinear = kButtonHoveredLinear;

    ImGui::BeginGroup();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(kWindowButtonSpacing, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_Button, kButtonTransparentLinear);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kButtonHoveredLinear);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, kButtonActiveLinear);

    // Minimize button
    if (ImGui::Button("##minimizeButton", ImVec2(buttonWidth, buttonHeight)))
    {
        // Handle minimize
        appWindow.SetMinimized(true);
    }
    // Draw the Minimize Icon:
    {
        const auto min = ImGui::GetItemRectMin();
        const float x = min.x + (buttonWidth - iconSize) * 0.5f;
        const float y = min.y + (buttonHeight) * 0.5f;
        drawList->AddLine(ImVec2(x, y), ImVec2(x + iconSize, y), kIconDefaultLineColor, kIconLineThickness);
    }
    ImGui::SameLine();
    
    // Maximize/Restore button
    if (ImGui::Button("##maximizeButton", ImVec2(buttonWidth, buttonHeight)))
    {
        const bool isMaximized = appWindow.IsMaximized();
        appWindow.SetMaximized(!isMaximized);
    }
    // Draw the Maximize Icon:
    {
        const auto min = ImGui::GetItemRectMin();
        const float x = min.x + (buttonWidth - iconSize) * 0.5f;
        const float y = min.y + (buttonHeight - iconSize) * 0.5f;
        drawList->AddRect(ImVec2(x, y), ImVec2(x + iconSize, y + iconSize), kIconDefaultLineColor, rounding, ImDrawFlags_None, kIconLineThickness);
    }
    ImGui::SameLine();
    
    // Close button - different hover color (red)
    static constexpr auto kRedLinear = ImVec4(0.8f, 0.1f, 0.1f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kRedLinear);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, kRedLinear);
    if (ImGui::Button("##closeButton", ImVec2(buttonWidth, buttonHeight)))
    {
        Quit();
    }
    // Draw the 'X' Icon:
    {
        const auto min = ImGui::GetItemRectMin();
        const float x = min.x + (buttonWidth - iconSize) * 0.5f;
        const float y = min.y + (buttonHeight - iconSize) * 0.5f;
        drawList->AddLine(ImVec2(x, y), ImVec2(x + iconSize, y + iconSize), kIconDefaultLineColor, kIconLineThickness);
        drawList->AddLine(ImVec2(x + iconSize, y), ImVec2(x, y + iconSize), kIconDefaultLineColor, kIconLineThickness);
    }
    ImGui::PopStyleColor(5); // Pop all button colors
    ImGui::PopStyleVar(2);
    ImGui::EndGroup();

    //------------------------------
    // Drag Area:
    //------------------------------
    // Calculate the draggable area (everything except the window buttons on the right)
    const float dragAreaWidth = windowButtonsStartX - windowPadding.x;
    ImGui::SetCursorPos(ImVec2(windowPadding.x, windowPadding.y));

    // [TODO]: Make these member variables, instead of static values in the function.
    static float moveOffsetX = 0.f;
    static float moveOffsetY = 0.f;
    auto* rootWindow = ImGui::GetCurrentWindow()->RootWindow;
    ImGui::SetNextItemAllowOverlap();
    if (ImGui::InvisibleButton("##titleBarDragZone", ImVec2(dragAreaWidth, topBarHeight), ImGuiButtonFlags_PressedOnClick))
    {
        const ImVec2 point = ImGui::GetMousePos();
        const ImRect rect = rootWindow->Rect();
        
        // Calculate the difference between the cursor pos and window pos
        moveOffsetX = point.x - rect.Min.x;
        moveOffsetY = point.y - rect.Min.y;
    }
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
    {
        // [TODO]: Maximize on double-clicking the area:
    }
    if (!appWindow.IsMaximized() && ImGui::IsItemActive())
    {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            // [TODO]: Restore the window when dragging, instead of disallowing the
            // move altogether. I need to know what the restored size is. When I call maximize, I don't have
            // the previous resolution saved.
            // if (appWindow.IsMaximized())
            // {
            //     // Restore the window:
            //     appWindow.SetMaximized(false);
            // }

            // Move the Window:
            const ImVec2 point = ImGui::GetMousePos();
            appWindow.SetPosition(static_cast<int>(point.x - moveOffsetX),  static_cast<int>(point.y - moveOffsetY));
        }
    }

    //------------------------------
    // Top Bar Buttons:
    //------------------------------
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(kWindowButtonSpacing, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 0));
    ImGui::PushStyleColor(ImGuiCol_Button, kButtonTransparentLinear);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kButtonHoveredLinear);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, kButtonActiveLinear);
    ImGui::SetCursorPos(ImVec2(windowPadding.x, windowPadding.y));

    // Set the background to be opaque.
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    static constexpr auto kFileMenuSpace = ImVec2(8.f, 5.f);

    // File Menu
    if (ImGui::Button("File", ImVec2(0.f, buttonHeight)))
    {
        const auto min = ImGui::GetItemRectMin();
        const auto max = ImGui::GetItemRectMax();
        ImGui::SetNextWindowPos(ImVec2(min.x, max.y));
        ImGui::OpenPopup("##FileMenuPopup");
    }
    if (ImGui::BeginPopup("##FileMenuPopup"))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, kFileMenuSpace);
        if (ImGui::MenuItem("Close"))
        {
            Quit();
        }
        ImGui::PopStyleVar();
    
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    
    // Window Menu
    if (ImGui::Button("Window", ImVec2(0.f, buttonHeight)))
    {
        const auto min = ImGui::GetItemRectMin();
        const auto max = ImGui::GetItemRectMax();
        ImGui::SetNextWindowPos(ImVec2(min.x, max.y));
        ImGui::OpenPopup("##WindowMenuPopup");
    }
    if (ImGui::BeginPopup("##WindowMenuPopup"))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, kFileMenuSpace);
        m_windowManager.RenderWindowMenu();
        ImGui::PopStyleVar();

        ImGui::EndPopup();
    }
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
    ImGui::SameLine();

    //------------------------------
    // Simulation Controls
    //------------------------------
    iconSize = topBarHeight * 0.3f;
    const nes::EWorldSimState editorWorldState = m_pEditorWorld->GetSimState();

    // Right-align the controls, with a space between the window buttons.
    const float groupWidth = buttonWidth * 2.f;
    static constexpr float kSpaceFromWindowControls = 40.f;
    ImGui::SetCursorPos(ImVec2(windowButtonsStartX - kSpaceFromWindowControls - groupWidth, 0.f));

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    
    // Play/Pause Button (Green when stopped/paused, normal when playing)
    {
        const bool worldReadyToSimulate = m_pEditorWorld->GetCurrentWorldAsset() != nullptr;
        const bool showPlay = (editorWorldState == nes::EWorldSimState::Stopped || editorWorldState == nes::EWorldSimState::Paused);
        
        // Transparent background
        ImGui::PushStyleColor(ImGuiCol_Button, kButtonTransparentLinear);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kButtonHoveredLinear);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, kButtonActiveLinear);
    
        ImGui::BeginDisabled(!worldReadyToSimulate);
        if (ImGui::Button("##PlayPause", ImVec2(buttonWidth, buttonHeight)))
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
        const ImVec2 buttonMin = ImGui::GetItemRectMin();
        const ImVec2 buttonMax = ImGui::GetItemRectMax();
        const float iconMinX = buttonMin.x + (buttonWidth - iconSize) * 0.5f;
        const float iconMinY = buttonMin.y + (buttonHeight - iconSize) * 0.5f;
        
        const ImVec2 center = ImVec2((buttonMin.x + buttonMax.x) * 0.5f, (buttonMin.y + buttonMax.y) * 0.5f);
        const ImU32 iconColor = worldReadyToSimulate? ImGui::ColorConvertFloat4ToU32(kGreenColorLinear) : IM_COL32(100, 100, 100, 255);
        
        if (showPlay)
        {
            // Draw play triangle
            const ImVec2 p1 = ImVec2(iconMinX, iconMinY);
            const ImVec2 p2 = ImVec2(iconMinX, iconMinY + iconSize);
            const ImVec2 p3 = ImVec2(iconMinX + iconSize, center.y);
            drawList->AddTriangle(p1, p2, p3, iconColor, kIconLineThickness);
        }
        else
        {
            // Draw pause bars
            const float barSpacing = iconSize * 0.15f;
            const float barWidth = (iconSize - barSpacing) * 0.5f;
            
            // Left bar
            drawList->AddRect
            (
                ImVec2(iconMinX, iconMinY),
                ImVec2(iconMinX + barWidth, iconMinY + iconSize),
                iconColor,
                0.f,
                0,
                kIconLineThickness
            );
            
            // Right bar
            drawList->AddRect
            (
                ImVec2(iconMinX + barSpacing + barWidth, iconMinY),
                ImVec2(iconMinX + iconSize, iconMinY + iconSize),
                iconColor,
                0.f,
                0,
                kIconLineThickness
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
            // Red Background:
            ImGui::PushStyleColor(ImGuiCol_Button, kRedLinear);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kRedLinear);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, kRedLinear);
        }
        else
        {
            // Transparent Background.
            ImGui::PushStyleColor(ImGuiCol_Button, kButtonTransparentLinear);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kButtonHoveredLinear);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, kButtonActiveLinear);
        }
        
        ImGui::BeginDisabled(!isActive);
        if (ImGui::Button("##Stop", ImVec2(buttonWidth, buttonHeight)))
        {
            // End the simulation
            m_pEditorWorld->EndSimulation();
        }
        
        // Draw stop square
        ImU32 iconColor = isActive ? kIconDefaultLineColor : IM_COL32(100, 100, 100, 255);

        const auto min = ImGui::GetItemRectMin();
        const float x = min.x + (buttonWidth - iconSize) * 0.5f;
        const float y = min.y + (buttonHeight - iconSize) * 0.5f;
        drawList->AddRect(ImVec2(x, y), ImVec2(x + iconSize, y + iconSize), iconColor, rounding, ImDrawFlags_None, kIconLineThickness);
        
        ImGui::EndDisabled();
        ImGui::PopStyleColor(3);
        
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Stop");
    }

    ImGui::PopStyleVar(2);
}

void PBRExampleApp::RenderWindowBottomBar(const float bottomBarHeight)
{
    auto* pImGuiWindow = ImGui::GetCurrentWindow();
    const ImVec2 windowPadding = pImGuiWindow->WindowPadding;
    const auto windowSize = pImGuiWindow->Size;
    ImGui::SetCursorPos(ImVec2(windowPadding.x, windowSize.y - bottomBarHeight - windowPadding.y));

    // Screen space:
    const ImVec2 bottomBarMin = ImGui::GetCursorScreenPos();
    const ImVec2 bottomBarMax = { bottomBarMin.x + windowSize.x - windowPadding.x * 2.0f,
                                 bottomBarMin.y + bottomBarHeight };
    auto* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(bottomBarMin, bottomBarMax, IM_COL32(21, 21, 21, 255));
    
    static constexpr float kOffsetFromEdge = 20.f;

    if (ImGui::BeginChild("##BottomBarChild"))
    {
        if (!m_loadingDisplayString.empty())
        {
            // Get the available region in the child window
            const ImVec2 childSize = ImGui::GetContentRegionAvail();
        
            // Calculate sizes
            const ImVec2 textSize = ImGui::CalcTextSize(m_loadingDisplayString.c_str());
            static constexpr float kProgressBarWidth = 100.f;
            const float progressBarHeight = bottomBarHeight * 0.5f;
            const float spacing = ImGui::GetStyle().ItemSpacing.x;
        
            // Total width of text + spacing + progress bar
            const float totalWidth = textSize.x + spacing + kProgressBarWidth;
        
            // Calculate horizontal offset for right alignment
            const float offsetX = childSize.x - totalWidth - kOffsetFromEdge;
        
            // Calculate vertical offset for centering
            const float offsetY = (childSize.y - progressBarHeight) * 0.5f;
        
            // Set cursor position
            ImGui::SetCursorPosX(offsetX);
            ImGui::SetCursorPosY(offsetY);
            
            // Progress bar with context
            ImGui::Text(m_loadingDisplayString.c_str());
            ImGui::SameLine();
            ImGui::ProgressBar(m_loadProgress, ImVec2(kProgressBarWidth, progressBarHeight), "");
        }
        else
        {
            // Check mark or "ready"
            static constexpr const char* kReadyText = "Ready";  // Ready
            ImVec2 childSize = ImGui::GetContentRegionAvail();
            ImVec2 textSize = ImGui::CalcTextSize(kReadyText);
    
            ImGui::SetCursorPosX(childSize.x - textSize.x - kOffsetFromEdge);
            ImGui::SetCursorPosY((childSize.y - textSize.y) * 0.5f);
            ImGui::TextColored(kGreenColorLinear, kReadyText);
        }
        
        ImGui::EndChild();
    }
    
    
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

    SaveUserSettings();
    
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
        .EnableVsync(false)
        .RemoveDefaultDecoration();

    RendererDesc rendererDesc = nes::RendererDesc()
        .EnableValidationLayer()
        .RequireQueueType(EQueueType::Graphics)
        .RequireQueueType(EQueueType::Transfer);
    
    return std::make_unique<PBRExampleApp>(std::move(appDesc), std::move(windowDesc), std::move(rendererDesc));
}

NES_MAIN()