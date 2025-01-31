#pragma once

#include <string>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Static class API for getting the current time and format strings.                                                                                       
    //----------------------------------------------------------------------------------------------------
    class Time
    {
    public:
        enum class Format : uint8_t
        {
            LocalTime,        // "HH:MM:SS {TimeOfDay}" Example: "04:15:00 PM"
            Date,             // "DD-MMM-YY" Example: "27-Dec-23"
            Filename,         // Compatible Filename: "DD-MMM-YY HH.MM.SS". Example: "29-Jan-25 14.27.25"
        };

        static uint64_t Now();
        static std::string ToString(const Format format); 
    };
}