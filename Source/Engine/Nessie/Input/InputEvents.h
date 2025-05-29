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

        EKeyCode m_keyCode = EKeyCode::Unknown;
        EKeyAction m_action = EKeyAction::Unknown;
        Modifiers m_modifiers = Modifiers{};

    public:
        KeyEvent(EKeyCode code, EKeyAction action, Modifiers modifiers);

        [[nodiscard]] EKeyCode GetKeyCode() const        { return m_keyCode; }
        [[nodiscard]] EKeyAction GetAction() const       { return m_action; }
        [[nodiscard]] Modifiers GetModifiers() const    { return m_modifiers; }
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Event called when a Mouse Button is activated.
    //----------------------------------------------------------------------------------------------------
    class MouseButtonEvent final : public Event
    {
        NES_EVENT(MouseButtonEvent)

        EMouseButton m_button = EMouseButton::Unknown;
        EMouseAction m_action = EMouseAction::Unknown;
        Modifiers m_modifiers = Modifiers{};
        Vector2f m_position;

    public:
        MouseButtonEvent(const EMouseButton button, const EMouseAction action, const Modifiers modifiers, const float xPos, const float yPos);

        [[nodiscard]] EMouseButton GetButton() const     { return m_button; }
        [[nodiscard]] EMouseAction GetAction() const     { return m_action; }
        [[nodiscard]] Modifiers GetModifiers() const    { return m_modifiers; }
        [[nodiscard]] Vector2f GetPosition() const      { return m_position; }
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
        [[nodiscard]] Vector2f GetDelta() const    { return m_delta; }
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