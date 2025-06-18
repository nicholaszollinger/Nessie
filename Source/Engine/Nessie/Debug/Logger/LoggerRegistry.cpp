// LoggerRegistry.cpp
#include "LoggerRegistry.h"
#include "LogFormatters/PatternFormatter.h"
#include "Platform/Platform.h"

namespace nes
{
    LoggerRegistry::LoggerRegistry()
        : m_pDefaultFormatter(new PatternFormatter(Logger::kDefaultLogPattern))
    {
        // Create the default Logger
        const char* kDefaultLoggerName = "";
        m_pDefaultLogger = std::make_shared<Logger>(kDefaultLoggerName);
        m_loggers[kDefaultLoggerName] = m_pDefaultLogger;
        
        auto pDefaultTarget = Platform::CreateDefaultLogTarget();
        pDefaultTarget->SetLevel(m_globalLogLevel);
        
        // Initialize logger would add these targets 
        m_pDefaultLogger->GetTargets().push_back(pDefaultTarget);
        m_pDefaultLogger->SetFormatter(m_pDefaultFormatter->Clone());
    }

    void LoggerRegistry::InitializeLogger(std::shared_ptr<Logger> pLogger) const
    {
        auto& defaultTargets = m_pDefaultLogger->GetTargets();
        
        for (auto& pTarget : defaultTargets)
        {
            pLogger->GetTargets().push_back(pTarget);
        }
    }

    LoggerRegistry& LoggerRegistry::Instance()
    {
        static LoggerRegistry instance;
        return instance;
    }

    void LoggerRegistry::Internal_Init()
    {
        // [TODO]:
        // Set Global level?
        // Create the file target?
    }

    void LoggerRegistry::Internal_Shutdown()
    {
        // [TODO]: 
    }
}
