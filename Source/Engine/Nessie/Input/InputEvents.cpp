// InputEvents.cpp

#include "InputEvents.h"

namespace nes
{
    KeyEvent::KeyEvent(KeyCode code, KeyAction action, Modifiers modifiers)
        : m_keyCode(code)
        , m_action(action)
        , m_modifiers(modifiers)
    {
        //
    }

    MouseButtonEvent::MouseButtonEvent(const MouseButton button, const MouseAction action, const Modifiers modifiers, const float xPos, const float yPos)
        : m_button(button)
        , m_action(action)
        , m_modifiers(modifiers)
        , m_position(xPos, yPos)
    {
        //
    }

    MouseMoveEvent::MouseMoveEvent(const float xPos, const float yPos, const float xDelta, const float yDelta)
        : m_position(xPos, yPos)
        , m_delta(xDelta, yDelta)
    {
        //
    }

    MouseScrollEvent::MouseScrollEvent(const float xDelta, const float yDelta)
        : m_delta(xDelta, yDelta)
    {
        //
    }
}