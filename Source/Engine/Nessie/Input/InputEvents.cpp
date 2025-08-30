// InputEvents.cpp
#include "InputEvents.h"

namespace nes
{
    KeyEvent::KeyEvent(EKeyCode code, EKeyAction action, Modifiers modifiers)
        : m_keyCode(code)
        , m_action(action)
        , m_modifiers(modifiers)
    {
        //
    }

    MouseButtonEvent::MouseButtonEvent(const EMouseButton button, const EMouseAction action, const Modifiers modifiers, const float xPos, const float yPos)
        : m_button(button)
        , m_action(action)
        , m_modifiers(modifiers)
        , m_position(xPos, yPos)
    {
        //
    }

    MouseMoveEvent::MouseMoveEvent(const float xPos, const float yPos)
        : m_position(xPos, yPos)
    {
        //
    }

    MouseScrollEvent::MouseScrollEvent(const float xDelta, const float yDelta)
        : m_delta(xDelta, yDelta)
    {
        //
    }
}