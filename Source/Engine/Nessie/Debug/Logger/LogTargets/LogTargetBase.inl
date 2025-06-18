// LogTargetBase.inl
#pragma once
#include "Debug/Logger/LogFormatters/PatternFormatter.h"

namespace nes
{
    template <MutexType Mutex>
    LogTargetBase<Mutex>::LogTargetBase()
        : m_pFormatter(std::make_unique<PatternFormatter>())
    {
        //
    }

    template <MutexType Mutex>
    void LogTargetBase<Mutex>::Internal_Log(const internal::LogMessage& message)
    {
        std::lock_guard<Mutex> lock(m_mutex);
        LogImpl(message);
    }

    template <MutexType Mutex>
    void LogTargetBase<Mutex>::Internal_Flush()
    {
        std::lock_guard<Mutex> lock(m_mutex);
        FlushImpl();
    }

    template <MutexType Mutex>
    void LogTargetBase<Mutex>::SetPattern(const std::string& pattern)
    {
        std::lock_guard<Mutex> lock(m_mutex);
        SetPatternImpl(pattern);   
    }

    template <MutexType Mutex>
    void LogTargetBase<Mutex>::SetFormatter(std::unique_ptr<LogFormatter> formatter)
    {
        std::lock_guard<Mutex> lock(m_mutex);
        SetFormatterImpl(std::move(formatter));
    }

    template <MutexType Mutex>
    void LogTargetBase<Mutex>::SetPatternImpl(const std::string& pattern)
    {
        SetFormatterImpl(std::make_unique<PatternFormatter>(pattern));
    }

    template <MutexType Mutex>
    void LogTargetBase<Mutex>::SetFormatterImpl(std::unique_ptr<LogFormatter> formatter)
    {
        m_pFormatter = std::move(formatter);
    }
}
