// GLFW_InputConversions.cpp

#include "GLFW_InputConversions.h"

#include "Debug/CheckedCast.h"
#ifdef NES_WINDOW_API_GLFW
#include <GLFW/glfw3.h>
#include "Input/InputManager.h"

namespace nes::glfw
{
    static int ConvertToGLFWKey(const KeyCode keyCode)
    {
        switch (keyCode)
        {
            case KeyCode::A: return GLFW_KEY_A;
            case KeyCode::B: return GLFW_KEY_B;
            case KeyCode::C: return GLFW_KEY_C;
            case KeyCode::D: return GLFW_KEY_D;
            case KeyCode::E: return GLFW_KEY_E;
            case KeyCode::F: return GLFW_KEY_F;
            case KeyCode::G: return GLFW_KEY_G;
            case KeyCode::H: return GLFW_KEY_H;
            case KeyCode::I: return GLFW_KEY_I;
            case KeyCode::J: return GLFW_KEY_J;
            case KeyCode::K: return GLFW_KEY_K;
            case KeyCode::L: return GLFW_KEY_L;
            case KeyCode::M: return GLFW_KEY_M;
            case KeyCode::N: return GLFW_KEY_N;
            case KeyCode::O: return GLFW_KEY_O;
            case KeyCode::P: return GLFW_KEY_P;
            case KeyCode::Q: return GLFW_KEY_Q;
            case KeyCode::R: return GLFW_KEY_R;
            case KeyCode::S: return GLFW_KEY_S;
            case KeyCode::T: return GLFW_KEY_T;
            case KeyCode::U: return GLFW_KEY_U;
            case KeyCode::V: return GLFW_KEY_V;
            case KeyCode::W: return GLFW_KEY_W;
            case KeyCode::X: return GLFW_KEY_X;
            case KeyCode::Y: return GLFW_KEY_Y;
            case KeyCode::Z: return GLFW_KEY_Z;
            case KeyCode::Num0: return GLFW_KEY_0;
            case KeyCode::Num1: return GLFW_KEY_1;
            case KeyCode::Num2: return GLFW_KEY_2;
            case KeyCode::Num3: return GLFW_KEY_3;
            case KeyCode::Num4: return GLFW_KEY_4;
            case KeyCode::Num5: return GLFW_KEY_5;
            case KeyCode::Num6: return GLFW_KEY_6;
            case KeyCode::Num7: return GLFW_KEY_7;
            case KeyCode::Num8: return GLFW_KEY_8;
            case KeyCode::Num9: return GLFW_KEY_9;
            case KeyCode::Space: return GLFW_KEY_SPACE;
            case KeyCode::Enter: return GLFW_KEY_ENTER;
            case KeyCode::Escape: return GLFW_KEY_ESCAPE;
            case KeyCode::Backspace: return GLFW_KEY_BACKSPACE;
            case KeyCode::Delete: return GLFW_KEY_DELETE;
            case KeyCode::Tab: return GLFW_KEY_TAB;
            case KeyCode::Insert: return GLFW_KEY_INSERT;
            case KeyCode::Capslock: return GLFW_KEY_CAPS_LOCK;
            case KeyCode::NumLock: return GLFW_KEY_NUM_LOCK;
            case KeyCode::PrintScreen: return GLFW_KEY_PRINT_SCREEN;
            case KeyCode::Pause: return GLFW_KEY_PAUSE;
            case KeyCode::Comma: return GLFW_KEY_COMMA;
            case KeyCode::Period: return GLFW_KEY_PERIOD;
            case KeyCode::LeftBracket: return GLFW_KEY_LEFT_BRACKET;
            case KeyCode::RightBracket: return GLFW_KEY_RIGHT_BRACKET;
            case KeyCode::Backslash: return GLFW_KEY_BACKSLASH;
            case KeyCode::GraveAccent: return GLFW_KEY_GRAVE_ACCENT;
            case KeyCode::Up: return GLFW_KEY_UP;
            case KeyCode::Down: return GLFW_KEY_DOWN;
            case KeyCode::Left: return GLFW_KEY_LEFT;
            case KeyCode::Right: return GLFW_KEY_RIGHT;
            case KeyCode::PageUp: return GLFW_KEY_PAGE_UP;
            case KeyCode::PageDown: return GLFW_KEY_PAGE_DOWN;
            case KeyCode::Home: return GLFW_KEY_HOME;
            case KeyCode::End: return GLFW_KEY_END;
            case KeyCode::LeftControl: return GLFW_KEY_LEFT_CONTROL;
            case KeyCode::LeftShift: return GLFW_KEY_LEFT_SHIFT;
            case KeyCode::LeftAlt: return GLFW_KEY_LEFT_ALT;
            case KeyCode::LeftSuper: return GLFW_KEY_LEFT_SUPER;
            case KeyCode::RightControl: return GLFW_KEY_RIGHT_CONTROL;
            case KeyCode::RightShift: return GLFW_KEY_RIGHT_SHIFT;
            case KeyCode::RightAlt: return GLFW_KEY_RIGHT_ALT;
            case KeyCode::RightSuper: return GLFW_KEY_RIGHT_SUPER;

            default: return GLFW_KEY_UNKNOWN;
        }
    }

