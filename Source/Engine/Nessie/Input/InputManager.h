// InputManager.h
#pragma once
#include "InputEvents.h"

namespace nes
{
    class Window;

    enum class CursorMode : uint8_t
    {
        Visible,  // Cursor is visible on screen.
        Hidden,   // Cursor is invisible, but still moves around the screen as normal.
        Disabled, // Cursor is locked to the center of the screen, useful for things FPS cameras.
        Captured, // The Cursor is locked to the bounds of the window.
    };
    
    class InputManager
    {
        friend class Application; // Change to "Platform" when you can.
        Window* m_pWindow = nullptr;
        CursorMode m_cursorMode = CursorMode::Visible;
        Vector2 m_cursorPosition {};
        Vector2 m_cursorDelta    {};
        
    public:
        static bool IsKeyDown(const KeyCode key);
        static bool IsKeyUp(const KeyCode key);
        
        static bool IsMouseButtonDown(const MouseButton button);
        static bool IsMouseButtonUp(const MouseButton button);
        static void SetCursorMode(const CursorMode mode);

        [[nodiscard]] static CursorMode GetCursorMode();
        [[nodiscard]] static Vector2 GetCursorPosition();
        [[nodiscard]] static Vector2 GetCursorDelta();
        
    private:
        bool Init(Window* pWindow);
        void Shutdown();
        void Update(const double deltaTime);

        // Platform Defined Implementations:
        Vector2d GetCursorPosition_Impl(void* pNativeWindow);
        bool IsKeyDown_Impl(void* pNativeWindow, const KeyCode key);
        bool IsKeyUp_Impl(void* pNativeWindow, const KeyCode key);
        bool IsMouseButtonDown_Impl(void* pNativeWindow, const MouseButton button);
        bool IsMouseButtonUp_Impl(void* pNativeWindow, const MouseButton button);
        void SetCursorMode_Impl(void* pNativeWindow, const CursorMode mode);
    };
}
