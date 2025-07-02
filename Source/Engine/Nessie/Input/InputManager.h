// InputManager.h
#pragma once
#include "InputEvents.h"

namespace nes
{
    class ApplicationWindow;

    enum class ECursorMode : uint8_t
    {
        Visible,  // Cursor is visible on screen.
        Hidden,   // Cursor is invisible, but still moves around the screen as normal.
        Disabled, // Cursor is locked to the center of the screen, useful for things FPS cameras.
        Captured, // The Cursor is locked to the bounds of the window.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : InputManager is a static API to query the current input states of keys,
    ///     mouse position, etc.
    //----------------------------------------------------------------------------------------------------
    class InputManager
    {
    public:
        static bool         IsKeyDown(const EKeyCode key);
        static bool         IsKeyUp(const EKeyCode key);
        
        static bool         IsMouseButtonDown(const EMouseButton button);
        static bool         IsMouseButtonUp(const EMouseButton button);
        static void         SetCursorMode(const ECursorMode mode);

        static ECursorMode  GetCursorMode();
        static Vec2         GetCursorPosition();
        static Vec2         GetCursorDelta();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the Input Manager with the window that we will be reading input from. 
        ///	@param pWindow : The window that we are reading input from.
        //----------------------------------------------------------------------------------------------------
        bool                Init(ApplicationWindow* pWindow);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Shutdown the InputManager, which will set the static instance to null.
        //----------------------------------------------------------------------------------------------------
        void                Shutdown();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Update current input states based on delta time.
        //----------------------------------------------------------------------------------------------------
        void                Update(const double deltaTime);
        
    private:
        /// Platform Defined Implementations:
        Double2             GetCursorPosition_Impl(void* pNativeWindow);
        bool                IsKeyDown_Impl(void* pNativeWindow, const EKeyCode key);
        bool                IsKeyUp_Impl(void* pNativeWindow, const EKeyCode key);
        bool                IsMouseButtonDown_Impl(void* pNativeWindow, const EMouseButton button);
        bool                IsMouseButtonUp_Impl(void* pNativeWindow, const EMouseButton button);
        void                SetCursorMode_Impl(void* pNativeWindow, const ECursorMode mode);

    private:
        friend class Application;
        
        ApplicationWindow*             m_pWindow = nullptr;
        ECursorMode         m_cursorMode = ECursorMode::Visible;
        Vec2                m_cursorPosition{};
        Vec2                m_cursorDelta{};
    };
}
