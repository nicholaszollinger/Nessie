// TestGameApp.cpp

#include "TestGameApp.h"
#include "imgui.h"
#include "Math/Circle.h"
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
    
    static constexpr nes::Vector2 kHalfWindowSize = {800.f, 450.f};
    static nes::Circle circle(kHalfWindowSize, 400.f);
    static nes::Vector2 testPoint(kHalfWindowSize);
    static bool isInside = true;
    
    // Primitives...
    // Test Circle
    if (isInside)
        renderer.DrawCircle(circle.m_center, circle.m_radius, nes::LinearColor::Green());
    else
        renderer.DrawCircle(circle.m_center, circle.m_radius, nes::LinearColor::Red());
        
    // Test Point
    renderer.DrawCircle(testPoint, 2.f, nes::LinearColor::White());
    
    //renderer.DrawLine(kHalfWindowSize + nes::Vector2f{50.f, 50.f}, nes::Vector2f{5.f, 0.f}, nes::LinearColor::Green());
    //renderer.DrawRect(nes::Rectf(kHalfWindowSize.x, kHalfWindowSize.y + 100.f, 100.f, 100.f), nes::LinearColor::White());
    //renderer.DrawFillRect(nes::Rectf(kHalfWindowSize.x, kHalfWindowSize.y + 200.f, 100.f, 100.f), nes::LinearColor::Red());

    auto& io = ImGui::GetIO();
    ImGui::Begin("App Stats");
    ImGui::Text("Frame Rate: %.1f FPS", io.Framerate);
    ImGui::End();
    
    // Matrix Tests
    ImGui::Begin("Math Tests");
    ImGui::DragFloat2("Test Point", &testPoint.x);

#if 0
    static Matrix2x2 aMatrix = Matrix2x2::Identity();
    ImGui::InputFloat4("A Matrix", aMatrix.m[0]);
    
    static Matrix2x2 bMatrix = Matrix2x2::Identity();
    ImGui::InputFloat4("B Matrix", bMatrix.m[0]);
    
    Matrix2x2 result = aMatrix + bMatrix;
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
    static Matrix3x3 aMatrix3 = Matrix3x3(kValues);

    ImGui::Text("3x3 Matrix: %s", aMatrix3.ToString().c_str());
    ImGui::Text("3x3 Determinant: %.2f", aMatrix3.CalculateDeterminant());

    if (Matrix3x3 inverseMatrix; aMatrix3.TryGetInverse(inverseMatrix))
    {
        ImGui::Text("3x3 Inverse: %s", inverseMatrix.ToString().c_str());
    }

    else
    {
        ImGui::Text("3x3 Inverse: None");
    }
    
    // Triangles
    Triangle2D triangle{};
    isInside = triangle.ContainsPoint(testPoint);
    ImGui::Text("Triangle2D: %s", triangle.ToString().c_str());
    ImGui::Text("Triangle Contains Test Point: %s", isInside? "true" : "false");
    ImGui::Text("Signed Area: %.2f", triangle.Area());
#endif
    
    // Circles
    ImGui::DragFloat("Circle Radius", &circle.m_radius);
    isInside = circle.ContainsPoint(testPoint);
    ImGui::Text("Circle Contains Test Point: %s", isInside? "true" : "false");
    
    ImGui::End();
}