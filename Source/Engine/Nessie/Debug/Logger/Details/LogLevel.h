// LogLevel.h
#pragma once
#include <cstdint>
#include <string>

namespace nes
{
    enum class ELogLevel : uint8_t
    {
        Trace = 0,  /// Trace messages
        Debug,      /// Debug messages
        Info,       /// Info messages - Default level.
        Warn,       /// Warning messages
        Error,      /// Error messages
        Fatal,      /// Fatal Error messages, the program should/has crashed.
        Off,        /// No logs will be output at this level.
        NumLevels,
    };

    namespace internal
    {
        static const std::string_view s_logLevelNames[]
        {
            std::string_view("Trace", 5),
            std::string_view("Debug", 5),
            std::string_view("Info", 4),
            std::string_view("Warning", 7),
            std::string_view("Error", 5),
            std::string_view("Fatal", 5),
            std::string_view("Off", 3),
        };

        static constexpr size_t kLogLevelCount = std::size(s_logLevelNames);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a string representation of a ELogLevel.
        //----------------------------------------------------------------------------------------------------
        inline const std::string_view& GetLogLevelName(const ELogLevel level)
        {
            const size_t levelIndex = std::min(static_cast<size_t>(level), std::size(s_logLevelNames));
            return s_logLevelNames[levelIndex];
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Convert a string to a Log Level. If not found, this returns ELogLevel::Off. 
        //----------------------------------------------------------------------------------------------------
        inline ELogLevel LogLevelFromString(const std::string& level)
        {
            auto it = std::find(std::begin(s_logLevelNames), std::end(s_logLevelNames), level);
            if (it != std::end(s_logLevelNames))
            {
                return static_cast<ELogLevel>(it - std::begin(s_logLevelNames));
            }

            return ELogLevel::Off;
        }
    }
}
