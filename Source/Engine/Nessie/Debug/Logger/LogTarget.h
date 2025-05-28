// LogTarget.h
#pragma once
#include "LogFormatters/LogFormatter.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for all LogTargets. A LogTarget represents a destination for log messages (e.g., 
    ///     the console, a file, etc.). To make your own LogTarget, don't inherit from this, class, but rather
    ///     LogTargetBase, which will provide the thread-safe functionality based on a mutex type.
    //----------------------------------------------------------------------------------------------------
    class LogTarget
    {
    public:
        virtual ~LogTarget() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the pattern for Log Messages for this Log Target.
        //----------------------------------------------------------------------------------------------------
        virtual void    SetPattern(const std::string& pattern) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the log level for this Log Target. This will override the Logger's level. 
        //----------------------------------------------------------------------------------------------------
        void            SetLevel(ELogLevel level)                               { m_level.store(level, std::memory_order_relaxed); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current log level for this Target. 
        //----------------------------------------------------------------------------------------------------
        ELogLevel       GetLevel() const                                        { return m_level.load(std::memory_order_relaxed); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the formatter for this Log Target.
        //----------------------------------------------------------------------------------------------------
        virtual void    SetFormatter(std::unique_ptr<LogFormatter> formatter) = 0;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Log a message to the Log Target.
        /// @note : Do not call directly, this is called by the Logger class!
        //----------------------------------------------------------------------------------------------------
        virtual void    Internal_Log(const internal::LogMessage& message) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Flush log messages.
        /// @note : Do not call directly, this is called by the Logger class!
        //----------------------------------------------------------------------------------------------------
        virtual void    Internal_Flush() = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns if a log message with the given level should be logged on this Log Target.
        //----------------------------------------------------------------------------------------------------
        bool            Internal_ShouldLog(const ELogLevel level) const         { return level >= m_level.load(std::memory_order_relaxed); }
        
    protected:
        std::atomic<ELogLevel> m_level{ ELogLevel::Trace };  /// The current log level for this Target.  
    };

    using LogTargetPtr = std::shared_ptr<LogTarget>;
}