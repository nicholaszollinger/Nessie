// TestGameApp.cpp

#include "TestGameApp.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Math/Matrix.h"
#include "Tests/BoundingVolumesDemo.h"
#include "Tests/CircleDemo.h"
#include "Tests/TriangleDemo.h"
#include "Tests/VectorDemo.h"

static constexpr float kMinimumControlPanelWidth = 300.f;

nes::Application* CreateApplication(const nes::CommandLineArgs& args)
{
    return new TestGameApp(args);
}

TestGameApp::TestGameApp(const nes::CommandLineArgs& args)
    : Application(args)
{
    m_demos.emplace_back(new VectorDemo());
    m_demos.emplace_back(new CircleDemo());
    m_demos.emplace_back(new TriangleDemo());
    m_demos.emplace_back(new BoundingVolumesDemo());
}

bool TestGameApp::PostInit()
{
    for (auto* pDemo : m_demos)
    {
        if (!pDemo->Init())
        {
            NES_LOG("Failed to initialize Demo: ", pDemo->GetName(), "!");
            return false;
        }
        pDemo->Reset();
    }
    
    return true;
}

void TestGameApp::Update([[maybe_unused]] double deltaTime)
{
    // Clear the Screen
    static constexpr nes::LinearColor kClearColor = { 0.12f, 0.12f, 0.12f };
    const auto& renderer = GetRenderer();
    renderer.Clear(kClearColor);

    // Render the Window Panel for controlling the Demos:
    static constexpr ImGuiWindowFlags kControlWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
    const nes::WindowExtent windowPixelSize = GetWindow().GetExtent();
    const float viewportHeight = static_cast<float>(windowPixelSize.m_height);
    
    ImGui::SetNextWindowSize({ kMinimumControlPanelWidth, viewportHeight }, ImGuiCond_Once);
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSizeConstraints(ImVec2(kMinimumControlPanelWidth, viewportHeight), ImVec2(FLT_MAX, viewportHeight));
    ImGui::Begin("Test Demos", nullptr, kControlWindowFlags);

    const auto currentPanelWidth = ImGui::GetWindowWidth();
    RenderMenuBar();

    // Render the Current Demo, if selected:
    if (m_currentDemo >= 0 && m_currentDemo < static_cast<int>(m_demos.size()))
    {
        const nes::Rectf worldViewport(currentPanelWidth, 0.f, static_cast<float>(windowPixelSize.m_width) - currentPanelWidth, viewportHeight);
        RenderCurrentDemo(renderer, worldViewport);
    }
    else
    {
        ImGui::TextWrapped("No Demo Selected.\nUse the \"Demo\" dropdown to select a demo to run.");
    }
    ImGui::End();
    
    // FPS Counter:
    auto& io = ImGui::GetIO();
    static constexpr ImGuiWindowFlags kOverlayFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    static constexpr float kPadding = 5.f;
    
    const ImGuiViewport* pViewPort = ImGui::GetMainViewport();
    ImVec2 workPos = pViewPort->WorkPos;
    ImVec2 workSize = pViewPort->WorkSize;
    
    ImVec2 overlayPosition{workPos.x + workSize.x - kPadding, workPos.y + workSize.y - kPadding};
    constexpr ImVec2 overlayPivot(1.f, 1.f);
    ImGui::SetNextWindowPos(overlayPosition, ImGuiCond_Always, overlayPivot);
    
    ImGui::Begin("FPS Counter", nullptr, kOverlayFlags);
    ImGui::TextColored(ImVec4(1, 1, 1, 1), "%.1f FPS", io.Framerate);
    ImGui::End();
}

void TestGameApp::RenderMenuBar()
{
    if (ImGui::BeginMenuBar())
    {
        // File Menu
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Close"))
            {
                Quit();
            }

            ImGui::EndMenu();
        }
        
        // Select a Demo
        if (ImGui::BeginMenu("Demo"))
        {
            for (size_t i = 0; i < m_demos.size(); ++i)
            {
                if (ImGui::MenuItem(m_demos[i]->GetName()))
                {
                    m_currentDemo = static_cast<int>(i);
                    m_demos[m_currentDemo]->Reset();
                }
            }
            
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void TestGameApp::RenderCurrentDemo(const nes::Renderer& renderer, const nes::Rectf& worldViewport) const
{
    auto* pDemo = m_demos[m_currentDemo];

    // [TODO]: Add a formal 2D Camera.
    const nes::Vector2 worldCenter = worldViewport.Center();
    nes::Matrix3x3 viewMatrix = nes::Matrix3x3::Identity();
    viewMatrix.m[1][1] = -1.f; // Flip the Y Axis value.
    viewMatrix.m[2][0] = worldCenter.x; 
    viewMatrix.m[2][1] = worldCenter.y;

    // Render the Scene:
    pDemo->Render(renderer, viewMatrix);

    // Render the standard UI:
    ImGui::BeginChild(pDemo->GetName());
    ImGui::SeparatorText(pDemo->GetName());
    if (ImGui::Button("Reset"))
    {
        pDemo->Reset();
    }
    ImGui::Separator();

    // Render the Demo UI:
    pDemo->RenderImGui();
    ImGui::EndChild();
}
