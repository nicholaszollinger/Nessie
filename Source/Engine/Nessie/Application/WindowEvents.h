#pragma once
// WindowEvents.h

#include "Core/Events/Event.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Invoked when the Window is closed.
    //----------------------------------------------------------------------------------------------------
    class WindowClosedEvent final : public Event
    {
        NES_EVENT(WindowClosedEvent)
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Invoked when the Window is resized.
    //----------------------------------------------------------------------------------------------------
    class WindowResizeEvent final : public Event
    {
        NES_EVENT(WindowResizeEvent)

        uint32_t m_width = 0;
        uint32_t m_height = 0;

    public:
        explicit WindowResizeEvent(const uint32_t width, const uint32_t height) : m_width(width), m_height(height) { }

        [[nodiscard]] uint32_t GetWidth() const { return m_width; }
        [[nodiscard]] uint32_t GetHeight() const { return m_height; }
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Invoked when the Window is minimized or un-minimized.
    //----------------------------------------------------------------------------------------------------
    class WindowMinimizeEvent final : public Event
    {
        NES_EVENT(WindowMinimizeEvent)

        bool m_isMinimized = false;

    public:
        explicit WindowMinimizeEvent(const bool isMinimized) : m_isMinimized(isMinimized) { }

        [[nodiscard]] bool IsMinimized() const { return m_isMinimized; }
    };

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Invoked when the Window is focused or unfocused.
    //----------------------------------------------------------------------------------------------------
    class WindowFocusEvent final : public Event
    {
        NES_EVENT(WindowFocusEvent)

        bool m_hasFocus = false;

    public:
        explicit WindowFocusEvent(const bool hasFocus) : m_hasFocus(hasFocus) { }

        [[nodiscard]] bool HasFocus() const { return m_hasFocus; }
    };
}