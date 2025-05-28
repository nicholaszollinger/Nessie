// LogMessage.inl
#pragma once

namespace nes::internal
{
    inline LogMessage::LogMessage(LogTimePoint logTime, const LogSource& source, const std::string_view tagName, const ELogLevel level, const std::string_view msg)
            : m_tagName(tagName)
            , m_payload(msg)
            , m_time(logTime)
            , m_source(source)
            , m_threadID(std::this_thread::get_id())
            , m_level(level)
    {
        //
    }      

    inline LogMessage::LogMessage(const LogSource& source, const std::string_view tagName, const ELogLevel level, const std::string_view msg)
        : LogMessage(LogClock::now(), source, tagName, level, msg)
    {
        //
    }

    inline LogMessage::LogMessage(const std::string_view tagName, const ELogLevel level, const std::string_view msg)
        : LogMessage(LogClock::now(), LogSource(), tagName, level, msg)
    {
        //
    }
}
