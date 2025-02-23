// SDL_Input.cpp

#include "SDL_Input.h"

#include <SDL_keyboard.h>
#include <SDL_keycode.h>
#include <SDL_mouse.h>
#include "Core/Config.h"
#include "Input/InputManager.h"

namespace nes::SDL
{
    KeyCode ToKeyCode(SDL_Scancode key)
    {
        switch (key)
        {
            case SDL_SCANCODE_0: return KeyCode::Num0;
            case SDL_SCANCODE_1: return KeyCode::Num1;
            case SDL_SCANCODE_2: return KeyCode::Num2;
            case SDL_SCANCODE_3: return KeyCode::Num3;
            case SDL_SCANCODE_4: return KeyCode::Num4;
            case SDL_SCANCODE_5: return KeyCode::Num5;
            case SDL_SCANCODE_6: return KeyCode::Num6;
            case SDL_SCANCODE_7: return KeyCode::Num7;
            case SDL_SCANCODE_8: return KeyCode::Num8;
            case SDL_SCANCODE_9: return KeyCode::Num9;

            case SDL_SCANCODE_A: return KeyCode::A;
            case SDL_SCANCODE_B: return KeyCode::B;
            case SDL_SCANCODE_C: return KeyCode::C;
            case SDL_SCANCODE_D: return KeyCode::D;
            case SDL_SCANCODE_E: return KeyCode::E;
            case SDL_SCANCODE_F: return KeyCode::F;
            case SDL_SCANCODE_G: return KeyCode::G;
            case SDL_SCANCODE_H: return KeyCode::H;
            case SDL_SCANCODE_I: return KeyCode::I;
            case SDL_SCANCODE_J: return KeyCode::J;
            case SDL_SCANCODE_K: return KeyCode::K;
            case SDL_SCANCODE_L: return KeyCode::L;
            case SDL_SCANCODE_M: return KeyCode::M;
            case SDL_SCANCODE_N: return KeyCode::N;
            case SDL_SCANCODE_O: return KeyCode::O;
            case SDL_SCANCODE_P: return KeyCode::P;
            case SDL_SCANCODE_Q: return KeyCode::Q;
            case SDL_SCANCODE_R: return KeyCode::R;
            case SDL_SCANCODE_S: return KeyCode::S;
            case SDL_SCANCODE_T: return KeyCode::T;
            case SDL_SCANCODE_U: return KeyCode::U;
            case SDL_SCANCODE_V: return KeyCode::V;
            case SDL_SCANCODE_W: return KeyCode::W;
            case SDL_SCANCODE_X: return KeyCode::X;
            case SDL_SCANCODE_Y: return KeyCode::Y;
            case SDL_SCANCODE_Z: return KeyCode::Z;

            case SDL_SCANCODE_LEFT: return KeyCode::Left;
            case SDL_SCANCODE_RIGHT: return KeyCode::Right;
            case SDL_SCANCODE_UP: return KeyCode::Up;
            case SDL_SCANCODE_DOWN: return KeyCode::Down;
            case SDL_SCANCODE_SPACE: return KeyCode::Space;
            case SDL_SCANCODE_ESCAPE: return KeyCode::Escape;
            case SDL_SCANCODE_RETURN: return KeyCode::Enter;
            case SDL_SCANCODE_BACKSPACE: return KeyCode::Backspace;
            case SDL_SCANCODE_DELETE: return KeyCode::Delete;
            case SDL_SCANCODE_TAB: return KeyCode::Tab;
            case SDL_SCANCODE_LEFTBRACKET: return KeyCode::LeftBracket;
            case SDL_SCANCODE_RIGHTBRACKET: return KeyCode::RightBracket;
            case SDL_SCANCODE_HOME: return KeyCode::Home;
            case SDL_SCANCODE_END: return KeyCode::End;
            case SDL_SCANCODE_PAGEUP: return KeyCode::PageUp;
            case SDL_SCANCODE_PAGEDOWN: return KeyCode::PageDown;
            case SDL_SCANCODE_CAPSLOCK: return KeyCode::Capslock;
            case SDL_SCANCODE_PRINTSCREEN: return KeyCode::PrintScreen;
            case SDL_SCANCODE_PAUSE: return KeyCode::Pause;
            case SDL_SCANCODE_INSERT: return KeyCode::Insert;
            case SDL_SCANCODE_PERIOD: return KeyCode::Period;

            // [TODO]: I am missing some more.
            

            default: return KeyCode::Unknown;
        }
    }

