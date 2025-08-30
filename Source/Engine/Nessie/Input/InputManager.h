// InputManager.h
#pragma once
#include "Cursor.h"
#include "InputEvents.h"

namespace nes
{
    class ApplicationWindow;

    //----------------------------------------------------------------------------------------------------
    /// @brief : InputManager contains a static API to query the current input states of keys,
    ///     mouse position, etc.
    //----------------------------------------------------------------------------------------------------
    class InputManager
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if a key is pressed. 
        //----------------------------------------------------------------------------------------------------
        static bool         IsKeyDown(const EKeyCode key);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if a key is not pressed.
        //----------------------------------------------------------------------------------------------------
        static bool         IsKeyUp(const EKeyCode key);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if a mouse button is pressed.
        //----------------------------------------------------------------------------------------------------
        static bool         IsMouseButtonDown(const EMouseButton button);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if a mouse button is not pressed.
        //----------------------------------------------------------------------------------------------------
        static bool         IsMouseButtonUp(const EMouseButton button);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set how the cursor behaves in the window.
        //----------------------------------------------------------------------------------------------------
        static void         SetCursorMode(const ECursorMode mode);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current cursor mode.
        //----------------------------------------------------------------------------------------------------
        static ECursorMode  GetCursorMode();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the position of the cursor in relative to the window.
        //----------------------------------------------------------------------------------------------------
        static Vec2         GetCursorPosition();
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the amount the cursor has moved last frame.
        //----------------------------------------------------------------------------------------------------
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

        //----------------------------------------------------------------------------------------------------
        /// @brief : Handle input events from the Window. 
        //----------------------------------------------------------------------------------------------------
        void                OnInputEvent(Event& event);

    private:
        using KeyStates         = std::array<EKeyAction, static_cast<size_t>(EKeyCode::MaxNum)>;
        using MouseButtonStates = std::array<EMouseAction, static_cast<size_t>(EMouseButton::MaxNum)>;
        
        ApplicationWindow*  m_pWindow = nullptr;
        KeyStates           m_keyStates{};
        MouseButtonStates   m_mouseButtonStates{};
        ECursorMode         m_cursorMode = ECursorMode::Visible;
        Vec2                m_cursorPosition{};
        Vec2                m_cursorDelta{};
    };
}
