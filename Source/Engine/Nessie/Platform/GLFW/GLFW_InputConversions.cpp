// GLFW_InputConversions.cpp

#include "GLFW_InputConversions.h"

#include "Nessie/Debug/CheckedCast.h"
#ifdef NES_WINDOW_API_GLFW
#include <GLFW/glfw3.h>
#include "Nessie/Input/InputManager.h"

namespace nes::glfw
{
    static int ConvertToGLFWKey(const EKeyCode keyCode)
    {
        switch (keyCode)
        {
            case EKeyCode::A: return GLFW_KEY_A;
            case EKeyCode::B: return GLFW_KEY_B;
            case EKeyCode::C: return GLFW_KEY_C;
            case EKeyCode::D: return GLFW_KEY_D;
            case EKeyCode::E: return GLFW_KEY_E;
            case EKeyCode::F: return GLFW_KEY_F;
            case EKeyCode::G: return GLFW_KEY_G;
            case EKeyCode::H: return GLFW_KEY_H;
            case EKeyCode::I: return GLFW_KEY_I;
            case EKeyCode::J: return GLFW_KEY_J;
            case EKeyCode::K: return GLFW_KEY_K;
            case EKeyCode::L: return GLFW_KEY_L;
            case EKeyCode::M: return GLFW_KEY_M;
            case EKeyCode::N: return GLFW_KEY_N;
            case EKeyCode::O: return GLFW_KEY_O;
            case EKeyCode::P: return GLFW_KEY_P;
            case EKeyCode::Q: return GLFW_KEY_Q;
            case EKeyCode::R: return GLFW_KEY_R;
            case EKeyCode::S: return GLFW_KEY_S;
            case EKeyCode::T: return GLFW_KEY_T;
            case EKeyCode::U: return GLFW_KEY_U;
            case EKeyCode::V: return GLFW_KEY_V;
            case EKeyCode::W: return GLFW_KEY_W;
            case EKeyCode::X: return GLFW_KEY_X;
            case EKeyCode::Y: return GLFW_KEY_Y;
            case EKeyCode::Z: return GLFW_KEY_Z;
            case EKeyCode::Num0: return GLFW_KEY_0;
            case EKeyCode::Num1: return GLFW_KEY_1;
            case EKeyCode::Num2: return GLFW_KEY_2;
            case EKeyCode::Num3: return GLFW_KEY_3;
            case EKeyCode::Num4: return GLFW_KEY_4;
            case EKeyCode::Num5: return GLFW_KEY_5;
            case EKeyCode::Num6: return GLFW_KEY_6;
            case EKeyCode::Num7: return GLFW_KEY_7;
            case EKeyCode::Num8: return GLFW_KEY_8;
            case EKeyCode::Num9: return GLFW_KEY_9;
            case EKeyCode::Space: return GLFW_KEY_SPACE;
            case EKeyCode::Enter: return GLFW_KEY_ENTER;
            case EKeyCode::Escape: return GLFW_KEY_ESCAPE;
            case EKeyCode::Backspace: return GLFW_KEY_BACKSPACE;
            case EKeyCode::Delete: return GLFW_KEY_DELETE;
            case EKeyCode::Tab: return GLFW_KEY_TAB;
            case EKeyCode::Insert: return GLFW_KEY_INSERT;
            case EKeyCode::Capslock: return GLFW_KEY_CAPS_LOCK;
            case EKeyCode::NumLock: return GLFW_KEY_NUM_LOCK;
            case EKeyCode::PrintScreen: return GLFW_KEY_PRINT_SCREEN;
            case EKeyCode::Pause: return GLFW_KEY_PAUSE;
            case EKeyCode::Comma: return GLFW_KEY_COMMA;
            case EKeyCode::Period: return GLFW_KEY_PERIOD;
            case EKeyCode::LeftBracket: return GLFW_KEY_LEFT_BRACKET;
            case EKeyCode::RightBracket: return GLFW_KEY_RIGHT_BRACKET;
            case EKeyCode::Backslash: return GLFW_KEY_BACKSLASH;
            case EKeyCode::GraveAccent: return GLFW_KEY_GRAVE_ACCENT;
            case EKeyCode::Up: return GLFW_KEY_UP;
            case EKeyCode::Down: return GLFW_KEY_DOWN;
            case EKeyCode::Left: return GLFW_KEY_LEFT;
            case EKeyCode::Right: return GLFW_KEY_RIGHT;
            case EKeyCode::PageUp: return GLFW_KEY_PAGE_UP;
            case EKeyCode::PageDown: return GLFW_KEY_PAGE_DOWN;
            case EKeyCode::Home: return GLFW_KEY_HOME;
            case EKeyCode::End: return GLFW_KEY_END;
            case EKeyCode::LeftControl: return GLFW_KEY_LEFT_CONTROL;
            case EKeyCode::LeftShift: return GLFW_KEY_LEFT_SHIFT;
            case EKeyCode::LeftAlt: return GLFW_KEY_LEFT_ALT;
            case EKeyCode::LeftSuper: return GLFW_KEY_LEFT_SUPER;
            case EKeyCode::RightControl: return GLFW_KEY_RIGHT_CONTROL;
            case EKeyCode::RightShift: return GLFW_KEY_RIGHT_SHIFT;
            case EKeyCode::RightAlt: return GLFW_KEY_RIGHT_ALT;
            case EKeyCode::RightSuper: return GLFW_KEY_RIGHT_SUPER;

            default: return GLFW_KEY_UNKNOWN;
        }
    }

