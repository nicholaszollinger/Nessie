// TestGameApp.cpp

#include "TestGameApp.h"
#include "imgui.h"

nes::Application* CreateApplication(const nes::CommandLineArgs& args)
{
    return new TestGameApp(args);
}

TestGameApp::TestGameApp(const nes::CommandLineArgs& args)
    : Application(args)
{
    //
}

void TestGameApp::Update([[maybe_unused]] double deltaTime)
{
    static constexpr nes::LinearColor kClearColor = { 0.12f, 0.12f, 0.12f };

    const auto& renderer = GetRenderer();
    renderer.Clear(kClearColor);

    // Primitives...
    renderer.DrawCircle(nes::Vec2{0.f, 0.f}, 500.f, nes::LinearColor::Blue());
    renderer.DrawLine(nes::Vec2{50.f, 50.f}, nes::Vec2{5.f, 0.f}, nes::LinearColor::Green());
    renderer.DrawRect(nes::Rectf(0.f, 100.f, 100.f, 100.f), nes::LinearColor::White());
    renderer.DrawFillRect(nes::Rectf(0.f, 200.f, 100.f, 100.f), nes::LinearColor::Red());

    auto& io = ImGui::GetIO();
    ImGui::Begin("App Stats");
    ImGui::Text("Frame Rate: %.1f FPS", io.Framerate);
    ImGui::End();
}