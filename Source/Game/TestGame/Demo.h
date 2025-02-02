// Tester.h
#pragma once
#include "Graphics/Renderer.h"

class Demo
{
public:
    virtual ~Demo() = default;
    
    virtual void Reset() = 0;
    virtual void Render(const nes::Renderer& renderer, const nes::Rectf& worldViewport) = 0;
    virtual void RenderImGui() = 0;
    
    [[nodiscard]] virtual const char* GetName() const = 0;
};
