// Logger.cpp

#include "Logger.h"
#include <filesystem>
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace nes
{
    static LogTarget& GetLogTarget()
    {
        static LogTarget s_target;
        return s_target;
    }

    static std::ofstream& GetOutFile()
    {
        static std::ofstream s_outFile;
        return s_outFile;
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Initialize the Logger. NOTE: If you set the Logger to be multithreaded, know that this is not thread safe; 
    ///              this is intended to be called before anything happens in the program, on the main thread.
    ///		@param logOutputDir : Directory that the log files will be posted to.
    //-----------------------------------------------------------------------------------------------------------------------------
    bool Logger::Init(const std::string& logOutputDir)
    {
        // Make sure that we have the Log directory made.
        const std::filesystem::path logDirPath(logOutputDir);
        std::filesystem::create_directory(logDirPath);

        // Create the name for the Log file.
        const auto newFilename = CombineIntoString(logOutputDir, Time::ToString(Time::Format::Filename), ".txt");
        auto& outFile = GetOutFile();
        outFile.open(newFilename);

        if (!outFile.is_open())
            return false;

        outFile << Time::ToString(Time::Format::Date) << " " << Time::ToString(Time::Format::LocalTime) << "\n\n";

        auto& categories = GetCategories();

        // Add the assert category to our categories.
        categories.emplace("Assertion Failed!", LogCategory("Assertion Failed!", ELogOutputLevel::All));

        // Initialize our LogTarget.
        return GetLogTarget().Init();
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Load Log categories from a yaml file.
    ///		@returns : False if the file was unable to load.
    //----------------------------------------------------------------------------------------------------
    bool Logger::LoadCategories(const std::string& filepath)
    {
        const auto categoryData = YAML::LoadFile(filepath);

        // If the file path is invalid, return.
        if (!categoryData.IsDefined())
            return false;

        auto& categories = GetCategories();

        for (auto it = categoryData.begin(); it != categoryData.end(); ++it)
        {
            const StringID name = it->first.as<std::string>();
            const auto outputLevel = static_cast<ELogOutputLevel>(it->second.as<int>());

            categories.emplace(name, LogCategory(name, outputLevel));
        }

        return true;
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Close the logger. NOTE: If you set the logger to be multithreaded, know that this function is not thread safe-
    ///              This is intended to be called at the very end of the program's lifetime, on the main thread.
    //-----------------------------------------------------------------------------------------------------------------------------
    void Logger::Close()
    {
        auto& categories = GetCategories();
        categories.clear();

        GetLogTarget().Close();
        GetOutFile().close();

    #ifdef NES_DEBUG
        // Link to the issue and resolution: https://github.com/microsoft/STL/issues/2504
	    // to prevent the tzdb allocations from being reported as memory leaks
	    std::chrono::get_tzdb_list().~tzdb_list();
    #endif
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : This is the stage we actually present the log onto the LogTarget. This is thread-safe if LOGGER_IS_MULTITHREADED
    ///             is set to true.
    //-----------------------------------------------------------------------------------------------------------------------------
    void Logger::PostToLogTarget(const ELogSeverity type, const std::string& msg)
    {
    #if NES_LOGGER_IS_MULTITHREADED
        std::lock_guard lock(GetMutex());
    #endif

        auto& logTarget = GetLogTarget();
        logTarget.PrePost(type);
        logTarget.Post(msg); 
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Write the message to the output file defined in Logger::Init(). This is thread-safe if LOGGER_IS_MULTITHREADED
    ///             is set to true.
    //-----------------------------------------------------------------------------------------------------------------------------
    void Logger::WriteToFile(const std::string& msg)
    {
    #if NES_LOGGER_IS_MULTITHREADED
        std::lock_guard lock(GetMutex());
    #endif

        GetOutFile() << msg;
    }

    Logger::CategoriesContainer& Logger::GetCategories()
    {
        static CategoriesContainer categories;
        return categories;
    }

    std::mutex& Logger::GetMutex()
    {
        static std::mutex s_mutex;
        return s_mutex;
    }

}
