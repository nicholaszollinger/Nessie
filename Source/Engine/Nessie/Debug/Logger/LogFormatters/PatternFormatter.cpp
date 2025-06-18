// PatternFormatter.cpp
#include "PatternFormatter.h"

namespace nes
{
    PatternFormatter::PatternFormatter(std::string pattern, std::string eol, CustomFlags customFlags)
        : m_customFlags(std::move(customFlags))
        , m_pattern(std::move(pattern))
        , m_eol(std::move(eol))
        , m_lastLogSeconds(0)
        , m_needUpdateCachedTime(false)
    {
        std::memset(&m_cachedTmTime, 0, sizeof(m_cachedTmTime));
        CompilePattern(m_pattern);
    }

    std::unique_ptr<LogFormatter> PatternFormatter::Clone() const
    {
        // Create new flag formatters:
        CustomFlags clonedFormatters;
        for (const auto& [flag, formatter] : m_customFlags)
        {
            clonedFormatters.emplace(flag, formatter->Clone());
        }

        auto cloned = std::make_unique<PatternFormatter>(m_pattern, m_eol, std::move(clonedFormatters));
        cloned->m_needUpdateCachedTime = m_needUpdateCachedTime;
        return cloned;
    }

    void PatternFormatter::Format(const internal::LogMessage& msg, LogMemoryBuffer& dest)
    {
        // Update the cached time
        if (m_needUpdateCachedTime)
        {
            const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(msg.m_time.time_since_epoch());
            if (seconds != m_lastLogSeconds)
            {
                m_cachedTmTime = GetTime(msg);
                m_lastLogSeconds = seconds;
            }
        }

        for (auto& pFlagFormatter : m_flagFormatters)
        {
            pFlagFormatter->Format(msg, m_cachedTmTime, dest);
        }

        // Write the EOL
        FormatHelpers::AppendStringView(m_eol, dest);
    }

    std::tm PatternFormatter::GetTime(const internal::LogMessage& msg)
    {
        // [TODO]: I need to write - local time vs gm time???
        auto result = LogClock::to_time_t(msg.m_time);
        std::tm tmTime;
        localtime_s(&tmTime, &result);
        return tmTime;
    }

    void PatternFormatter::CompilePattern(const std::string& pattern)
    {
        auto end = pattern.end();

        std::unique_ptr<internal::AggregateFormatter> userChars;
        m_flagFormatters.clear();

        for (auto it = pattern.begin(); it != end; ++it)
        {
            if (*it == '%')
            {
                // Append raw characters found so far:
                if (userChars)
                {
                    m_flagFormatters.push_back(std::move(userChars));
                }

                auto padding = HandlePadSpec(++it, end);

                if (it != end)
                {
                    if (padding.m_isEnabled)
                    {
                        HandleFlag<internal::ScopedPadder>(*it, padding);
                    }
                    else
                    {
                        HandleFlag<internal::NullScopedPadder>(*it, padding);
                    }
                }
                
                /// We're done:
                else
                {
                    break;
                }
            }
            
            else
            {
                if (!userChars)
                {
                    userChars = std::make_unique<internal::AggregateFormatter>();
                }
                
                userChars->AddChar(*it);
            }
            
            // Append raw characters found so far:
            if (userChars)
            {
                m_flagFormatters.push_back(std::move(userChars));
            }
        }

    }

    internal::PaddingInfo PatternFormatter::HandlePadSpec(std::string::const_iterator& it, std::string::const_iterator end)
    {
        using internal::PaddingInfo;
        using internal::ScopedPadder;

        static constexpr size_t kMaxWidth = 64;
        if (it == end)
        {
            return PaddingInfo{};
        }

        PaddingInfo::EPaddingSide side;
        switch(*it)
        {
            case '-':
            {
                side = PaddingInfo::EPaddingSide::Right;
                ++it;
                break;
            }

            case '=':
            {
                side = PaddingInfo::EPaddingSide::Center;
                ++it;
                break;
            }

            default:
            {
                side = PaddingInfo::EPaddingSide::Left;
                break;
            }
        }

        // No padding if no digit found:
        if (it == end || !std::isdigit(static_cast<unsigned char>(*it)))
            return PaddingInfo{};

        auto width = static_cast<size_t>(*it) - '0';
        for (++it; it != end && std::isdigit(static_cast<unsigned char>(*it)); ++it)
        {
            const auto digit = static_cast<size_t>(*it) - '0';
            width = width * 10 + digit;
        }

        // Search for the optional truncate marker '!'
        bool truncate;
        if (it != end && *it == '!')
        {
            truncate = true;
            ++it;
        }
        else
        {
            truncate = false;
        }
        
        return PaddingInfo{ side, std::min(width, kMaxWidth), truncate };
    }
}
