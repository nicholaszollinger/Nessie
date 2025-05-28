// ErrorHandling.h
#pragma once
#include "Log.h"
#include "Platform/Platform.h"

#ifndef NES_RELEASE
    #define NES_ASSERTS_ENABLED
#endif

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
    inline std::string AssertFailedHelper(const LogSource& source, [[maybe_unused]] const char* expression, FormatString<Args...> pFormat, Args&&...args)
    {
        // Format just the incoming format string and arguments
        std::string formattedMessage;
        fmt::vformat_to(formattedMessage, pFormat, fmt::make_format_args(args...));

        // Pass off to the AssertFailed helper function to add the source location information.
        return AssertFailedHelper(source, formattedMessage.c_str()); 
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Formats the Fatal Error message and posts the log if NES_LOGGING_ENABLED is defined. 
    //----------------------------------------------------------------------------------------------------
    template <typename ...Args>
    inline std::string FatalErrorHelper(const LogSource& source, FormatString<Args...> pFormat, Args&&...args)
    {
        // Format just the incoming format string and arguments
        std::string formattedMessage;
        fmt::vformat_to(formattedMessage, pFormat, fmt::make_format_args(args...));

        // Pass off to the AssertFailed helper function to add the source location information.
        return AssertFailedHelper(source, formattedMessage.c_str()); 
    }
}

#ifdef NES_ASSERTS_ENABLED
//----------------------------------------------------------------------------------------------------
/// @brief : Checks that the expression is true. If not, this will log an assert failed error and
///     break into the debugger if attached.
/// @note : This accepts a format string same as the logging functions. For more info on formatting logs,
///     see NES_LOG().
//----------------------------------------------------------------------------------------------------
#define NES_ASSERT(expression, ...)                                                                                                     \
do                                                                                                                                      \
{                                                                                                                                       \
    if (!(expression))                                                                                                                  \
    {                                                                                                                                   \
        auto message = nes::internal::AssertFailedHelper(nes::internal::LogSource(__FILE__, __LINE__, __FUNCTION__), #expression, ##__VA_ARGS__); \
        nes::Platform::HandleFatalError("Assertion Failed!", message);                                       \
        NES_BREAKPOINT;                                                                                                               \
    }                                                                                                                                   \
} while (false)

#else
#define NES_ASSERT(expression, ...) void(0)
#endif

//----------------------------------------------------------------------------------------------------
/// @brief : Post a fatal error message. The program will exit as a result of this call.
//----------------------------------------------------------------------------------------------------
#define NES_FATAL(...) \
do                                                                                                                                          \
{                                                                                                                                           \
    auto message = nes::internal::FatalErrorHelper(nes::internal::LogSource(__FILE__, __LINE__, __FUNCTION__), __VA_ARGS__); \
    nes::Platform::HandleFatalError("Fatal Error!", message);                                       \                                                                                        \
    NES_BREAKPOINT;                                                                                                               \
} while (false)