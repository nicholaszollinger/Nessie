// LoggerRegistry.inl
#pragma once

namespace nes
{
    template <typename LoggerType>
    std::shared_ptr<LoggerType> LoggerRegistry::CreateLogger(const Logger::CreateInfo& info, const bool shouldInitialize)
    {
        std::lock_guard<std::mutex> lock(m_loggersMutex);
        // [TODO]: Ensure that the logger doesn't exist already. Error if it does.

        std::shared_ptr<Logger> pLogger = std::make_shared<LoggerType>(info.m_name);
        pLogger->SetLevel(info.m_level);
        pLogger->SetFormatter(m_pDefaultFormatter->Clone());
        
        m_loggers[info.m_name] = pLogger;

        if (shouldInitialize)
            InitializeLogger(pLogger);
        
        return pLogger;
    }
}

