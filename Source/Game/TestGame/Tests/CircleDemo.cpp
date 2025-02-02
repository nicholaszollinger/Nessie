// CircleDemo.cpp

#include "CircleDemo.h"
#include "imgui.h"

void CircleDemo::Reset()
{
    m_circle = nes::Circle(nes::Vector2::GetZeroVector(), 100.f);
    m_testPoint = nes::Vector2::GetZeroVector();
    m_pointInside = false;
}

void CircleDemo::Render(const nes::Renderer& renderer, [[maybe_unused]] const nes::Rectf& worldViewport)
{
    // Circle, colored based on the test point intersection
    const nes::Vector2 circlePos = m_circle.m_center + worldViewport.Center();
    
    if (m_circle.ContainsPoint(m_testPoint))
        renderer.DrawCircle(circlePos, m_circle.m_radius, nes::LinearColor::Green());
    else
        renderer.DrawCircle(circlePos, m_circle.m_radius, nes::LinearColor::Red());

    // Point
    renderer.DrawCircle(m_testPoint + worldViewport.Center(), 2.f, nes::LinearColor::White());
}

void CircleDemo::RenderImGui()
{
    ImGui::DragFloat2("Test Point", &m_testPoint[0]);
    ImGui::Separator();
    
    ImGui::DragFloat2("Circle Center", &m_circle.m_center[0]);
    ImGui::DragFloat("Circle Radius", &m_circle.m_radius);

}