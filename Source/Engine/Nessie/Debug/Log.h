// Log.h
#pragma once
#include "Core/Config.h"
#include "Logger/LoggerRegistry.h"

#undef NES_LOGGING_ENABLED
#ifndef NES_RELEASE
    #define NES_LOGGING_ENABLED 1
#else
    #define NES_LOGGING_ENABLED 0
#endif

#if NES_LOGGING_ENABLED
    #define NES_IF_LOGGING_ENABLED(...) __VA_ARGS__
    #define NES_IF_LOGGING_DISABLED(...)
#else
    #define NES_IF_LOGGING_ENABLED(...)
    #define NES_IF_LOGGING_DISABLED(...) __VA_ARGS__
#endif

namespace nes::internal
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper function to parse log parameters. This uses the default logger.
    //----------------------------------------------------------------------------------------------------
    template <typename...Args>
    void LogParamHelper(const LogSource& source, const ELogLevel level, FormatString<Args...> pFormat, Args&&... args)
    {
        nes::LoggerRegistry::Instance().GetDefaultLogger()->Log(source, level, pFormat, std::forward<Args>(args)...);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper function to parse log parameters. This overload uses the passed in logger to post
    ///     the message.
    //----------------------------------------------------------------------------------------------------
    template <typename...Args>
    void LogParamHelper(const LogSource& source, const ELogLevel level, const std::shared_ptr<Logger>& pLogger, FormatString<Args...> pFormat, Args&&... args)
    {
        pLogger->Log(source, level, pFormat, std::forward<Args>(args)...);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper function to parse log parameters. This overload contains a log tag parameter.
    //----------------------------------------------------------------------------------------------------
    template <typename...Args>
    void LogParamHelper(const LogSource& source, const ELogLevel level, const LogTag& tag, FormatString<Args...> pFormat, Args&&... args)
    {
        nes::LoggerRegistry::Instance().GetDefaultLogger()->Log(source, level, tag, pFormat, std::forward<Args>(args)...);
    }

    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper function to parse log parameters. This overload contains a log tag parameter as a
    ///     string.
    //----------------------------------------------------------------------------------------------------
    template <typename...Args>
    void LogParamHelper(const LogSource& source, const ELogLevel level, const std::string& tagName, FormatString<Args...> pFormat, Args&&... args)
    {
        const LogTag tag(tagName, level);
        nes::LoggerRegistry::Instance().GetDefaultLogger()->Log(source, level, tag, pFormat, std::forward<Args>(args)...);
    }
}

#ifdef NES_LOGGING_ENABLED

//----------------------------------------------------------------------------------------------------
// Formatted Examples - For more information, see fmt documentation: https://fmt.dev/11.1/syntax/
//     Positional Arguments:
//         NES_LOG("{0}, {1}, {2}", 'a', 'b', 'c');
//         - Result: "a, b, c"
//
//         NES_LOG("{}, {}, {}", 'a', 'b', 'c');
//         - Result: "a, b, c"
//
//         NES_LOG("{2}, {1}, {0}", 'a', 'b', 'c');
//         - Result: "c, b, a"
//
//         NES_LOG("{0}{1}{0}", "abra", "cad");   arguments' indices can be repeated
//         - Result: "abracadabra"
//
//     Precision:
//         NES_LOG("{:.2f}", 3.14159)
//         - Result: "3.14"
//
//         NES_LOG("{:.{}f}", 3.14, 1)  You can use another argument for the precision
//         - Result: "3.1"
//
//     Sign:
//         NES_LOG("{:+f}; {:+f}", 3.14, -3.14); -- show it always
//         - Result: "+3.140000; -3.140000"
//
//         NES_LOG("{: f}; {: f}", 3.14, -3.14); -- show a space for positive numbers
//         - Result: " 3.140000; -3.140000"
//
//         NES_LOG("{:-f}; {:-f}", 3.14, -3.14); -- show only the minus -- same as '{:f}; {:f}'
//         - Result: "3.140000; -3.140000"
//
/// @brief : Log an Info-level message. You can pass a format string followed by a number of arguments.
///     Nessie uses the fmt library for formatting logs and strings.
/// @note : If you want to use a specific Logger, pass it as the first argument, before the format string.
/// @note : For more information, see fmt documentation: https://fmt.dev/11.1/syntax/ 
//----------------------------------------------------------------------------------------------------
#define NES_LOG(...)        nes::internal::LogParamHelper(nes::internal::LogSource(__FILE__, __LINE__, NES_FUNCTION_NAME), nes::ELogLevel::Info, __VA_ARGS__)

//----------------------------------------------------------------------------------------------------
/// @brief : Log a Trace-level Message. You can pass a format string followed by a number of arguments.
///     For how to format a log message, see: "NES_LOG()". 
/// @note : If you want to use a specific Logger, pass it as the first argument, before the format string.
//----------------------------------------------------------------------------------------------------
#define NES_TRACE(...)      nes::internal::LogParamHelper(nes::internal::LogSource(__FILE__, __LINE__, NES_FUNCTION_NAME), nes::ELogLevel::Trace, __VA_ARGS__)

//----------------------------------------------------------------------------------------------------
/// @brief : Log a Debug-level message. You can pass a format string followed by a number of arguments.
///     For how to format a log message, see: "NES_LOG()". 
/// @note : If you want to use a specific Logger, pass it as the first argument, before the format string.
//----------------------------------------------------------------------------------------------------
#define NES_DLOG(...)       nes::internal::LogParamHelper(nes::internal::LogSource(__FILE__, __LINE__, NES_FUNCTION_NAME), nes::ELogLevel::Debug, __VA_ARGS__)

//----------------------------------------------------------------------------------------------------
/// @brief : Log a Warning-level message. You can pass a format string followed by a number of arguments.
///     For how to format a log message, see: "NES_LOG()". 
/// @note : If you want to use a specific Logger, pass it as the first argument, before the format string.
//----------------------------------------------------------------------------------------------------
#define NES_WARN(...)       nes::internal::LogParamHelper(nes::internal::LogSource(__FILE__, __LINE__, NES_FUNCTION_NAME), nes::ELogLevel::Warn, __VA_ARGS__)

//----------------------------------------------------------------------------------------------------
/// @brief : Log an Error-level message. You can pass a format string followed by a number of arguments.
///     For how to format a log message, see: "NES_LOG()". 
/// @note : If you want to use a specific Logger, pass it as the first argument, before the format string.
//----------------------------------------------------------------------------------------------------
#define NES_ERROR(...)      nes::internal::LogParamHelper(nes::internal::LogSource(__FILE__, __LINE__, NES_FUNCTION_NAME), nes::ELogLevel::Error, __VA_ARGS__)

#else
    #define NES_TRACE(...)  void(0)
    #define NES_DLOG(...) void(0)
    #define NES_LOG(...)    void(0)
    #define NES_WARN(...)   void(0)
    #define NES_ERROR(...)  void(0)
#endif
