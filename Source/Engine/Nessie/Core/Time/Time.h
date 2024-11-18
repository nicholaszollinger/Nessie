#pragma once

#include <string>

namespace nes
{
    uint64_t GetCurrentTime();
    std::string GetDateAsString();
    std::string GetTimeStampFilename();
    std::string GetCurrentTimeAsString();
    std::string GetCurrentLocalTimeString();

}