// Event.h
#pragma once
#include <concepts>
#include "Nessie/Core/Hash.h"

//----------------------------------------------------------------------------------------------------
/// @brief : Use this in the body of an Event class to define the EventID and GetName functions.
///	@param type : Type of the Event. Example: KeyEvent, WindowResizeEvent, etc.
//----------------------------------------------------------------------------------------------------
#define NES_EVENT(type)                                                                     \
    private:                                                                                \
        static constexpr nes::EventID   kEventID = HashString32(#type);                     \
    public:                                                                                 \
        static constexpr nes::EventID   GetStaticEventID() { return kEventID; }             \
        virtual nes::EventID            GetEventID() const override { return kEventID; }    \
        virtual const char*             GetName() const override { return #type; }          \
    private:

namespace nes
{
    using EventID = uint32;

    template <typename Type>
    concept EventType = requires(Type value)
    {
        { Type::GetStaticEventID() } -> std::same_as<EventID>;
        { value.GetEventID() } -> std::same_as<EventID>;
    };

    template <typename Type, typename EventType>
    concept ValidEventCallaback = requires(Type value, EventType& eventValue)
    {
        { value.OnEvent(eventValue) };
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for any Event that can be pushed to the Application.
    //----------------------------------------------------------------------------------------------------
    class Event
    {
    public:
        Event() = default;
        virtual ~Event() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Setting an Event as handled will early out - no other listeners will get to respond to
        ///     this event.
        //----------------------------------------------------------------------------------------------------
        void                  SetHandled() { m_isHandled = true; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check whether this event has been handled. 
        //----------------------------------------------------------------------------------------------------
        bool                  IsHandled() const { return m_isHandled; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the unique identifier for the Event type.
        /// @note : Do not override this function manually, use the NES_EVENT(type) macro.
        //----------------------------------------------------------------------------------------------------
        virtual EventID       GetEventID() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the name of this Event type.
        /// @note : Do not override this function manually, use the NES_EVENT(type) macro.
        //----------------------------------------------------------------------------------------------------
        virtual const char*   GetName() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if a generic event is a specific event type.
        //----------------------------------------------------------------------------------------------------
        template <EventType Type>
        bool                  IsType() const { return GetEventID() == Type::GetStaticEventID(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : If the base event class is the specific type, it will return this event as requested Type*.
        ///     Otherwise, this will return nullptr.
        //----------------------------------------------------------------------------------------------------
        template <EventType Type>
        Type*                 Cast() { return IsType<Type>() ? reinterpret_cast<Type*>(this) : nullptr; }

    private:
        bool m_isHandled = false;
    };
}