// Logger.inl
#pragma once

namespace nes
{
    inline Logger::Logger(std::string name)
        : m_name(std::move(name))
    {
        //
    }

    inline Logger::Logger(std::string name, LogTargetPtr pTarget)
        : m_name(std::move(name))
        , m_targets{ std::move(pTarget) }
    {
        //
    }

    template <typename Iterator>
    inline Logger::Logger(std::string name, Iterator begin, Iterator end)
        : m_name(std::move(name))
        , m_targets(begin, end)
    {
        //
    }

    inline Logger::Logger(std::string name, const std::initializer_list<LogTargetPtr>& targets)
        : m_name(std::move(name))
        , m_targets(targets.begin(), targets.end())
    {
        //
    }

    inline Logger::Logger(const Logger& other)
        : m_name(other.m_name)
        , m_targets(other.m_targets)
        , m_level(other.m_level.load(std::memory_order_relaxed))
        , m_flushLevel(other.m_flushLevel.load(std::memory_order_relaxed))
    {
        //
    }

    inline Logger::Logger(Logger&& other) noexcept
        : m_name(std::move(other.m_name))
        , m_targets(std::move(other.m_targets))
        , m_level(other.m_level.load(std::memory_order_relaxed))
        , m_flushLevel(other.m_flushLevel.load(std::memory_order_relaxed))
    {
        //
    }

    inline Logger& Logger::operator=(const Logger& other)
    {
        if (this != &other)
        {
            m_name = other.m_name;
            m_targets = other.m_targets;
            m_level = other.m_level.load(std::memory_order_relaxed);
            m_flushLevel = other.m_flushLevel.load(std::memory_order_relaxed);
        }

        return *this;
    }

    inline Logger& Logger::operator=(Logger&& other) noexcept
    {
        if (this != &other)
        {
            m_name = std::move(other.m_name);
            m_targets = std::move(other.m_targets);
            m_level = other.m_level.load(std::memory_order_relaxed);
            m_flushLevel = other.m_flushLevel.load(std::memory_order_relaxed);
        }

        return *this;
    }

    template <typename ... Args>
    void Logger::Log(const internal::LogSource& source, ELogLevel level, FormatString<Args...> pFormat, Args&&... args)
    {
        Log(source, level, LogTag("", level), pFormat, std::forward<Args>(args)...);
    }

    template <typename ... Args>
    void Logger::Log(const internal::LogSource& source, ELogLevel level, const LogTag& tag, FormatString<Args...> pFormat, Args&&... args)
    {
        // If the tag's level is less than the message's exit. 
        if (tag.m_level < level)
            return;

        // Check if this level is enabled.
        const bool isEnabled = LevelIsEnabled(level);
        //const bool tracebackEnabled = m_tracer.IsEnabled();
        if (!isEnabled)
            return;

        // [TODO]: This is wrapped in a try-catch.
        LogMemoryBuffer buffer;
        fmt::vformat_to(fmt::appender(buffer), pFormat, fmt::make_format_args(args...));
        internal::LogMessage message(source, tag.m_name, level, std::string_view(buffer.data(), buffer.size()));
        LogMessage(message, isEnabled, false);        
    }

    inline void Logger::Log(const internal::LogSource& source, ELogLevel level, std::string_view msg)
    {
        Log(source, level, LogTag("", level), msg);
    }

    inline void Logger::Log(const internal::LogSource& source, ELogLevel level, const LogTag& tag, std::string_view msg)
    {
        // If the tag's level is less than the message's exit. 
        if (tag.m_level < level)
            return;

        // Check if the level is enabled.
        const bool isEnabled = LevelIsEnabled(level);
        if (!isEnabled)
            return;

        const internal::LogMessage message(source, tag.m_name, level, msg);
        LogMessage(message, isEnabled, false);
    }

    inline void Logger::SetFormatter(std::unique_ptr<LogFormatter> pFormatter)
    {
        for (auto it = m_targets.begin(); it != m_targets.end(); ++it)
        {
            // We can move the formatter into the final target
            if (std::next(it) == m_targets.end())
            {
                (*it)->SetFormatter(std::move(pFormatter));
                break;
            }
            else
            {
                (*it)->SetFormatter(pFormatter->Clone());
            }
        }
    }

    inline void Logger::LogMessage(const internal::LogMessage& message, const bool levelIsEnabled, [[maybe_unused]] const bool tracebackEnabled)
    {
        if (levelIsEnabled)
            LogToAllTargets(message);

        // [TODO]: 
        //if (tracebackEnabled)
            //m_tracer.push_back(message);
    }

    inline void Logger::PostError([[maybe_unused]] const std::string& msg)
    {
        // [TODO]: 
    }

    inline void Logger::FlushAllTargets()
    {
        for (auto& pTarget : m_targets)
        {
            pTarget->Internal_Flush();
        }
    }

    inline void Logger::LogToAllTargets(const internal::LogMessage& message)
    {
        for (auto& pTarget : m_targets)
        {
            if (pTarget->Internal_ShouldLog(message.m_level))
            {
                // This is wrapped in a try-catch, catching the source location.
                pTarget->Internal_Log(message);
            }
        }

        if (ShouldFlush(message))
            FlushAllTargets();
    }

    inline bool Logger::ShouldFlush(const internal::LogMessage& message) const
    {
        const uint8_t flushLevel = static_cast<uint8_t>(m_flushLevel.load(std::memory_order_relaxed));
        return (static_cast<uint8_t>(message.m_level) >= flushLevel && message.m_level != ELogLevel::Off);
    }
}