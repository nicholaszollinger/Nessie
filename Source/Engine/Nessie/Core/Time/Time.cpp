// Time.cpp

#include "Time.h"
#include <chrono>
#include <format>

#include "Debug/Assert.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the current time as the number of ticks since epoch.
    //----------------------------------------------------------------------------------------------------
    uint64_t Time::Now()
    {
        auto time = std::chrono::high_resolution_clock::now();
        return time.time_since_epoch().count();
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Get the Current Time as a readable string. 
    ///		@param format : Type of format you want the string in.  
    //----------------------------------------------------------------------------------------------------
    std::string Time::ToString(const Format format)
    {
        const auto time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        
        switch (format)
        {
            case Format::LocalTime:
                return std::format("{:%r}", time);
            
            case Format::Date:
                return std::format("{:%d-%b-%y}",time);
            
            case Format::Filename:
                return std::format("{:%d-%b-%y %H.%M.%OS}", time);
        }

        // This shouldn't happen.
        NES_ASSERT(false);
        // I am returning a default format.
        return std::format("{:%X}", time);
    }
}
