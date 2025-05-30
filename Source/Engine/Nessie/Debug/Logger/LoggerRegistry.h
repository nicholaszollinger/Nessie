// LoggerRegistry.h
#pragma once
#include <unordered_map>
#include "Logger.h"
#include "Core/Thread/Mutex.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : The Logger Registry handles creating and managing Loggers used by the Application.
    ///     This class must be initialized at the start of the application, and shutdown at the end.
    //----------------------------------------------------------------------------------------------------
    class LoggerRegistry
    {
        LoggerRegistry();
        ~LoggerRegistry() = default;
        
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the default Logger. This logger is used for all NES_LOG(), NES_WARN(), etc calls when
        ///     no specific Logger is specified.
        //----------------------------------------------------------------------------------------------------
        std::shared_ptr<Logger>         GetDefaultLogger() const                                    { return m_pDefaultLogger; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the default Logger. This logger is used for all NES_LOG(), NES_WARN(), etc calls when
        ///     no specific Logger is specified.
        //----------------------------------------------------------------------------------------------------
        void                            SetDefaultLogger(const std::shared_ptr<Logger>& pLogger)    { m_pDefaultLogger = pLogger; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a new Logger of a given type. This logger will be registered, but no LogTargets
        ///     will be set. If you want to add the default LogTargets, you will need to call InitializeLogger()
        ///     passing in the newly created logger.
        /// @param info : Info used to create the logger.
        /// @param shouldInitialize : If true, the created Logger will have the default log targets set. 
        //----------------------------------------------------------------------------------------------------
        template <typename LoggerType>
        std::shared_ptr<LoggerType>     CreateLogger(const Logger::CreateInfo& info, bool shouldInitialize = true);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initializes the Logger with the default LogTargets from the Registry.
        //----------------------------------------------------------------------------------------------------
        void                            InitializeLogger(std::shared_ptr<Logger> pLogger) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the Logger Registry instance. This creates the instance on the first call.
        //----------------------------------------------------------------------------------------------------
        static LoggerRegistry&          Instance();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the Registry. 
        //----------------------------------------------------------------------------------------------------
        void                            Internal_Init();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Shutdown the Registry. At this point, no logging will be valid. This should be done at
        ///     the end of the main loop.
        //----------------------------------------------------------------------------------------------------
        void                            Internal_Shutdown();
        
    private:
        using LoggerMap = std::unordered_map<std::string, std::shared_ptr<Logger>>;
        
        LoggerMap                       m_loggers;
        std::mutex                      m_loggersMutex;
        std::unique_ptr<LogFormatter>   m_pDefaultFormatter;
        std::shared_ptr<Logger>         m_pDefaultLogger;
        ELogLevel                       m_globalLogLevel = ELogLevel::Info;
    };
}

#include "LoggerRegistry.inl"