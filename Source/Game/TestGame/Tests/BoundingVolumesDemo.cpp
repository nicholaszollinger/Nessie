// BoundingVolumesDemo.cpp

#include "BoundingVolumesDemo.h"
#include "imgui.h"
#include "Math/ConvexHull2.h"
#include "yaml-cpp/yaml.h"

#include "Math/OrientedBox.h"
#include "Math/Sphere.h"

bool BoundingVolumesDemo::Init()
{
    // Load the Vertex Data from a filepath.
    std::string path = NES_CONTENT_DIR;
    path += "BoundingVolumeDemo.yaml";
    
    auto demoData = YAML::LoadFile(path);
    if (!demoData)
        return false;

    auto demo = demoData["Demo"];
    if (!demo)
        return false;
    
    // Load Poly A
    auto polyA = demo["PolyA"];
    if (!polyA)
        return false;

    std::vector<nes::Vector2> vertices{};
    vertices.reserve(polyA["Vertices"].size());
    auto vertexArray = polyA["Vertices"];
    for (const auto vertexValue : vertexArray)
    {
        vertices.emplace_back();
        for (size_t i = 0; i < 2; ++i)
        {
            vertices.back()[i] = vertexValue[i].as<float>();
        }
    }
    m_polyA = nes::Polygon2D(vertices);
    if (!m_convexHullA.TrySolve(m_polyA.GetVertices()))
    {
        NES_WARN("Failed to solve Polygon A Convex Hull!");
    }

    // Load Poly B
    auto polyB  = demo["PolyB"];
    if (!polyB)
        return false;

    vertices.clear();
    vertices.reserve(polyB["Vertices"].size());
    vertexArray = polyB["Vertices"];
    for (const auto vertexValue : vertexArray)
    {
        vertices.emplace_back();
        for (size_t i = 0; i < 2; ++i)
        {
            vertices.back()[i] = vertexValue[i].as<float>();
        }
    }
    m_polyB = nes::Polygon2D(vertices);
    if (!m_convexHullB.TrySolve(m_polyB.GetVertices()))
    {
        NES_WARN("Failed to solve Polygon B Convex Hull!");
    }
    
    return true;
}

void BoundingVolumesDemo::Reset()
{
    // Reset PolyA transform.
    m_transformA.m_position = { -100.f, 0.f};
    m_transformA.m_rotation = {};
    m_transformA.m_scale = {1.f, 1.f};

    // Reset PolyB transform
    m_transformB.m_position = { 100.f, 0.f};
    m_transformB.m_rotation = {};
    m_transformB.m_scale = {1.f, 1.f};
}

void BoundingVolumesDemo::Render(const nes::Renderer& renderer, const nes::Matrix3x3& viewMatrix)
{
    // Transform A's Vertices and Draw the Polygon
    const nes::Matrix3x3 transformA = nes::Matrix3x3::Concatenate(m_transformA.ToMatrix(), viewMatrix);
    std::vector<nes::Vector2> translatedVerticesA{};
    translatedVerticesA.reserve(m_polyA.GetVertices().size());
    for (const auto vertexValue : m_polyA.GetVertices())
    {
        translatedVerticesA.emplace_back(transformA.TransformPoint(vertexValue));   
    }
    renderer.DrawPolygon2D(translatedVerticesA.data(), translatedVerticesA.size(), nes::LinearColor::Cyan());

    // Transform B's Vertices and Draw the Polygon
    const nes::Matrix3x3 transformB = nes::Matrix3x3::Concatenate(m_transformB.ToMatrix(), viewMatrix);
    std::vector<nes::Vector2> translatedVerticesB{};
    translatedVerticesB.reserve(m_polyB.GetVertices().size());
    for (const auto vertexValue : m_polyB.GetVertices())
    {
        translatedVerticesB.emplace_back(transformB.TransformPoint(vertexValue));   
    }
    renderer.DrawPolygon2D(translatedVerticesB.data(), translatedVerticesB.size(), nes::LinearColor::Magenta());

    // Test and Render Bounding Volumes:
    switch (m_boundingType)
    {
        case BoundingVolumeType::AABB:
        {
            const nes::Box2D boxA = nes::Box2D(translatedVerticesA.data(), translatedVerticesA.size()); 
            const nes::Box2D boxB = nes::Box2D(translatedVerticesB.data(), translatedVerticesB.size());
            const nes::LinearColor color = boxA.Intersects(boxB) ? nes::LinearColor::Green() : nes::LinearColor::Red();
            const nes::Rect rectA(boxA.Min(), boxA.Size());
            const nes::Rect rectB(boxB.Min(), boxB.Size());
    
            renderer.DrawRect(rectA, color);
            renderer.DrawRect(rectB, color);
            
            break;
        }
        
        // [TODO]:
        // case BoundingVolumeType::OBB:
        // {
        //     break;
        // }
        
        case BoundingVolumeType::Sphere:
        {
            const auto sphereA = nes::Circle(translatedVerticesA.data(), translatedVerticesA.size());
            const auto sphereB = nes::Circle(translatedVerticesB.data(), translatedVerticesB.size());
            const nes::LinearColor color = sphereA.Intersects(sphereB) ? nes::LinearColor::Green() : nes::LinearColor::Red();
            
            renderer.DrawCircle(sphereA.m_center, sphereA.m_radius, color);
            renderer.DrawCircle(sphereB.m_center, sphereB.m_radius, color);
            break;
        }

        case BoundingVolumeType::ConvexHull:
        {
            // [TODO]: Collision between two polygons:
            
            renderer.DrawPolygon2D(translatedVerticesA, m_convexHullA.GetHullIndices(), nes::LinearColor::Green());
            renderer.DrawPolygon2D(translatedVerticesB, m_convexHullB.GetHullIndices(), nes::LinearColor::Green());
            break;
        }
    
        default: break;
    }
}

void BoundingVolumesDemo::RenderImGui()
{
    // Dropdown for Bounding Volume Types
    const uint8_t selected = static_cast<uint8_t>(m_boundingType);
    if (ImGui::BeginCombo("Test Type", s_boundingTypenames[selected].c_str()))
    {
        for (uint8_t i = 0; i < static_cast<uint8_t>(BoundingVolumeType::Num); ++i)
        {
            bool isSelected = i == selected;

            if (ImGui::Selectable(s_boundingTypenames[i].c_str(), isSelected))
            {
                m_boundingType = static_cast<BoundingVolumeType>(i);
            }
        }
        
        ImGui::EndCombo();
    }
    
    // Poly A:
    {
        ImGui::SeparatorText("Poly A");
        ImGui::DragFloat2("Position##A", &m_transformA.m_position.x);
        ImGui::SliderAngle("Rotation##A", &m_transformA.m_rotation);
        ImGui::DragFloat2("Scale##A", &m_transformA.m_scale.x);
    }
    
    // Poly B:
    {
        ImGui::SeparatorText("Poly B");
        ImGui::DragFloat2("Position##B", &m_transformB.m_position.x);
        ImGui::SliderAngle("Rotation##B", &m_transformB.m_rotation);
        ImGui::DragFloat2("Scale##B", &m_transformB.m_scale.x);
    }
    
}
