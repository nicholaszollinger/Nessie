#pragma once
// Event.h
#include <concepts>
#include "Core/Generic/Hash.h"

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Use this in the body of an Event class to define the EventID and GetName functions.
///		@param type : Type of the Event. Example: KeyEvent, WindowResizeEvent, etc.
//----------------------------------------------------------------------------------------------------
#define NES_EVENT(type)                                                         \
    private:                                                                    \
        static constexpr nes::EventID kEventID = HashString32(#type);           \
    public:                                                                     \
        static constexpr nes::EventID GetStaticEventID() { return kEventID; }   \
        virtual nes::EventID GetEventID() const override { return kEventID; }   \
        virtual const char* GetName() const override { return #type; }          \
    private:

namespace nes
{
    using EventID = uint32_t;

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
    //		NOTES:
    //		
    ///		@brief : Base class for any Event that can be pushed to the Application.
    //----------------------------------------------------------------------------------------------------
    class Event
    {
        bool m_isHandled = false;

    public:
        Event() = default;
        virtual ~Event() = default;

        void SetHandled();
        [[nodiscard]] bool IsHandled() const;
        [[nodiscard]] virtual EventID GetEventID() const = 0;
        [[nodiscard]] virtual const char* GetName() const = 0;
    };
}