    static int ConvertToGLFWMouseButton(const EMouseButton button)
    {
        const int glfwButton = static_cast<int>(button);
        NES_ASSERT(glfwButton < GLFW_MOUSE_BUTTON_6);
        return glfwButton;
    }

    static int ConvertToGLFWCursorMode(const ECursorMode mode)
    {
        switch (mode)
        {
            case ECursorMode::Hidden: return GLFW_CURSOR_HIDDEN;
            case ECursorMode::Disabled: return GLFW_CURSOR_DISABLED;
            case ECursorMode::Captured: return GLFW_CURSOR_CAPTURED;
            default: return GLFW_CURSOR_NORMAL;
        }
    }
    
    EKeyCode ConvertToKeyCode(const int key)
    {
        switch (key)
        {
            // Letters
            case GLFW_KEY_A: return nes::EKeyCode::A;
            case GLFW_KEY_B: return nes::EKeyCode::B;
            case GLFW_KEY_C: return nes::EKeyCode::C;
            case GLFW_KEY_D: return nes::EKeyCode::D;
            case GLFW_KEY_E: return nes::EKeyCode::E;
            case GLFW_KEY_F: return nes::EKeyCode::F;
            case GLFW_KEY_G: return nes::EKeyCode::G;
            case GLFW_KEY_H: return nes::EKeyCode::H;
            case GLFW_KEY_I: return nes::EKeyCode::I;
            case GLFW_KEY_J: return nes::EKeyCode::J;
            case GLFW_KEY_K: return nes::EKeyCode::K;
            case GLFW_KEY_L: return nes::EKeyCode::L;
            case GLFW_KEY_M: return nes::EKeyCode::M;
            case GLFW_KEY_N: return nes::EKeyCode::N;
            case GLFW_KEY_O: return nes::EKeyCode::O;
            case GLFW_KEY_P: return nes::EKeyCode::P;
            case GLFW_KEY_Q: return nes::EKeyCode::Q;
            case GLFW_KEY_R: return nes::EKeyCode::R;
            case GLFW_KEY_S: return nes::EKeyCode::S;
            case GLFW_KEY_T: return nes::EKeyCode::T;
            case GLFW_KEY_U: return nes::EKeyCode::U;
            case GLFW_KEY_V: return nes::EKeyCode::V;
            case GLFW_KEY_W: return nes::EKeyCode::W;
            case GLFW_KEY_X: return nes::EKeyCode::X;
            case GLFW_KEY_Y: return nes::EKeyCode::Y;
            case GLFW_KEY_Z: return nes::EKeyCode::Z;

            // Numbers
            case GLFW_KEY_0: return nes::EKeyCode::Num0;
            case GLFW_KEY_1: return nes::EKeyCode::Num1;
            case GLFW_KEY_2: return nes::EKeyCode::Num2;
            case GLFW_KEY_3: return nes::EKeyCode::Num3;
            case GLFW_KEY_4: return nes::EKeyCode::Num4;
            case GLFW_KEY_5: return nes::EKeyCode::Num5;
            case GLFW_KEY_6: return nes::EKeyCode::Num6;
            case GLFW_KEY_7: return nes::EKeyCode::Num7;
            case GLFW_KEY_8: return nes::EKeyCode::Num8;
            case GLFW_KEY_9: return nes::EKeyCode::Num9;

            // Navigation
            case GLFW_KEY_UP: return nes::EKeyCode::Up;
            case GLFW_KEY_DOWN: return nes::EKeyCode::Down;
            case GLFW_KEY_LEFT: return nes::EKeyCode::Left;
            case GLFW_KEY_RIGHT: return nes::EKeyCode::Right;
            case GLFW_KEY_PAGE_UP: return nes::EKeyCode::PageUp;
            case GLFW_KEY_PAGE_DOWN: return nes::EKeyCode::PageDown;
            case GLFW_KEY_HOME: return nes::EKeyCode::Home;
            case GLFW_KEY_END: return nes::EKeyCode::End;

            case GLFW_KEY_COMMA: return nes::EKeyCode::Comma;
            case GLFW_KEY_PERIOD: return nes::EKeyCode::Period;
            case GLFW_KEY_ESCAPE: return nes::EKeyCode::Escape;
            case GLFW_KEY_SPACE: return nes::EKeyCode::Space;
            case GLFW_KEY_ENTER: return nes::EKeyCode::Enter;
            case GLFW_KEY_BACKSPACE: return nes::EKeyCode::Backspace;
            case GLFW_KEY_DELETE: return nes::EKeyCode::Delete;
            case GLFW_KEY_TAB: return nes::EKeyCode::Tab;
            case GLFW_KEY_INSERT: return nes::EKeyCode::Insert;
            case GLFW_KEY_CAPS_LOCK: return nes::EKeyCode::Capslock;
            case GLFW_KEY_NUM_LOCK: return nes::EKeyCode::NumLock;
            case GLFW_KEY_PRINT_SCREEN: return nes::EKeyCode::PrintScreen;
            case GLFW_KEY_PAUSE: return nes::EKeyCode::Pause;

            // Modifiers
            case GLFW_KEY_LEFT_CONTROL: return nes::EKeyCode::LeftControl;
            case GLFW_KEY_RIGHT_CONTROL: return nes::EKeyCode::RightControl;
            case GLFW_KEY_LEFT_SHIFT: return nes::EKeyCode::LeftShift;
            case GLFW_KEY_RIGHT_SHIFT: return nes::EKeyCode::RightShift;
            case GLFW_KEY_LEFT_ALT: return nes::EKeyCode::LeftAlt;
            case GLFW_KEY_RIGHT_ALT: return nes::EKeyCode::RightAlt;
            case GLFW_KEY_LEFT_SUPER: return nes::EKeyCode::LeftSuper;
            case GLFW_KEY_RIGHT_SUPER: return nes::EKeyCode::RightSuper;

            default: return nes::EKeyCode::Unknown;
        }
    }

