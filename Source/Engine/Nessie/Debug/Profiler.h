#pragma once
// Profiler.h
#include "Core/Log/Log.h"
#include "Core/Time/Timer.h"

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
    class SimpleInstrumentationProfiler
    {
    public:
        explicit SimpleInstrumentationProfiler(const std::string& label);
        ~SimpleInstrumentationProfiler();

        SimpleInstrumentationProfiler(const SimpleInstrumentationProfiler&) = delete;
        SimpleInstrumentationProfiler(SimpleInstrumentationProfiler&&) noexcept = delete;
        SimpleInstrumentationProfiler& operator=(const SimpleInstrumentationProfiler&) = delete;
        SimpleInstrumentationProfiler& operator=(SimpleInstrumentationProfiler&&) noexcept = delete;

    private:
        Timer       m_timer;
        std::string m_label;
    };
}
