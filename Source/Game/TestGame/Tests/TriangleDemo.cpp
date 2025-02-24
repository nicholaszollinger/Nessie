// TriangleDemo.cpp

#include "TriangleDemo.h"
#include "imgui.h"
#include "Math/Transform.h"

void TriangleDemo::Reset()
{
    m_triangle = {};
    
    m_transform.m_position = {};
    m_transform.m_rotation = {};
    m_transform.m_scale = {100.f, 100.f};
    m_testPoint = {0.f, 0.f};
}

void TriangleDemo::Render(const nes::Renderer& renderer, const nes::Matrix3x3& viewMatrix)
{
    const auto transform = nes::Matrix3x3::Concatenate(m_transform.ToMatrix(), viewMatrix);
    
    const nes::Triangle2D drawTriangle = m_triangle.Transformed(transform);
    const nes::Vector2 testPos = viewMatrix.TransformPoint(m_testPoint);
    
    if (drawTriangle.ContainsPoint(testPos))
        renderer.DrawTriangle(drawTriangle, nes::LinearColor::Green());
    else
        renderer.DrawTriangle(drawTriangle, nes::LinearColor::Red());
    
    renderer.DrawCircle(testPos, 2.f, nes::LinearColor::White());
}

void TriangleDemo::RenderImGui()
{
    ImGui::DragFloat2("Position", &m_transform.m_position.x);
    ImGui::SliderAngle("Rotation", &m_transform.m_rotation);
    ImGui::DragFloat2("Scale", &m_transform.m_scale.x);

    // [TODO]: Have a Triangle property drawer for the vertices
    ImGui::Separator();
    ImGui::DragFloat2("Test Point", &m_testPoint.x);
}