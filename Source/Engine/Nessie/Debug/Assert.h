#pragma once
// Assert.h
#include "ErrorHandling.h"

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
do                                                                                                                              \
{                                                                                                                               \
    auto message = nes::internal::FatalErrorHelper(nes::internal::LogSource(__FILE__, __LINE__, __FUNCTION__), __VA_ARGS__);    \
    nes::Platform::HandleFatalError("Fatal Error!", message);                                                                   \
    NES_BREAKPOINT;                                                                                                             \
} while (false)