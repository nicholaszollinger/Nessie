#pragma once
// CheckedCast.h

#include "Assert.h"
#include "Core/Generic/Concepts.h"

//-----------------------------------------------------------------------------------------------------------------------------
///		@brief : If logs are enabled, this will perform a dynamic_cast. Otherwise, this will perform a static_cast.
///             Both To and From must be pointers.
//-----------------------------------------------------------------------------------------------------------------------------
template <nes::PointerType To, nes::PointerType From>
constexpr To checked_cast(From pFrom)
{
#if NES_LOGGING_ENABLED
    // if pFrom is already null, just return null without any special casting
    if (!pFrom)
        return nullptr;

    if constexpr (std::is_same_v<From, void*>)
    {
        To pResult = static_cast<To>(pFrom);
        NES_ASSERTV(pResult, "checked_cast failed.");
        return pResult;
    }

    else
    {
        To pResult = dynamic_cast<To>(pFrom);
        NES_ASSERTV(pResult, "checked_cast failed.");
        return pResult;
    }
    
#else
    return static_cast<To>(pFrom);
#endif
}

//----------------------------------------------------------------------------------------------------
//		NOTES:
//		
///		@brief : Checked cast for reference types. If Logs are enabled, this will perform a dynamic_cast
///              on the pointers, and return the dereferenced result. Otherwise, this will
///              perform a static_cast.
///		@tparam To : Reference type to cast to. Must be a derived class of From.
///		@tparam From : Type to cast from. Must be a base class of To.
///		@param from : Reference value to cast.
///		@returns : Casted reference value.
//----------------------------------------------------------------------------------------------------
export template <nes::ReferenceType To, typename From>
constexpr To checked_cast(From& from) requires std::is_base_of_v<From, std::remove_reference_t<To>>
{
#if NES_LOGGING_ENABLED
    using PointerType = std::add_pointer_t<std::remove_reference_t<To>>;
    return *(checked_cast<PointerType>(&from));
#else
    return static_cast<To>(from);
#endif
}