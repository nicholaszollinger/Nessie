// GLFW_InputConversions.h
#pragma once
#include "Core/Config.h"
#ifdef NES_WINDOW_API_GLFW
#include "Input/InputEvents.h"

namespace nes::glfw
{
    EKeyCode ConvertToKeyCode(const int key);
    EMouseButton ConvertToMouseButton(const int button);
    Modifiers ConvertToModifiers(const int modifiers);
    EKeyAction ConvertToKeyAction(const int action);
    EMouseAction ConvertToMouseAction(const int action);
}
#endif