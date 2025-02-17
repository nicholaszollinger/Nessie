// GLFW_InputConversions.h
#pragma once
#include "Core/Config.h"
#ifdef NES_WINDOW_API_GLFW
#include "Input/InputEvents.h"

namespace nes::glfw
{
    KeyCode ConvertToKeyCode(const int key);
    MouseButton ConvertToMouseButton(const int button);
    Modifiers ConvertToModifiers(const int modifiers);
    KeyAction ConvertToKeyAction(const int action);
    MouseAction ConvertToMouseAction(const int action);
}
#endif