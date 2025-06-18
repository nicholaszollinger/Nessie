// LogMessage.h
#pragma once
#include <thread>
#include "LogCommon.h"
#include "LogLevel.h"
#include "LogSource.h"

namespace nes::internal
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Contains a message and information about the log call, including the source, level, etc.
    //----------------------------------------------------------------------------------------------------
    struct LogMessage
    {
        LogMessage() = default;
        LogMessage(LogTimePoint logTime, const LogSource& source, std::string_view tagName, const ELogLevel level, std::string_view msg);
        LogMessage(const LogSource& source, std::string_view tagName, const ELogLevel level, std::string_view msg);
        LogMessage(std::string_view tagName, const ELogLevel level, std::string_view msg);
        
        std::string_view    m_tagName;                  /// Name of the LogTag associated with this message. Can be empty/null.
        std::string_view    m_payload;                  /// Formatted log message.
        LogTimePoint        m_time;                     /// Time of the log message.
        LogSource           m_source;                   /// Location of the Log call.
        std::thread::id     m_threadID{};               /// Thread ID of the thread that output this message.
        mutable size_t      m_colorRangeStart = 0;      /// Used by a pattern formatter, this marks the first character that needs to be colored.
        mutable size_t      m_colorRangeEnd   = 0;      /// Used by a pattern formatter, this marks the last character that needs to be colored.
        ELogLevel           m_level = ELogLevel::Trace; /// Log level of this message.
    };
}

#include "LogMessage.inl"