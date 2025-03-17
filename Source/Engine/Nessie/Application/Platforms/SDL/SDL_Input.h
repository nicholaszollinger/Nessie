// SDL_Input.h
#pragma once
#include "Core/Config.h"
#ifdef NES_WINDOW_API_SDL
#include <SDL_events.h>
#include <SDL_scancode.h>
#include "Input/InputEvents.h"

namespace nes::SDL
{
    KeyCode ToKeyCode(SDL_Scancode key);
    MouseButton ToMouseButton(int button);
    Modifiers CurrentModifiers();
    KeyAction ToKeyAction(const SDL_KeyboardEvent& keyEvent);
    MouseAction ToMouseAction(const SDL_MouseButtonEvent& mouseEvent);
}
#endif