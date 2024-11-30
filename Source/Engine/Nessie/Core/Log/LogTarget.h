#pragma once
// LogTarget.h
#include <string>

namespace nes
{
    enum class LogSeverity : int
    {
        kLog,
        kWarning,
        kError,
        kCritical
    };

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      The implementation is defined in external source files. Only one should be defined based on platform.
    //		
    ///		@brief : A Log Target represents a visual interface for Log messages. The default version
    ///             is the console window.
    //----------------------------------------------------------------------------------------------------
    class LogTarget
    {
    public:
        bool Init();
        void Close();
        void PrePost(const LogSeverity type);
        void Post(const std::string& msg);
    };
}
