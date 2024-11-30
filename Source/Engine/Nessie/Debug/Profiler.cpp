// Profiler.cpp
#include "Profiler.h"

namespace nes
{
    SimpleInstrumentationProfiler::SimpleInstrumentationProfiler(const std::string& label)
        : m_label(label)
    {
        m_timer.Start();
    }

    SimpleInstrumentationProfiler::~SimpleInstrumentationProfiler()
    {
        [[maybe_unused]] const double result = m_timer.GetElapsedTime();
        NES_LOGV("Profiler", "[", m_label, "] Result: ", result, "ms.");
    }
}