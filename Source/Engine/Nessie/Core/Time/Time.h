// Time.h
#pragma once
#include <string>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///	@brief : Static class API for getting the current time and format strings.                                                                                       
    //----------------------------------------------------------------------------------------------------
    class Time
    {
    public:
        enum class EFormat : uint8_t
        {
            LocalTime,        /// "HH:MM:SS {TimeOfDay}" Example: "04:15:00 PM"
            Date,             /// "DD-MMM-YY" Example: "27-Dec-23"
            Filename,         /// Compatible Filename: "DD-MMM-YY HH.MM.SS". Example: "29-Jan-25 14.27.25"
        };

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns the current time as the number of ticks since epoch.
        //----------------------------------------------------------------------------------------------------
        static uint64_t     Now();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Get the current time as a readable string. 
        ///	@param format : Type of format you want the string in.  
        //----------------------------------------------------------------------------------------------------
        static std::string  ToString(const EFormat format); 
    };
}