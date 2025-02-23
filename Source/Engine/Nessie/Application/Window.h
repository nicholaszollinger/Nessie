#pragma once
// Window.h
#include <string>
#include "WindowEvents.h"
#include "Math/Vector2.h"

namespace nes
{
    class Platform;

    enum class WindowMode
    {
        Windowed,
        Fullscreen,
        FullscreenBorderless,
    };

    struct WindowExtent
    {
        uint32_t m_width{};
        uint32_t m_height{};
    };
    
    struct WindowProperties
    {
        std::string m_label = "App Window";
        WindowExtent m_extent = { 1600, 900 };
        WindowMode m_windowMode = WindowMode::Windowed;
        bool m_isResizable = true;
        bool m_vsyncEnabled = false;
        bool m_isMinimized = false;
    };

    class Window
    {
        friend class Application;
        
        WindowProperties m_properties{};
        void* m_pWindowContext = nullptr;
        Vector2f m_cursorPosition{};

    private:
        Window() = default;

    public:
        ~Window() = default;

        // No Move or Copy
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&) noexcept = delete;
        Window& operator=(Window&&) noexcept = delete;

    private:
        bool Init(Application& app, const WindowProperties& props);
        void ProcessEvents();
        bool ShouldClose();
        void Close();

    public:
        void SetIsMinimized(const bool minimized);
        void SetVsync(const bool enabled);
        WindowExtent Resize(const WindowExtent& extent);
        WindowExtent Resize(const uint32_t width, const uint32_t height);

        [[nodiscard]] void* GetNativeWindowHandle() const;
        [[nodiscard]] void* GetWindowContext() const         { return m_pWindowContext; }
        [[nodiscard]] const WindowExtent& GetExtent() const { return m_properties.m_extent; }
        [[nodiscard]] WindowMode GetWindowMode() const      { return m_properties.m_windowMode; }
        [[nodiscard]] const Vector2f& GetCursorPosition() const { return m_cursorPosition; }
        [[nodiscard]] bool IsResizable() const              { return m_properties.m_isResizable; }
        [[nodiscard]] bool IsVsyncEnabled() const           { return m_properties.m_vsyncEnabled; }
        [[nodiscard]] bool IsMinimized() const              { return m_properties.m_isMinimized; }
    };
}