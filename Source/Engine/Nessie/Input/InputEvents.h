// InputEvents.h
#pragma once
#include "Nessie/Input/InputCodes.h"
#include "Nessie/Core/Events/Event.h"
#include "Nessie/Math/Vec2.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///	@brief : Keyboard Event that contains the KeyCode, KeyAction, and Modifiers.
    //----------------------------------------------------------------------------------------------------
    class KeyEvent final : public Event
    {
        NES_EVENT(KeyEvent)
    
    public:
        KeyEvent(EKeyCode code, EKeyAction action, Modifiers modifiers);

        EKeyCode    GetKeyCode() const      { return m_keyCode; }
        EKeyAction  GetAction() const       { return m_action; }
        Modifiers   GetModifiers() const    { return m_modifiers; }

    private:
        EKeyCode    m_keyCode   = EKeyCode::Unknown;
        EKeyAction  m_action    = EKeyAction::Unknown;
        Modifiers   m_modifiers = Modifiers{};
    };

    //----------------------------------------------------------------------------------------------------
    ///	@brief : Event called when a Mouse Button is activated.
    //----------------------------------------------------------------------------------------------------
    class MouseButtonEvent final : public Event
    {
        NES_EVENT(MouseButtonEvent)

    public:
        MouseButtonEvent(const EMouseButton button, const EMouseAction action, const Modifiers modifiers, const float xPos, const float yPos);

        EMouseButton    GetButton() const       { return m_button; }
        EMouseAction    GetAction() const       { return m_action; }
        Modifiers       GetModifiers() const    { return m_modifiers; }
        Vec2            GetPosition() const     { return m_position; }

    private:
        EMouseButton    m_button    = EMouseButton::Unknown;
        EMouseAction    m_action    = EMouseAction::Unknown;
        Modifiers       m_modifiers = Modifiers{};
        Vec2            m_position{};
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Event called when the Mouse is moved.
    //----------------------------------------------------------------------------------------------------
    class MouseMoveEvent final : public Event
    {
        NES_EVENT(MouseMoveEvent)

    public:
        MouseMoveEvent(const float xPos, const float yPos);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current position of the mouse. 
        //----------------------------------------------------------------------------------------------------
        Vec2 GetPosition() const { return m_position; }
    
    private:
        Vec2 m_position{};
    };


    //----------------------------------------------------------------------------------------------------
    ///	@brief : Event called when the Mouse Wheel is scrolled.
    //----------------------------------------------------------------------------------------------------
    class MouseScrollEvent final : public Event
    {
        NES_EVENT(MouseScrollEvent)
    
    public:
        MouseScrollEvent(const float xDelta, const float yDelta);

        float   GetDeltaX() const { return m_delta.x; }
        float   GetDeltaY() const { return m_delta.y; }

    private:
        Vec2 m_delta{};
    };
}