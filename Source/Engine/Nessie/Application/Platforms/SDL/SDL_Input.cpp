// SDL_Input.cpp

#include "SDL_Input.h"

#include <SDL_keyboard.h>
#include <SDL_keycode.h>
#include <SDL_mouse.h>

namespace nes::SDL
{
    KeyCode ConvertToKeyCode(SDL_Scancode key)
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
    
    MouseButton ConvertToMouseButton(int button)
    {
        switch (button)
        {
            case SDL_BUTTON_LEFT: return MouseButton::Left;
            case SDL_BUTTON_RIGHT: return MouseButton::Right;
            case SDL_BUTTON_MIDDLE: return MouseButton::Middle;

            default: return MouseButton::Unknown;
        }
    }
    
    Modifiers GetCurrentModifiers()
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
    
    KeyAction GetKeyAction(const SDL_KeyboardEvent& keyEvent)
    {
        if (keyEvent.type == SDL_KEYDOWN)
        {
            return keyEvent.repeat > 0? KeyAction::Repeat : KeyAction::Pressed;
        }

        return KeyAction::Released;
    }
    
    MouseAction GetMouseAction(const SDL_MouseButtonEvent& mouseEvent)
    {
        if (mouseEvent.type == SDL_MOUSEBUTTONDOWN)
        {
            return MouseAction::Pressed;
        }

        return MouseAction::Released;
    }
}
