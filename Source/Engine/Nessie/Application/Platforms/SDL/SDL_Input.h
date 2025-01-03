// SDL_Input.h
#pragma once

#include <SDL_events.h>
#include <SDL_scancode.h>
#include "Input/InputEvents.h"

namespace nes::SDL
{
    KeyCode ConvertToKeyCode(SDL_Scancode key);
    MouseButton ConvertToMouseButton(int button);
    Modifiers GetCurrentModifiers();
    KeyAction GetKeyAction(const SDL_KeyboardEvent& keyEvent);
    MouseAction GetMouseAction(const SDL_MouseButtonEvent& mouseEvent);
}
