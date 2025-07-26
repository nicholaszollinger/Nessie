// RenderDevice.cpp
#include "RenderDevice.h"

namespace nes
{
    void RenderDevice::ReportMessage(const ELogLevel level, const char* file, const uint32 line, const char* message, const LogTag& tag) const
    {
        if (m_debugMessenger.m_callback)
        {
            m_debugMessenger.SendMessage(level, file, line, message, tag);
        }
    }
}
