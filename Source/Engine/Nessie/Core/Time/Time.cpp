// Time.cpp

#include "Time.h"
#include <chrono>

namespace nes
{
    uint64_t GetCurrentTime()
    {
        auto time = std::chrono::high_resolution_clock::now();
        return time.time_since_epoch().count();
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //
    ///		@brief : Returns the current time as a string with the format: "HH:MM:SS {TimeOfDay}". Example: "04:15:00 PM"
    //-----------------------------------------------------------------------------------------------------------------------------
    std::string GetCurrentLocalTimeString()
    {
        const auto time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        return std::format("{:%r}", time);
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //
    ///		@brief : Returns the current date as a string with the format: "DD-MMM-YY". Example: "27-Dec-23"
    //-----------------------------------------------------------------------------------------------------------------------------
    std::string GetDateAsString()
    {
        const auto time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        return std::format("{:%d-%b-%y}",time);
    }

    std::string GetTimeStampFilename()
    {
        const auto time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        return std::format("{:%d-%b-%y %H.%M.%OS}", time);
    }

    std::string GetCurrentTimeAsString()
    {
        const auto time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        return std::format("{:%X}", time);
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Get a string of the current time using a format string.
    //-----------------------------------------------------------------------------------------------------------------------------
    std::string FormatCurrentTime()
    {
        //const auto time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        return GetCurrentTimeAsString();
    }
}
