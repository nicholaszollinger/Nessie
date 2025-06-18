// Platform.h
#pragma once
#include "Debug/Logger/LogTarget.h"

namespace nes::platform
{
#ifdef NES_PLATFORM_WINDOWS
    /// 'End of Line' character.
    static constexpr const char* kEOL = "\r\n";

    /// Array of valid Folder separators for a platform.
    static constexpr char kFolderSeparators[] = "\\/";
#else
    /// 'End of Line' character.
    static constexpr const char* kEOL = "\n";
    
    /// Array of valid Folder separators for a platform.
    static constexpr char kFolderSeparators[] = "/"; 
#endif
}

namespace nes
{
    class Platform
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create the default log target for a given platform. This is used as the default logger
        ///     when initializing the LogRegistry.
        //----------------------------------------------------------------------------------------------------
        static LogTargetPtr CreateDefaultLogTarget();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Platform-specific fatal error handling. This function may exit the program. 
        ///	@param reason : Reason for the fatal error, e.g. "Assertion Failed!".
        ///	@param message : Optional message to go with the Error.
        //----------------------------------------------------------------------------------------------------
        static void         HandleFatalError(const std::string& reason, const std::string& message = {});
    };
}