    static int ConvertToGLFWMouseButton(const MouseButton button)
    {
        const int glfwButton = static_cast<int>(button);
        NES_ASSERT(glfwButton < GLFW_MOUSE_BUTTON_6);
        return glfwButton;
    }

    static int ConvertToGLFWCursorMode(const CursorMode mode)
    {
        switch (mode)
        {
            case CursorMode::Hidden: return GLFW_CURSOR_HIDDEN;
            case CursorMode::Disabled: return GLFW_CURSOR_DISABLED;
            case CursorMode::Captured: return GLFW_CURSOR_CAPTURED;
            default: return GLFW_CURSOR_NORMAL;
        }
    }
    
    KeyCode ConvertToKeyCode(const int key)
    {
        switch (key)
        {
            // Letters
            case GLFW_KEY_A: return nes::KeyCode::A;
            case GLFW_KEY_B: return nes::KeyCode::B;
            case GLFW_KEY_C: return nes::KeyCode::C;
            case GLFW_KEY_D: return nes::KeyCode::D;
            case GLFW_KEY_E: return nes::KeyCode::E;
            case GLFW_KEY_F: return nes::KeyCode::F;
            case GLFW_KEY_G: return nes::KeyCode::G;
            case GLFW_KEY_H: return nes::KeyCode::H;
            case GLFW_KEY_I: return nes::KeyCode::I;
            case GLFW_KEY_J: return nes::KeyCode::J;
            case GLFW_KEY_K: return nes::KeyCode::K;
            case GLFW_KEY_L: return nes::KeyCode::L;
            case GLFW_KEY_M: return nes::KeyCode::M;
            case GLFW_KEY_N: return nes::KeyCode::N;
            case GLFW_KEY_O: return nes::KeyCode::O;
            case GLFW_KEY_P: return nes::KeyCode::P;
            case GLFW_KEY_Q: return nes::KeyCode::Q;
            case GLFW_KEY_R: return nes::KeyCode::R;
            case GLFW_KEY_S: return nes::KeyCode::S;
            case GLFW_KEY_T: return nes::KeyCode::T;
            case GLFW_KEY_U: return nes::KeyCode::U;
            case GLFW_KEY_V: return nes::KeyCode::V;
            case GLFW_KEY_W: return nes::KeyCode::W;
            case GLFW_KEY_X: return nes::KeyCode::X;
            case GLFW_KEY_Y: return nes::KeyCode::Y;
            case GLFW_KEY_Z: return nes::KeyCode::Z;

            // Numbers
            case GLFW_KEY_0: return nes::KeyCode::Num0;
            case GLFW_KEY_1: return nes::KeyCode::Num1;
            case GLFW_KEY_2: return nes::KeyCode::Num2;
            case GLFW_KEY_3: return nes::KeyCode::Num3;
            case GLFW_KEY_4: return nes::KeyCode::Num4;
            case GLFW_KEY_5: return nes::KeyCode::Num5;
            case GLFW_KEY_6: return nes::KeyCode::Num6;
            case GLFW_KEY_7: return nes::KeyCode::Num7;
            case GLFW_KEY_8: return nes::KeyCode::Num8;
            case GLFW_KEY_9: return nes::KeyCode::Num9;

            // Navigation
            case GLFW_KEY_UP: return nes::KeyCode::Up;
            case GLFW_KEY_DOWN: return nes::KeyCode::Down;
            case GLFW_KEY_LEFT: return nes::KeyCode::Left;
            case GLFW_KEY_RIGHT: return nes::KeyCode::Right;
            case GLFW_KEY_PAGE_UP: return nes::KeyCode::PageUp;
            case GLFW_KEY_PAGE_DOWN: return nes::KeyCode::PageDown;
            case GLFW_KEY_HOME: return nes::KeyCode::Home;
            case GLFW_KEY_END: return nes::KeyCode::End;

            case GLFW_KEY_COMMA: return nes::KeyCode::Comma;
            case GLFW_KEY_PERIOD: return nes::KeyCode::Period;
            case GLFW_KEY_ESCAPE: return nes::KeyCode::Escape;
            case GLFW_KEY_SPACE: return nes::KeyCode::Space;
            case GLFW_KEY_ENTER: return nes::KeyCode::Enter;
            case GLFW_KEY_BACKSPACE: return nes::KeyCode::Backspace;
            case GLFW_KEY_DELETE: return nes::KeyCode::Delete;
            case GLFW_KEY_TAB: return nes::KeyCode::Tab;
            case GLFW_KEY_INSERT: return nes::KeyCode::Insert;
            case GLFW_KEY_CAPS_LOCK: return nes::KeyCode::Capslock;
            case GLFW_KEY_NUM_LOCK: return nes::KeyCode::NumLock;
            case GLFW_KEY_PRINT_SCREEN: return nes::KeyCode::PrintScreen;
            case GLFW_KEY_PAUSE: return nes::KeyCode::Pause;

            // Modifiers
            case GLFW_KEY_LEFT_CONTROL: return nes::KeyCode::LeftControl;
            case GLFW_KEY_RIGHT_CONTROL: return nes::KeyCode::RightControl;
            case GLFW_KEY_LEFT_SHIFT: return nes::KeyCode::LeftShift;
            case GLFW_KEY_RIGHT_SHIFT: return nes::KeyCode::RightShift;
            case GLFW_KEY_LEFT_ALT: return nes::KeyCode::LeftAlt;
            case GLFW_KEY_RIGHT_ALT: return nes::KeyCode::RightAlt;
            case GLFW_KEY_LEFT_SUPER: return nes::KeyCode::LeftSuper;
            case GLFW_KEY_RIGHT_SUPER: return nes::KeyCode::RightSuper;

            default: return nes::KeyCode::Unknown;
        }
    }

