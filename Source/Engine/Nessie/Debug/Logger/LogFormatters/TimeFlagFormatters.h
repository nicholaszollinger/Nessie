// TimeFlagFormatters.h
#pragma once
#include <array>
#include "FlagFormatter.h"

namespace nes::internal
{
    static inline const char* GetAMPM(const tm& tmTime)
    {
        return tmTime.tm_hour >= 12 ? "PM" : "AM";
    }

    static inline int Get12HourTime(const tm& tmTime)
    {
        return tmTime.tm_hour > 12 ? tmTime.tm_hour - 12 : tmTime.tm_hour;
    }

    static std::array<const char*, 7>   s_shortDays     { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    static std::array<const char*, 7>   s_days          { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
    static std::array<const char*, 12>  s_shortMonths   { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    static std::array<const char*, 12>  s_months        { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds an abbreviated weekday name, e.g. "Sun", "Mon", etc. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class ShortWeekDayFormatter final : public FlagFormatter
    {
    public:
        explicit ShortWeekDayFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}
        
        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            std::string_view value = { s_shortDays[static_cast<size_t>(tmTime.tm_wday)] };
            ScopedPadder padder(value.size(), m_paddingInfo, dest);
            FormatHelpers::AppendStringView(value, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds a full weekday name, e.g. "Sunday", "Monday", etc. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class WeekDayFormatter final : public FlagFormatter
    {
    public:
        explicit WeekDayFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}
        
        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            std::string_view value = { s_days[static_cast<size_t>(tmTime.tm_wday)] };
            ScopedPadder padder(value.size(), m_paddingInfo, dest);
            FormatHelpers::AppendStringView(value, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds an abbreviated month name, e.g. "Jan", "Feb", etc. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class ShortMonthFormatter final : public FlagFormatter
    {
    public:
        explicit ShortMonthFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}
        
        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            std::string_view value = { s_shortMonths[static_cast<size_t>(tmTime.tm_mon)] };
            ScopedPadder padder(value.size(), m_paddingInfo, dest);
            FormatHelpers::AppendStringView(value, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds a full month name, e.g. "January", "February", etc. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class MonthFormatter final : public FlagFormatter
    {
    public:
        explicit MonthFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            std::string_view value = { s_months[static_cast<size_t>(tmTime.tm_mon)] };
            ScopedPadder padder(value.size(), m_paddingInfo, dest);
            FormatHelpers::AppendStringView(value, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds a Date & Time representation (Wed Dec 27 12:32:46 2025) 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class DateTimeFormatter final : public FlagFormatter
    {
    public:
        explicit DateTimeFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 24;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);

            // Date
            FormatHelpers::AppendStringView(s_shortDays[static_cast<size_t>(tmTime.tm_wday)], dest);
            dest.push_back(' ');
            FormatHelpers::AppendStringView(s_shortMonths[static_cast<size_t>(tmTime.tm_mon)], dest);
            dest.push_back(' ');
            FormatHelpers::AppendInt(tmTime.tm_mday, dest);
            dest.push_back(' ');

            // Time
            FormatHelpers::Pad2(tmTime.tm_hour, dest);
            dest.push_back(' ');
            FormatHelpers::Pad2(tmTime.tm_min, dest);
            dest.push_back(' ');
            FormatHelpers::Pad2(tmTime.tm_sec, dest);
            dest.push_back(' ');
            FormatHelpers::AppendInt(tmTime.tm_year + 1900, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds a Date representation (MM/DD/YY). Ex: 12/27/25 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class CalendarDateFormatter final : public FlagFormatter
    {
    public:
        explicit CalendarDateFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 8;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);

            FormatHelpers::Pad2(tmTime.tm_mon + 1, dest);
            dest.push_back('/');
            FormatHelpers::Pad2(tmTime.tm_mday, dest);
            dest.push_back('/');
            FormatHelpers::Pad2(tmTime.tm_year % 100, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds a 4 digit year, e.g. 2025. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class YearFormatter final : public FlagFormatter
    {
    public:
        explicit YearFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 4;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);
            FormatHelpers::AppendInt(tmTime.tm_year + 1900, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds a 2 digit year, e.g. 25. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class Year2DigitFormatter final : public FlagFormatter
    {
    public:
        explicit Year2DigitFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 2;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);
            FormatHelpers::AppendInt(tmTime.tm_year % 100, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the month digit value, 1-12. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class MonthDigitFormatter final : public FlagFormatter
    {
    public:
        explicit MonthDigitFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 2;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);
            FormatHelpers::AppendInt(tmTime.tm_mon + 1, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the day digit value, 1-31. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class DayDigitFormatter final : public FlagFormatter
    {
    public:
        explicit DayDigitFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 2;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);
            FormatHelpers::AppendInt(tmTime.tm_mday, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the hour digit value in 24-format, 0-23. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class Hour24Formatter final : public FlagFormatter
    {
    public:
        explicit Hour24Formatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 2;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);
            FormatHelpers::AppendInt(tmTime.tm_hour, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the hour digit value in 12-format, 1-12. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class Hour12Formatter final : public FlagFormatter
    {
    public:
        explicit Hour12Formatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 2;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);
            FormatHelpers::AppendInt(Get12HourTime(tmTime), dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the minute digit value, 0-59. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class MinuteFormatter final : public FlagFormatter
    {
    public:
        explicit MinuteFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 2;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);
            FormatHelpers::AppendInt(tmTime.tm_min, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the seconds digit value, 0-59. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class SecondFormatter final : public FlagFormatter
    {
    public:
        explicit SecondFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 2;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);
            FormatHelpers::AppendInt(tmTime.tm_sec, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the number of milliseconds. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class MillisecondFormatter final : public FlagFormatter
    {
    public:
        explicit MillisecondFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 3;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);
            
            const auto milliseconds = FormatHelpers::TimeFraction<std::chrono::milliseconds>(msg.m_time);
            FormatHelpers::Pad3(static_cast<uint32_t>(milliseconds.count()), dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the number of microseconds. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class MicrosecondFormatter final : public FlagFormatter
    {
    public:
        explicit MicrosecondFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 6;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);

            const auto microseconds = FormatHelpers::TimeFraction<std::chrono::microseconds>(msg.m_time);
            FormatHelpers::Pad6(static_cast<size_t>(microseconds.count()), dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the number of nanoseconds.
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class NanosecondFormatter final : public FlagFormatter
    {
    public:
        explicit NanosecondFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 9;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);
            
            const auto nanoseconds = FormatHelpers::TimeFraction<std::chrono::nanoseconds>(msg.m_time);
            FormatHelpers::Pad9(static_cast<size_t>(nanoseconds.count()), dest);
        }
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the time since epoch.
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class TimeSinceEpochFormatter final : public FlagFormatter
    {
    public:
        explicit TimeSinceEpochFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const internal::LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 10;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);

            auto duration = msg.m_time.time_since_epoch();
            const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
            FormatHelpers::AppendInt(seconds, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds either "AM/PM" based on current time.
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class AMPMFormatter final : public FlagFormatter
    {
    public:
        explicit AMPMFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 2;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);
            FormatHelpers::AppendStringView(GetAMPM(tmTime), dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds 12-hour clock representation of time: 02:55:02 PM 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class Clock12HourFormatter final : public FlagFormatter
    {
    public:
        explicit Clock12HourFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 11;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);

            FormatHelpers::Pad2(Get12HourTime(tmTime), dest);
            dest.push_back(':');
            FormatHelpers::Pad2(tmTime.tm_min, dest);
            dest.push_back(':');
            FormatHelpers::Pad2(tmTime.tm_sec, dest);
            dest.push_back(' ');
            FormatHelpers::AppendStringView(GetAMPM(tmTime), dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds 24-hour clock representation of time (HH:MM). 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class Clock24HourFormatter final : public FlagFormatter
    {
    public:
        explicit Clock24HourFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 5;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);

            FormatHelpers::Pad2(tmTime.tm_hour, dest);
            dest.push_back(':');
            FormatHelpers::Pad2(tmTime.tm_min, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds ISO 8601 time format of time (HH:MM:SS). 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class ISO8601TimeFormatter final : public FlagFormatter
    {
    public:
        explicit ISO8601TimeFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage&, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            constexpr size_t kFieldSize = 8;
            ScopedPadder padder(kFieldSize, m_paddingInfo, dest);

            FormatHelpers::Pad2(tmTime.tm_hour, dest);
            dest.push_back(':');
            FormatHelpers::Pad2(tmTime.tm_min, dest);
            dest.push_back(':');
            FormatHelpers::Pad2(tmTime.tm_sec, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the elapsed time since the last message.
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder, typename Units>
    class ElapsedTimeFormatter final : public FlagFormatter
    {
    public:
        explicit ElapsedTimeFormatter(const PaddingInfo paddingInfo)
            : FlagFormatter(paddingInfo)
            , m_lastMessageTime(LogClock::now())
        {
            //
        }

        virtual void Format(const LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            const auto delta = std::max(msg.m_time - m_lastMessageTime, LogClock::duration::zero());
            const auto deltaUnits = std::chrono::duration_cast<Units>(delta);
            m_lastMessageTime = msg.m_time;
            
            const auto deltaCount = static_cast<size_t>(deltaUnits.count());
            const auto numDigits = static_cast<size_t>(ScopedPadder::CountDigits(deltaCount));
            ScopedPadder padder(numDigits, m_paddingInfo, dest);
            FormatHelpers::AppendInt(deltaCount, dest);
        }

    private:
        LogTimePoint m_lastMessageTime;
    };
}