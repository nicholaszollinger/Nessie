// CircleDemo.cpp

#include "CircleDemo.h"
#include "imgui.h"

void CircleDemo::Reset()
{
    m_testPoint = nes::Vector2::GetZeroVector();

    // Triangle of points.
    m_testPoints[0] = nes::Vector2(-0.5f, -0.5f) * 100.f;
    m_testPoints[1] = nes::Vector2(0.f, 0.5f) * 100.f;
    m_testPoints[2] = nes::Vector2(0.5f, -0.5f) * 100.f;

    // Create bounding Circle triangle of points
    m_circle = nes::Circle(m_testPoints, 3);
    
    m_pointInside = false;
}

void CircleDemo::Render(const nes::Renderer& renderer, const nes::Matrix3x3& viewMatrix)
{
    const auto transform = nes::Matrix3x3::Concatenate(m_transform.ToMatrix(), viewMatrix);
    
    // Circle, colored based on the test point intersection
    const nes::Vector2 circlePos = transform.TransformPoint(m_circle.m_center);
    const nes::Vector2 testPos = transform.TransformPoint(m_testPoint);
    
    if (m_circle.ContainsPoint(m_testPoint))
        renderer.DrawCircle(circlePos, m_circle.m_radius, nes::LinearColor::Green());
    else
        renderer.DrawCircle(circlePos, m_circle.m_radius, nes::LinearColor::Red());

    // Points within the Circle
    renderer.DrawCircle(transform.TransformPoint(m_testPoints[0]), 2.f, nes::LinearColor::White());
    renderer.DrawCircle(transform.TransformPoint(m_testPoints[1]), 2.f, nes::LinearColor::White());
    renderer.DrawCircle(transform.TransformPoint(m_testPoints[2]), 2.f, nes::LinearColor::White());

    // Test Points
    renderer.DrawCircle(testPos, 2.f, nes::LinearColor::Yellow());
}

void CircleDemo::RenderImGui()
{
    ImGui::SeparatorText("Description:");
    ImGui::TextWrapped("The circle is created as a bounding box around the 3 (white) points. The circle will be green if the test point (yellow) is contained by the circle.");

    ImGui::SeparatorText("Controls:");
    ImGui::DragFloat2("Test Point", &m_testPoint[0]);
    ImGui::Separator();
    
    //ImGui::DragFloat2("Circle Center", &m_circle.m_center[0]);
    //ImGui::DragFloat("Circle Radius", &m_circle.m_radius);
}