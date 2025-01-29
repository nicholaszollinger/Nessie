// Renderer.h
#pragma once
#include "Application/Window.h"
#include "Core/Generic/Color.h"
#include "Math/Rect.h"

namespace nes
{
    class Renderer
    {
        friend class Application;
        Window* m_pWindow = nullptr;

    private:
        Renderer() = default;
        ~Renderer() = default;
        
    public:
        // No Copy or Move
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;
        
        void Clear(const Color& color) const;
        void Clear(const LinearColor& color) const;

        // 2D
        void DrawLine(Vector2f from, Vector2f to, const LinearColor& color) const;
        void DrawRect(const Rectf& rect, const LinearColor& color) const;
        void DrawFillRect(const Rectf& rect, const LinearColor& color) const;
        void DrawCircle(Vector2f position, float radius, const LinearColor& color) const;
        void DrawFillCircle(Vector2f position, float radius, const LinearColor& color) const;
        
    protected:
        bool Init(Window* pWindow);
        void Close();
        
        void BeginFrame();
        void SubmitFrame();
    };
}
