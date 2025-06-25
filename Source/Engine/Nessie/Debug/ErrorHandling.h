// ErrorHandling.h
#pragma once
#include "Log.h"
#include "Platform/Platform.h"

namespace nes::internal
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Formats the Assertion failed message and posts the log if NES_LOGGING_ENABLED is defined. 
    //----------------------------------------------------------------------------------------------------
    inline std::string AssertFailedHelper(const LogSource& source, const char* expression, const char* message = nullptr)
    {
        // Format the message, optionally Log if enabled, return the formatted result.
        const char* pLogMessage = message == nullptr ? expression : message;
        std::string formatted = fmt::format("{0}({2}) '{1}': {3}", source.m_fileName, source.m_functionName, source.m_line, pLogMessage);

#if NES_LOGGING_ENABLED
        LoggerRegistry::Instance().GetDefaultLogger()->Log(source, ELogLevel::Fatal, pLogMessage);
#endif

        return formatted;
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Formats the Assertion failed message and posts the log if NES_LOGGING_ENABLED is defined. 
    //----------------------------------------------------------------------------------------------------
    template <typename...Args>
    inline std::string AssertFailedHelper(const LogSource& source, [[maybe_unused]] const char* expression, TFormatString<Args...> pFormat, Args&&...args)
    {
        // Format just the incoming format string and arguments
        LogMemoryBuffer buffer;
        fmt::vformat_to(fmt::appender(buffer), pFormat, fmt::make_format_args(args...));

        // Pass off to the AssertFailed helper function to add the source location information.
        return AssertFailedHelper(source, fmt::to_string(buffer).c_str()); 
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Formats the Fatal Error message and posts the log if NES_LOGGING_ENABLED is defined. 
    //----------------------------------------------------------------------------------------------------
    template <typename ...Args>
    inline std::string FatalErrorHelper(const LogSource& source, TFormatString<Args...> pFormat, Args&&...args)
    {
        // Format just the incoming format string and arguments
        LogMemoryBuffer buffer;
        fmt::vformat_to(fmt::appender(buffer), pFormat, fmt::make_format_args(args...));

        // Pass off to the AssertFailed helper function to add the source location information.
        return AssertFailedHelper(source, fmt::to_string(buffer).c_str()); 
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Formats the Fatal Error message and posts the log if NES_LOGGING_ENABLED is defined. 
    //----------------------------------------------------------------------------------------------------
    template <typename ...Args>
    inline std::string FatalErrorHelper(const LogSource& source, [[maybe_unused]] const LogTag& tag, TFormatString<Args...> pFormat, Args&&...args)
    {
        // Format just the incoming format string and arguments
        LogMemoryBuffer buffer;
        fmt::vformat_to(fmt::appender(buffer), pFormat, fmt::make_format_args(args...));
        std::string formattedMessage = fmt::to_string(buffer);
        
        // Format the fatal error message.
        std::string formattedFinal = fmt::format("{0}({2}) '{1}': {3}", source.m_fileName, source.m_functionName, source.m_line, formattedMessage);
        
#if NES_LOGGING_ENABLED
        LoggerRegistry::Instance().GetDefaultLogger()->Log(source, ELogLevel::Fatal, tag, formattedMessage);
#endif
        
        return formattedFinal;
    }
}