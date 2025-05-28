// PatternFormatter.inl
#pragma once
#include "CommonFlagFormatters.h"
#include "SourceFlagFormatters.h"
#include "TimeFlagFormatters.h"
#include "Debug/Logger/Details/LogFormatHelpers.h"

namespace nes
{
    template <typename Padder>
    inline void PatternFormatter::HandleFlag(const char flag, internal::PaddingInfo& padding)
    {
        // Check for Custom Flags:
        auto it = m_customFlags.find(flag);
        if (it != m_customFlags.end())
        {
            auto pCustomHandler = it->second->Clone();
            pCustomHandler->SetPaddingInfo(padding);
            m_flagFormatters.push_back(std::move(pCustomHandler));
            return;
        }

        // Process built-in Flags
        switch (flag)
        {
            case '+': // Default Formatter, full info.
            {
                m_flagFormatters.push_back(std::make_unique<internal::FullInfoFormatter>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'n': // Logger Name
            {
                m_flagFormatters.push_back(std::make_unique<internal::LoggerNameFormatter<Padder>>(padding));
                break;
            }

            case 'l': // Log Level
            {
                m_flagFormatters.push_back(std::make_unique<internal::LogLevelFormatter<Padder>>(padding));
                break;
            }

            case 'N': // Log Level & Logger Name: "[Warning] LoggerName"
            {
                m_flagFormatters.push_back(std::make_unique<internal::LoggerNameAndLevelFormatter<Padder>>(padding));
                break;
            }

            case 't': // Thread ID
            {
                // [TODO]:
                break;
            }

            case 'v': // Message Text
            {
                m_flagFormatters.push_back(std::make_unique<internal::MessageFormatter<Padder>>(padding));
                break;
            }

            case 'a': // Short Weekday
            {
                m_flagFormatters.push_back(std::make_unique<internal::ShortWeekDayFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'A': // Full Weekday
            {
                m_flagFormatters.push_back(std::make_unique<internal::WeekDayFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'b': // Short Month
            {
                m_flagFormatters.push_back(std::make_unique<internal::ShortMonthFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
            }

            case 'B': // Full Month
            {
                m_flagFormatters.push_back(std::make_unique<internal::MonthFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'c': // Date Time
            {
                m_flagFormatters.push_back(std::make_unique<internal::DateTimeFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'C': // Year 2 digits
            {
                m_flagFormatters.push_back(std::make_unique<internal::Year2DigitFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'Y': // Year 4 digits
            {
                m_flagFormatters.push_back(std::make_unique<internal::YearFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'D': // Date time MM/DD/YY
            {
                m_flagFormatters.push_back(std::make_unique<internal::CalendarDateFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'm': // Day of the month (1-12)
            {
                m_flagFormatters.push_back(std::make_unique<internal::MonthDigitFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'd': // Day of the month (1-31)
            {
                m_flagFormatters.push_back(std::make_unique<internal::DayDigitFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'H': // Hours 24
            {
                m_flagFormatters.push_back(std::make_unique<internal::Hour24Formatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'I': // Hours 12
            {
                m_flagFormatters.push_back(std::make_unique<internal::Hour12Formatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'M': // Minutes
            {
                m_flagFormatters.push_back(std::make_unique<internal::MinuteFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'S': // Seconds
            {
                m_flagFormatters.push_back(std::make_unique<internal::SecondFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'e': // Milliseconds
            {
                m_flagFormatters.push_back(std::make_unique<internal::MillisecondFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'f': // Microseconds
            {
                m_flagFormatters.push_back(std::make_unique<internal::MicrosecondFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'F': // Nanoseconds
            {
                m_flagFormatters.push_back(std::make_unique<internal::NanosecondFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'E': // Seconds since Epoch
            {
                m_flagFormatters.push_back(std::make_unique<internal::TimeSinceEpochFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'p': // AM/PM
            {
                m_flagFormatters.push_back(std::make_unique<internal::AMPMFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'r': // 12-hour clock 02:55:02 PM
            {
                m_flagFormatters.push_back(std::make_unique<internal::Clock12HourFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'R': // 24-hour clock 14:55
            {
                m_flagFormatters.push_back(std::make_unique<internal::Clock24HourFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'T': // ISO 8601 time format (HH:MM:SS)
            {
                m_flagFormatters.push_back(std::make_unique<internal::ISO8601TimeFormatter<Padder>>(padding));
                m_needUpdateCachedTime = true;
                break;
            }

            case 'P': // [TODO]: Process ID
            {
                break;
            }

            case '^': // Color Begin
            {
                m_flagFormatters.push_back(std::make_unique<internal::ColorBeginFormatter>(padding));
                break;
            }

            case '$': // Color End
            {
                m_flagFormatters.push_back(std::make_unique<internal::ColorEndFormatter>(padding));
                break;
            }

            case '@': // Source location: (fullFilename(lineNumber))
            {
                m_flagFormatters.push_back(std::make_unique<internal::SourceLocationFormatter<Padder>>(padding));
                break;
            }

            case 's': // Short source filename - without the directory.
            {
                m_flagFormatters.push_back(std::make_unique<internal::ShortFilenameFormatter<Padder>>(padding));
                break;
            }

            case 'g': // Full filename - with the directory.
            {
                m_flagFormatters.push_back(std::make_unique<internal::SourceFilenameFormatter<Padder>>(padding));
                break;
            }

            case '#': // Source line number
            {
                m_flagFormatters.push_back(std::make_unique<internal::SourceLineNumberFormatter<Padder>>(padding));
                break;
            }

            case '!': // Source Function name
            {
                m_flagFormatters.push_back(std::make_unique<internal::SourceFunctionNameFormatter<Padder>>(padding));
                break;
            }

            case '%': // % char
            {
                m_flagFormatters.push_back(std::make_unique<internal::CharFormatter>('%'));
                break;
            }

            case 'u': // Elapsed time since the last log call in nanoseconds.
            {
                m_flagFormatters.push_back(std::make_unique<internal::ElapsedTimeFormatter<Padder, std::chrono::nanoseconds>>(padding));
                break;
            }

            case 'i': // Elapsed time since the last log call in microseconds.
            {
                m_flagFormatters.push_back(std::make_unique<internal::ElapsedTimeFormatter<Padder, std::chrono::microseconds>>(padding));
                break;
            }

            case 'o': // Elapsed time since the last log call in milliseconds.
            {
                m_flagFormatters.push_back(std::make_unique<internal::ElapsedTimeFormatter<Padder, std::chrono::milliseconds>>(padding));
                break;
            }

            case 'O': // Elapsed time since the last log call in seconds.
            {
                m_flagFormatters.push_back(std::make_unique<internal::ElapsedTimeFormatter<Padder, std::chrono::seconds>>(padding));
                break;
            }

            default: // Unknown-flag appears as in the string
            {
                auto unknownFlag = std::make_unique<internal::AggregateFormatter>();

                if (!padding.m_truncate)
                {
                    unknownFlag->AddChar('%');
                    unknownFlag->AddChar(flag);
                    m_flagFormatters.push_back(std::move(unknownFlag));
                }

                // Fixes an issue where the previous character was '!' and should have been treated as a function name flag
                // instead of truncating the flag.
                //      Logger->SetPattern("[%10!] %v") => "[        main] some message"
                //      Logger->SetPattern("[%3!!] %v") => "[mai] some message" 
                else
                {
                    padding.m_truncate = false;
                    m_flagFormatters.push_back(std::make_unique<internal::SourceFunctionNameFormatter<Padder>>(padding));
                    unknownFlag->AddChar(flag);
                    m_flagFormatters.push_back(std::move(unknownFlag));
                }

                break;
            }
        }
    } 
}