    MouseButton ConvertToMouseButton(const int button)
    {
        if (button < GLFW_MOUSE_BUTTON_6)
	    {
		    return static_cast<MouseButton>(button);
	    }

	    return MouseButton::Unknown;
    }

    KeyAction ConvertToKeyAction(const int action)
    {
        switch (action)
        {
            case GLFW_PRESS: return KeyAction::Pressed;
            case GLFW_RELEASE: return KeyAction::Released;
            case GLFW_REPEAT: return KeyAction::Repeat;
            default: return KeyAction::Unknown;
        }
    }

    MouseAction ConvertToMouseAction(const int action)
    {
        switch (action)
        {
            case GLFW_PRESS: return MouseAction::Pressed;
            case GLFW_RELEASE: return MouseAction::Released;
            default: return MouseAction::Unknown;
        }
    }

    Modifiers ConvertToModifiers(const int modifiers)
    {
        Modifiers result{};
        result.m_control    = modifiers & GLFW_MOD_CONTROL;
        result.m_alt        = modifiers & GLFW_MOD_ALT;
        result.m_shift      = modifiers & GLFW_MOD_SHIFT;
        result.m_super      = modifiers & GLFW_MOD_SUPER;
        result.m_capsLock   = modifiers & GLFW_MOD_CAPS_LOCK;
        result.m_numLock    = modifiers & GLFW_MOD_NUM_LOCK;
        return result;
    }
}

namespace nes
{
    bool InputManager::IsKeyDown_Impl(void* pNativeWindow, const KeyCode key)
    {
        NES_ASSERT(pNativeWindow != nullptr);
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(pNativeWindow);

        const auto action = glfw::ConvertToKeyAction(glfwGetKey(pWindow, glfw::ConvertToGLFWKey(key)));
        return action == KeyAction::Pressed || action == KeyAction::Repeat;
    }

    bool InputManager::IsKeyUp_Impl(void* pNativeWindow, const KeyCode key)
    {
        NES_ASSERT(pNativeWindow != nullptr);
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(pNativeWindow);

        const auto action = glfw::ConvertToKeyAction(glfwGetKey(pWindow, glfw::ConvertToGLFWKey(key)));
        return action == KeyAction::Released;
    }

    bool InputManager::IsMouseButtonUp_Impl(void* pNativeWindow, const MouseButton button)
    {
        NES_ASSERT(pNativeWindow != nullptr);
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(pNativeWindow);
        
        const auto action = glfw::ConvertToMouseAction(glfwGetMouseButton(pWindow, glfw::ConvertToGLFWMouseButton(button)));
        return action == MouseAction::Pressed;
    }

    bool InputManager::IsMouseButtonDown_Impl(void* pNativeWindow, const MouseButton button)
    {
        NES_ASSERT(pNativeWindow != nullptr);
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(pNativeWindow);

        const auto action = glfw::ConvertToMouseAction(glfwGetMouseButton(pWindow, glfw::ConvertToGLFWMouseButton(button)));
        return action == MouseAction::Released;
    }

    void InputManager::SetCursorMode_Impl(void* pNativeWindow, const CursorMode mode)
    {
        if (m_cursorMode == mode)
            return;
        
        NES_ASSERT(pNativeWindow != nullptr);
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(pNativeWindow);
        
        glfwSetInputMode(pWindow, GLFW_CURSOR, glfw::ConvertToGLFWCursorMode(mode));

        // If we are setting the mouse cursor to disabled, 
        if (glfwRawMouseMotionSupported())
        {
            if (mode == CursorMode::Disabled)
            {
                glfwSetInputMode(pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }

            // If changing from disabled, change the raw mouse motion back. 
            else if (m_cursorMode == CursorMode::Disabled)
            {
                glfwSetInputMode(pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
            }
        }
        
        m_cursorMode = mode;
    }

    Vector2d InputManager::GetCursorPosition_Impl(void* pNativeWindow)
    {
        NES_ASSERT(pNativeWindow != nullptr);
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(pNativeWindow);

        Vector2d cursorPosition{};
        glfwGetCursorPos(pWindow, &cursorPosition.x, &cursorPosition.y);
        return cursorPosition;
    }

}

#endif