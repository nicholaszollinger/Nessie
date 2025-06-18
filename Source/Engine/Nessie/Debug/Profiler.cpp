// Profiler.cpp
#include "Profiler.h"

namespace nes
{
    SimpleScopedProfiler::SimpleScopedProfiler(const std::string& label)
        : m_label(label)
    {
        m_timer.Start();
    }

    SimpleScopedProfiler::~SimpleScopedProfiler()
    {
        [[maybe_unused]] const double result = m_timer.ElapsedTime();
        NES_LOG("[Profiler] [", m_label, "] Result: ", result, "ms.");
    }
}