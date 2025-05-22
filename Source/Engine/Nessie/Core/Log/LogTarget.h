// LogTarget.h
#pragma once
#include <string>

namespace nes
{
    enum class ELogSeverity : uint8
    {
        Log,
        Warning,
        Error,
        Critical
    };

    //----------------------------------------------------------------------------------------------------
    //  NOTES:
    //  The implementation is defined in external source files. Only one should be defined based on the platform.
    //		
    ///	@brief : A Log Target represents a visual interface for Log messages. The default version
    ///     is the console window.
    //----------------------------------------------------------------------------------------------------
    class LogTarget
    {
    public:
        bool Init();
        void Close();
        void PrePost(const ELogSeverity type);
        void Post(const std::string& msg);
    };
}