    SDL_Scancode ConvertToSDLScancode(const KeyCode key)
    {
        switch (key)
        {
            case KeyCode::A: return SDL_SCANCODE_A;
            case KeyCode::B: return SDL_SCANCODE_B;
            case KeyCode::C: return SDL_SCANCODE_C;
            case KeyCode::D: return SDL_SCANCODE_D;
            case KeyCode::E: return SDL_SCANCODE_E;
            case KeyCode::F: return SDL_SCANCODE_F;
            case KeyCode::G: return SDL_SCANCODE_G;
            case KeyCode::H: return SDL_SCANCODE_H;
            case KeyCode::I: return SDL_SCANCODE_I;
            case KeyCode::J: return SDL_SCANCODE_J;
            case KeyCode::K: return SDL_SCANCODE_K;
            case KeyCode::L: return SDL_SCANCODE_L;
            case KeyCode::M: return SDL_SCANCODE_M;
            case KeyCode::N: return SDL_SCANCODE_N;
            case KeyCode::O: return SDL_SCANCODE_O;
            case KeyCode::P: return SDL_SCANCODE_P;
            case KeyCode::Q: return SDL_SCANCODE_Q;
            case KeyCode::R: return SDL_SCANCODE_R;
            case KeyCode::S: return SDL_SCANCODE_S;
            case KeyCode::T: return SDL_SCANCODE_T;
            case KeyCode::U: return SDL_SCANCODE_U;
            case KeyCode::V: return SDL_SCANCODE_V;
            case KeyCode::W: return SDL_SCANCODE_W;
            case KeyCode::X: return SDL_SCANCODE_X;
            case KeyCode::Y: return SDL_SCANCODE_Y;
            case KeyCode::Z: return SDL_SCANCODE_Z;
            case KeyCode::Num0: return SDL_SCANCODE_0;
            case KeyCode::Num1: return SDL_SCANCODE_1;
            case KeyCode::Num2: return SDL_SCANCODE_2;
            case KeyCode::Num3: return SDL_SCANCODE_3;
            case KeyCode::Num4: return SDL_SCANCODE_4;
            case KeyCode::Num5: return SDL_SCANCODE_5;
            case KeyCode::Num6: return SDL_SCANCODE_6;
            case KeyCode::Num7: return SDL_SCANCODE_7;
            case KeyCode::Num8: return SDL_SCANCODE_8;
            case KeyCode::Num9: return SDL_SCANCODE_9;
            case KeyCode::Space: return SDL_SCANCODE_SPACE;
            case KeyCode::Enter: return SDL_SCANCODE_RETURN;
            case KeyCode::Escape: return SDL_SCANCODE_ESCAPE;
            case KeyCode::Backspace: return SDL_SCANCODE_BACKSPACE;
            case KeyCode::Delete: return SDL_SCANCODE_DELETE;
            case KeyCode::Tab: return SDL_SCANCODE_TAB;
            case KeyCode::Insert: return SDL_SCANCODE_INSERT;
            case KeyCode::Capslock: return SDL_SCANCODE_CAPSLOCK;
            case KeyCode::NumLock: return SDL_SCANCODE_NUMLOCKCLEAR;
            case KeyCode::PrintScreen: return SDL_SCANCODE_PRINTSCREEN;
            case KeyCode::Pause: return SDL_SCANCODE_PAUSE;
            case KeyCode::Comma: return SDL_SCANCODE_COMMA;
            case KeyCode::Period: return SDL_SCANCODE_PERIOD;
            case KeyCode::Exclamation: return SDL_SCANCODE_KP_EXCLAM;
            case KeyCode::Pound: return SDL_SCANCODE_KP_HASH;
            case KeyCode::Percent: return SDL_SCANCODE_KP_PERCENT;
            case KeyCode::And: return SDL_SCANCODE_KP_AMPERSAND;
            case KeyCode::Star: return SDL_SCANCODE_KP_MULTIPLY;
            case KeyCode::LeftParen: return SDL_SCANCODE_KP_LEFTPAREN;
            case KeyCode::RightParen: return SDL_SCANCODE_KP_RIGHTPAREN;
            case KeyCode::LeftBracket: return SDL_SCANCODE_LEFTBRACKET;
            case KeyCode::RightBracket: return SDL_SCANCODE_RIGHTBRACKET;
            case KeyCode::Backslash: return SDL_SCANCODE_BACKSLASH;
            case KeyCode::GraveAccent: return SDL_SCANCODE_GRAVE;
            case KeyCode::Up: return SDL_SCANCODE_UP;
            case KeyCode::Down: return SDL_SCANCODE_DOWN;
            case KeyCode::Left: return SDL_SCANCODE_LEFT;
            case KeyCode::Right: return SDL_SCANCODE_RIGHT;
            case KeyCode::PageUp: return SDL_SCANCODE_PAGEUP;
            case KeyCode::PageDown: return SDL_SCANCODE_PAGEDOWN;
            case KeyCode::Home: return SDL_SCANCODE_HOME;
            case KeyCode::End: return SDL_SCANCODE_END;
            case KeyCode::LeftControl: return SDL_SCANCODE_LCTRL;
            case KeyCode::LeftShift: return SDL_SCANCODE_LSHIFT;
            case KeyCode::LeftAlt: return SDL_SCANCODE_LALT;
            case KeyCode::LeftSuper: return SDL_SCANCODE_LGUI;
            case KeyCode::RightControl: return SDL_SCANCODE_RCTRL;
            case KeyCode::RightShift: return SDL_SCANCODE_RSHIFT;
            case KeyCode::RightAlt: return SDL_SCANCODE_RALT;
            case KeyCode::RightSuper: return SDL_SCANCODE_RGUI;

            default: return SDL_SCANCODE_UNKNOWN;
        }
    }
    
