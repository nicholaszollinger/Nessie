// MatrixTest.h
#pragma once
#include "../Demo.h"
#include "Math/Circle.h"

class CircleDemo final : public Demo
{
    nes::Circle m_circle{};
    nes::Vector2 m_testPoint;
    bool m_pointInside = false;
    
public:
    virtual void Reset() override;
    virtual void Render(const nes::Renderer& renderer, const nes::Rectf& worldViewport) override;
    virtual void RenderImGui() override;
    virtual const char* GetName() const override { return "Circle"; }
};