// WindowsInclude.h
#pragma once
#include "Core/Config.h"

#ifdef NES_PLATFORM_WINDOWS
    // Prevent Windows from redefining min and max macros
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#endif
