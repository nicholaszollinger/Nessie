// MatrixTest.h
#pragma once
#include "../Demo.h"
#include "Math/Sphere.h"
#include "Math/Transform.h"

class CircleDemo final : public Demo
{
    nes::Transform2D m_transform{};
    nes::Circle m_circle{};
    nes::Vector2 m_testPoint;
    nes::Vector2 m_testPoints[3]{};
    bool m_pointInside = false;
    
public:
    virtual void Reset() override;
    virtual void Render(const nes::Renderer& renderer, const nes::Matrix3x3& viewMatrix) override;
    virtual void RenderImGui() override;
    virtual const char* GetName() const override { return "Circle"; }
};