    EMouseButton ConvertToMouseButton(const int button)
    {
        if (button < GLFW_MOUSE_BUTTON_6)
	    {
		    return static_cast<EMouseButton>(button);
	    }

	    return EMouseButton::Unknown;
    }

    EKeyAction ConvertToKeyAction(const int action)
    {
        switch (action)
        {
            case GLFW_PRESS: return EKeyAction::Pressed;
            case GLFW_RELEASE: return EKeyAction::Released;
            case GLFW_REPEAT: return EKeyAction::Repeat;
            default: return EKeyAction::Unknown;
        }
    }

    EMouseAction ConvertToMouseAction(const int action)
    {
        switch (action)
        {
            case GLFW_PRESS: return EMouseAction::Pressed;
            case GLFW_RELEASE: return EMouseAction::Released;
            default: return EMouseAction::Unknown;
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
    bool InputManager::IsKeyDown_Impl(void* pNativeWindow, const EKeyCode key)
    {
        NES_ASSERT(pNativeWindow != nullptr);
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(pNativeWindow);

        const auto action = glfw::ConvertToKeyAction(glfwGetKey(pWindow, glfw::ConvertToGLFWKey(key)));
        return action == EKeyAction::Pressed || action == EKeyAction::Repeat;
    }

    bool InputManager::IsKeyUp_Impl(void* pNativeWindow, const EKeyCode key)
    {
        NES_ASSERT(pNativeWindow != nullptr);
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(pNativeWindow);

        const auto action = glfw::ConvertToKeyAction(glfwGetKey(pWindow, glfw::ConvertToGLFWKey(key)));
        return action == EKeyAction::Released;
    }

    bool InputManager::IsMouseButtonUp_Impl(void* pNativeWindow, const EMouseButton button)
    {
        NES_ASSERT(pNativeWindow != nullptr);
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(pNativeWindow);
        
        const auto action = glfw::ConvertToMouseAction(glfwGetMouseButton(pWindow, glfw::ConvertToGLFWMouseButton(button)));
        return action == EMouseAction::Pressed;
    }

    bool InputManager::IsMouseButtonDown_Impl(void* pNativeWindow, const EMouseButton button)
    {
        NES_ASSERT(pNativeWindow != nullptr);
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(pNativeWindow);

        const auto action = glfw::ConvertToMouseAction(glfwGetMouseButton(pWindow, glfw::ConvertToGLFWMouseButton(button)));
        return action == EMouseAction::Released;
    }

    void InputManager::SetCursorMode_Impl(void* pNativeWindow, const ECursorMode mode)
    {
        if (m_cursorMode == mode)
            return;
        
        NES_ASSERT(pNativeWindow != nullptr);
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(pNativeWindow);
        
        glfwSetInputMode(pWindow, GLFW_CURSOR, glfw::ConvertToGLFWCursorMode(mode));

        // If we are setting the mouse cursor to disabled, 
        if (glfwRawMouseMotionSupported())
        {
            if (mode == ECursorMode::Disabled)
            {
                glfwSetInputMode(pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }

            // If changing from disabled, change the raw mouse motion back. 
            else if (m_cursorMode == ECursorMode::Disabled)
            {
                glfwSetInputMode(pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
            }
        }
        
        m_cursorMode = mode;
    }

    Double2 InputManager::GetCursorPosition_Impl(void* pNativeWindow)
    {
        NES_ASSERT(pNativeWindow != nullptr);
        GLFWwindow* pWindow = checked_cast<GLFWwindow*>(pNativeWindow);

        Double2 cursorPosition{};
        glfwGetCursorPos(pWindow, &cursorPosition.x, &cursorPosition.y);
        return cursorPosition;
    }
}

#endif