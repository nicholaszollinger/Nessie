// Logger.h
#pragma once
#include <vector>
#include "LogTarget.h"
#include "Details/LogTag.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A logger is essentially a group of destinations to post log messages. It shouldn't be used
    ///     directly - you should use NES_LOG().
    //----------------------------------------------------------------------------------------------------
    class Logger
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Info used to create a logger. 
        //----------------------------------------------------------------------------------------------------
        struct CreateInfo
        {
            std::string m_name;
            ELogLevel   m_level;
        };

        /// Default Log Pattern for created loggers:
        /// Ex: "[01:29:07 PM] Main.cpp(5) [Info]: Hello World!", or 
        ///     "[01:29:07 PM] Main.cpp(5) [Info] AI: Hello World!" if a Logger is given.
        static constexpr const char* kDefaultLogPattern = "[%r] %s(%#) %^%N%$: %v";
        
    public:
        explicit Logger(std::string name);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Ctor for creating a Logger with a single Target.
        //----------------------------------------------------------------------------------------------------
        Logger(std::string name, LogTargetPtr pTarget);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Logger that uses iterators to initialize the std::vector of LogTargetPtrs. 
        //----------------------------------------------------------------------------------------------------
        template <typename Iterator>
        Logger(std::string name, Iterator begin, Iterator end);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Ctor for creating a Logger with multiple Targets. 
        //----------------------------------------------------------------------------------------------------
        Logger(std::string name, const std::initializer_list<LogTargetPtr>& targets);

        Logger(const Logger& other);
        Logger(Logger&& other) noexcept;
        virtual ~Logger() = default;

        /// Assignment Operators    
        Logger&         operator=(const Logger& other);
        Logger&         operator=(Logger&& other) noexcept;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Log a message to all registered Targets. 
        //----------------------------------------------------------------------------------------------------
        template <typename...Args>
        void                        Log(const internal::LogSource& source, ELogLevel level, const LogTag& tag, FormatString<Args...> pFormat, Args&&...args);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Log overload with no LogTag.
        //----------------------------------------------------------------------------------------------------
        template <typename...Args>
        void                        Log(const internal::LogSource& source, ELogLevel level, FormatString<Args...> pFormat, Args&&...args);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Log overload for format string types that can't be converted to a format string.
        //----------------------------------------------------------------------------------------------------
        template <IsNotConvertibleToAnyFormatString Type> 
        void                        Log(const internal::LogSource& source, ELogLevel level, const LogTag& tag, const Type& message)   { Log(source, level, tag, "{}", message); }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Log overload for format string types that can't be converted to a format string.
        //----------------------------------------------------------------------------------------------------
        template <IsNotConvertibleToAnyFormatString Type> 
        void                        Log(const internal::LogSource& source, ELogLevel level, const Type& message) { Log(source, level, "{}", message); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Non-templated overload for a non-formatted log message.
        //----------------------------------------------------------------------------------------------------
        void                        Log(const internal::LogSource& source, ELogLevel level, const LogTag& tag, std::string_view msg);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Non-templated overload for a non-formatted log message.
        //----------------------------------------------------------------------------------------------------
        void                        Log(const internal::LogSource& source, ELogLevel level, std::string_view msg);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Non-templated overload for a simple log message.
        //----------------------------------------------------------------------------------------------------
        void                        Log(const ELogLevel level, const std::string_view message)                  { Log(internal::LogSource(), level, message); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the level for this Logger. Incoming logs with a lower level will be ignored.
        //----------------------------------------------------------------------------------------------------
        void                        SetLevel(const ELogLevel level)                     { m_level.store(level, std::memory_order_relaxed); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the Formatter object for this Logger. This determines how logs will be written to
        ///     its registered Log Targets.
        //----------------------------------------------------------------------------------------------------
        void                        SetFormatter(std::unique_ptr<LogFormatter> pFormatter);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if logging is enabled for the given level.
        //----------------------------------------------------------------------------------------------------
        bool                        LevelIsEnabled(const ELogLevel level) const         { return static_cast<uint8_t>(level) >= static_cast<uint8_t>(m_level.load(std::memory_order_relaxed)); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Flush all Log Targets.
        //----------------------------------------------------------------------------------------------------
        void                        Flush()                                             {  FlushAllTargets(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the flush level for this Logger and all LogTargets.
        //----------------------------------------------------------------------------------------------------
        void                        SetFlushLevel(ELogLevel level);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current flush level for this logger.
        //----------------------------------------------------------------------------------------------------
        ELogLevel                   GetFlushLevel() const                               { return m_flushLevel.load(std::memory_order_relaxed); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the name of the Logger.
        //----------------------------------------------------------------------------------------------------
        const std::string&          GetName() const                                     { return m_name; }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the array of LogTargets that this Logger will output to. 
        //----------------------------------------------------------------------------------------------------
        std::vector<LogTargetPtr>&  GetTargets()                                        { return m_targets; } 

    protected:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Log a given message to each registered LogTarget and save the trace if enabled.
        //----------------------------------------------------------------------------------------------------
        void                        LogMessage(const internal::LogMessage& message, const bool levelIsEnabled, const bool tracebackEnabled);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Post an error to the error handler. 
        //----------------------------------------------------------------------------------------------------
        void                        PostError(const std::string& msg);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Flush all registered Log Targets. 
        //----------------------------------------------------------------------------------------------------
        virtual void                FlushAllTargets();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Log a given message to each registered Log Target.
        //----------------------------------------------------------------------------------------------------
        virtual void                LogToAllTargets(const internal::LogMessage& message);

        // [TODO]: void DumpBacktrace();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Return whether this message should be flushed based on the Logger's flush level.
        //----------------------------------------------------------------------------------------------------
        bool                        ShouldFlush(const internal::LogMessage& message) const;
        
    protected:
        std::string                 m_name;                               /// Name of the Logger
        std::vector<LogTargetPtr>   m_targets;                            /// Targets that this Logger will post messages to.
        std::atomic<ELogLevel>      m_level{ELogLevel::Info};       /// Base Log Level for this Logger. Messages that are lower priority than this will not be logged. 
        std::atomic<ELogLevel>      m_flushLevel{ELogLevel::Off};   /// Base Log Level to flush 
    };
}

#include "Logger.inl"