// Time.cpp

#include "Time.h"
#include <chrono>
#include <format>

#include "Debug/Assert.h"

namespace nes
{
    uint64_t Time::Now()
    {
        auto time = std::chrono::high_resolution_clock::now();
        return time.time_since_epoch().count();
    }
    
    std::string Time::ToString(const EFormat format)
    {
        const auto time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
        
        switch (format)
        {
            case EFormat::LocalTime:
                return std::format("{:%r}", time);
            
            case EFormat::Date:
                return std::format("{:%d-%b-%y}",time);
            
            case EFormat::Filename:
                return std::format("{:%d-%b-%y %H.%M.%OS}", time);
        }

        // This shouldn't happen.
        NES_ASSERT(false);
        // I am returning a default format.
        return std::format("{:%X}", time);
    }
}
