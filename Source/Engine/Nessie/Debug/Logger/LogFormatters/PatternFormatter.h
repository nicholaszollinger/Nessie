// PatternFormatter.h
#pragma once
#include <unordered_map>
#include <vector>
#include "CustomFlagFormatter.h"
#include "Nessie/Core/PlatformConstants.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Formats a log message based on a set of Flags. Flags must be preceded with a '%' symbol.
    ///
    /// Example: "[%r] %s(%#) %^[%l]%$: %n%v"
    /// - Possible output: "[01:29:07 PM] Main.cpp(5) [Info] AI: Hello World!"
    ///
    /// Common Flags:
    /// - '%+': Default formatter. Adds full info of a log - Time, Logger Name, Log Level, Source Location, and Message.
    /// - '%n': Log Tag name ("AI", "Application", etc.)
    /// - '%l': The Log Level of the message, (Error, Info, Warning, etc.)
    /// - '%t': ID of the thread that the message was created on. 
    /// - '%v': The actual message text.
    /// - '%^': Begin Color. Only one range of color is allowed.
    /// - '%$': End Color. Only one range of color is allowed.
    /// - '%%': The actual % symbol.
    ///
    /// Time Flags
    /// - '%a': Short Weekday ("Mon", "Tue", etc.)
    /// - '%A': Full Weekday ("Monday", "Tuesday", etc.)
    /// - '%b': Short Month ("Jan", "Feb", etc.)
    /// - '%B': Full Month ("January", "February", etc.)
    /// - '%c': Date and Time (Wed Dec 27 12:32:46 2025)
    /// - '%C': 2 Digit Year (if 2025, -> 25)
    /// - '%Y': 4 Digit Year (2025)
    /// - '%D': Calendar Date (MM/DD/YY)
    /// - '%m': Month numerical value (1-12)
    /// - '%d': Day of the month (1-31)
    /// - '%H': Hour in 24 format
    /// - '%I': Hour in 12 format
    /// - '%M': Minutes
    /// - '%S': Seconds
    /// - '%e': Milliseconds
    /// - '%f': Microseconds
    /// - '%F': Nanoseconds
    /// - '%E': Seconds since epoch.
    /// - '%p': AM/PM
    /// - '%r': 12-hour clock (02:55:02 PM)
    /// - '%R': 24-hour clock (14:55)
    /// - '%T': ISO 8601 time format (HH:MM:SS)
    /// - '%u': Elapsed time since the last log call in nanoseconds.
    /// - '%i': Elapsed time since the last log call in microseconds.
    /// - '%o': Elapsed time since the last log call in milliseconds.
    /// - '%O': Elapsed time since the last log call in seconds.
    ///
    /// Source Location
    /// - '%@': Source Location: C://SourceFolder/Main.cpp(5)
    /// - '%s': Short source filename (Main.cpp)
    /// - '%g': Full source filename (C://SourceFolder/Main.cpp)
    /// - '%#': Source line number
    /// - '%!': Source function name.
    ///
    /// Alignment: Width value up to 64. 
    /// - %<width><flag>: Right Align.                  "%8l"   -> "   info".
    /// - %-<width><flag>: Left Align.                  "%-8l"  -> "info   ".
    /// - %=<width><flag>: Center Align.                "%=8l"  -> "  info  ".
    /// - %<width>!<flag>: Right Align or Truncate:     "%3!l"  -> "inf"
    /// - %-<width>!<flag>: Left Align or Truncate:     "%2!l"  -> "in"
    /// - %=<width>!<flag>: Center Align or Truncate:   "%1!l"  -> "i"
    //----------------------------------------------------------------------------------------------------
    class PatternFormatter final : public LogFormatter
    {
    public:
        using CustomFlags = std::unordered_map<char, std::unique_ptr<CustomFlagFormatter>>;

    public:
        explicit PatternFormatter(std::string pattern = "%+", std::string eol = nes::platformConstants::kEOL, CustomFlags customFlags = {});
        
        PatternFormatter(const PatternFormatter&) = delete;
        PatternFormatter(PatternFormatter&&) noexcept = delete;
        PatternFormatter& operator=(const PatternFormatter&) = delete;
        PatternFormatter& operator=(PatternFormatter&&) noexcept = delete;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Make a clone of this Formatter. 
        //----------------------------------------------------------------------------------------------------
        virtual std::unique_ptr<LogFormatter> Clone() const override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Format a log message, storing the result in the given memory buffer. 
        //----------------------------------------------------------------------------------------------------
        virtual void                        Format(const internal::LogMessage& msg, LogMemoryBuffer& dest) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set if this formatter needs to update its internal cached time point.
        //----------------------------------------------------------------------------------------------------
        void                                SetNeedUpdateCachedTime(const bool shouldUpdate = true) { m_needUpdateCachedTime = shouldUpdate; }
        
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the time based on the log message.
        //----------------------------------------------------------------------------------------------------
        std::tm                             GetTime(const internal::LogMessage& msg);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Handles updating the formatted message based on the passed in flag.
        //----------------------------------------------------------------------------------------------------
        template <typename Padder>
        void                                HandleFlag(const char flag, internal::PaddingInfo& padding);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the formatter objects for the given pattern.
        //----------------------------------------------------------------------------------------------------
        void                                CompilePattern(const std::string& pattern);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Handle the padding specifier.
        //----------------------------------------------------------------------------------------------------
        static internal::PaddingInfo        HandlePadSpec(std::string::const_iterator& it, std::string::const_iterator end);

    private:
        std::vector<std::unique_ptr<internal::FlagFormatter>> m_flagFormatters;
        CustomFlags                         m_customFlags;
        std::string                         m_pattern;
        std::string                         m_eol;
        std::tm                             m_cachedTmTime;
        std::chrono::seconds                m_lastLogSeconds;
        bool                                m_needUpdateCachedTime;
    };
}

#include "PatternFormatter.inl"
