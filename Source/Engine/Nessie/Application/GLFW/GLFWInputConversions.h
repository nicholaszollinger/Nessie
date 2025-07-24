// GLFW_InputConversions.h
#pragma once
#include "Nessie/Core/Config.h"
#include "Nessie/Input/Cursor.h"
#include "Nessie/Input/InputEvents.h"

namespace nes::glfw
{
    EKeyCode        ConvertToKeyCode(const int key);
    EMouseButton    ConvertToMouseButton(const int button);
    Modifiers       ConvertToModifiers(const int modifiers);
    EKeyAction      ConvertToKeyAction(const int action);
    EMouseAction    ConvertToMouseAction(const int action);
    int             ConvertToGLFWCursorMode(const ECursorMode mode);
}