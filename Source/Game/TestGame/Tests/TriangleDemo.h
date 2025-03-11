// TriangleDemo.h
#pragma once

#include "../Demo.h"
#include "Math/Transform2.h"

class TriangleDemo final : public Demo
{
    nes::Triangle2D m_triangle{};
    nes::Transform2D m_transform{};
    nes::Vector2 m_testPoint{};

public:
    virtual void Reset() override;
    virtual void Render(const nes::Renderer& renderer, const nes::Matrix3x3& viewMatrix) override;
    virtual void RenderImGui() override;
    [[nodiscard]] virtual const char* GetName() const override { return "Triangles"; }
};
