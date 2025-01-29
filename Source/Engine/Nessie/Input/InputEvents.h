#pragma once
// InputEvents.h
#include "Input/InputCodes.h"
#include "Core/Events/Event.h"
#include "Math/Vector2.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Keyboard Event that contains the KeyCode, KeyAction, and Modifiers.
    //----------------------------------------------------------------------------------------------------
    class KeyEvent final : public Event
    {
        NES_EVENT(KeyEvent)

        KeyCode m_keyCode = KeyCode::Unknown;
        KeyAction m_action = KeyAction::Unknown;
        Modifiers m_modifiers = Modifiers{};

    public:
        KeyEvent(KeyCode code, KeyAction action, Modifiers modifiers);

        [[nodiscard]] KeyCode GetKeyCode() const        { return m_keyCode; }
        [[nodiscard]] KeyAction GetAction() const       { return m_action; }
        [[nodiscard]] Modifiers GetModifiers() const    { return m_modifiers; }
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Event called when a Mouse Button is activated.
    //----------------------------------------------------------------------------------------------------
    class MouseButtonEvent final : public Event
    {
        NES_EVENT(MouseButtonEvent)

        MouseButton m_button = MouseButton::Unknown;
        MouseAction m_action = MouseAction::Unknown;
        Modifiers m_modifiers = Modifiers{};
        Vector2f m_position;

    public:
        MouseButtonEvent(const MouseButton button, const MouseAction action, const Modifiers modifiers, const float xPos, const float yPos);

        [[nodiscard]] MouseButton GetButton() const     { return m_button; }
        [[nodiscard]] MouseAction GetAction() const     { return m_action; }
        [[nodiscard]] Modifiers GetModifiers() const    { return m_modifiers; }
        [[nodiscard]] Vector2f GetPosition() const          { return m_position; }
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Event called when the Mouse is moved.
    //----------------------------------------------------------------------------------------------------
    class MouseMoveEvent final : public Event
    {
        NES_EVENT(MouseMoveEvent)

        Vector2f m_position{};
        Vector2f m_delta{};

    public:
        MouseMoveEvent(const float xPos, const float yPos, const float xDelta, const float yDelta);

        [[nodiscard]] Vector2f GetPosition() const { return m_position; }
        [[nodiscard]] Vector2f GetDelta() const { return m_delta; }
    };


    //----------------------------------------------------------------------------------------------------
    ///		@brief : Event called when the Mouse Wheel is scrolled.
    //----------------------------------------------------------------------------------------------------
    class MouseScrollEvent final : public Event
    {
        NES_EVENT(MouseScrollEvent)

        Vector2f m_delta{};

    public:
        MouseScrollEvent(const float xDelta, const float yDelta);

        [[nodiscard]] float GetDeltaX() const { return m_delta.x; }
        [[nodiscard]] float GetDeltaY() const { return m_delta.y; }
    };
}