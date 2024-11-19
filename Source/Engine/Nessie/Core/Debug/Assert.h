#pragma once
// Assert.h

#include "Log.h"
#if NES_LOGGING_ENABLED

//----------------------------------------------------------------------------------------------------
///		@brief : Assert that a condition is true. If not, log an error message and break.
//----------------------------------------------------------------------------------------------------
#define NES_ASSERT(condition)                                                                                                               \
do                                                                                                                                   \
{                                                                                                                                    \
    if (!(condition))                                                                                                                \
    {                                                                                                                                \
        NES_ERRORV("Assertion Failed! ", (#condition), "\n\t", GET_CURRENT_FILENAME, " - ", __FUNCTION__, "() - Line: ", __LINE__);       \
        __debugbreak();                                                                                                              \
    }                                                                                                                                \
} while(0)

//----------------------------------------------------------------------------------------------------
///		@brief : Assert that a condition is true, with an additional message. If not true,
///             log an error message and break.
//----------------------------------------------------------------------------------------------------
#define NES_ASSERTV(condition, ...)                                                                                                             \
do                                                                                                                                          \
{                                                                                                                                           \
    if (!(condition))                                                                                                                       \
    {                                                                                                                                       \
        NES_ERRORV("Assertion Failed! ", (#condition), __VA_ARGS__, "\n\t", GET_CURRENT_FILENAME, " - ", __FUNCTION__, "() - Line: ", __LINE__); \
        __debugbreak();                                                                                                                     \
    }                                                                                                                                       \
} while(0)

#else
#define NES_ASSERT(condition) void(0)
#define NES_ASSERTV(condition, ...) void(0)

#endif