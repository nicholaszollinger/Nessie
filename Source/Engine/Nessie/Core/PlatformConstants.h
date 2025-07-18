// PlatformConstants.h
// [TODO]: Change to OS, move the Windows files to Core/OS/Windows
#pragma once
#include "Config.h"
#include <string>

namespace nes::platformConstants
{
#ifdef NES_PLATFORM_WINDOWS
#define NES_FILE_SEPARATOR '\\'
    
    /// 'End of Line' character.
    static constexpr const char* kEOL = "\r\n";

    /// Array of valid Folder separators for a platform.
    static constexpr char kFolderSeparators[] = "\\/";

#else
#define NES_FILE_SEPARATOR '/'
    
    /// 'End of Line' character.
    static constexpr const char* kEOL = "\n";

    /// Array of valid Folder separators for a platform.
    static constexpr char kFolderSeparators[] = "/"; 
#endif
}