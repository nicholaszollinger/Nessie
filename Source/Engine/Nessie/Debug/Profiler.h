#pragma once
// Profiler.h
#include "Nessie/Debug/Log.h"
#include "Nessie/Core/Time/Timer.h"

#if NES_LOGGING_ENABLED
//----------------------------------------------------------------------------------------------------
///	@brief : Use this to create a scope-based profiler that will log the time taken to
///     execute the scope. This is a quick way to profile a function or a block of code.
///	@param label : Label to give the test.
//----------------------------------------------------------------------------------------------------
#define NES_PROFILE_SCOPE(label) nes::SimpleInstrumentationProfiler profile(label)

#else
#define NES_PROFILE_SCOPE(label) void(0)
#endif

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : On construction, saves a time point. On destruction, logs the time that has passed
    ///     since construction.
    //----------------------------------------------------------------------------------------------------
    class SimpleScopedProfiler
    {
    public:
        explicit SimpleScopedProfiler(const std::string& label);
        ~SimpleScopedProfiler();

        SimpleScopedProfiler(const SimpleScopedProfiler&) = delete;
        SimpleScopedProfiler(SimpleScopedProfiler&&) noexcept = delete;
        SimpleScopedProfiler& operator=(const SimpleScopedProfiler&) = delete;
        SimpleScopedProfiler& operator=(SimpleScopedProfiler&&) noexcept = delete;

    private:
        Timer       m_timer;
        std::string m_label;
    };
}
