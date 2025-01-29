// TestGameApp.cpp

#include "TestGameApp.h"
#include "imgui.h"
#include "Math/Matrix.h"
#include "Math/Triangle.h"

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

    constexpr nes::Vector2f kHalfWindowSize = {800.f, 450.f};

    // Primitives...
    renderer.DrawCircle(kHalfWindowSize, 400.f, nes::LinearColor::Blue());
    renderer.DrawLine(kHalfWindowSize + nes::Vector2f{50.f, 50.f}, nes::Vector2f{5.f, 0.f}, nes::LinearColor::Green());
    renderer.DrawRect(nes::Rectf(kHalfWindowSize.x, kHalfWindowSize.y + 100.f, 100.f, 100.f), nes::LinearColor::White());
    renderer.DrawFillRect(nes::Rectf(kHalfWindowSize.x, kHalfWindowSize.y + 200.f, 100.f, 100.f), nes::LinearColor::Red());

    auto& io = ImGui::GetIO();
    ImGui::Begin("App Stats");
    ImGui::Text("Frame Rate: %.1f FPS", io.Framerate);
    ImGui::End();
    
    // Matrix Tests
    ImGui::Begin("Matrices");
    static nes::Matrix2x2f aMatrix = nes::Matrix2x2f::Identity();
    ImGui::InputFloat4("A Matrix", aMatrix.m[0]);
    
    static nes::Matrix2x2f bMatrix = nes::Matrix2x2f::Identity();
    ImGui::InputFloat4("B Matrix", bMatrix.m[0]);
    
    nes::Matrix2x2f result = aMatrix + bMatrix;
    ImGui::Text("A + B: %s", result.ToString().c_str());

    result = aMatrix - bMatrix;
    ImGui::Text("A - B: %s", result.ToString().c_str());

    result = aMatrix * bMatrix;
    ImGui::Text("A * B: %s", result.ToString().c_str());
    
    result = bMatrix * aMatrix;
    ImGui::Text("B * A: %s", result.ToString().c_str());

    // Transpose
    ImGui::Text("Transpose of A: %s", aMatrix.GetTranspose().ToString().c_str());
    ImGui::Text("Determinant of A: %.2f", aMatrix.CalculateDeterminant());

    constexpr float kValues[]
    {
        -4.f, -3.f, 3.f,
        0.f, 2.f, -2.f,
        1.f, 4.f, -1.f,
    };
    
    // 3x3
    static nes::Matrix3x3f aMatrix3 = nes::Matrix3x3f(kValues);

    ImGui::Text("3x3 Matrix: %s", aMatrix3.ToString().c_str());
    ImGui::Text("3x3 Determinant: %.2f", aMatrix3.CalculateDeterminant());

    if (nes::Matrix3x3f inverseMatrix; aMatrix3.TryGetInverse(inverseMatrix))
    {
        ImGui::Text("3x3 Inverse: %s", inverseMatrix.ToString().c_str());
    }

    else
    {
        ImGui::Text("3x3 Inverse: None");
    }

    static Vector2 position2D{};
    
    ImGui::InputFloat2("Test Point", &position2D.x);
    nes::TTriangle2<float> triangle{};
    const bool isInside = triangle.ContainsPoint(position2D);
    ImGui::Text("Triangle2D: %s", triangle.ToString().c_str());
    ImGui::Text("Triangle Contains Test Point: %s", isInside? "true" : "false");
    ImGui::Text("Signed Area: %.2f", triangle.Area());
    
    ImGui::End();
}