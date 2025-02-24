// VectorTest.cpp

#include "VectorDemo.h"

#include "imgui.h"
#include "Math/Line.h"
#include "Math/Transform.h"
#include "Math/Vector3.h"

static constexpr float kArrowDrawLength = 10.f;
static constexpr float kArrowAngleDegrees = 30.f;

void VectorDemo::Reset()
{
    m_vectorA = nes::Vector2::GetRightVector(); 
    m_vectorB = nes::Vector2::GetUpVector();
    m_vectorDrawScale = 100.f;
    m_rotateAngleRadians = nes::math::DegreesToRadians() * 45.f;
    m_vectorDrawScale = 100.f;
    m_resultScalar = 0.f;
}

void VectorDemo::Render(const nes::Renderer& renderer, const nes::Matrix3x3& viewMatrix)
{
    const nes::Transform2D transform(nes::Vector2::GetZeroVector(), nes::Vector2(m_vectorDrawScale, m_vectorDrawScale), 0.f);
    const auto matrix = viewMatrix * transform.ToMatrix();
    const nes::Vector2 center = matrix.TransformPoint(nes::Vector2::GetZeroVector());
    
    switch (m_testType)
    {
        case VectorTest::Addition:
        {
            m_resultVector = m_vectorA + m_vectorB;

            const nes::Vector2 transformedA = matrix.TransformVector(m_vectorA);
            const nes::Vector2 transformedResult = matrix.TransformVector(m_resultVector);
            
            DrawArrow(renderer, center, center + transformedA, nes::LinearColor::Red());
            DrawArrow(renderer, center + transformedA, center + transformedResult, nes::LinearColor::Green());
            DrawArrow(renderer, center, center + transformedResult, nes::LinearColor::Cyan());
            
            break;
        }
        
        case VectorTest::DotProduct:
        {
            // Vector A is a rotation of the Right Vector.
            m_vectorA = nes::Vector2::GetRightVector().Rotated(m_rotateAngleRadians * nes::math::RadiansToDegrees());
            m_resultScalar = m_vectorA.Dot(m_vectorB);

            const nes::Vector2 transformedA = matrix.TransformVector(m_vectorA);
            const nes::Vector2 transformedB = matrix.TransformVector(m_vectorB);
            const nes::Vector2 projectionVec = matrix.TransformVector(m_vectorA * m_resultScalar);

            DrawArrow(renderer, center, center + transformedB, nes::LinearColor::Green());
            DrawArrow(renderer, center, center + transformedA, nes::LinearColor::Red());

            // Draw the "shadow" of B onto vector A
            renderer.DrawLine(center + transformedB, center + projectionVec, nes::LinearColor::White());
            
            // Draw the signed distance as a point.
            renderer.DrawLine(center, center + projectionVec, nes::LinearColor::Yellow());
            
            break;
        }
        
        default: break;
    }
}

void VectorDemo::RenderImGui()
{
    // Drop down for test type.
    const uint8_t selected = static_cast<uint8_t>(m_testType);
    if (ImGui::BeginCombo("Test Type", s_testTypeNames[selected].c_str()))
    {
        for (uint8_t i = 0; i < static_cast<uint8_t>(VectorTest::Num); ++i)
        {
            bool isSelected = i == selected;

            if (ImGui::Selectable(s_testTypeNames[i].c_str(), isSelected))
            {
                m_testType = static_cast<VectorTest>(i);
                Reset();
            }
        }
        
        ImGui::EndCombo();
    }

    // Test GUI
    ImGui::Separator();
    switch (m_testType)
    {
        case VectorTest::Addition:
        {
            ImGui::SeparatorText("Description:");
            ImGui::TextWrapped("Result of Adding vector A with vector B.");
            
            ImGui::SeparatorText("Controls:");
            ImGui::DragFloat2("A", &m_vectorA.x);
            ImGui::DragFloat2("B", &m_vectorB.x);
            
            // [TODO]: Readonly property drawer for result Vector
            ImGui::SeparatorText("Result:");
            ImGui::Text("Result Vector: %s", m_resultVector.ToString().c_str());
            
            break;
        }
        
        case VectorTest::DotProduct:
        {
            ImGui::SeparatorText("Description:");
            ImGui::TextWrapped("The Dot product is the result of projecting Vector B onto Direction Vector A. The result's value represents the Signed Distance of that projection.");
            
            ImGui::SeparatorText("Controls:");
            ImGui::DragFloat2("B", &m_vectorB.x);
            ImGui::SliderAngle("A Angle", &m_rotateAngleRadians, 0.f, 360.f);

            ImGui::SeparatorText("Result:");
            ImGui::Text("Dot Product: %.2f", m_resultScalar);
            
            break;
        }
        
        default: break;
    }

    ImGui::SeparatorText("Draw Settings");
    ImGui::DragFloat("Draw Scale", &m_vectorDrawScale, 1.f, 0.f);
}

//----------------------------------------------------------------------------------------------------
//      [TODO]: Move this to the Renderer? 
///		@brief : Draw an arrow starting from the origin to the origin + vec. 
//----------------------------------------------------------------------------------------------------
void VectorDemo::DrawArrow(const nes::Renderer& renderer, const nes::Vector2& from, const nes::Vector2& to, const nes::LinearColor& color, [[maybe_unused]] float thickness) const
{
    const nes::Vector2 directionVec = (to - from).Normalized();
    const nes::Vector2 rightArrowEnd = to + (-directionVec.Rotated(kArrowAngleDegrees) * kArrowDrawLength);
    const nes::Vector2 leftArrowEnd = to + (-directionVec.Rotated(-kArrowAngleDegrees) * kArrowDrawLength);
    renderer.DrawLine(from, to, color);
    renderer.DrawLine(to, leftArrowEnd, color);
    renderer.DrawLine(to, rightArrowEnd, color);
}
