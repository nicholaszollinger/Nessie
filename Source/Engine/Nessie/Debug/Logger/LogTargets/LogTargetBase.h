// LogTargetBase.h
#pragma once
#include "Nessie/Core/Thread/StdMutex.h"
#include "Nessie/Debug/Logger/LogTarget.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for all LogTargets, that handles the thread-safe functionality.
    ///	@tparam Mutex : Type of mutex to use. Pass in the NullMutex if you want to use a single-threaded log target. 
    //----------------------------------------------------------------------------------------------------
    template <MutexType Mutex>
    class LogTargetBase : public LogTarget
    {
    public:
        LogTargetBase();
        virtual ~LogTargetBase() override = default;
        LogTargetBase(const LogTargetBase&) = delete;
        LogTargetBase(LogTargetBase&&) noexcept = delete;
        LogTargetBase& operator=(const LogTargetBase&) = delete;
        LogTargetBase& operator=(LogTargetBase&&) noexcept = delete;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the pattern string for Log Messages for this Log Target.
        /// @note : To see pattern string flags and examples, see "PatternFormatter.h"
        //----------------------------------------------------------------------------------------------------
        virtual void SetPattern(const std::string& pattern) override final;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the formatter, the object that decides how incoming messages will be formatted.
        //----------------------------------------------------------------------------------------------------
        virtual void SetFormatter(std::unique_ptr<LogFormatter> formatter) override final;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Log a message to the Target under lock protection. (If the mutex type is NullMutex, then
        ///     no lock is acquired).
        //----------------------------------------------------------------------------------------------------
        virtual void Internal_Log(const internal::LogMessage& message) override final;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Flush any outstanding messages to the Target under lock protection.
        ///     (If the mutex type is NullMutex, then no lock is acquired).
        //----------------------------------------------------------------------------------------------------
        virtual void Internal_Flush() override final;
        
    protected:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Log function for derived classes to override.
        //----------------------------------------------------------------------------------------------------
        virtual void LogImpl(const internal::LogMessage& message) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Flush function for derived classes to override. 
        //----------------------------------------------------------------------------------------------------
        virtual void FlushImpl() = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the pattern for Log Messages for this Log Target. 
        //----------------------------------------------------------------------------------------------------
        virtual void SetPatternImpl(const std::string& pattern);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the log formatter for this LogTarget. 
        //----------------------------------------------------------------------------------------------------
        virtual void SetFormatterImpl(std::unique_ptr<LogFormatter> formatter);
        
    protected:
        std::unique_ptr<LogFormatter>   m_pFormatter;  /// The formatter for this Log Target.
        Mutex                           m_mutex;       /// Mutex for thread safety.
    };
}

#include "LogTargetBase.inl"