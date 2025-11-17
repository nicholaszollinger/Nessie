// EditorTestApp.cpp
#include "EditorTestApp.h"

#include "Nessie/Application/EntryPoint.h"
#include "Nessie/Application/Device/DeviceManager.h"
#include "Nessie/Asset/AssetManager.h"
#include "Nessie/Editor/EditorInspector.h"
#include "Nessie/Editor/Windows/EditorConsole.h"
#include "Nessie/Editor/Windows/HierarchyWindow.h"
#include "Nessie/Editor/Windows/InspectorWindow.h"
#include "Nessie/Editor/Windows/ViewportWindow.h"
#include "Nessie/Graphics/RenderDevice.h"
#include "Nessie/Graphics/Renderer.h"
#include "Nessie/Input/InputEvents.h"
#include "Nessie/Input/InputManager.h"
#include "Nessie/Editor/Inspectors/Components/TransformComponentInspector.h"
#include "World/Components/TextureAssetComponent.h"
#include "Nessie/Graphics/Texture.h"

EditorTestApp::EditorTestApp(nes::ApplicationDesc&& appDesc, nes::WindowDesc&& windowDesc, nes::RendererDesc&& rendererDesc)
    : nes::Application(std::move(appDesc), std::move(windowDesc), std::move(rendererDesc))
{
    //
}

void EditorTestApp::PushEvent(nes::Event&)
{
    //
}

bool EditorTestApp::Init()
{
    NES_REGISTER_ASSET_TYPE(nes::WorldAsset);
    NES_REGISTER_ASSET_TYPE(nes::Texture);
    NES_REGISTER_COMPONENT(TextureAssetComponent);
    
    nes::EditorInspectorRegistry::RegisterInspector<nes::TransformComponentInspector>();
    nes::EditorInspectorRegistry::RegisterInspector<TextureAssetComponentInspector>();
    
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
    
    if (!m_windowManager.Init())
    {
        NES_ERROR("Failed to initialize EditorWindowManager!");
        return false;
    }

    // Create the runtime world
    m_pWorld = nes::Create<TestWorld>();
    if (!m_pWorld->Init())
    {
        NES_ERROR("Failed to initialize Test World!");
        return false;
    }
    
    // Load the World Asset:
    std::filesystem::path path = NES_CONTENT_DIR;
    path /= "Worlds/EditorTestWorld.yaml";
    if (nes::AssetManager::LoadSync<nes::WorldAsset>(m_currentWorldAsset, path) != nes::ELoadResult::Success)
    {
        NES_ERROR("Failed to load World Asset!");
        return false;
    }

    auto pWorldAsset = nes::AssetManager::GetAsset<nes::WorldAsset>(m_currentWorldAsset);
    NES_ASSERT(pWorldAsset);
    auto& assetPack = pWorldAsset->GetAssetPack();

    // Load the World's Assets, asynchronously:
    auto onComplete = [this](const bool succeeded)
    {
        if (succeeded)
        {
            NES_LOG("World load successful!");
    
            // Merge the entities from the World Asset into the runtime world.
            auto pWorldAsset = nes::AssetManager::GetAsset<nes::WorldAsset>(m_currentWorldAsset);
            NES_ASSERT(pWorldAsset);
            m_pWorld->MergeWorld(*pWorldAsset);
            m_windowManager.SetWorld(m_pWorld);
        }
        else
        {
            NES_ERROR("Failed to load World!");
            Quit();
        }
    };
    nes::AssetManager::LoadAssetPackAsync(assetPack, onComplete);
    
    return true;
}

void EditorTestApp::PreShutdown()
{
    // Save the world information to disc.
    auto pWorldAsset = nes::AssetManager::GetAsset<nes::WorldAsset>(m_currentWorldAsset);
    if (pWorldAsset && m_pWorld)
    {
        m_pWorld->ExportToAsset(*pWorldAsset);
        nes::AssetManager::SaveAssetSync(m_currentWorldAsset);
    }
    
    if (m_pWorld)
    {
        m_pWorld->Destroy();
        m_pWorld = nullptr;
    }
    
    m_viewportWindow = nullptr;
    m_windowManager.Shutdown();
    
    // Close ImGui.
    m_imgui.Shutdown();
}

void EditorTestApp::Update(const float deltaTime)
{
    if (m_viewportWindow)
        m_viewportWindow->Tick(deltaTime);

    if (m_pWorld)
        m_pWorld->Tick(deltaTime);
}

void EditorTestApp::OnResize(const uint32, const uint32)
{
    // [TODO]: Handle DPI/Content scaling appropriately with ImGui?
}

void EditorTestApp::Render(nes::CommandBuffer& commandBuffer, const nes::RenderFrameContext& context)
{
    // Render the World into the offscreen targets (non-swapchain targets)
    if (m_viewportWindow)
    {
        m_viewportWindow->RenderWorld(commandBuffer, context);
    }
    
    // Record ImGui Draw calls:
    m_imgui.BeginFrame();
    RenderImGuiEditor();
    m_imgui.CreateRenderData();
    
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

void EditorTestApp::RenderImGuiEditor()
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