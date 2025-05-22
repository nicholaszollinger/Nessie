#pragma once
// Logger.h
#include <mutex>
#include "LogCategory.h"
#include "LogTarget.h"
#include "Core/Time/Time.h"

#define NES_LOGGER_IS_MULTITHREADED 1
#define NES_USE_DEFAULT_LOG_TARGET 1

namespace nes
{
    class Logger
    {
        using CategoriesContainer = std::unordered_map<StringID, LogCategory, StringIDHasher>;

    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the Logger. 
        ///	@param logOutputDir : Directory where log files will be created.
        //----------------------------------------------------------------------------------------------------
        static bool                 Init(const std::string& logOutputDir);
        static void                 Close();
        static bool                 LoadCategories(const std::string& filepath);

        static void                 QuickLog(const ELogSeverity type, AddableOrToStringType auto&&...args);
        static void                 Log(const StringID categoryName, AddableOrToStringType auto&&...args);
        static void                 VitalLog(const StringID categoryName, const ELogSeverity type, AddableOrToStringType auto&&...args);

    private:
        static void                 PostToLogTarget(const ELogSeverity type, const std::string& msg);
        static void                 WriteToFile(const std::string& msg);
        static std::string          FormatQuickLog(const ELogSeverity type, AddableOrToStringType auto&&...args);
        static std::string          FormatLog(const ELogSeverity type, const char* pCategory, AddableOrToStringType auto&&...args);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Access the Categories Container. This is created on the first call to this function.
        //----------------------------------------------------------------------------------------------------
        static CategoriesContainer& GetCategories();
        static std::mutex&          GetMutex();
    };
}

namespace nes
{
    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : A quick log is a log that doesn't have a LogCategory. It will always be displayed. Warnings and Errors will be
    ///             written to the file.
    ///		@param type : Type of log.
    ///		@param args : What we are logging.
    //-----------------------------------------------------------------------------------------------------------------------------
    void Logger::QuickLog(const ELogSeverity type, AddableOrToStringType auto&&... args)
    {
        const std::string msg = FormatQuickLog(type, args...);
        PostToLogTarget(type, msg);

        if (type == ELogSeverity::Error || type == ELogSeverity::Warning)
        {
            WriteToFile(msg);
        }
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Log a message to the log target. If the LogCategory isn't on, then this will NOT post the message.
    ///              HOWEVER, if this encounters a LogCategory that doesn't exist, then it will create the category and post
    ///              the log.
    ///		@param categoryName : Name of the category that this log is associated with.
    ///		@param args : Arguments to pass to the log.
    //-----------------------------------------------------------------------------------------------------------------------------
    void Logger::Log(const StringID categoryName, AddableOrToStringType auto&&... args)
    {
    #if NES_LOGGER_IS_MULTITHREADED
        GetMutex().lock();
    #endif

        auto& categories = GetCategories();

        // If we don't have the category, we aew going to make one that is display only.
        if (!categories.contains(categoryName))
            categories.emplace(categoryName, LogCategory(categoryName, ELogOutputLevel::LogTarget));

        const auto category = categories[categoryName];

    #if NES_LOGGER_IS_MULTITHREADED
        GetMutex().unlock();
    #endif

        const auto output = category.GetOutputLevel();

        // If the output level is none, then we are skipping this log.
        if (output == ELogOutputLevel::None)
            return;

        // Format the log:
        const auto msg = FormatLog(ELogSeverity::Log, categoryName.CStr(), args...);

        // Post to the target if we are displaying it.
        if (output == ELogOutputLevel::LogTarget || output == ELogOutputLevel::All)
            PostToLogTarget(ELogSeverity::Log, msg);

        // Write it to the file if we the output level allows it.
        if (output == ELogOutputLevel::File || output == ELogOutputLevel::All)
            WriteToFile(msg);
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : 'Vital' logs are any warnings, errors, or critical messages.
    ///		@param categoryName : Name of the category that this log is associated with.
    ///		@param type : Type of vital log we are posting.
    ///		@param args : Arguments to pass into the log.
    //-----------------------------------------------------------------------------------------------------------------------------
    void Logger::VitalLog(const StringID categoryName, const ELogSeverity type, AddableOrToStringType auto&&... args)
    {
        const std::string msg = FormatLog(type, categoryName.CStr(), args...);
        PostToLogTarget(type, msg);
        WriteToFile(msg);
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Get the formatted quick log - a timestamp followed by the log message.
    //-----------------------------------------------------------------------------------------------------------------------------
    std::string Logger::FormatQuickLog(const ELogSeverity type, AddableOrToStringType auto&&... args)
    {
        switch (type)
        {
            case ELogSeverity::Log:
            {
                // Format: [CurrentTime] - [CategoryName] Message
                return CombineIntoString("[", Time::ToString(Time::Format::LocalTime), "] - ", args..., "\n");
            }

            case ELogSeverity::Warning:
            {
                // Format: [CurrentTime] - [WARNING:CategoryName] Message
                return CombineIntoString("[", Time::ToString(Time::Format::LocalTime), "] - [WARNING] ", args..., "\n");
            }

            case ELogSeverity::Error:
            {
                // Format: [CurrentTime] - [ERROR:CategoryName] Message
                return CombineIntoString("[", Time::ToString(Time::Format::LocalTime), "] - [ERROR] ", args..., "\n");
            }

            case ELogSeverity::Critical:
            {
                // Format: [CurrentTime] - [CRITICAL:CategoryName] Message
                return CombineIntoString("[", Time::ToString(Time::Format::LocalTime), "] - [CRITICAL] ", args..., "\n");
            }
        }

        return "INVALID LOG TYPE!";
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Get the quick log - a timestamp followed by the category and log message.
    //-----------------------------------------------------------------------------------------------------------------------------
    std::string Logger::FormatLog(const ELogSeverity type, const char* pCategory, AddableOrToStringType auto&&... args)
    {
        switch (type)
        {
            case ELogSeverity::Log:
            {
                // Format: [CurrentTime] - [CategoryName] Message
                return CombineIntoString("[", Time::ToString(Time::Format::LocalTime), "] - [", pCategory, "] ", args..., "\n");
            }

            case ELogSeverity::Warning:
            {
                // Format: [CurrentTime] - [WARNING:CategoryName] Message
                return CombineIntoString("[", Time::ToString(Time::Format::LocalTime), "] - [WARNING:", pCategory, "] ", args..., "\n");
            }

            case ELogSeverity::Error:
            {
                // Format: [CurrentTime] - [ERROR:CategoryName] Message
                return CombineIntoString("[", Time::ToString(Time::Format::LocalTime), "] - [ERROR:", pCategory, "] ", args..., "\n");
            }

            case ELogSeverity::Critical:
            {
                // Format: [CurrentTime] - [CRITICAL:CategoryName] Message
                return CombineIntoString("[", Time::ToString(Time::Format::LocalTime), "] - [CRITICAL:", pCategory, "] ", args..., "\n");
            }
        }

        return "INVALID LOG TYPE!";
    }
}