    MouseButton ToMouseButton(int button)
    {
        switch (button)
        {
            case SDL_BUTTON_LEFT: return MouseButton::Left;
            case SDL_BUTTON_RIGHT: return MouseButton::Right;
            case SDL_BUTTON_MIDDLE: return MouseButton::Middle;

            default: return MouseButton::Unknown;
        }
    }
    
    Modifiers CurrentModifiers()
    {
        const auto modState = SDL_GetModState();
        Modifiers result{};

        result.m_control = (modState & KMOD_CTRL);
        result.m_alt = (modState & KMOD_ALT);
        result.m_shift = (modState & KMOD_SHIFT);
        result.m_capsLock = (modState & KMOD_CAPS);
        result.m_numLock = (modState & KMOD_NUM);

        return result;
    }
    
    KeyAction ToKeyAction(const SDL_KeyboardEvent& keyEvent)
    {
        if (keyEvent.type == SDL_KEYDOWN)
        {
            return keyEvent.repeat > 0? KeyAction::Repeat : KeyAction::Pressed;
        }

        return KeyAction::Released;
    }
    
    MouseAction ToMouseAction(const SDL_MouseButtonEvent& mouseEvent)
    {
        if (mouseEvent.type == SDL_MOUSEBUTTONDOWN)
        {
            return MouseAction::Pressed;
        }

        return MouseAction::Released;
    }
}

#ifdef NES_WINDOW_API_SDL
namespace nes
{
    bool InputManager::IsKeyDown_Impl([[maybe_unused]] void* pNativeWindow, const KeyCode key)
    {
        const uint8_t* state = SDL_GetKeyboardState(nullptr);

        const SDL_Scancode scanCode = SDL::ConvertToSDLScancode(key);
        return state[scanCode];
    }

    bool InputManager::IsKeyUp_Impl([[maybe_unused]] void* pNativeWindow, const KeyCode key)
    {
        const uint8_t* state = SDL_GetKeyboardState(nullptr);

        const SDL_Scancode scanCode = SDL::ConvertToSDLScancode(key);
        return state[scanCode] == 0;
    }

    bool InputManager::IsMouseButtonDown_Impl([[maybe_unused]] void* pNativeWindow, const MouseButton button)
    {
        [[maybe_unused]] int posX;
        [[maybe_unused]] int posY;
        const uint32_t mouseState = SDL_GetMouseState(&posX, &posY);

        switch (button)
        {
            case MouseButton::Left: return mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);
            case MouseButton::Right: return mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT);
            case MouseButton::Middle: return mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE);
            
            default: return false;
        }
    }
    
    bool InputManager::IsMouseButtonUp_Impl([[maybe_unused]] void* pNativeWindow, const MouseButton button)
    {
        [[maybe_unused]] int posX;
        [[maybe_unused]] int posY;
        const uint32_t mouseState = SDL_GetMouseState(&posX, &posY);

        switch (button)
        {
            case MouseButton::Left: return (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) == 0;
            case MouseButton::Right: return (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) == 0;
            case MouseButton::Middle: return (mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE)) == 0;
            
            default: return false;
        }
    }
    
    Vector2d InputManager::GetCursorPosition_Impl([[maybe_unused]] void* pNativeWindow)
    {
        int posX;
        int posY;
        [[maybe_unused]] const uint32_t mouseState = SDL_GetMouseState(&posX, &posY);
        return Vector2d(static_cast<double>(posX), static_cast<double>(posY));
    }

    void InputManager::SetCursorMode_Impl([[maybe_unused]] void* pNativeWindow, const CursorMode mode)
    {
        switch (mode)
        {
            case CursorMode::Visible:
                SDL_ShowCursor(SDL_ENABLE);
                SDL_SetRelativeMouseMode(SDL_FALSE);
                break;
            
            case CursorMode::Hidden:
                SDL_ShowCursor(SDL_DISABLE);
                SDL_SetRelativeMouseMode(SDL_FALSE);
                break;
            
            case CursorMode::Disabled:
                SDL_ShowCursor(SDL_DISABLE);
                SDL_SetRelativeMouseMode(SDL_TRUE);

            // Operates the same as disable for now.
            case CursorMode::Captured:
                SDL_ShowCursor(SDL_DISABLE);
                SDL_SetRelativeMouseMode(SDL_TRUE);
                break;
        }

        m_cursorMode = mode;
    }
}
#endif