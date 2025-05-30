#pragma once
// InputEvents.h
#include "Input/InputCodes.h"
#include "Core/Events/Event.h"
#include "Math/Vector2.h"

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
        Vector2f        GetPosition() const     { return m_position; }

    private:
        EMouseButton    m_button    = EMouseButton::Unknown;
        EMouseAction    m_action    = EMouseAction::Unknown;
        Modifiers       m_modifiers = Modifiers{};
        Vector2f        m_position;
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Event called when the Mouse is moved.
    //----------------------------------------------------------------------------------------------------
    class MouseMoveEvent final : public Event
    {
        NES_EVENT(MouseMoveEvent)

    public:
        MouseMoveEvent(const float xPos, const float yPos, const float xDelta, const float yDelta);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current position of the mouse. 
        //----------------------------------------------------------------------------------------------------
        Vector2f GetPosition() const { return m_position; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the delta position that the mouse has moved since the last frame.
        //----------------------------------------------------------------------------------------------------
        Vector2f GetDelta() const    { return m_delta; }

    private:
        Vector2f m_position{};
        Vector2f m_delta{};
    };


    //----------------------------------------------------------------------------------------------------
    ///		@brief : Event called when the Mouse Wheel is scrolled.
    //----------------------------------------------------------------------------------------------------
    class MouseScrollEvent final : public Event
    {
        NES_EVENT(MouseScrollEvent)
    
    public:
        MouseScrollEvent(const float xDelta, const float yDelta);

        float   GetDeltaX() const { return m_delta.x; }
        float   GetDeltaY() const { return m_delta.y; }

    private:
        Vector2f m_delta{};
    };
}