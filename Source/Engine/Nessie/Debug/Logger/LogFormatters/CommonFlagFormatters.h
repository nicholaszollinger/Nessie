// CommonFlagFormatters.h
#pragma once
#include "FlagFormatter.h"

namespace nes::internal
{
    // [TODO]: Thread ID formatter
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Formatter to add the logger name to the log message. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class LogTagFormatter final : public FlagFormatter
    {
    public:
        explicit LogTagFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            if (msg.m_tagName.empty())
                return;

            const size_t textSize = msg.m_tagName.size() + 2; // +2 for ': '. 
            ScopedPadder padder(textSize, m_paddingInfo, dest);
            FormatHelpers::AppendStringView(msg.m_tagName, dest);
            dest.push_back(':');
            dest.push_back(' ');
        }
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Formatter to add the level name to the log message. E.g., "Trace", "Info", "Error", etc.
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class LogLevelFormatter final : public FlagFormatter
    {
    public:
        explicit LogLevelFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}
        
        virtual void Format(const LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            const std::string_view levelName = GetLogLevelName(msg.m_level);
            ScopedPadder padder(levelName.size(), m_paddingInfo, dest);
            FormatHelpers::AppendStringView(levelName, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Formatter to add the level name followed by the logger name to the log message. This will
    ///     Example: "[Warning] AI" or "[Warning]
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class LoggerNameAndLevelFormatter final : public FlagFormatter
    {
    public:
        explicit LoggerNameAndLevelFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            const std::string_view levelName = GetLogLevelName(msg.m_level);
            size_t textSize = levelName.size() + 2; // +2 for the brackets.
            
            const bool hasName = !msg.m_tagName.empty();
            if (hasName)
            {
                textSize += 2 + msg.m_tagName.size(); // +2 for ': '
            }

            ScopedPadder padder(textSize, m_paddingInfo, dest);

            // Add the Level Name
            dest.push_back('[');
            FormatHelpers::AppendStringView(levelName, dest);
            dest.push_back(']');

            // Add Name, if present
            if (hasName)
            {
                dest.push_back(':');
                dest.push_back(' ');
                FormatHelpers::AppendStringView(msg.m_tagName, dest);
            }
        }
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds the LogMessage message value to the formatted message. 
    //----------------------------------------------------------------------------------------------------
    template <typename ScopedPadder>
    class MessageFormatter final : public FlagFormatter
    {
    public:
        explicit MessageFormatter(const internal::PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const internal::LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            ScopedPadder padder(msg.m_payload.size(), m_paddingInfo, dest);
            FormatHelpers::AppendStringView(msg.m_payload, dest);
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds a character to the formatter string.
    //----------------------------------------------------------------------------------------------------
    class CharFormatter final : public FlagFormatter
    {
    public:
        explicit CharFormatter(const char c) : m_char(c) {}

        virtual void Format(const LogMessage&, const std::tm&, LogMemoryBuffer& dest) override
        {
            dest.push_back(m_char);
        }

    private:
        char m_char;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Aggregates user characters to display as is. 
    //----------------------------------------------------------------------------------------------------
    class AggregateFormatter final : public FlagFormatter
    {
    public:
        AggregateFormatter() = default;

        void AddChar(const char c) { m_str += c; }
        
        virtual void Format(const LogMessage&, const std::tm&, LogMemoryBuffer& dest) override
        {
            FormatHelpers::AppendStringView(m_str, dest);            
        }
        
    private:

        std::string m_str;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Marks the current position in the formatted message to begin color. 
    //----------------------------------------------------------------------------------------------------
    class ColorBeginFormatter final : public FlagFormatter
    {
    public:
        explicit ColorBeginFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            msg.m_colorRangeStart = dest.size();
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Marks the current position in the formatted message to end color. 
    //----------------------------------------------------------------------------------------------------
    class ColorEndFormatter final : public FlagFormatter
    {
    public:
        explicit ColorEndFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage& msg, const std::tm&, LogMemoryBuffer& dest) override
        {
            msg.m_colorRangeEnd = dest.size();
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Adds full info of a log - Time, Logger Name, Log Level, Source Location, and Message.
    ///     Pattern: [%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%s(#)] %v
    //----------------------------------------------------------------------------------------------------
    class FullInfoFormatter final : public FlagFormatter
    {
    public:
        explicit FullInfoFormatter(const PaddingInfo paddingInfo) : FlagFormatter(paddingInfo) {}

        virtual void Format(const LogMessage& msg, const std::tm& tmTime, LogMemoryBuffer& dest) override
        {
            using std::chrono::duration_cast;
            using std::chrono::milliseconds;
            using std::chrono::seconds;

            // Cache the Date/Time part of the next second.
            const auto duration = msg.m_time.time_since_epoch();
            const auto secs = duration_cast<seconds>(duration);

            if (m_cachedTimestamp != secs || m_cachedDateTime.size() == 0)
            {
                m_cachedDateTime.clear();
                m_cachedDateTime.push_back('[');

                // Year
                FormatHelpers::AppendInt(tmTime.tm_year + 1900, m_cachedDateTime);
                m_cachedDateTime.push_back('-');

                // Month
                FormatHelpers::Pad2(tmTime.tm_mon + 1, m_cachedDateTime);
                m_cachedDateTime.push_back('-');

                // Day
                FormatHelpers::Pad2(tmTime.tm_mday, m_cachedDateTime);
                m_cachedDateTime.push_back(' ');

                // Hour
                FormatHelpers::Pad2(tmTime.tm_hour, m_cachedDateTime);
                m_cachedDateTime.push_back(':');

                // Minute
                FormatHelpers::Pad2(tmTime.tm_min, m_cachedDateTime);
                m_cachedDateTime.push_back(':');

                // Second
                FormatHelpers::Pad2(tmTime.tm_sec, m_cachedDateTime);
                m_cachedDateTime.push_back('.');

                m_cachedTimestamp = secs;
            }
            dest.append(m_cachedDateTime.begin(), m_cachedDateTime.end());

            auto millis = FormatHelpers::TimeFraction<milliseconds>(msg.m_time);
            FormatHelpers::Pad3(millis.count(), dest);
            dest.push_back(']');
            dest.push_back(' ');

            // Append Logger name if it exists
            if (!msg.m_tagName.empty())
            {
                dest.push_back('[');
                FormatHelpers::AppendStringView(msg.m_tagName, dest);
                dest.push_back(']');
                dest.push_back(' ');
            }

            // Log Level
            // Wrap Log Level with color:
            dest.push_back('[');
            msg.m_colorRangeStart = dest.size();
            FormatHelpers::AppendStringView(GetLogLevelName(msg.m_level), dest);
            msg.m_colorRangeEnd = dest.size();
            dest.push_back(']');
            dest.push_back(' ');

            // Append Source Location if present
            if (msg.m_source.IsValid())
            {
                dest.push_back('[');
                const char* filename = FormatHelpers::GetFileBasename(msg.m_source.m_fileName);
                FormatHelpers::AppendStringView(filename, dest);
                dest.push_back('(');
                FormatHelpers::AppendInt(msg.m_source.m_line, dest);
                dest.push_back(')');
                dest.push_back(']');
                dest.push_back(' ');
            }

            // Append Message
            FormatHelpers::AppendStringView(msg.m_payload, dest);
        }

    private:
        std::chrono::seconds    m_cachedTimestamp{0};
        LogMemoryBuffer         m_cachedDateTime;
    };
}