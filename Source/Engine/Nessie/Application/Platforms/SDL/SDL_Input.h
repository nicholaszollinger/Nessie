// SDL_Input.h
#pragma once

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
