// LogGroup.h
#pragma once
#include "LogLevel.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Optional tag that can be used to be used to identify a grouping of log calls and to filter out
    ///     the calls using the tag's log level.
    ///
    ///     For example, say you create a log group with NES_DEFINE_LOG_TAG(kAILogTag, "AI", Error);
    ///     Then, somewhere in code you have two different log calls:
    ///         - NES_LOG(kAILogTag, "AI changed state");
    ///         - NES_ERROR(kAILogTag, "Failed to change state! Missing state parameter!");
    ///     Since the LogTag has been set with ELogLevel::Error, only the second log message will be
    ///     displayed. Lowering the log level to ELogLevel::Info will display both messages.
    //----------------------------------------------------------------------------------------------------
    struct LogTag
    {
        constexpr LogTag(const std::string_view name, const ELogLevel level)
            : m_name(name)
            , m_level(level)
        {
            //
        }
        
        std::string_view    m_name;
        ELogLevel           m_level;
    };
}

//----------------------------------------------------------------------------------------------------
/// @brief : Creates a Log Tag, to be used to identify a grouping of log calls and to filter out
///     the calls using the tag's log level.
///
///     Example Usage:
///     ... In a global scope:
///     NES_DEFINE_LOG_TAG(kAILogTag, "AI", Warn);
///
///     ... somewhere in code:
///     NES_WARN(kAILogTag, "Returning to Null State as a fallback for missing state.");
///
///     ... Example output:
///     -> "[Warning] AI: Returning to Null State as a fallback for missing state."
///
///	@param varName : Name of the variable for the tag. This variable should be passed to a Log call. 
///	@param name : Name that will show up in the log message, e.g. "AI".
///	@param level : Log level for this group.
//----------------------------------------------------------------------------------------------------
#define NES_DEFINE_LOG_TAG(varName, name, level) inline nes::LogTag varName{ name,  nes::ELogLevel::level }