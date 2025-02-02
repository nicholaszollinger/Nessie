// VectorTester.h
#pragma once

#include "../Demo.h"
#include "Math/Vector2.h"

class VectorDemo final : public Demo
{
    nes::Vector2 m_vectorA = nes::Vector2::GetUpVector(); 
    nes::Vector2 m_vectorB = nes::Vector2::GetRightVector(); 
    nes::Vector2 m_resultVector = nes::Vector2::GetZeroVector();
    float m_rotateAngleRadians = 45.f;
    float m_vectorDrawScale = 10.f;
    float m_vectorWidth = 5.f;
    
public:
    virtual void Reset() override;
    virtual void Render(const nes::Renderer& renderer, const nes::Rectf& worldViewport) override;
    virtual void RenderImGui() override;
    virtual const char* GetName() const override { return "Vectors"; }

private:
    void DrawArrow(const nes::Renderer& renderer, const nes::Vector2& origin, const nes::Vector2& offset, const nes::LinearColor& color, float thickness = 1.f) const;
};
