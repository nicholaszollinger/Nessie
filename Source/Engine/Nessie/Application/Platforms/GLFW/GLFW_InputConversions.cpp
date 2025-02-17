// GLFW_InputConversions.cpp

#include "GLFW_InputConversions.h"
#ifdef NES_WINDOW_API_GLFW
#include <GLFW/glfw3.h>

namespace nes::glfw
{
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
#endif