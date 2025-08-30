// ScopedTimer.h
#pragma once
#include "Nessie/Core/ScopeExit.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Starts a timer and sets a member variable value on scope exit.
    /// @param memberVar : Name of the member variable you want to set the elapsed time to.
    /// @param timePeriod : Interval of time you want to get the resulting time in. Ex: Timer::Milliseconds.
    ///     Usage: NES_SCOPED_TIMER(m_variable, Timer::Milliseconds);
    //----------------------------------------------------------------------------------------------------
    #define NES_SCOPED_TIMER_MEMBER(memberVar, timePeriod)                                 \
        Timer NES_SCOPED_TAG(timer); NES_SCOPED_TAG(timer).Start();                     \
        NES_ON_SCOPE_EXIT([this, &NES_SCOPED_TAG(timer)]()                       \
        {                                                                               \
            memberVar = static_cast<decltype(memberVar)>(NES_SCOPED_TAG(timer).Stop<timePeriod>());    \
        })
}
