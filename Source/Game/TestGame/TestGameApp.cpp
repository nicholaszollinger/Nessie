// TestGameApp.cpp

#include "TestGameApp.h"
#include "imgui.h"
#include "Math/Matrix.h"

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
    
    // Matrix Tests
    //ImGui::InputFloat("Scalar to Multiply", &s_value);
    // static int s_row = 0;
    // static int s_column = 0;
    // ImGui::SliderInt("Matrix Row", &s_row, 0, 1);
    // const auto span = result[s_row];
    // ImGui::SliderInt("Matrix Column", &s_column, 0, 1);
    // ImGui::Text("Value: %.2f", span[s_column]);

    ImGui::Begin("Matrices");
    static nes::SquareMatrix<2> aMatrix = nes::SquareMatrix<2>::Identity();
    ImGui::InputFloat4("A Matrix", aMatrix.GetData());
    
    static nes::SquareMatrix<2> bMatrix = nes::SquareMatrix<2>::Identity();
    ImGui::InputFloat4("B Matrix", bMatrix.GetData());
    // static float s_value = 1.f;
    // ImGui::InputFloat("Set the 3rd Value", &s_value);
    // result[1][0] = s_value;
    nes::SquareMatrix<2> result = aMatrix + bMatrix;
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

    // 3x3
    static nes::Matrix3x3 aMatrix3 = nes::Matrix3x3::Identity();
    aMatrix3[0][0] = -4;
    aMatrix3[0][1] = -3;
    aMatrix3[0][2] = 3;

    aMatrix3[1][0] = 0;
    aMatrix3[1][1] = 2;
    aMatrix3[1][2] = -2;

    aMatrix3[2][0] = 1;
    aMatrix3[2][1] = 4;
    aMatrix3[2][2] = -1;

    ImGui::Text("3x3 Matrix: %s", aMatrix3.ToString().c_str());
    ImGui::Text("3x3 Determinant: %.2f", aMatrix3.CalculateDeterminant());

    if (nes::SquareMatrix<3> inverseMatrix; aMatrix3.TryGetInverse(inverseMatrix))
    {
        ImGui::Text("3x3 Inverse: %s", inverseMatrix.ToString().c_str());
    }

    else
    {
        ImGui::Text("3x3 Inverse: None");
    }
    
    
    ImGui::End();
}