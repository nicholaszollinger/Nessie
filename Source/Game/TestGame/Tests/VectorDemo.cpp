// VectorTest.cpp

#include "VectorDemo.h"

#include "imgui.h"
//#include "Math/Quaternion.h"
#include "Math/Vector3.h"

static constexpr float kArrowDrawLength = 10.f;
static constexpr float kArrowAngleDegrees = 30.f;

void VectorDemo::Reset()
{
    m_vectorA = nes::Vector2::GetRightVector(); 
    m_vectorB = nes::Vector2::GetUpVector(); 
    m_resultVector = nes::Vector2::GetZeroVector();
    m_vectorDrawScale = 50.f;
    m_vectorWidth = 5.f;
    m_rotateAngleRadians = nes::math::DegreesToRadians() * 45.f;
}

void VectorDemo::Render(const nes::Renderer& renderer, const nes::Rectf& worldViewport)
{
    const nes::Vector2 worldCenter = worldViewport.Center();

    // Vector A
    DrawArrow(renderer, worldCenter, m_vectorA, nes::LinearColor::Red());

    // Vector B
    DrawArrow(renderer, worldCenter, m_vectorB, nes::LinearColor::Green());
    
    // Result Vector
    m_resultVector = m_vectorA.Rotated(m_rotateAngleRadians * nes::math::RadiansToDegrees());
    DrawArrow(renderer, worldCenter, m_resultVector, nes::LinearColor::Blue());
}

void VectorDemo::RenderImGui()
{
    ImGui::DragFloat2("A", &m_vectorA.x);
    ImGui::DragFloat2("B", &m_vectorB.x);
    ImGui::SliderAngle("Rotate Angle", &m_rotateAngleRadians, -360.f, 360.f);
    ImGui::Text("Result Vector: %s", m_resultVector.ToString().c_str());

    ImGui::SeparatorText("Draw Settings");
    ImGui::DragFloat("Draw Scale", &m_vectorDrawScale, 1.f, 0.f);
    ImGui::DragFloat("Draw Width", &m_vectorWidth, 1.f, 0.f);
}

void VectorDemo::DrawArrow(const nes::Renderer& renderer, const nes::Vector2& origin, const nes::Vector2& vec, const nes::LinearColor& color, [[maybe_unused]] float thickness) const
{
    const nes::Vector2 flippedY(vec.x, -vec.y);
    const nes::Vector2 arrowTip = origin + flippedY * m_vectorDrawScale;
    nes::Vector2 rightArrowEnd = arrowTip + ((-flippedY.Normalized()).Rotated(kArrowAngleDegrees) * kArrowDrawLength);
    nes::Vector2 leftArrowEnd = arrowTip + ((-flippedY.Normalized()).Rotated(-kArrowAngleDegrees) * kArrowDrawLength);
    renderer.DrawLine(origin, arrowTip, color);
    renderer.DrawLine(arrowTip, leftArrowEnd, color);
    renderer.DrawLine(arrowTip, rightArrowEnd, color);
}
