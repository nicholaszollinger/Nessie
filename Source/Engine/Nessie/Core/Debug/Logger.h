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
        static bool Init(const std::string& logOutputDir);
        static void Close();
        static bool LoadCategories(const std::string& filepath);

        static void QuickLog(const LogSeverity type, AddableOrToStringType auto&&...args);
        static void Log(const StringID categoryName, AddableOrToStringType auto&&...args);
        static void VitalLog(const StringID categoryName, const LogSeverity type, AddableOrToStringType auto&&...args);

    private:
        static void PostToLogTarget(const LogSeverity type, const std::string& msg);
        static void WriteToFile(const std::string& msg);
        static std::string FormatQuickLog(const LogSeverity type, AddableOrToStringType auto&&...args);
        static std::string FormatLog(const LogSeverity type, const char* pCategory, AddableOrToStringType auto&&...args);

        // Static member accessors, to prevent the static initialization order fiasco.
        static CategoriesContainer& GetCategories();
        static std::mutex& GetMutex();
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
    void Logger::QuickLog(const LogSeverity type, AddableOrToStringType auto&&... args)
    {
        const std::string msg = FormatQuickLog(type, args...);
        PostToLogTarget(type, msg);

        if (type == LogSeverity::kError || type == LogSeverity::kWarning)
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
            categories.emplace(categoryName, LogCategory(categoryName, LogOutputLevel::kDisplay));

        const auto category = categories[categoryName];

    #if NES_LOGGER_IS_MULTITHREADED
        GetMutex().unlock();
    #endif

        const auto output = category.GetOutputLevel();

        // If the output level is none, then we are skipping this log.
        if (output == LogOutputLevel::kNone)
            return;

        // Format the log:
        const auto msg = FormatLog(LogSeverity::kLog, categoryName.GetCStr(), args...);

        // Post to the target if we are displaying it.
        if (output == LogOutputLevel::kDisplay || output == LogOutputLevel::kAll)
            PostToLogTarget(LogSeverity::kLog, msg);

        // Write it to the file if we the output level allows it.
        if (output == LogOutputLevel::kFile || output == LogOutputLevel::kAll)
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
    void Logger::VitalLog(const StringID categoryName, const LogSeverity type, AddableOrToStringType auto&&... args)
    {
        const std::string msg = FormatLog(type, categoryName.GetCStr(), args...);
        PostToLogTarget(type, msg);
        WriteToFile(msg);
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Get the formatted quick log - a timestamp followed by the log message.
    //-----------------------------------------------------------------------------------------------------------------------------
    std::string Logger::FormatQuickLog(const LogSeverity type, AddableOrToStringType auto&&... args)
    {
        switch (type)
        {
            case LogSeverity::kLog:
            {
                // Format: [CurrentTime] - [CategoryName] Message
                return CombineIntoString("[", GetCurrentTimeAsString(), "] - ", args..., "\n");
            }

            case LogSeverity::kWarning:
            {
                // Format: [CurrentTime] - [WARNING:CategoryName] Message
                return CombineIntoString("[", GetCurrentTimeAsString(), "] - [WARNING]", args..., "\n");
            }

            case LogSeverity::kError:
            {
                // Format: [CurrentTime] - [ERROR:CategoryName] Message
                return CombineIntoString("[", GetCurrentTimeAsString(), "] - [ERROR]", args..., "\n");
            }

            case LogSeverity::kCritical:
            {
                // Format: [CurrentTime] - [CRITICAL:CategoryName] Message
                return CombineIntoString("[", GetCurrentTimeAsString(), "] - [CRITICAL]", args..., "\n");
            }
        }

        return "INVALID LOG TYPE!";
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Get the quick log - a timestamp followed by the category and log message.
    //-----------------------------------------------------------------------------------------------------------------------------
    std::string Logger::FormatLog(const LogSeverity type, const char* pCategory, AddableOrToStringType auto&&... args)
    {
        switch (type)
        {
            case LogSeverity::kLog:
            {
                // Format: [CurrentTime] - [CategoryName] Message
                return CombineIntoString("[", GetCurrentTimeAsString(), "] - [", pCategory, "] ", args..., "\n");
            }

            case LogSeverity::kWarning:
            {
                // Format: [CurrentTime] - [WARNING:CategoryName] Message
                return CombineIntoString("[", GetCurrentTimeAsString(), "] - [WARNING:", pCategory, "] ", args..., "\n");
            }

            case LogSeverity::kError:
            {
                // Format: [CurrentTime] - [ERROR:CategoryName] Message
                return CombineIntoString("[", GetCurrentTimeAsString(), "] - [ERROR:", pCategory, "] ", args..., "\n");
            }

            case LogSeverity::kCritical:
            {
                // Format: [CurrentTime] - [CRITICAL:CategoryName] Message
                return CombineIntoString("[", GetCurrentTimeAsString(), "] - [CRITICAL:", pCategory, "] ", args..., "\n");
            }
        }

        return "INVALID LOG TYPE!";
    }
}
